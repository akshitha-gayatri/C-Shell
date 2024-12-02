#ifndef LOG_H
#define LOG_H

#define MAX_COMMANDS 15
#define MAX_COMMAND_LENGTH 4096



void init_log();
void add_to_log(const char *command);
void print_log();
void purge_log();
void execute_log_command(int index);
void execute_log_command(int index);
int count_commands_in_file();
void init_log_filename();
void log_execut(char *args[4096],int arg_count);

#endif
