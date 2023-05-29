#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>

typedef int pipe_t[2];

int main(int argc, char const *argv[])
{
    int N;
    int fd;
    int pid;
    int ritorno, status, pidFiglio;
    int X; // numero linee
    char *primiCar;
    int i, j;
    pipe_t *piped;
    char c,ok;
    int pipePF[2];

    if (argc < 4)
    {
        printf("NUMERO DEI PARAMETRI INSUFFICIENTE\n");
        exit(1);
    }

    N = argc - 2;
    primiCar = malloc(N * sizeof(char));

    X = atoi(argv[argc - 1]);

    piped = (pipe_t *)malloc(N * sizeof(pipe_t));

    for (i = 0; i < N; i++)
    {
        if (pipe(piped[i]) < 0)
        {
            printf("ERRORE NELLA CREAZIONE DELLA PIPE\n");
            exit(2);
        }
    }
    if(pipe(pipePF)<0){
        printf("errore\n");
    }

    for (i = 0; i < N; i++)
    {
        if (i == 0)
        {

            if ((pid = fork()) < 0)
            {
                printf("errore nella creazione del processo %d\n", i);
                exit(3);
            }

            if (pid == 0)
            {
                close(pipePF[1]);
                for(j=0;j<N;j++){
                    close(piped[i][0]);
                    if(j!=i)close(piped[i][1]);
                }

                if((fd=open(argv[i+1],O_RDONLY))<0){
                    printf("ERRORE NELL'APERTURA DEL FILE %s\n",argv[i+1]);
                    exit(4);
                }
                j=0;
                while(read(fd,&c,1)){
                    if(j==0){
                        primiCar[i]=c;
                    }
                    j++;
                    if(c=='\n'){
                        write(piped[i][1],primiCar,sizeof(primiCar));
                        j=0;
                        read(pipePF[0],&ok,1);
                    }
                }
                exit(c);
            }
        }
        else
        {
            read(piped[i-1][0],&primiCar,sizeof(primiCar));

            if ((pid = fork()) < 0)
            {
                printf("errore nella creazione del processo %d\n", i);
                exit(3);
            }

            if (pid == 0)
            {
                close(pipePF[1]);
                close(pipePF[0]);
                for(j=0;j<N;j++){
                    close(piped[i][0]);
                    if(j!=i)close(piped[i][1]);
                }

                if((fd=open(argv[i+1],O_RDONLY))<0){
                    printf("ERRORE NELL'APERTURA DEL FILE %s\n",argv[i+1]);
                    exit(4);
                }
                
                while(read(fd,&c,1)){
                    if(j==0){
                        primiCar[i]=c;
                    }
                    j++;
                    if(c=='\n'){
                        write(piped[i][1],primiCar,sizeof(primiCar));
                        j=0;
                    }
                }
                exit(c);
            }
            
        }
    }

    close(pipePF[0]);
    for(i=0;i<N;i++){
        close(piped[i][1]);
        if(i!=(N-1))close(piped[i][0]);
    }
    j=0;
    while(j<X){
        read(piped[N-1][0],&primiCar,sizeof(primiCar));
        for(i=0;i<N;i++){
            printf("il figlio di indice %d, ha trovato nel file %s, %c come primo carattere nella linea n° %d\n",i,argv[i+1],primiCar[i],j+1);
        }
        j++;
        write(pipePF[1],&ok,sizeof(char));
    }

    for (i = 0; i < N; i++)
    {
        if ((pidFiglio = wait(&status)) < 0)
        {
            printf("Errore in wait\n");
            exit(10);
        }

        if ((status & 0xFF) != 0)
            printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il figlio con pid=%d ha ritornato il valore %d (se 255 problemi)\n", pidFiglio, ritorno);
        }
    }

    return 0;
}
