#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <sched.h>

// Define constants for priority levels
#define MIN_PRIORITY 1
#define MAX_PRIORITY 4
#define DEFAULT_PRIORITY 1

long get_time() {
    struct timeval time, *address_time = &time;
    if (gettimeofday(address_time, NULL) != 0) {
        printf("Error in getting the time.\n");
        exit(1);
    }
    long epoch_time = time.tv_sec * 1000;
    return epoch_time + time.tv_usec / 1000;
}

char history[100][100];
int pid_history[100], child_pid;
long time_history[100][2], start_time;
bool flag_for_Input = true;
int count_history = 0, queue_head = 0, queue_tail = 0, NCPU, TSLICE, priority = DEFAULT_PRIORITY;
pid_t queue[100];

typedef struct {
    int pid;
    char command[100];
    int priority; // Priority of the job
    long start_time;
    long end_time;
} Job;

Job jobs[100]; // Array to store job information

int add_to_history(char *command, int pid, int priority, long start_time_ms, long end_time_ms, int count_history) {
    strcpy(history[count_history], command);
    pid_history[count_history] = pid;
    jobs[count_history].pid = pid;
    jobs[count_history].priority = priority;
    jobs[count_history].start_time = start_time_ms;
    jobs[count_history].end_time = end_time_ms;
    return ++count_history;
}

void display_history() {
    printf("-------------------------------\n");
    printf("\n Command History: \n");
    printf("-------------------------------\n");

    for (int i = 0; i < count_history; i++) {
        printf("Command: %s\n", history[i]);
        printf("PID: %d\n", pid_history[i]);
        printf("Priority: %d\n", jobs[i].priority);
        printf("Start_Time: %ld\n", jobs[i].start_time);
        printf("End_Time: %ld\n", jobs[i].end_time);
        printf("-------------------------------\n");
    }
}

void signal_handler(int signum) {
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

bool newline_checker(char *line, int len) {
    bool flag1 = false, flag2 = false;
    if (line[len - 1] == '\n') {
        flag1 = true;
    }
    if (line[len - 1] == '\r') {
        flag2 = true;
    }
    return flag1 || flag2;
}

void executeCommand(char **argv) {
    int pid = fork();
    child_pid = pid;

    if (pid < 0) {
        printf("Forking child failed.\n");
        exit(1);
    } else if (pid == 0) { // Child process
        setpriority(PRIO_PROCESS, 0, priority); // Set the process priority
        execvp(argv[0], argv);
        printf("Command failed.\n");
        exit(1);
    } else {
        int queue_index = queue_tail;
        queue[queue_index] = pid;
        queue_tail++;
    }
}

int queue_empty() {
    return queue_head == queue_tail;
}

void schedule() {
    int cpu_counter = 0;   // tells how many processes have been continued
    int old_head = queue_head;
    printf("Starting commands\n");
    pid_t pid;

    while (cpu_counter != NCPU && !queue_empty()) { // to take in NCPU processes or till the queue is empty
        pid = queue[queue_head++];
        kill(pid, SIGCONT); // continue the process
        cpu_counter++;
    }

    printf("Running commands\n");

    // timer code will be here , for now assume there is some timer which runs the code for a specific TSLICE
    int status; // to check exit status of the porcesses
    int i = 0;

    while (i < cpu_counter) { // stops the processes and adds to queue only if they are not finished
        pid = queue[old_head++];
        kill(pid, SIGSTOP);
        waitpid(pid, &status, 0);

        if (!WIFEXITED(status)) {
            queue[queue_tail++] = pid;
        }
        i++;
    }
}

char **break_spaces(char *str) {
    char **command;
    char *sep = " \n";
    command = (char **)malloc(sizeof(char *) * 100);
    int len = 0;
    if (command == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    int i = 0;
    char *token = strtok(str, sep);
    while (token != NULL) {
        len = strlen(token);
        command[i] = (char *)malloc(len + 1);
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

char *Input() {
    char *input_str = (char *)malloc(100);
    if (input_str == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    flag_for_Input = false;
    fgets(input_str, 100, stdin);

    if (strlen(input_str) != 0 && input_str[0] != '\n' && input_str[0] != ' ') {
        flag_for_Input = true;
    }
    return input_str;
}

int main(int argc, char *argv[]) {
    char *str;
    if (argc != 3 && argc != 4) {
        printf("All arguments not entered !");
        exit(1);
    }

    NCPU = atoi(argv[1]);
    TSLICE = atoi(argv[2]);
    int queue[NCPU];
    char str_for_history[100]; // Declares str_for_history
    setup_signal_handler();

    if (argc == 4) {
        priority = atoi(argv[3]);
        if (priority < MIN_PRIORITY || priority > MAX_PRIORITY) {
            fprintf(stderr, "Priority must be in the range 1-4.\n");
            exit(1);
        }
    }

    char c[100];
    printf("\n\nSHELL STARTED\n\n----------------------------\n\n");

    while (1) {
        getcwd(c, sizeof(c));
        printf("Shell> %s>>> ", c);
        str = Input(); // Get user input

        if (flag_for_Input == true) {
            char **command_1 = break_spaces(str);

            // Create a new process and execute it
            executeCommand(command_1);

            strcpy(str_for_history, str);
            start_time = get_time();
            count_history = add_to_history(str_for_history, child_pid, priority, start_time, get_time(), count_history);
        }
    }

    free(str_for_history);

    return 0;
}