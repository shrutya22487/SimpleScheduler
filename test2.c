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
int main(int argc, char const *argv[])
{   

    int pid = fork();
    char *p = "./test_1";
    char *args[] = {"./test_1" , NULL};

    if (pid < 0) {
        printf("Forking child failed.\n");
        exit(1);
    } else if (pid == 0) {
        execvp( p , args );
        printf("Command failed.\n");
        exit(1);

    } else {
        sleep(1);
        kill(pid,SIGUSR1);

    } 
    return 0;
}
