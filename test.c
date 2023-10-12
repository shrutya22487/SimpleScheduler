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
#include <errno.h>

// Define constants for priority levels
#define MIN_PRIORITY 1
#define MAX_PRIORITY 100

timer_t timerid;
struct itimerspec timer_spec;  // to configure the timer's properties.


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

int queue_empty (){
    return queue_head == queue_tail;
}

void schedule() {
    int cpu_counter = 0;
    int old_head = queue_head;
    printf("Starting commands\n");
    pid_t pid;

    while (cpu_counter != NCPU && !queue_empty()) {
        pid = queue[queue_head++];
        kill(pid, SIGCONT);
        cpu_counter++;
    }

    printf("Running commands\n");

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

    while (i < cpu_counter) {
        pid = queue[old_head++];
        kill(pid, SIGSTOP);
        waitpid(pid, &status, WNOHANG);

        if (!WIFEXITED(status)) {
            queue[queue_tail++] = pid;
        }
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
        // This block is executed when the timer expires (TSLICE time has passed)
        schedule();  // Call your schedule function or any other relevant actions
    }
}

void setup_signal_handler() {
    struct sigaction sh_int, sh_alarm;

    sh_int.sa_handler = signal_handler;
    if (sigaction(SIGINT, &sh_int, NULL) != 0) {
        printf("Signal handling for SIGINT failed.\n");
        exit(1);
    }

    sh_alarm.sa_handler = signal_handler;
    if (sigaction(SIGALRM, &sh_alarm, NULL) != 0) {
        printf("Signal handling for SIGALRM failed.\n");
        exit(1);
    }
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

// void executeCommand(char **argv, int queue[], int priority) {
//     int pid = fork();
//     child_pid = pid;

//     if (pid < 0) {
//         printf("Forking child failed.\n");
//         exit(1);
//     } else if (pid == 0) { // Child process
//         signal(SIGCONT, SIG_DFL);
//         execvp(argv[0], argv);
//         printf("Command failed.\n");
//         exit(1);
//     } else {
//         queue[queue_head++] = pid;
//         int ret;
//         int pid = wait(&ret);

//         if (WIFEXITED(ret)) {
//             if (WEXITSTATUS(ret) == -1) {
//                 printf("Exit = -1\n");
//             }
//         } else {
//             printf("\nAbnormal termination with pid :%d\n", pid);
//         }

//         return;
//     }
// }

// void schedule(int signum, int queue[], int priorities[], int queue_size) {
//     // Sort the jobs in the queue based on priority (higher priority first)
//     for (int i = 0; i < queue_size - 1; i++) {
//         for (int j = i + 1; j < queue_size; j++) {
//             if (priorities[i] < priorities[j]) {
//                 int temp_pid = queue[i];
//                 int temp_priority = priorities[i];
//                 queue[i] = queue[j];
//                 priorities[i] = priorities[j];
//                 queue[j] = temp_pid;
//                 priorities[j] = temp_priority;
//             }
//         }
//     }

//     // Signal the first NCPU processes in the ready queue to start execution
//     for (int i = 0; i < NCPU && i < queue_size; i++) {
//         int pid = queue[i];
//         kill(pid, SIGCONT);
//     }

//     // Pause the running processes after TSLICE milliseconds
//     usleep(TSLICE * 1000);

//     // Check for completed processes and remove them from the queue
//     int i = queue_head;
//     while (i < queue_size) {
//         int pid = queue[i];
//         int status;
//         int result = waitpid(pid, &status, WNOHANG);
//         if (result == -1) {
//             // Error handling
//         } else if (result == 0) {
//             // The process is still running
//             i++;
//         } else {
//             // The process has terminated, remove it from the queue
//             queue_head++;
//         }
//     }

//     // Requeue the paused processes to the rear of the ready queue
//     for (int i = queue_tail; i < NCPU && i < queue_size; i++) {
//         int pid = queue[i];
//         kill(pid, SIGSTOP);
//         queue[queue_head++] = pid;
//     }
// }


int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printf("All arguments not entered\n");
        exit(1);
    }

    NCPU = atoi(argv[1]);
    TSLICE = atoi(argv[2]);
    int queue[NCPU];
    int priorities[NCPU];
    setup_signal_handler();
    char *str, *str_for_history = (char *)malloc(100);
    if (str_for_history == NULL) {
        printf("Error allocating memory\n");
        exit(1);
    }

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