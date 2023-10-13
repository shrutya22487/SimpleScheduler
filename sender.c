#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>


int main() {
    printf("_______\n");

    mkfifo("fifo_pipe", 0666); // Create a named pipe (FIFO)

    char *p = "./receiver";
    char *args[] = {"./receiver", NULL};

    int pid = fork();

    if (pid < 0) {
        printf("Forking child failed.\n");
        exit(1);
    } else if (pid == 0) {
        // Child process (receiver)
        execvp(p, args);
        printf("Command failed.\n");
        exit(1);
    } else {
        // Parent process (sender)
        sleep(1);
        int fd = open("fifo_pipe", O_WRONLY); // Open the pipe for writing
        if (fd == -1) {
            perror("Error opening the FIFO");
            exit(1);
        }

        // Send the message "submit ./a.out to the receiver" through the FIFO
        char *message = "submit ./a.out to the receiver";
        write(fd, message, strlen(message) + 1);
        close(fd);

        printf("Message sent through FIFO.\n");

    }

    return 0;
}
