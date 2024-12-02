#ifndef PROC_H
#define PROC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 4096
#define MAX_TOKENS 64
void print_process_info(pid_t pid);
void proc_execut(int argc, char *argv[]);

#endif