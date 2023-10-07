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

long get_time(){
    struct timeval time, *address_time = &time;
    if (gettimeofday(address_time, NULL) != 0) {
        printf("Error in getting the time.\n");
        exit(1);
    }
    long epoch_time = time.tv_sec * 1000;
    return epoch_time + time.tv_usec / 1000;
}

bool and_flag = false;
char history[100][100];
int pid_history[100],  child_pid;
long time_history[100][2],start_time;
bool flag_for_Input = true;
int count_history = 0;

int add_to_history(char *command, int pid, long start_time_ms, long end_time_ms, int count_history) {
    strcpy(history[count_history], command);
    pid_history[count_history] = pid;
    time_history[count_history][0] = start_time_ms;
    time_history[count_history][1] = end_time_ms;
    return ++count_history;
}

void display_history() {
    printf("-------------------------------\n");
    printf("\n Command History: \n");
    printf("-------------------------------\n");

    for (int i = 0; i < count_history; i++) {
        printf("Command: %s\n", history[i]);
        printf("PID: %d\n", pid_history[i]);
        printf("Start_Time: %ld\n", time_history[i][0]);
        printf("End_Time: %ld\n", time_history[i][1]);
        printf("-------------------------------\n");
    }
}

void signal_handler(int signum) { // check
    if (signum == SIGINT) {
        printf("\n---------------------------------\n");
        display_history();        
        exit(0);
    }
}

void setup_signal_handler() {
    struct sigaction sh;
    sh.sa_handler = signal_handler;
    if (sigaction(SIGINT, &sh, NULL) != 0) {
        printf("Signal handling failed.\n");
        exit(1);
    }
    sigaction(SIGINT, &sh, NULL);
}

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
    child_pid = pid;

    if (pid < 0) {
        printf("Forking child failed.\n");
        exit(1);
    }

    else if (pid == 0) { //child process

        if (and_flag) signal(SIGHUP, SIG_IGN);

        execvp(argv[0], argv); 
        printf("Command failed.\n");
        exit(1);
    }

    else { 

        if (!and_flag)
        {
            int ret;
            int pid = wait(&ret);

            if (WIFEXITED(ret)) {
                if (WEXITSTATUS(ret) == -1)
                {
                    printf("Exit = -1\n");
                }
            } else {
                printf("\nAbnormal termination with pid :%d\n" , pid);
            }
        }
        return;
    }
}
void executePipe(char ***commands) {  // CHECK
    int i = 0, pid;
    int inputfd = STDIN_FILENO;  

    while (commands[i] != NULL) {
        int fd[2];
        pipe(fd);

        pid = fork();
        child_pid = pid;

        if (pid < 0) {
            printf("Forking child failed.\n");
            exit(1);

        } else if (pid == 0) {

            close(fd[0]); // close read end
            if (inputfd != STDIN_FILENO) {
                dup2(inputfd, STDIN_FILENO);
                close(inputfd);
            }
            if (commands[i + 1] != NULL) {
                dup2(fd[1], STDOUT_FILENO);
            }
            execvp(commands[i][0], commands[i]);
            exit(1);
        } else {
            close(fd[1]); // close write end
            if (inputfd != STDIN_FILENO) {
                close(inputfd);
            }
            inputfd = fd[0];// for next iteration
            i++;
        }
    }
    int wait_process;
    
    do
    {
        wait_process = wait(NULL);
    } while (wait_process > 0);
    
}

char** break_pipes_1(char *str) {
    char **commands;
    char* sep = "|";
    int len = 0 ;
    commands = (char**)malloc(sizeof(char*) * 100); 
    
    if (commands == NULL) {
        printf("Memory allocation failed\n");
        exit(1); 
    }

    int i = 0;

    char *token = strtok(str, sep);

    while (token != NULL) {
        len = strlen(token);
        commands[i] = (char*)malloc( len + 1);
        if (commands[i] == NULL) {
            printf("Memory allocation failed\n");
            exit(1); 
        }
        strcpy(commands[i], token);
        token = strtok(NULL, sep);
        i++;
    }

    commands[i] = NULL;  // Null-terminate the command array
    return commands;
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

char*** break_pipes_2( char **str){ // breaks the command we got from the last function on the basis of spaces
                                    //eg : {"cat helloworld.c" , "grep print" , "wc -l", NULL} is broken into 
                                    //{{"cat" , "helloworld.c",NULL},{"grep" , "print",NULL}, {"wc" , "-l", NULL} , NULL}
                                    //this is then passed to executePipe
    char ***command;
    command = (char***)malloc(sizeof(char**)*100);
    if (command == NULL) {
        printf("Memory allocation failed\n");
        exit(1); 
    }

    int len = 0 ,i = 0;
    while (str[len] != NULL)
    {
        len++;
    }
    for (i = 0; i < len; i++)
    {
        command[i] = break_spaces( str[i] );
    }
    command[i] = NULL;
    return command;

}

bool check_for_pipes(char* str) {

    for (int i = 0; str[i] != '\0'; i++) { // Loop until the end of the string
        if (str[i] == '|') { // Use single quotes for character literals
            return true;
        }
    }
    return false;
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

bool check_and(char* str){

    if (str[strlen(str) - 2] == '&')
    {
        str[strlen(str) - 2] = '\0';
        return true;
    }
    return false;
    
}

void executeScript(char *filename) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("Error opening script file\n");
        return;
    }

    char line[100];
    while (fgets(line, sizeof(line), file) != NULL) {
        int len = strlen(line);
        while (len > 0 && newline_checker(line , len)) {
            line[len - 1] = '\0';
            len -= 1 ;
        }
        if (len == 0) {
            continue;
        }
        if (check_for_pipes(line)) {
            char **command_1 = break_pipes_1(line);
            char ***command_2 = break_pipes_2(command_1);
            executePipe(command_2);
        } else {
            char **command_1 = break_spaces(line);
            executeCommand(command_1);
        }
    }

    fclose(file);
}

int main(int argc, char const *argv[]) {
    setup_signal_handler(); 
    char *str, *str_for_history = (char *)malloc(100);
    if (str_for_history == NULL) {
        printf("Error allocating memory\n");
        exit(1);
    }

    char c[100]; // to print the current directory
    printf("\n\nSHELL STARTED\n\n----------------------------\n\n");

    while (1) {
        getcwd(c, sizeof(c));
        printf("Shell> %s>>> ", c);
        str = Input(); // Get user input
        if (flag_for_Input == true) {
            strcpy(str_for_history, str);
            start_time = get_time();

            and_flag = check_and(str);

            if (str[0] == '@') { // @ means script file
                str[strlen(str) - 1] = '\0';
                executeScript(++str); // Skip the special character
            } else {
                if (check_for_pipes(str)) {
                    char **command_1 = break_pipes_1(str);
                    char ***command_2 = break_pipes_2(command_1);
                    executePipe(command_2);
                } else {
                    char **command_1 = break_spaces(str);
                    executeCommand(command_1);
                }
            }

            count_history =  add_to_history(str_for_history, child_pid, start_time, get_time() , count_history);
        }
    }

    free(str_for_history);

    return 0;
}
