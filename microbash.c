#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

char* currentDir(const char* path){
    char* buf;
    if((buf = calloc(strlen(path)+1, sizeof(char))) == NULL)
        perror("calloc error: ");

    strcpy(buf, path);

    char *dir = NULL;
    char *saveptr = NULL;

    for (buf = strtok_r(buf, "/", &saveptr); buf != NULL; buf = strtok_r(NULL, "/", &saveptr))
        dir = buf;

    return dir;
}

void cd(char* path, char* np){
    char* buf;
    if((buf = calloc(strlen(path)+1, sizeof(char))) == NULL)
        perror("calloc error: ");
    strcpy(buf, path);

    if(*np == '/')
        strcpy(path, np);
    else if(!strncmp(np,"..",2)){
        ;
    }else{
        strcat(path,"/");
        strcat(path,np);
    }

    if(chdir(np)) {
        printf("[%s]",np);
        strcpy(path, buf);
    }
    free(buf);
}

void parser(char* path, char* cmd){
    cmd[strlen(cmd)-1]=0; //tolgo \n alla fine della riga

    char* buf;
    if((buf = calloc(strlen(cmd)+1, sizeof(char))) == NULL)
        perror("calloc error: ");

    strcpy(buf, cmd);

    char *dir = NULL;
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
            printf("Arg:%s\n", arg);
            argv[i]=malloc(strlen(arg));
            strcpy(argv[i],arg);
        }

        if(!strcmp(argv[0],"cd"))
            cd(path, argv[1]);
        else
            ; //roba varia noiosa

    }
}

int main() {
    char* path;
    long size = pathconf(".", _PC_PATH_MAX);
    if((path = (char *)malloc((size_t)size)) == NULL)
        perror("malloc error: ");
    if (getcwd(path, (size_t) size) == 0)
        perror("getcwd error: ");

    char cmd[500];

    for(;;) {
        printf("%s $ ", currentDir(path));
        fgets(cmd, 500, stdin);
        parser(path, cmd);
    }
}
