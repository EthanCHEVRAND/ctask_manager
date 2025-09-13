#include "tasks.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static char *strdup_nullsafe(const char *s) {
    if (!s) return NULL ;
    size_t n = strlen(s) ;
    char *r = malloc(n+1) ;
    if (!r) return NULL ;
    memcpy(r, s, n+1) ;
    return r ;
}

void tasklist_init(TaskList *tl) {
    tl->items = NULL ;
    tl->len = 0 ;
    tl->cap = 0 ;
}

void free_task(Task *t) {
    if (!t) return ;

    for (size_t i = 0 ; i < t->tag_count ; i++) {
        free(t->tags[i]) ;
    }
    free(t->tags) ;

    free(t->created_at) ;
    free(t->desc) ;
}

void tasklist_free(TaskList *tl) {
    if (!tl) return ;
    for (size_t i=0 ; i<tl->len ; i++) free_task(&tl->items[i]) ;
    free(tl->items) ;
    tl->items = NULL ;
    tl->len = tl->cap = 0 ;
}

static int ensure_cap(TaskList *tl, size_t need) {
    if (tl->cap >= need) return 0 ;
    size_t ncap = tl->cap ? tl->cap*2 : 8 ;
    while (ncap < need) ncap *= 2 ;
    Task *tmp = realloc(tl->items, ncap * sizeof(Task)) ;
    if (!tmp) return -1 ;
    tl->items = tmp ;
    tl->cap = ncap ;
    return 0 ;
}

int next_id(const TaskList *tl) {
    int max = 0 ;
    for (size_t i=0 ; i<tl->len ; i++) if (tl->items[i].id > max) max = tl->items[i].id ;
    return max+1 ;
}

int add_task(TaskList *tl, const char *desc, int priority, const char **tags, size_t tag_count) {
    if (!desc) return -1 ;
    if (ensure_cap(tl, tl->len+1) != 0) return -1 ;
    Task *t = &tl->items[tl->len] ;
    t->id = next_id(tl) ;
    t->done = false ;
    t->priority = priority ;

    time_t now = time(NULL) ;
    struct tm *tm = localtime(&now) ;
    char buf[64] ;
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", tm) ;
    t->created_at = strdup_nullsafe(buf) ;
    t->desc = strdup_nullsafe(desc) ;

    if (tag_count > 0) {
        t->tags = malloc(sizeof(char*) * tag_count) ;
        t->tag_count = 0 ;
        for (size_t i = 0 ; i < tag_count ; i++) {
            t->tags[t->tag_count++] = strdup_nullsafe(tags[i]) ;
        }
    } else {
        t->tags = NULL ;
        t->tag_count = 0 ;
    }

    if (!t->created_at || !t->desc) {
        free_task(t) ;
        return -1 ;
    }
    tl->len++ ;
    return t->id ;
}

Task *find_task(const TaskList *tl, int id) {
    for (size_t i=0 ; i<tl->len ; i++) if (tl->items[i].id == id) return (Task*)&tl->items[i] ;
    return NULL ;
}

int mark_done(TaskList *tl, int id) {
    Task *t = find_task(tl, id) ;
    if (!t) return -1 ;
    t->done = true ;
    return 0 ;
}

int remove_task(TaskList *tl, int id) {
    size_t idx = (size_t)-1 ;
    for (size_t i=0 ; i<tl->len ; i++) if (tl->items[i].id == id) { idx = i ; break ; }
    if (idx == (size_t)-1) return -1 ;
    free_task(&tl->items[idx]) ;
    for (size_t i=idx ; i+1<tl->len ; i++) tl->items[i] = tl->items[i+1] ;
    tl->len-- ;
    return 0 ;
}

int load_tasks(TaskList *tl) {
    FILE *f = fopen(DB_PATH, "r") ;
    if (!f) return 0 ;
    fseek(f, 0, SEEK_END) ;
    long len = ftell(f) ;
    rewind(f) ;
    char *data = malloc(len+1) ;
    fread(data, 1, len, f) ;
    data[len] = '\0' ;
    fclose(f) ;

    cJSON *root = cJSON_Parse(data) ;
    free(data) ;
    if (!root || !cJSON_IsArray(root)) { cJSON_Delete(root) ; return -1 ; }

    int n = cJSON_GetArraySize(root) ;
    for (int i = 0 ; i<n ; i++) {
        cJSON *obj = cJSON_GetArrayItem(root, i) ;
        if (!cJSON_IsObject(obj)) continue ;
        int id = cJSON_GetObjectItem(obj, "id")->valueint ;
        bool done = cJSON_IsTrue(cJSON_GetObjectItem(obj, "done")) ;
        const char *created = cJSON_GetObjectItem(obj, "created_at")->valuestring ;
        const char *desc = cJSON_GetObjectItem(obj, "desc")->valuestring ;

        if (ensure_cap(tl, tl->len+1) != 0) continue ;
        Task *t = &tl->items[tl->len] ;
        t->id = id ;
        t->done = done ;
        t->created_at = strdup_nullsafe(created) ;
        t->desc = strdup_nullsafe(desc) ;
        if (!t->created_at || !t->desc) { free_task(t) ; continue ; }
        tl->len++ ;

        cJSON *json_tags = cJSON_GetObjectItem(obj, "tags") ;
        if (json_tags && cJSON_IsArray(json_tags)) {
            int n = cJSON_GetArraySize(json_tags) ;
            t->tags = malloc(sizeof(char*) * n) ;
            t->tag_count = 0 ;
            for (int j = 0 ; j < n ; j++) {
                cJSON *tag = cJSON_GetArrayItem(json_tags, j) ;
                if (cJSON_IsString(tag)) {
                    t->tags[t->tag_count++] = strdup_nullsafe(tag->valuestring) ;
                }
            }
        } else {
            t->tags = NULL ;
            t->tag_count = 0 ;
        }
    }
    cJSON_Delete(root) ;
    return 0 ;
}

int save_tasks(const TaskList *tl) {
    cJSON *root = cJSON_CreateArray() ;
    for (size_t i=0 ; i<tl->len ; i++) {
        const Task *t = &tl->items[i] ;
        cJSON *obj = cJSON_CreateObject() ;
        cJSON_AddNumberToObject(obj, "id", t->id) ;
        cJSON_AddBoolToObject(obj, "done", t->done) ;
        cJSON_AddStringToObject(obj, "created_at", t->created_at ? t->created_at : "") ;
        cJSON_AddStringToObject(obj, "desc", t->desc ? t->desc : "") ;
        cJSON_AddNumberToObject(obj, "priority", t->priority) ;
        cJSON_AddItemToArray(root, obj) ;
        cJSON *json_tags = cJSON_CreateArray() ;
        for (size_t j = 0 ; j < t->tag_count ; j++) {
            cJSON_AddItemToArray(json_tags, cJSON_CreateString(t->tags[j])) ;
        }
        cJSON_AddItemToObject(obj, "tags", json_tags) ;
    }
    char *out = cJSON_Print(root) ;
    cJSON_Delete(root) ;

    FILE *f = fopen(DB_PATH, "w") ;
    if (!f) { free(out) ; return -1 ; }
    fputs(out, f) ;
    fclose(f) ;
    free(out) ;
    return 0 ;
}