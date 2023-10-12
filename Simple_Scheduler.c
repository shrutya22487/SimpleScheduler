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
#include "Simple_Scheduler.h"

int front= 0 , rear = 0 , NCPU , TSLICE;

typedef struct {
    int pid;
    char** command;
    int priority; 
    long start_time;
    long end_time;
} Job;

Job queue[200];
int count_jobs;



void queue_command(char** command){
    int i = 0 , j = 0;
    while (command[i] != NULL)
    {
        i++;
    }

    Job job;
    job.command = (char**)malloc(sizeof(char*));
    job.command[0] = (char*)malloc(sizeof(char)*100);
    job.command[0] = command[1];

    if (i == 3)
    {
        job.priority = atoi(command[2]);
    }
    else{
        job.priority = 1;
    }
    queue[rear] = job;
    
    int pid = fork();

    if (pid < 0) {
        printf("Forking child failed.\n");
        exit(1);
    } else if (pid == 0) {
        execvp( job.command[0] , job.command );
        printf("Command failed.\n");
        exit(1);

    } else { 
        queue[rear++].pid = pid;
        kill(pid, SIGSTOP);
    }   
    
}


void sort_queue(){

    //
}

void simple_scheduler(int ncpu , int tslice , char **command){
    queue_command(command);
    NCPU = ncpu;
    TSLICE = tslice;    
}

void round_robin(){
    int cpu_counter = 0;
    int old_head = front;
    printf("Starting commands\n");
    pid_t pid;

    while (cpu_counter != NCPU && !queue_empty()) {
        pid = queue[queue_head++];
        kill(pid, SIGCONT);
        cpu_counter++;
    }

    printf("Running commands\n");

    usleep(TSLICE * 1000);
    int status;
    int i = 0;

    while (i < cpu_counter) { // Change the loop condition to i < cpu_counter
        pid = queue[old_head++];
        kill(pid, SIGSTOP);
        waitpid(pid, &status, 0);

        if (!WIFEXITED(status)) {
            queue[queue_tail++] = pid;
        }
        i++;
    }

}

//round robin code goes here

int main(int argc, char const *argv[])
{
    // handle Ctrl+C signal here


    return 0;
}
