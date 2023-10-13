#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
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
int main() {
    const char* fifoName = "/tmp/simplescheduler_fifo";
    int fifo_fd = open(fifoName, O_RDONLY);

    char command[256];  
    ssize_t bytes_read;

    bytes_read = read(fifo_fd, command, sizeof(command));
    close(fifo_fd);

    return 0;
}
