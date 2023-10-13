#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
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
    mkfifo(fifoName, 0666);
    int fifo_fd = open(fifoName, O_WRONLY);
    const char* command = "submit ./a.out";
    write(fifo_fd, command, strlen(command) + 1);
    close(fifo_fd);
    unlink(fifoName);
    return 0;
}
