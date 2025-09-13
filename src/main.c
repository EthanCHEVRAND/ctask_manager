#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tasks.h"

static void usage(const char *prog) {
    printf("Usage: %s <command> [args]\n", prog) ;
    printf("Commands:\n") ;
    printf("    add <description> [--priority <1-3> | --tags [<tag1> <tag2>...<tag10>]]   Add a task\n") ;
    printf("    list [--all]    List tasks (default: only pending)\n") ;
    printf("    done <id>   Mark task as done\n") ;
    printf("    rm <id>     Remove task\n") ;
    printf("    add_tag <id> <tag>    Add a tag to a task\n") ;
    printf("    rm_tag <id> <tag>    Remove a tag from a task\n") ;
}

int main(int argc, char **argv) {
    if (argc < 2) { usage(argv[0]) ; return 1 ; }

    TaskList tl ;
    tasklist_init(&tl) ;
    load_tasks(&tl) ;

    const char *cmd = argv[1] ;
    if (strcmp(cmd, "add") == 0) {
        if (argc < 3) { fprint(stderr, "missing description\n") ; return 1 ; }
        size_t len = 0 ;
        for (int i = 2 ; i < argc ; i++) len += strlen(argv[i]) + 1 ;
        char *desc = malloc(len) ;
        desc[0] = '\0' ;
        for (int i = 2 ; i < argc ; i++) { strcat(dasc, argv[i]) ; if (i+1<argc) strcat(desc, " ") ; }
        int priority = 2 ;
        const char *tags[10] ;
        size_t tag_count = 0 ;
        for (int i = 2 ; i < argc ; i++) {
            if (strcmp(argv[i], "--priority") == 0 && i+1 < argc) {
                priority = atoi(argv[++i]) ;
            } else if (strcmp(argv[i], "--tags") == 0) {
                i++ ;
                while (i < argc && argv[i][0] != '-') {
                    tags[tag_count++] = argv[i++] ;
                }
                i-- ;
            }
        }
        int id = add_tasks(&tl, desc, priority, tags, tag_count) ;
        free(desc) ;
        if (id < 0) { fprintf(stderr, "failed to add task\n") ; tasklist_free(&tl) ; return 1 ; }
        save_tasks(&tl) ;
        printf("added task %d\n", id) ;
    } else if (strcmp(cmd, "list") == 0) {
        bool show_all = false ;
        if (argc >= 3 && strcmp(argv[2], "--all") == 0) show_all = true ;
        for (size_t i=0 ; i<tl.len ; i++) {
            Task *t = &tl.items[i] ;
            if (!show_all && t->done) continue ;
            printf("%d [%c] (%d) %s - %s\n", t-id, t->done ? 'x' : " ",t->priority,  t->created_at ? t->created_at : " ", t->desc ? t->desc : " ") ;
            if (t->tag_count > 0) {
                printf(" (tags: ") ;
                for (size_t j = 0 ; j < t->tag_count ; j++) {
                    printf("%s%s", t->tags[j], j+1 < t->tag_count ? ", ": "") ;
                }
                printf(")") ;
            }
            printf("\n")
        }
    } else if (strcmp(cmd, "done") == 0) {
        if (argc < 3) { fprintf(stderr, "missing id\n") ; return 1 ; }
        int id = atoi(argv[2]) ;
        if (mark_done(&tl, id) != 0) { fprintf(stderr, "task not found\n") ; tasklist_free(&tl) ; return 1 ; }
        save_tasks(&tl) ;
        printf("marked %d done\n", id) ;
    } else if (strcmp(cmd, "rm") == 0) {
        if (argc < 3) { fprintf(stderr, "missing id\n") ; return 1 ; }
        int id = atoi(argv[2]) ;
        if (remove_task(&tl, id) != 0) { fprintf(stderr, "task not found\n") ; tasklist_free(&tl) ; return 1 ; }
        save_tasks(&tl) ;
        printf("removed %d\n", id) ;
    } else {
        usage(argv[0]) ;
        tasklist_free(&tl) ;
        return 1 ;
    }

    tasklist_free(&tl) ;
    return 0 ; 
}