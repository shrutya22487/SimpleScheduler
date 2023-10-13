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
    printf("meow\n");

    if (pid < 0) {
        printf("Forking child failed.\n");
        exit(1);
    } else if (pid == 0) {
        execvp( p , args );
        printf("Command failed.\n");
        exit(1);

    } else {
        sleep(1);


        char *str = "submit ./a.out";
        int fd[2];
        int check = pipe(fd);
        if (check == -1)
        {
            printf("Error in creating pipe\n");
            exit(1);
        }


        close(fd[0]);
        write( fd[1] , str , sizeof(str) );
        close(fd[1]);


        kill(pid,SIGUSR1);


    } 
    return 0;
}
