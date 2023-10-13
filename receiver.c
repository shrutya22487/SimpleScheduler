#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

int fd;

void read_pipe() {
    char message[100];
    // Read the message from the FIFO
    read(fd, message, sizeof(message));
    printf("Received message: %s\n", message);
}


void signal_handler(int signum) { 
    if (signum == SIGUSR1) {
        printf("\ncaught sigusr1\n");
        read_pipe();    
        exit(0);
    }
}

void setup_signal_handler() {
    struct sigaction sh;
    sh.sa_handler = signal_handler;
    if (sigaction(SIGUSR1, &sh, NULL) != 0) {
        printf("Signal handling failed.\n");
        exit(1);
    }
    sigaction(SIGUSR1, &sh, NULL);
}

int main() {
    setup_signal_handler();
    printf("receiver started\n");

    mkfifo("fifo_pipe", 0666); // Create a named pipe (FIFO)

    fd = open("fifo_pipe", O_RDONLY); // Open the pipe for reading
    read_pipe(fd);

    close(fd);

    unlink("fifo_pipe"); // Remove the FIFO file from the filesystem

    return 0;
}
