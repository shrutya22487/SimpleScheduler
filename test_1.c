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

void signal_handler(int signum) { 
    if (signum == SIGUSR1) {
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
