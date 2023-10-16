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
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

int front= 0 , rear = 0 , NCPU , TSLICE , count_Submits , fd , cpu_counter , old_head;
char history[100][100];
int pid_history[100],  child_pid;
long time_history[100][2],start_time , wait_history[100];
bool flag_for_Input = true;
int count_history = 0;
char message_str[256];

typedef struct {
    int pid , priority;
    char** command; 
    long start_time , end_time , wait_time;
    int start_flag;
} Submit;
Submit queue[200];
struct itimerspec timer_spec; 
timer_t timerid;

void sort_queue() {
    int count , j;
    Submit temp;
    for (int i = front; i < rear; i++) {
        count = 0;
        j = i;
        while (j < rear - 1)
        {
            if (queue[j].priority > queue[j + 1].priority) {
                temp = queue[1];
                queue[j] = queue [ j + 1];
                queue[j + 1] = temp;
                count++;;
            }
            j++;
        }
        if (count <= 0) {
            break;
        }
    }
}

long get_time(){
    struct timeval time, *address_time = &time;
    if (gettimeofday(address_time, NULL) != 0) {
        printf("Error in getting the time.\n");
        exit(1);
    }
    long epoch_time = time.tv_sec * 1000;
    return epoch_time + time.tv_usec / 1000;
}

void add_to_history(char *command, int pid, long start_time_ms, long end_time_ms , long wait_time) {
    strcpy(history[count_history], command);
    pid_history[count_history] = pid;
    time_history[count_history][0] = start_time_ms;
    time_history[count_history][1] = end_time_ms;
    wait_history[count_history] = wait_time;
    count_history++;
}

void display_history() {
    printf("-------------------------------\n");
    printf("\n Command History for Scheduler: \n");
    printf("-------------------------------\n");

    for (int i = 0; i < count_history; i++) {
        printf("Command: %s\n", history[i]);
        printf("PID: %d\n", pid_history[i]);
        printf("Execution Time: %ld\n", time_history[i][1] - time_history[i][0]);
        printf("Wait Time: %ld\n", wait_history[i]);
        printf("-------------------------------\n");
    }

    //calculating averages
    long avg_waiting = 0 , avg_execution = 0;
    for (int i = 0; i < count_history; i++)
    {
        avg_execution += time_history[i][1] - time_history[i][0];
        avg_waiting += wait_history[i]; 
    }
    avg_execution /= count_history;
    avg_waiting /= count_history; 
    printf("Average execution Time: %ld\n\n", avg_execution);
    printf("Average Waiting Time: %ld\n", avg_waiting);
    printf("-------------------------------\n");
}

int queue_empty(){
    return front == rear;

}

void print_queue(){
    printf("front: %d , rear: %d\n" ,front , rear );
    for (int i = front; i < rear; i++)
    {
        printf("\npid: %d , Command_string : %s\n" , queue[i].pid , queue[i].command[0] );
    }
    
}

void add_waittime( ){
    for (int i = front ; i < rear; i++)
    {
        queue[i].wait_time += TSLICE;
    }
}

void stop_processes(){

    int status , i = 0;
    while (i < cpu_counter) { 
        int pid = queue[old_head].pid;
        kill(pid, SIGSTOP);
        //printf("stopping process with pid :%d\n" , pid );

        waitpid(pid, &status, WNOHANG);

        if (!WIFEXITED(status)) {
            queue[rear] = queue[old_head];
            rear++;
        }
        else{
            queue[old_head].end_time = get_time();
            Submit submit = queue[old_head];
            add_to_history(submit.command[0] , submit.pid , submit.start_time , submit.end_time , submit.wait_time);
            sleep(1);
        }
        old_head++;
        i++;    
    }

}

void sigalrm_handler(int signum){
    if (signum == SIGALRM) {
        //printf("received sigalrm\n");
        stop_processes();
        return;
    }
}

