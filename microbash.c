#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

void currentDir(char* dir, long size){
    char* path;
    char *saveptr = NULL;

    if((path = (char *)malloc((size_t)size)) == NULL)
        perror("malloc error: ");
    //path contiene il percorso assoluto attuale
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




//richiede siri
void modificaOutput(char* comando){
    char *saveptr = NULL; 
    char* buff = strtok_r(comando,">",&saveptr);
    printf("%s\n",buff);
    //codice che funge ma non penso vada bene
    int fw=open(buff,O_CREAT|O_WRONLY|O_APPEND);
    if(fw<0)
        perror("errore apertura file: ");
    write(fw,"Ciao\n",5);
    close(fw);
    
    /*
    stackoverflow che non capisco
    int fd;
    fpos_t pos;
    printf("stdout, ");
    fflush(stdout);
    fgetpos(stdout,&pos);
    fd = dup(fileno(stdout));
    printf("stdout in f()");
    fflush(stdout);
    dup2(fd,fileno(stdout));
    close(fd);
    clearerr(stdout);
    printf("stdout again\n");*/
    /*

    codice abortoso incompleto ora che da problemi
    int fw=open(buff,O_CREAT|O_WRONLY|O_APPEND);
    if(fw<0)
        perror("errore apertura file: ");
    close(1);
    int tmp = dup(fw);
    
    //dup2 
    if(tmp<0)
        perror("errore copiatura file descriptor: ");
    close(fw);
    dup(1);
    */
}

void clean(char ** v){
    int i=0;
    while(v[i]!=NULL)
        free(v[i++]);
    free(v);
}

int my_exec(char* cmd, char** argv, char **argve, int fdIn, int fdOut){
    printf("%p ", argv);
    printf("%p \n", argve);
    pid_t pid = fork();
    if(pid<0){
        perror("Error fork: ");
        return EXIT_FAILURE;
    }
    if(pid == 0){
        //redirezione
        //exec
        ;//figlio
    }
    clean(argv);
    clean(argve);
    return EXIT_SUCCESS;
}


void parserArg(char* cmd, char*** argv, char*** argve, int* fdIn, int* fdOut){
    if(cmd[strlen(cmd)-1]=='\n')
        cmd[strlen(cmd)-1]='\0';
    int i=0;
    int k=0;
    int argCount=2;
    int envCount=1;
    char* arg;
    char* saveptr;

    int check = 0;

    for(i=0; cmd[i]!='\0';i++) {
        if (cmd[i] == ' ') {
            argCount++;
            check = 1;
        }
        if (cmd[i] == '$'){
            if(check){
                argCount--;
                envCount++;
                check = 1;
            }
        }
    }
    *argv = malloc(sizeof(char*)*(argCount));
    *argve = malloc(sizeof(char*)*(envCount));


    *fdOut = STDOUT_FILENO;
    *fdIn = STDIN_FILENO;

    check = 0;

    for (arg = strtok_r(cmd, " ", &saveptr), i=0; arg != NULL; arg = strtok_r(NULL, " ", &saveptr)) {
        if(!strncmp(arg,"$",1)){
            char * name = arg+1;
            char * value = getenv(name);
            if(value==NULL){
                value=malloc(sizeof(char));
                strcpy(value,"");
            }

            if(!check){
                if(!strcmp(value,""))
                    continue;
                (*argv)[i]=malloc(strlen(value));
                strcpy((*argv)[i],value);
                i++;
                check = 1;
            }else{
                (*argve)[k]=malloc(strlen(name)+strlen(value));
                strcpy((*argve)[k],name);
                strcat((*argve)[k],"=");
                strcat((*argve)[k],value);
                k++;
            }

        }else if(arg[0]=='>') {
            *fdOut = open(arg + 1, O_WRONLY|O_CREAT);
            perror("errore >");
        }else if(arg[0]=='<'){
            *fdIn = open(arg + 1, O_RDONLY);
            perror("errore <");
        }else{
            (*argv)[i]=malloc(strlen(arg));
            strcpy((*argv)[i],arg);
            i++;
            check = 1;
        }
    }
    (*argv)[i]=NULL;
    (*argve)[k]=NULL;
}

/*
void parser(char* cmd){
    cmd[strlen(cmd)-1]=0; //tolgo \n alla fine della riga

    char* buf;
    if((buf = calloc(strlen(cmd)+1, sizeof(char))) == NULL)
        perror("calloc error: ");

    strcpy(buf, cmd);

    char *saveptr = NULL;
    char* saveptrarg = NULL;

    char* arg;

    int controllo = 0;

    for (buf = strtok_r(buf, "|", &saveptr); buf != NULL; buf = strtok_r(NULL, "|", &saveptr)) {
        int i=0;
        int k=0;
        int argCount=2;
        int envCount=1;
        for(i=0; buf[i]!='\0';i++) {
            if (buf[i] == ' ')
                argCount++;
            if (buf[i] == '$'){
                argCount--;
                envCount++;
            }
        }
        char ** argv = malloc(sizeof(char*)*(argCount));
        char ** argve = malloc(sizeof(char*)*(envCount));


        int fdOut = STDOUT_FILENO;
        int fdIn = STDIN_FILENO;
        for (arg = strtok_r(buf, " ", &saveptrarg), i=0; arg != NULL; arg = strtok_r(NULL, " ", &saveptrarg)) {
            if(!strncmp(arg,"$",1)){
                char * name = arg+1;
                char * value = getenv(name);
                argve[k]=malloc(strlen(name)+strlen(value));
                strcpy(argve[k],name);
                strcat(argve[k],"=");
                strcat(argve[k],value);
                k++;
            }else if(arg[0]=='>') {
                fdOut = open(arg + 1, O_WRONLY, O_CREAT);
            }else if(arg[0]=='<'){
                fdIn = open(arg + 1, O_RDONLY);
            }else{
                argv[i]=malloc(strlen(arg));
                strcpy(argv[i],arg);
                i++;
            }
        }
        argv[i]=NULL;
        argve[k]=NULL;



        int c1;
        for(c1=0;c1<argCount;c1++)
            printf("argv[%d]:%s\n",c1,argv[c1]);
        for(c1=0;c1<envCount;c1++)
            printf("argve[%d]:%s\n",c1,argve[c1]);
        printf("IN: %d OUT: %d\n",fdIn,fdOut);
        if(!strcmp(argv[0],"cd")) {
            if(controllo != 0){
                printf("error");
                return;
            }
            cd(argv[1]);
            controllo = 1;
        }else {
            if(controllo == 1){
                printf("error");
                return;
            }
            my_exec(argv[0], argv, argve, fdIn, fdOut);
            controllo = 2;
        }

    }
}
*/
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

        char** argv=NULL;
        char** argve=NULL;
        int fdIn;
        int fdOut;

        parserArg(cmd,&argv,&argve,&fdIn,&fdOut);
        printf("%s\n",cmd);
        printf("%d %d\n",fdIn,fdOut);
        int c=0;
        while(argv[c]!=0) {
            printf("argv[%d] = %s\n", c, argv[c]);
            c++;
        }
        c = 0;
        while(argve[c]!=0) {
            printf("argve[%d] = %s\n", c, argve[c]);
            c++;
        }
        //wait
    }
    free(dir);
}
