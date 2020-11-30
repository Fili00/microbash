#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

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

int cd(char** argv){
    if(argv[1] == NULL || argv[2] != NULL) {
        printf("cd has only one argument\n");
        return 0;
    }
    int res;
    if((res = chdir(argv[1])) != 0)
        printf("No such file or directory: %s\n", argv[1]);
    return res;
}

void clean(char ** v){
    int i=0;
    while(v[i]!=NULL)
        free(v[i++]);
    free(v);
    v=NULL;
}

void my_exec(char** argv, char **envp, int fdIn, int fdOut){
    pid_t pid = fork();
    if(pid<0){
        perror("Fork failed: ");
        printf("Insert new command\n");
        return;
    }
    if(pid == 0){
        if(fdOut!=STDOUT_FILENO) {
            dup2(fdOut, STDOUT_FILENO);
            close(fdOut);
        }
        if(fdIn!=STDIN_FILENO) {
            dup2(fdIn, STDIN_FILENO);
            close(fdIn);
        }

        execvp(argv[0],argv); //variabili d'ambiente
        perror("Error executing the command");
        clean(argv);
        clean(envp);
        exit(EXIT_FAILURE);
    }
    if(fdOut!=STDOUT_FILENO)
        close(fdOut);
    if(fdIn!=STDIN_FILENO)
        close(fdIn);
    clean(argv);
    clean(envp);
}

void parserArg(char* cmd, char*** argv, char*** envp, int* fdIn, int* fdOut){
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
    *envp = malloc(sizeof(char*)*(envCount));


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
                (*envp)[k]=malloc(strlen(name)+strlen(value));
                strcpy((*envp)[k],name);
                strcat((*envp)[k],"=");
                strcat((*envp)[k],value);
                k++;
            }

        }else if(arg[0]=='>') {
            if((*fdOut = open(arg + 1, O_WRONLY|O_CREAT, 0644)) < 0)
                perror("errore >");
        }else if(arg[0]=='<'){
            if((*fdIn = open(arg + 1, O_RDONLY)) < 0)
                perror("errore <");
        }else{
            (*argv)[i]=malloc(strlen(arg));
            strcpy((*argv)[i],arg);
            i++;
            check = 1;
        }
    }
    (*argv)[i]=NULL;
    (*envp)[k]=NULL;
}

void cmdHandler (char *cmd){
    char* saveptr;
    int n_proc=0;
    int i;
    int status;
    char** argv;
    char** envp;
    char* arg;
    int fdIn, fdOut;
    int pipefd[2];
    int check=0;

    arg = strtok_r(cmd, "|", &saveptr);
    while(arg != NULL) {
        parserArg(arg, &argv, &envp, &fdIn, &fdOut);
        if (check) {
            fdIn = pipefd[0];
        }

        if((arg = strtok_r(NULL, "|", &saveptr)) != NULL){
            pipe(pipefd);
            fdOut = pipefd[1];
            check=1;
        }


        if(!strcmp(argv[0], "cd")){
            cd(argv);
            return;
        }

        my_exec(argv,envp,fdIn,fdOut);
        n_proc++;
    }
    for(i=0; i<n_proc; i++) {
        wait(&status);
        //mettere controllo su status
    }
}

void removeMultipleSpace(char* cmd){
    int i;
    int correctIndex = 0;
    for(i=0; isspace(cmd[i]) && cmd[i] != '\0'; i++)
        ;
    for(;cmd[i] != '\0'; i++){
        while(isspace(cmd[i]) && isspace(cmd[i+1])) {
            i++;
        }
        cmd[correctIndex] = cmd[i];
        correctIndex++;
    }
    if(isspace(cmd[correctIndex-1]))
        correctIndex--;
    cmd[correctIndex] = '\0';
}


int validate(char* cmd){
    int i;
    int checkPipe = 0;
    int checkRedirect = 0;
    if(cmd[strlen(cmd)-1]=='\n')
        cmd[strlen(cmd)-1]='\0';
    removeMultipleSpace(cmd);
    int checkCD = !strncmp(cmd, "cd", 2);
    if(cmd[0] == '|')
        return 0;
    if(cmd[strlen(cmd)-1] == '|')
        return 0;
    if(cmd[0] == '<' || cmd[0] == '>')
        return 0;
    if(cmd[strlen(cmd)-1] == '<' || cmd[strlen(cmd)-1] == '>')
        return 0;
    for(i=0;cmd[i] != '\0'; i++){
        if(cmd[i] == '|')
            checkPipe = 1;
        if(cmd[i] == '|' && checkRedirect)
            return 0;
        if(checkPipe && cmd[i] == '<')
            return 0;
        if(cmd[i] == '>')
            checkRedirect = 1;
        if(cmd[i] == '|' && (cmd[i+2] == '<' || cmd[i+2] == '>'))
            return 0;
        if((cmd[i] == '|' || cmd[i] == '<' || cmd[i] == '>') && checkCD)
            return 0;
        if((cmd[i] == '>' || cmd[i] == '<') && isspace(cmd[i+1]))
            return 0;
        if(cmd[i]=='|' && ((isspace(cmd[i+1]) && cmd[i+2] == '|' ) || cmd[i+1]=='|') )
            return 0;

        if(cmd[i] == '|'){
            if(isspace(cmd[i+1]))
                i++;
            if (!strncmp(&cmd[i + 1], "cd", 2))
                return 0;
            i--;
        }
    }

    return 1;
}

int main() {
    char* dir;
    long size = pathconf(".", _PC_PATH_MAX);
    if((dir = (char *)malloc((size_t)size)) == NULL)
        perror("malloc error: ");

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
