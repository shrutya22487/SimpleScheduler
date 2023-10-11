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
int count_history = 0, queue_head= 0 , queue_tail = 0, NCPU = 2 , TSLICE = 100;
int queue[100];
int cpu_counter = 0 , old_head = 0;

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


void queue_command(char** argv) {
    pid_t pid = fork();

    if (pid < 0) {
        printf("Forking child failed.\n");
        exit(1);
    } else if (pid == 0) {
        wait(NULL);
        execvp(argv[0], argv);
        printf("Command failed.\n");
        exit(1);
    } else { 
        queue[queue_tail++] = pid;
        kill(queue[0], SIGSTOP);
        printf("Sleeping\n");

       // sleep(2);
        //kill(queue[0] , SIGCONT);
        printf("Child Paused___queue_tail = %d\n", queue_tail);
        return;
    }
}

int queue_empty (){
    return queue_head == queue_tail;
}\



// void timer_handler() {
//     pid_t pid;
//     int status;
//     int i = 0;
//     while (i < cpu_counter) {
//         pid = queue[old_head++];
//         kill(pid, SIGSTOP);
//         waitpid(pid, &status, 0);

//         if (!WIFEXITED(status)) {
//             queue[queue_tail++] = pid;
//         }
//         i++;
//     }
//     cpu_counter = 0;
// }

// void schedule() {
//     cpu_counter = 0;
//     old_head = queue_head;
//     printf("Starting commands\n");
//     pid_t pid;

//     while (cpu_counter != NCPU && !queue_empty()) {
//         pid = queue[queue_head++];
//         kill(pid, SIGCONT);
//         cpu_counter++;
//     }

//     printf("Running commands\n");

//     // Timer starts here
//     struct sigaction sa;
//     sa.sa_handler = timer_handler;
//     sa.sa_flags = 0;
//     sigemptyset(&sa.sa_mask);
//     sigaction(SIGALRM, &sa, NULL);

//     // Set an initial timer
//     struct itimerval timer;
//     timer.it_value.tv_sec = 0;
//     timer.it_value.tv_usec = TSLICE * 1000;
//     timer.it_interval = timer.it_value;
//     if (!queue_empty()) {
//         setitimer(ITIMER_REAL, &timer, NULL);
//     }
// }

void schedule(){
    kill(queue[0] , SIGCONT);
    kill(queue[1] , SIGCONT);
    sleep(1);
    kill(queue[0] , SIGKILL);
    kill(queue[1] , SIGKILL);
    printf("______________");
        kill(queue[0] , SIGCONT);
    kill(queue[1] , SIGCONT);
    sleep(1);
    kill(queue[0] , SIGKILL);
    kill(queue[1] , SIGKILL);
    printf("______________");
        kill(queue[0] , SIGCONT);
    kill(queue[1] , SIGCONT);
    sleep(1);
    kill(queue[0] , SIGKILL);
    kill(queue[1] , SIGKILL);
    printf("______________");
        kill(queue[0] , SIGCONT);
    kill(queue[1] , SIGCONT);
    sleep(1);
    kill(queue[0] , SIGKILL);
    kill(queue[1] , SIGKILL);
    printf("______________");
        kill(queue[0] , SIGCONT);
    kill(queue[1] , SIGCONT);
    sleep(1);
    kill(queue[0] , SIGKILL);
    kill(queue[1] , SIGKILL);
    printf("______________");


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


char* Input(){  
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
    queue_command(break_spaces(str));
    printf("Executing commands\n");
    printf("%d" , queue[0]);
    sleep(3);
    int pid = queue[0];
    kill( pid, SIGCONT);

        //schedule();
    
    
    return 0;
}