void set_round_robin_timer() {
    struct itimerval val;
    struct sigaction act;
    //doing signal masking
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART;
    act.sa_handler = sigalrm_handler;
    //what action will be taken on expiration of timer
    if (sigaction(SIGALRM, &act, NULL) == -1) {
        printf("sigaction not working\n");

        exit(1);
    }
    // setting up timer values
    val.it_value.tv_sec = TSLICE / 1000;
    val.it_value.tv_usec = (TSLICE % 1000) * 1000;
    val.it_interval.tv_sec = 0;
    val.it_interval.tv_usec = 0;
    //creates and starts the timer
    if (setitimer(ITIMER_REAL, &val, NULL) == -1) {
        printf("Not able to set time\n");
        exit(1);
    }
}

void round_robin(){
    //sort_queue();
    cpu_counter = 0;
    old_head = front;
    int pid ;

    while (cpu_counter != NCPU && !queue_empty()) {
        kill(queue[front].pid, SIGCONT);
        //printf("continuing process with pid :%d\n" ,queue[front].pid );

        if ( !queue[front].start_flag )
        {
            queue[front].start_flag = 1;
            queue[front].start_time = get_time();
        }
        cpu_counter++;
        front++;
    }

    add_waittime();
    sleep(1);
    //printf("timer running\n");
    set_round_robin_timer();
    sleep(1);
    
}


void sigusr_handler( int signum ){
    if ( signum == SIGUSR1 ) 
    {   while (!queue_empty())
        {
            round_robin();
        }
        return;
    }
}


void sigint_handler(int signum) { 
    if (signum == SIGINT) {
        printf("\n---------------------------------\n");
        display_history();
        exit(0);
    }
}

void setup_signal_handler() {
    struct sigaction sh_int, sh_usr1 , sh_stp;
    memset(&sh_int, 0, sizeof(sh_int));
    sh_int.sa_handler = sigint_handler; 
    if (sigaction(SIGINT, &sh_int, NULL) == -1)
    {
        printf("Error in handling SIGINT\n");
    }
    memset(&sh_usr1, 0, sizeof(sh_usr1));
    sh_usr1.sa_handler = sigusr_handler;
    if (sigaction(SIGUSR1, &sh_usr1, NULL) == -1)
    {
        printf("Error in handling SIGUSR1\n");
    }
    
    struct sigaction sh_alarm;   
    sh_alarm.sa_handler = sigalrm_handler;
    if (sigaction(SIGALRM, &sh_alarm, NULL) != 0) {
        printf("Signal handling for SIGALRM failed.\n");
        exit(1);
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

void queue_command( char* message){
    char **command = break_spaces(message);
    int i = 0 , j = 0;
    while (command[i] != NULL)
    {
        i++;
    }

    Submit submit;
    submit.command = (char**)malloc(sizeof(char*));
    submit.command[0] = (char*)malloc(sizeof(char)*100);
    submit.command[0] = command[1];
    if (i == 3)
    {
        submit.priority = atoi(command[2]);
    }
    else{
        submit.priority = 1;
    }
    submit.start_time = 0;
    submit.end_time = 0;
    submit.wait_time = 0;
    submit.start_flag = 0;
    queue[rear] = submit;

    int pid = fork();
    if (pid < 0) {
        printf("Forking child failed.\n");
        exit(1);
    } 
    else if (pid == 0) {
        execvp( submit.command[0] , submit.command );
        printf("Command failed.\n");
        exit(1);

    } else {
        kill(pid, SIGSTOP);
        queue[rear].pid = pid;
        //add_to_history( queue[rear].command[0] , queue[rear].pid , get_time(), 0);        
        rear++;
    }   
    
}

void read_pipe(){
    const char* pipename = "/tmp/simple__scheduler_fifo_";
    fd = open(pipename, O_RDONLY);
    // if (fd == -1) {
    //     printf("couldn't open fd\n");
    //     exit(1);
    // }
    char command[256];

    if (read(fd, command, sizeof(command)) > 0) {
        queue_command( command );
    }
    close(fd);
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
    //printf("Round Robin started\n");
    setup_signal_handler();
    NCPU = atoi(argv[1]);
    TSLICE = atoi(argv[2]);

    while (true)
    {
        read_pipe();

    }
    return 0;
}