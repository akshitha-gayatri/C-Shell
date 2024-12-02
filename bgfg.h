#ifndef BGFG_H
#define BGFG_H

#define MAX_ARGS 10000
#define MAX_BG_PROCESSES 10000
#define MAX_CMD_LEN 4096
#define RESET "\033[0m"
#define RED "\033[31m"


typedef struct{
    char *cmd;
} sigint_handler_data;

// void activities();
void print_prompt();
void print_prompt_in_fg();
void trim_whitespace(char *str);
void sigchld_handler();
void execute_commands(char *input);
void execute_command_fg(char *cmd, char *args[], int arg_count);
void execute_command_bg(char *cmd, char *args[], int arg_count);
void user_def(char *args[], int arg_count);
void add_to_activities(pid_t pid, const char *cmd);
void print_process_status(pid_t pid);

int compare(const void *a, const void *b);
void sigint_handler(int sig);
void sigtstp_handler(int sig);



void add_to_fg_processes(pid_t pid, const char *cmd, const char *extra_info);
void remove_fg_process(pid_t pid);
void clear_fg_processes();
void handle_signal(int sig);
void add_process(pid_t pid, const char *name);
const char *get_signal_name(int sig);
void init_signal_handling();
void update_process_status(pid_t pid, int status);
void remove_bg_process(pid_t pid);

#endif