#ifndef PREDIR_H
#define PREDIR_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

void execute_commands_2(char *input);
void execute_command_2(char *input,int bg);
int has_invalid_pipes(char *input);
int split_by_pipe(char *input, char *commands[]);
int split_by_semicolon(char *input, char *commands[]);
char *trim_whitespace2(char *str);
void tokenize_command(char *command, char **args, int *arg_count);



#endif