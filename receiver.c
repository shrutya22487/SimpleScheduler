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
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

int TSLICE = 1000;

void signal_handler(int signum) { 
    if (signum == SIGINT) {
        printf("\n---------------------------------\n");
        //display_history();
        exit(0);
    }
    else if (signum == SIGALRM) {
        printf("received sigalrm\n");
        // Add code here to perform a specific action in response to the timer
        // For example, you can switch tasks in a round-robin scheduler
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

    // Configure the initial timer expiration and interval
    val.it_value.tv_sec = TSLICE / 1000;
    val.it_value.tv_usec = (TSLICE % 1000) * 1000;
    val.it_interval.tv_sec = 0;
    val.it_interval.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &val, NULL) == -1) {
        perror("setitimer");
        exit(1);
    }
}

int main(int argc, char const *argv[]) {
    set_round_robin_timer();
    usleep(1000*1000);
    set_round_robin_timer();
    while (1) {
        // Add your main program logic here
    }
    
    return 0;
}
