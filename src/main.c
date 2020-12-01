#include "microbash.h"
#include <stdlib.h>
#include <stdio.h>

int main() {
    char* dir;
    long size = pathconf(".", _PC_PATH_MAX);
    if((dir = (char *)malloc((size_t)size)) == NULL) {
        perror("malloc error: ");
        exit(EXIT_FAILURE);
    }
    char cmd[1024];
    for(;;) {
        currentDir(dir, size);
        printf("%s $ ", dir);
        if(fgets(cmd, 1024, stdin) == 0) //EOF
            break;
        if(!validate(cmd)){
            printf("Syntax error.\n");
            continue;
        }
        if(!strcmp(cmd,"exit"))
            break;
        cmdHandler(cmd);

    }
    free(dir);
}


