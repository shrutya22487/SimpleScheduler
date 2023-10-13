#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <stdbool.h>
#include <signal.h> 
#include <sys/time.h>
#include <time.h>
#include <errno.h>

void read_pipe(){
    int fd[2];
    char *str = (char*)malloc(sizeof(char) * 100);
    int check = pipe(fd);
    if (check == -1)
    {
        printf("Pipe could not be created\n");
        exit(1);
    }
    close(fd[1]);
    read( fd[0] , str , sizeof(str) );
    close(fd[0]);
    puts(str);
    

}

void signal_handler(int signum) { 
    if (signum == SIGUSR1) {
        read_pipe();
        printf("\ncaught sigusr1\n");    
        exit(0);
    }
}

void setup_signal_handler() {
    struct sigaction sh;
    sh.sa_handler = signal_handler;
    if (sigaction(SIGUSR1, &sh, NULL) != 0) {
        printf("Signal handling failed.\n");
        exit(1);
    }
    sigaction(SIGUSR1, &sh, NULL);
}

int main(int argc, char const *argv[])
{
    setup_signal_handler();
    while (1)
    {
        /* code */
    }
    

    return 0;
}
