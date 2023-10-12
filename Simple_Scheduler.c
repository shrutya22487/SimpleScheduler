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
#include "simple-shell(1).h"

int front= 0 , rear = 0 , NCPU , TSLICE;

char history[100][100];
int pid_history[100],  child_pid;
long time_history[100][2],start_time;
bool flag_for_Input = true;
int count_history = 0, queue_head= 0 , queue_tail = 0, NCPU , TSLICE ;

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
timer_t timerid;

void signal_handler(int signum) {

    if (signum == SIGALRM) {
        // This block is executed when the timer expires (TSLICE time has passed)
        printf("received sigalrm\n");
        // Call your schedule function or any other relevant actions
    }
}
void setup_signal_handler() {
    struct sigaction sh_alarm;

    
    sh_alarm.sa_handler = signal_handler;
    if (sigaction(SIGALRM, &sh_alarm, NULL) != 0) {
        printf("Signal handling for SIGALRM failed.\n");
        exit(1);
    }
}
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

char** break_spaces(char *str) {  
    char **command;
    char *sep = " \n";
    command = (char**)malloc(sizeof(char*) * 100);
    int len = 0;
    if (command == NULL) {
        printf("Memory allocation failed\n");
        exit(1); 
    }

    int i = 0;
    char *token = strtok(str,sep ); 
    while (token != NULL) {
        len = strlen(token);
        command[i] = (char*)malloc( len + 1);
        if (command[i] == NULL) {
            printf("Memory allocation failed\n");
            exit(1); 
        }

        strcpy(command[i], token);
        token = strtok(NULL, sep);
        i++;
    }
    command[i] = NULL;
    return command;
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
            if (jobs[j].priority > jobs[j + 1].priority) {
                // Swap jobs[j] and jobs[j+1]
                Job temp = jobs[j];
                jobs[j] = jobs[j + 1];
                jobs[j + 1] = temp;
            }
        }
    }
}

void simple_scheduler(int ncpu , int tslice){
    
    while (true)
    {
        if (!queue_empty())
        {
            round_robin();
        }

    }
    
    
    
    //queue_command(command);
    NCPU = ncpu;
    TSLICE = tslice;    
}

void round_robin(){
    //sort_queue();
    
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
    printf("timer running1\n");
    // Use sigtimedwait to handle the timer expiration event
    // the timer expires over here due , and SIGALRM is sent due to which the program exits completely
    // some error handling should be done over here 
    if (sigtimedwait(&mask, NULL, &timer_spec.it_value) == -1) {
        if (errno == EAGAIN) {

            printf("timer running2\n");
            // The timer expired
        } else {
            printf("timer running3\n");
            perror("sigtimedwait");
            exit(1);
        }
    }

    printf("timer running4\n");

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

char* Input(){   // to take input from user , returns the string entered
    char *input_str = (char*)malloc(100);
    if (input_str == NULL) {
        printf("Memory allocation failed\n");
        exit(1); 
    }
    flag_for_Input = false;
    fgets(input_str ,100, stdin);
    
    if (strlen(input_str) != 0 && input_str[0] != '\n' && input_str[0] != ' ')
    {   
        flag_for_Input = true;
    }
    return input_str;
}


int main(int argc, char const *argv[])
{
    // handle Ctrl+C signal here
    // queue_command( break_spaces( Input() ) );
    // queue_command( break_spaces( Input() ) );
    // queue_command( break_spaces( Input() ) );

    NCPU = 2 , TSLICE = 4000;
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

    // Set the timer with the configured values
    if (timer_settime(timerid, 0, &timer_spec, NULL) == -1) {
        perror("timer_settime");
        exit(1);
    }
    round_robin();
    round_robin();

    return 0;
}
