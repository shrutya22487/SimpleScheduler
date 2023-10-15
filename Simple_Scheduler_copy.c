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


Submit queue[200];
struct itimerspec timer_spec; 
timer_t timerid;

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

int find_submit(int pid){
    for (int i = 0; i < count_history; i++)
    {
        if (pid == pid_history[i])
        {
            return i;
        }
        
    }
    
}

void add_waittime( ){
    //printf("cpu counter : %d , front : %d , rear ; %d " , cpu_counter , front , rear);
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
        usleep(500 * 1000);
        waitpid(pid, &status, WNOHANG);

        if (!WIFEXITED(status)) {
            queue[rear++] = queue[old_head];
        }
        else{
            queue[old_head].end_time = get_time();
            Submit submit = queue[old_head];
            add_to_history(submit.command[0] , submit.pid , submit.start_time , submit.end_time , submit.wait_time);
        }
        old_head++;
        i++;    
    }

}

void signal_handler(int signum) { 
    if (signum == SIGINT) {
        printf("\n---------------------------------\n");
        display_history();
        exit(0);
    }

    else if (signum == SIGALRM) {
        //printf("received sigalrm\n");
        stop_processes();
        return;
    }
}

void set_round_robin_timer() {
    struct itimerval val;
    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    action.sa_handler = signal_handler;

    if (sigaction(SIGALRM, &action, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    val.it_value.tv_sec = TSLICE / 1000;
    val.it_value.tv_usec = (TSLICE % 1000) * 1000;
    val.it_interval.tv_sec = 0;
    val.it_interval.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &val, NULL) == -1) {
        perror("setitimer");
        exit(1);
    }
}

void round_robin(){
    //printf("cpu counter : %d , front : %d , rear ; %d " , cpu_counter , front , rear);

    //sort_queue();
    cpu_counter = 0;
    old_head = front;
    int pid ;

    while (cpu_counter != NCPU && !queue_empty()) {
        kill(queue[front].pid, SIGCONT);
        if ( !queue[front].start_flag )
        {
            queue[front].start_flag = 1;
            queue[front].start_time = get_time();
        }
        cpu_counter++;
        front++;
    }

    add_waittime();
    printf("timer running\n");
    set_round_robin_timer();
    usleep(500 * 1000);
    
}

void stableSelectionSort(Submit arr[], int n) {
    int i, j, minIndex;
    Submit temp;

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

void sigusr_handler( int signum ){
    if ( signum == SIGUSR1 ) 
    {   while (!queue_empty())
        {
            round_robin();
        }
        return;
    }
}

void setup_signal_handler() {
    struct sigaction sh_int, sh_usr1;
    memset(&sh_int, 0, sizeof(sh_int));
    sh_int.sa_handler = signal_handler;
    sigaction(SIGINT, &sh_int, NULL); // Register handler for SIGINT

    memset(&sh_usr1, 0, sizeof(sh_usr1));
    sh_usr1.sa_handler = sigusr_handler;
    sigaction(SIGUSR1, &sh_usr1, NULL); // Register handler for SIGQUIT
   
    struct sigaction sh_alarm;   
    sh_alarm.sa_handler = signal_handler;
    if (sigaction(SIGALRM, &sh_alarm, NULL) != 0) {
        printf("Signal handling for SIGALRM failed.\n");
        exit(1);
    }
}

void sort_Submits(Submit Submits[], int count) {
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (Submits[j].priority > Submits[j + 1].priority) {
                // Swap Submits[j] and Submits[j+1]
                Submit temp = Submits[j];
                Submits[j] = Submits[j + 1];
                Submits[j + 1] = temp;
            }
        }
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
    const char* fifoName = "/tmp/simplescheduler_fifo";
    int fifo_fd = open(fifoName, O_RDONLY);

    char command[256];
    ssize_t bytes_read;

    bytes_read = read(fifo_fd, command, sizeof(command));
    close(fifo_fd);

    if (bytes_read > 0) {
        queue_command( command );
        //print_queue();
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
    printf("Round Robin started\n");
    setup_signal_handler();
    NCPU = atoi(argv[1]);
    TSLICE = atoi(argv[2]);

    while (true)
    {
        read_pipe();

    }
    return 0;
}
