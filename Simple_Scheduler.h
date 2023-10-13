#ifndef SIMPLE_SCHEDULER_H
#define SIMPLE_SCHEDULER_H

void queue_command(char** command);
void simple_scheduler(int ncpu , int tslice);
void change_RR_flag();

#endif 