#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void currentDir(char* dir, long size){
    char* path;
    char *saveptr = NULL;

    if((path = (char *)malloc((size_t)size)) == NULL)
        perror("malloc error: ");

    if (getcwd(path, (size_t) size) == 0)
        perror("getcwd error: ");

    strcpy(dir,"/");

    for (path = strtok_r(path, "/", &saveptr); path != NULL; path = strtok_r(NULL, "/", &saveptr))
        strcpy(dir,path);

    free(path);
}

int cd(char* np){
    int res;
    if((res = chdir(np))) {
        printf("No such file or directory: %s\n", np);
    }
    return res;
}

void parser(char* cmd){
    cmd[strlen(cmd)-1]=0; //tolgo \n alla fine della riga

    char* buf;
    if((buf = calloc(strlen(cmd)+1, sizeof(char))) == NULL)
        perror("calloc error: ");

    strcpy(buf, cmd);

    char *saveptr = NULL;
    char* saveptrarg = NULL;

    char* arg;

    for (buf = strtok_r(buf, "|", &saveptr); buf != NULL; buf = strtok_r(NULL, "|", &saveptr)) {
        int i=0;
        int countspace=0;
        for(i=0; buf[i]!='\0';i++)
            if (buf[i] == ' ')
                countspace++;

        char ** argv = malloc(sizeof(char*)*(countspace+1));
        i=0;
        for (arg = strtok_r(buf, " ", &saveptrarg); arg != NULL; arg = strtok_r(NULL, " ", &saveptrarg),i++) {
            argv[i]=malloc(strlen(arg));
            strcpy(argv[i],arg);
        }

        if(!strcmp(argv[0],"cd"))
            cd( argv[1]);
        else
            ; //roba varia noiosa
    }
}

int main() {
    char* dir;
    long size = pathconf(".", _PC_PATH_MAX);
    if((dir = (char *)malloc((size_t)size)) == NULL)
        perror("malloc error: ");

    char cmd[500];

    for(;;) {
        currentDir(dir, size);
        printf("%s $ ", dir);
        fgets(cmd, 500, stdin);
        parser(cmd);
    }
    free(dir);
}
