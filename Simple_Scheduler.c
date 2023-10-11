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
#include "Simple_Scheduler.h"

char** new_command;

int remove_submit(char** command){
    int i = 0 , j = 0;
    while (command[i] != NULL)
    {
        i++;
    }
    int priority = 1;
    new_command = (char**)malloc(sizeof(char*));
    new_command[0] = command[1];
    if (i == 3)
    {
        priority = atoi(command[2]);
    }
           
    return priority;
}


