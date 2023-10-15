#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

#define TIMER_TAG 12
int TSLICE = 1000;
struct itimerval timer_spec;

void signal_handler(int signum) {
    
    if (signum == SIGALRM) {
        printf("Caught SIGALRM\n");
        // Handle the timer expiration
    }
}

void set_round_robin_timer() {
    //signal(SIGALRM, signal_handler);

    timer_spec.it_interval.tv_sec = 0;
    timer_spec.it_interval.tv_usec = TSLICE;
    timer_spec.it_value = timer_spec.it_interval;

    if (setitimer(ITIMER_REAL, &timer_spec, NULL) == -1) {
        perror("setitimer");
        exit(1);
    }
}

int main(int argc, char const *argv[]) {

    set_round_robin_timer();

    while (1) {
        // Your main loop logic here

        // Sleep or do some work
        usleep(100000); // Sleep for 100 milliseconds
    }

    return 0;
}
