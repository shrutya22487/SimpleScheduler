CC = gcc
CFLAGS = -Wall -g

all: SimpleShell SimpleScheduler

SimpleShell:    SimpleShell.c
    $(CC)   $(CFLAGS)   -o  SimpleShell SimpleShell.c

SimpleScheduler: SimpleScheduler.c
    $(CC) $(CFLAGS) -o SimpleScheduler SimpleScheduler.c

clean:
    rm -f SimpleShell SimpleScheduler
