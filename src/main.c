#include "microbash.h"
#include <stdlib.h>
#include <stdio.h>
#define _GNU_SOURCE
#include <readline/readline.h>
#include <readline/history.h>
int main() {
    char* dir;
    long size = pathconf(".", _PC_PATH_MAX);
    if((dir = (char *)malloc((size_t)size+4)) == NULL) {
        perror("malloc error: ");
        exit(EXIT_FAILURE);
    }
    char* cmd;
    for(;;) {
        currentDir(dir, size);
        strcat(dir, " $ ");
        if((cmd = readline(dir)) == NULL)
            break;
        if(strcmp(cmd, ""))
            add_history(cmd);
        if(!validate(cmd)){
            printf("Syntax error.\n");
            continue;
        }
        if(!strcmp(cmd,"exit"))
            break;
        cmdHandler(cmd);
        free(cmd);
    }
    clear_history();
    free(cmd);
    free(dir);
}


