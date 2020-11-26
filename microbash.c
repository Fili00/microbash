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

    //inutile
    //strcpy(dir,"/");

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


//bug con $PATH, errore con la malloc. Sembra che il percorso sia troppo lungo, ma non sono certo
void variabileDiSistema(char* comando){
    char *saveptr = NULL;
    char* buff = getenv(strtok_r(comando,"$",&saveptr));
    buff!=NULL? strncpy(comando,buff,26) : strcpy(comando,"(null)");
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
            if(!strncmp(argv[0],"$",1)){
                variabileDiSistema(argv[0]); //roba varia noiosa
                printf("Stampa argv: %s\n",argv[0]); 
            }else{
                if(!strncmp(argv[0],">",1)){
                    //printf("STO ENTRANDO\n");
                    modificaOutput(argv[0]);
                    strcpy(argv[0],"\0");
                }
            }
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
