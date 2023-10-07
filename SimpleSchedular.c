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


int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        printf("All arguments not entered\n");
        exit(1);
    }
    
    int NCPU = atoi(argv[1]), TSLICE = atoi(argv[2]);
    system("./simple-shell");

    return 0;
}

