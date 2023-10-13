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
#include "Simple_Scheduler.h"

int front= 0 , rear = 0 , NCPU , TSLICE , count_jobs;
bool RR_flag = true;


typedef struct {
    int pid;
    char** command;
    int priority; 
    long start_time;
    long end_time;
} Job;

Job queue[200];
struct itimerspec timer_spec; 
timer_t timerid;

void stableSelectionSort(Job arr[], int n) {
    int i, j, minIndex;
    Job temp;

    for (i = 0; i < n - 1; i++) {
        minIndex = i;
        for (j = i + 1; j < n; j++) {
            if (arr[j].priority < arr[minIndex].priority || (arr[j].priority == arr[minIndex].priority && j < minIndex)) {
                minIndex = j;
            }
        }

        // Swap arr[i] and arr[minIndex]
        temp = arr[minIndex];
        for (j = minIndex; j > i; j--) {
            arr[j] = arr[j - 1];
        }
        arr[i] = temp;
    }
}


void change_RR_flag(){
    RR_flag = false;
}

void signal_handler(int signum) {

    if (signum == SIGALRM) {
        // This block is executed when the timer expires (TSLICE time has passed)
        printf("received sigalrm\n");
        // Call your schedule function or any other relevant actions
    }
    if (signum == SIGUSR1)
    {
        queue_command();
    }
    

}

void setup_signal_handler() {
    struct sigaction sh_alarm;   
    sh_alarm.sa_handler = signal_handler;
    if (sigaction(SIGALRM, &sh_alarm, NULL) != 0) {
        printf("Signal handling for SIGALRM failed.\n");
        exit(1);
    }

    struct sigaction sh;
    sh.sa_handler = signal_handler;
    if (sigaction(SIGINT, &sh, NULL) != 0) {
        printf("Signal handling failed.\n");
        exit(1);
    }
    sigaction(SIGUSR1, &sh, NULL);
}

void queue_command(){
    close(fd[1]);
    
    
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

void sort_jobs(Job jobs[], int count) {
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (jobs[j].priority > jobs[j + 1].priority) {
                // Swap jobs[j] and jobs[j+1]
                Job temp = jobs[j];
                jobs[j] = jobs[j + 1];
                jobs[j + 1] = temp;
            }
        }
    }
}


void round_robin(){
    //sort_queue();
    
    int cpu_counter = 0;
    int old_head = front;
    pid_t pid;

    while (cpu_counter != NCPU && !queue_empty()) {
        pid = queue[front++].pid;
        kill(pid, SIGCONT);
        cpu_counter++;
    }
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    if (sigtimedwait(&mask, NULL, &timer_spec.it_value) == -1) {
        if (errno == EAGAIN) {
            // The timer expired
        } else {
            perror("sigtimedwait");
            exit(1);
        }
    }

    printf("timer running\n");

    int status;
    int i = 0;

    while (i < cpu_counter) { 
        pid = queue[old_head].pid;
        kill(pid, SIGSTOP);
        waitpid(pid, &status, WNOHANG);

        if (!WIFEXITED(status)) {
            queue[rear++] = queue[old_head++];
        }
        i++;
    }
}


void simple_scheduler(int ncpu , int tslice){
    NCPU = ncpu;
    TSLICE = tslice;  

    struct sigevent sev;
    sev.sigev_notify = SIGEV_SIGNAL; // tells that event should be notified using a signal
    sev.sigev_signo = SIGALRM; //tells that SIGALRM should be used
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

    

    while (RR_flag)
    {
        if (!queue_empty())
        {
            round_robin();
        }
    }
    printf("Scheduler exited\n");
    if (timer_settime(timerid, 0, &timer_spec, NULL) == -1) {
        perror("timer_settime");
        exit(1);
    }
}
