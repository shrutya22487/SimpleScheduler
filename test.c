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


char history[100][100];
int pid_history[100],  child_pid;
long time_history[100][2],start_time;
bool flag_for_Input = true;
int count_history = 0, queue_head= 0 , queue_tail = 0, NCPU , TSLICE;
int queue[4];
bool newline_checker( char* line  , int len){
    bool flag1  = false , flag2 = false;
    if ( line[len - 1] == '\n' )
    {
        flag1 = true;
    }
    if (line[len - 1] == '\r')
    {
        flag2 = true;
    }
    return flag1 || flag2;
}


void executeCommand(char** argv) {  // check
    int pid = fork();

    if (pid < 0) {
        printf("Forking child failed.\n");
        exit(1);
    }

    else if (pid == 0) { //child process
        wait(NULL);
        // //signal(SIGSTOP , SIG_DFL);
        // printf("giving stop signal\n");
        // kill(pid, SIGSTOP);
        execvp(argv[0], argv); 
        printf("Command failed.\n");
        exit(1);
    }

    else { 
        int ret;
        //int pid = wait(&ret);
        queue[queue_tail++] = pid;
        kill(pid, SIGSTOP);
        printf("Child Paused\n");
        //printf("Child pid: %d , return status: %d ", pid , ret );


        // if (WIFEXITED(ret)) {
        //     if (WEXITSTATUS(ret) == -1)
        //     {
        //         printf("Exit = -1\n");
        //     }
        // } else {
        //     printf("\nAbnormal termination with pid :%d\n" , pid);
        // }
        
        return;
    }
}

void schedule(int signum , int queue[]) {
    int queue_size = queue_tail - queue_head + 1;
    // Signal the first NCPU processes in the ready queue to start execution
    for (int i = 0; i < NCPU && i < queue_size; i++) {
        int pid = queue[i];
        kill(pid, SIGCONT);
    }

    // Pause the running processes after TSLICE milliseconds
    usleep(TSLICE * 1000);

    // Check for completed processes and remove them from the queue
    int i = queue_head;
    while (i < queue_size) {
        int pid = queue[i];
        int status;
        int result = waitpid(pid, &status, WNOHANG);
        if (result == -1) {
            // Error handling
        } else if (result == 0) {
            // The process is still running
            i++;
        } else {
            // The process has terminated, remove it from the queue
            queue_head++;
        }
    }

    // Requeue the paused processes to the rear of the ready queue
    for (int i = queue_tail; i < NCPU && i < queue_size; i++) {
        int pid = queue[i];
        kill(pid, SIGSTOP);
        queue[queue_head++] = pid;
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
    printf("-------------------------------\n");
    printf("\n Shell started \n");
    printf("-------------------------------\n");


    char* str = Input();
    executeCommand(break_spaces(str));
    kill(queue[0], SIGCONT);
    printf("Child Resumed\n");

    return 0;
}

