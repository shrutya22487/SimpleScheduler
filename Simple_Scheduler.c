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
struct itimerspec timer_spec; 


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

int queue_empty(){
    return front == rear;

}

// Function to compare two jobs for sorting
int compare_jobs(const void *a, const void *b) {
    const Job *jobA = (const Job *)a;
    const Job *jobB = (const Job *)b;
    return jobB->priority - jobA->priority; // Sorting in descending order of priority
}

// Hardcoded sorting function for jobs
void sort_jobs(Job jobs[], int count) {
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (jobs[j].priority < jobs[j + 1].priority) {
                // Swap jobs[j] and jobs[j+1]
                Job temp = jobs[j];
                jobs[j] = jobs[j + 1];
                jobs[j + 1] = temp;
            }
        }
    }
}

void simple_scheduler(int ncpu , int tslice , char **command){
    queue_command(command);
    NCPU = ncpu;
    TSLICE = tslice;    
}

void round_robin(){
    sort_queue();
    
    int cpu_counter = 0;
    int old_head = front;
    printf("Starting commands\n");
    pid_t pid;

    while (cpu_counter != NCPU && !queue_empty()) {
        pid = queue[front++].pid;
        kill(pid, SIGCONT);
        cpu_counter++;
    }

    printf("Running commands\n");

    //Timer stuff goes here
    // Timer code: Wait for the timer to expire
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);

    // Use sigtimedwait to handle the timer expiration event
    if (sigtimedwait(&mask, NULL, &timer_spec.it_value) == -1) {
        if (errno == EAGAIN) {
            // The timer expired
        } else {
            perror("sigtimedwait");
            exit(1);
        }
    }

    int status;
    int i = 0;


    while (i < cpu_counter) { // Change the loop condition to i < cpu_counter
        pid = queue[old_head].pid;
        kill(pid, SIGSTOP);
        waitpid(pid, &status, 0);

        if (!WIFEXITED(status)) {
            queue[rear++] = queue[old_head++];
        }
        i++;
    }
}

//round robin code goes here

int main(int argc, char const *argv[])
{
    // handle Ctrl+C signal here


    struct sigevent sev;
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;
    sev.sigev_value.sival_ptr = &timerid;

    // Create a timer
    if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1) {
        perror("timer_create");
        exit(1);
    }

    // Configure the initial timer expiration and interval
    timer_spec.it_value.tv_sec = TSLICE / 1000;
    timer_spec.it_value.tv_nsec = (TSLICE % 1000) * 1000000;
    timer_spec.it_interval.tv_sec = 0;
    timer_spec.it_interval.tv_nsec = 0;

    // Set the timer with the configured values
    if (timer_settime(timerid, 0, &timer_spec, NULL) == -1) {
        perror("timer_settime");
        exit(1);
    }


    return 0;
}
