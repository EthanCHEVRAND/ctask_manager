#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tasks.h"

static void usage(const char *prog) {
    printf("Usage: %s <command> [args]\n", prog) ;
    printf("Commands:\n") ;
    printf("    add <description> [-p --priority <1-3> | -t --tags [<tag1> <tag2>...<tag10>]]   Add a task\n") ;
    printf("    list [-a --all]        List tasks (default: only pending)\n") ;
    printf("    done <id>           Mark task as done\n") ;
    printf("    rm <id>             Remove task\n") ;
}

// TODO add --tag option to list command
// TODO add simplified option (eg: --priority, -p)
int main(int argc, char **argv) {
    if (argc < 2) { usage(argv[0]) ; return 1 ; }

    TaskList tl ;
    tasklist_init(&tl) ;
    load_tasks(&tl) ;

    const char *cmd = argv[1] ;
    if (strcmp(cmd, "add") == 0) {
        if (argc < 3) {
            fprintf(stderr, "missing description\n");
            return 1;
        }

        char desc_buf[1024] = "";
        int priority = 2;
        const char *tags[10];
        size_t tag_count = 0;

        for (int i = 2; i < argc; i++) {
            if ((strcmp(argv[i], "--priority") == 0 || strcmp(argv[i], "-p") == 0)&& i + 1 < argc) {
                priority = atoi(argv[++i]);
                if (priority < 1) priority = 1;
                if (priority > 3) priority = 3;
            } else if (strcmp(argv[i], "--tags") == 0 || strcmp(argv[i], "-t") == 0) {
                i++;
                while (i < argc && argv[i][0] != '-') {
                    if (tag_count < 10) {
                        tags[tag_count++] = argv[i];
                    }
                    i++;
                }
                i--;
            } else {
                if (desc_buf[0] != '\0') strcat(desc_buf, " ");
                strcat(desc_buf, argv[i]);
            }
        }

        int id = add_task(&tl, desc_buf, priority, tags, tag_count);
        if (id < 0) {
            fprintf(stderr, "failed to add task\n");
            tasklist_free(&tl);
            return 1;
        }

        save_tasks(&tl);
        printf("added task %d\n", id);
    } else if (strcmp(cmd, "list") == 0) {
        bool show_all = false ;
        if (argc >= 3 && (strcmp(argv[2], "--all") == 0 || strcmp(argv[2], "-a") == 0)) show_all = true ;

        for (size_t i=0 ; i<tl.len ; i++) {
            Task *t = &tl.items[i] ;
            if (!show_all && t->done) continue ;

            printf("%d [%c] (prio:%d) %s\n  -> %s\n", t->id, t->done ? 'x' : ' ',t->priority,  t->created_at ? t->created_at : " ", t->desc ? t->desc : " ") ;

            if (t->tag_count > 0) {
                printf(" (tags: ") ;
                for (size_t j = 0 ; j < t->tag_count ; j++) {
                    printf("%s%s", t->tags[j], j+1 < t->tag_count ? ", ": "") ;
                }
                printf(")") ;
            }
            printf("\n") ;
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