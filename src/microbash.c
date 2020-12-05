#include "microbash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>

//Funzioni ausiliarie
//Funzione che si occupa di fare la free di un vettore di puntatori.
void clean(char ** v){
    int i=0;
    while(v[i]!=NULL)
        free(v[i++]);
    free(v);
    v=NULL;
}
//Funzione che si occupa di cambiare file descriptor.
void swapfd(int fd1, int fd2){
    if(dup2(fd1, fd2) < 0){
        perror("dup");
        exit(EXIT_FAILURE);
    }
    close(fd1);
}
//Funzione che si occupa di rimuovere tutti gli spazi iniziali, finali e di sostituire gli spazi multipli con uno singolo.
//Serve per facilitare il parsing della stringa contenente il comando
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
/* Funzione che fa il parser di un comando.
 * Inserisce in argv tutti gli argomenti del comando mettendo nella posizione 0 il comando.
 * Sostituisce tutte le variabili d'ambiente inserendo il loro valore dentro argv
 * Rileva eventuali redirezioni di input/output e le inserisce dentro fdIn/fdOut
 * Se non rileva redirezioni inserisce dentro fdIn/fdOut il valore 0/1
 */
int parserArg(char* cmd, char*** argv, int* fdIn, int* fdOut){
    if(cmd[strlen(cmd)-1]=='\n')
        cmd[strlen(cmd)-1]='\0';
    int i;
    int argCount=2;
    char* arg;
    char* saveptr;

    for(i=0; cmd[i]!='\0';i++) {
        if (cmd[i] == ' ')
            argCount++;
        else if (cmd[i] == '<' || cmd[i] == '>')
            argCount--;
    }
    *argv = malloc(sizeof(char*)*argCount);
    if(*argv == NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    *fdOut = STDOUT_FILENO;
    *fdIn = STDIN_FILENO;

    for (arg = strtok_r(cmd, " ", &saveptr), i=0; arg != NULL; arg = strtok_r(NULL, " ", &saveptr)) {
        if(!strncmp(arg,"$",1)){
            char * name = arg+1;
            char * value = getenv(name);
            if(value==NULL) //Se la variabile d'ambiente non esiste viene ignorata
                continue;
            (*argv)[i]=malloc(strlen(value)+1);
            if((*argv)[i] == NULL){
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            strcpy((*argv)[i],value);
            i++;
        }else if(arg[0]=='>') { //Si aprre un file in modalita' scrittura per la redirezione dell'output, se non esiste viene creato
            if((*fdOut = open(arg + 1, O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0) {
                perror("errore >");
                return 0;
            }
        }else if(arg[0]=='<'){ //Si aprre un file in modalita' lettura per la redirezione dell'input
            if((*fdIn = open(arg + 1, O_RDONLY)) < 0) {
                perror("errore <");
                return 0;
            }
        }else{
            (*argv)[i]=malloc(strlen(arg)+1);
            if((*argv)[i] == NULL){
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            strcpy((*argv)[i],arg);
            i++;
        }
    }
    (*argv)[i]=NULL;
    return 1;
}
//Funzione che preso argv contenente il comando e i suoi argomenti
//e il numero dei file descriptor di input/output si occupa di fare la exec
void my_exec(char** argv, int fdIn, int fdOut){
    pid_t pid = fork();
    if(pid<0){
        perror("Fork failed: ");
        exit(EXIT_FAILURE);
    }
    if(pid == 0){
        if(fdOut!=STDOUT_FILENO)
            swapfd(fdOut,STDOUT_FILENO);
        if(fdIn!=STDIN_FILENO)
            swapfd(fdIn,STDIN_FILENO);
        execvp(argv[0],argv);
        perror("Error executing the command");
        clean(argv);
        exit(EXIT_FAILURE);
    }
    if(fdOut!=STDOUT_FILENO)
        close(fdOut);
    if(fdIn!=STDIN_FILENO)
        close(fdIn);
    clean(argv);
}

//comandi built-in
//Si occupa di spostarsi all'interno delle cartelle
int cd(char** argv){
    if(argv[1] == NULL || argv[2] != NULL) {
        printf("cd has only one argument\n");
        return 0;
    }
    int res;
    if((res = chdir(argv[1])) != 0)
        printf("No such file or directory: %s\n", argv[1]);
    clean(argv);
    return res;
}

//funzioni accessibili
//Funzione che mette all'interno di dir il nome della directory corrente.
//Prende come parametro la dimensione massima che può avere
void currentDir(char* dir, long size){
    char* path;
    char *saveptr = NULL;
    if((path = ((char *)malloc(((size_t)size)+1))) == NULL) {
        perror("malloc error: ");
        exit(EXIT_FAILURE);
    }
    char * tmp = path;
    //path contiene il percorso assoluto attuale
    if (getcwd(path, (size_t) size) == 0)   
        perror("getcwd error: ");
    strcpy(dir,"/");
    for (path = strtok_r(path, "/", &saveptr); path != NULL; path = strtok_r(NULL, "/", &saveptr))
        strcpy(dir,path);
    free(tmp);
}
/* Funzione che presa una stringa contenente un comando lo divide in sottocomandi separati dal simbolo | e li mette
 * in relazione usando la funzione pipe().
 * Si occupa anche di aspettare che finiscano l'esecuzione i vari comandi e ne controlla il valore in uscita.
 */
void cmdHandler (char *cmd){
    char* saveptr;
    int n_proc=0;
    int i;
    int status;
    char** argv;
    char* arg;
    int fdIn, fdOut;
    int pipefd[2];
    int check=0;

    arg = strtok_r(cmd, "|", &saveptr); //Si inserisce dentro arg il primo comando della pipe, se non ci dovessero
                                             //essere pipe viene inserito l'intero comando.
    while(arg != NULL) {
        if(!parserArg(arg, &argv, &fdIn, &fdOut)) //Se si dovessero verificare errori facendo il parsing del comando si interrompe il ciclo
            break;
        if (check) { //Si controlla se e' stata creata una pipe precedentemente, in caso si imposta come file descriptor
                     //per l'input pipefd[0] (restituito dalla funzione pipe())
            fdIn = pipefd[0];
        }
        if((arg = strtok_r(NULL, "|", &saveptr)) != NULL){ //Si inserisce dentro arg il successivo comando e viene aperta una pipe
                                                                    //siccome si entra nell'if solamente se nella stringa cmd ci sono almeno 2 comandi
                                                                    //separati dal simbolo |
            pipe(pipefd);
            fdOut = pipefd[1];
            check=1; //Si segnala che è stata creata una pipe
        }
        if(!strcmp(argv[0], "cd")){
            cd(argv);
            return;
        }
        my_exec(argv,fdIn,fdOut);
        n_proc++; //Si conta il numero di processi per poter fare la wait
    }
    for(i=0; i<n_proc; i++) {
        wait(&status);
        if(WIFEXITED(status)){
            if(WEXITSTATUS(status))
                printf("Command failed\n");
        }
    }
}
//Si occupa di rilevare errori di sintassi nel comando.
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
        if(cmd[i] == '|'){
            if(cmd[i+1] != ' ')
                return 0;
            checkPipe = 1;
            if(checkRedirect)
                return 0;
            if(!strncmp(&cmd[i + 2], "cd", 2))
                return 0;
            if(cmd[i+2] == '|')
                return 0;
            if(cmd[i+2] == '<' || cmd[i+2] == '>')
                return 0;
        }else if(cmd[i] == '>'){
            checkRedirect = 1;
        }else if(cmd[i] == '<' && checkPipe)
            return 0;
        if((cmd[i] == '|' || cmd[i] == '<' || cmd[i] == '>') && checkCD)
            return 0;
        if((cmd[i] == '>' || cmd[i] == '<') && isspace(cmd[i+1]))
            return 0;
    }
    return 1;
}


