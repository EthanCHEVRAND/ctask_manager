#ifndef TASKS_H
#define TASKS_H

#include <stdbool.h>
#include <stddef.h>

#define DB_PATH "data/tasks.json"

typedef struct {
    int id;
    bool done;
    char *created_at; // ISO string
    char *desc;
    int priority; // 1=important ; 2=default ; 3=low
    char **tags;
    size_t tag_count;
} Task;

typedef struct {
    Task *items;
    size_t len;
    size_t cap;
} TaskList;

void tasklist_init(TaskList *tl);
void tasklist_free(TaskList *tl);

int load_tasks(TaskList *tl);
int save_tasks(const TaskList *tl);

int add_task(TaskList *tl, const char *desc, int priority, const char **tags, size_t tag_count);
Task *find_task(const TaskList *tl, int id);
int mark_done(TaskList *tl, int id);
int remove_task(TaskList *tl, int id);

#endif // TASKS_H