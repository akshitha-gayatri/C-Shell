#include "initialise.h"

char username[4096];
char hostname[4096];
char current_dir[4096] = "";
char prev_dir[4096] = "";
#define PROC_STATUS_PATH "/proc/%d/status"
#define MAX_FG_PROCESSES 4096 // Define maximum number of foreground processes
#define MAX_PROCESSES 4096

Process processes[4096];
int process_count = 0;

BackgroundProcess bg_processes[MAX_BG_PROCESSES];
int bg_process_count = 0;
int fg_process_count = 0;
int last_foreground_duration = -1;          // Duration in seconds of the last foreground process
char last_foreground_cmd[MAX_CMD_LEN] = ""; // Last foreground command

struct Node *dir_list = NULL;

volatile sig_atomic_t stopped = 0;

ForegroundProcess fg_processes[MAX_FG_PROCESSES]; // Array for foreground processes

int total_fg_time = 0;

void sigint_handler(int sig)
{
    stopped = 1;
}

void sigtstp_handler(int sig)
{
    stopped = 1;
    printf("\nProcess stopped (SIGTSTP)\n");
    fflush(stdout);
}

const char *get_signal_name(int sig)
{
    switch (sig)
    {
    case SIGINT:
        return "SIGINT";
    case SIGTERM:
        return "SIGTERM";
    case SIGKILL:
        return "SIGKILL";
    case SIGSTOP:
        return "SIGSTOP";
    // Add more signals as needed
    default:
        return "UNKNOWN";
    }
}

// Add a new process to the list
void add_process(pid_t pid, const char *name)
{
    if (process_count < MAX_PROCESSES)
    {
        processes[process_count].pid = pid;
        snprintf(processes[process_count].name, sizeof(processes[process_count].name), "%s", name);
        // processes[process_count].status = 0;
        process_count++;
    }
}

void remove_process(pid_t pid)
{

    for (int i = 0; i < process_count; i++)
    {
        if (processes[i].pid == pid)
        {
            for (int j = i; j < process_count - 1; j++)
            {
                processes[j] = processes[j + 1];
            }

            process_count--; // Reduce the process count
            // printf("Process with PID %d removed from activities.\n", pid);
            break;
        }
    }
}

void add_to_fg_processes(pid_t pid, const char *cmd, const char *extra_info)
{
    if (fg_process_count >= MAX_FG_PROCESSES)
    {
        fprintf(stderr, RED "Foreground process limit reached\n" RESET);
        return;
    }

    fg_processes[fg_process_count].pid = pid;
    strncpy(fg_processes[fg_process_count].cmddata, cmd, sizeof(fg_processes[fg_process_count].cmddata) - 1);
    fg_processes[fg_process_count].cmddata[sizeof(fg_processes[fg_process_count].cmddata) - 1] = '\0';
    strncpy(fg_processes[fg_process_count].data, extra_info, sizeof(fg_processes[fg_process_count].data) - 1);
    fg_processes[fg_process_count].data[sizeof(fg_processes[fg_process_count].data) - 1] = '\0';
    fg_process_count++;
}

void remove_fg_process(pid_t pid)
{
    for (int i = 0; i < fg_process_count; i++)
    {
        if (fg_processes[i].pid == pid)
        {
            // Shift remaining processes
            for (int j = i; j < fg_process_count - 1; j++)
            {
                fg_processes[j] = fg_processes[j + 1];
            }
            fg_process_count--;
            return;
        }
    }
}

void remove_bg_process(pid_t pid)
{
    for (int i = 0; i < bg_process_count; i++)
    {
        if (bg_processes[i].pid == pid)
        {
            // Shift remaining processes
            for (int j = i; j < bg_process_count - 1; j++)
            {
                bg_processes[j] = bg_processes[j + 1];
            }
            bg_process_count--;
            return;
        }
    }
}

void clear_fg_processes()
{
    fg_process_count = 0;
}

void print_prompt()
{
    struct utsname buffer;
    if (uname(&buffer) != 0)
    {
        fprintf(stderr, RED "Error getting system information\n" RESET);
        exit(1);
    }
    snprintf(hostname, sizeof(hostname), "%s", buffer.nodename);

    struct passwd *pw;
    pw = getpwuid(getuid());
    if (pw == NULL)
    {
        fprintf(stderr, RED "Error getting username\n" RESET);
        exit(1);
    }

    snprintf(username, sizeof(username), "%s", pw->pw_name);

    if (strncmp(current_dir, shell_dir, strlen(shell_dir)) == 0)
    {
        if (strcmp(current_dir, shell_dir) == 0)
            printf(YELLOW "<%s@%s:~>" RESET " ", username, hostname);
        else
            printf(YELLOW "<%s@%s:~%s>" RESET " ", username, hostname, current_dir + strlen(shell_dir));
    }
    else
        printf(YELLOW "<%s@%s:~%s>" RESET " ", username, hostname, current_dir);
}

void exec(pid_t pid)
{
    int idx = -1;
    for (int i = 0; i < bg_process_count; i++){
        if (pid == bg_processes[i].pid){
            idx = i;
            break;}
    }
    if (idx == -1){
        fprintf(stderr, RED "No process with pid %d found\n" RESET,pid);
        return;
    }
    char *str = strdup(bg_processes[idx].cmd);
    char num_str[16];
    sprintf(num_str, "%d", bg_processes[idx].status-6);
    strcat(str, " ");strcat(str, num_str);
    remove_bg_process(pid);
    remove_process(pid);
    execute_commands(str);
}

void print_prompt_in_fg()
{
    struct utsname buffer;
    if (uname(&buffer) != 0)
    {
        fprintf(stderr, RED "Error getting system information\n" RESET);
        exit(1);
    }
    snprintf(hostname, sizeof(hostname), "%s", buffer.nodename);

    struct passwd *pw;
    pw = getpwuid(getuid());
    if (pw == NULL)
    {
        fprintf(stderr, RED "Error getting username\n" RESET);
        exit(1);
    }

    snprintf(username, sizeof(username), "%s", pw->pw_name);

    // Determine prompt display
    if (strncmp(current_dir, shell_dir, strlen(shell_dir)) == 0)
    {
        if (strcmp(current_dir, shell_dir) == 0)
        {
            if (last_foreground_duration > -1)
            {
                printf(YELLOW "<%s@%s:~ ", username, hostname);
                for (int i = 0; i < fg_process_count; i++)
                {
                    if (atoi(fg_processes[i].data) > 2)
                        printf("%s : %s", fg_processes[i].cmddata, fg_processes[i].data);
                    if (i < fg_process_count - 1)
                        printf("; ");
                }
                printf(" %ds", total_fg_time);
                printf("> " RESET);
            }
            else
            {
                printf(YELLOW "<%s@%s:~>" RESET " ", username, hostname);
            }
        }
        else
        {
            if (last_foreground_duration > -1)
            {
                printf(YELLOW "<%s@%s:~%s ", username, hostname, current_dir + strlen(shell_dir));
                for (int i = 0; i < fg_process_count; i++)
                {
                    if (atoi(fg_processes[i].data) > 2)
                        printf("%s : %s", fg_processes[i].cmddata, fg_processes[i].data);
                    if (i < fg_process_count - 1)
                        printf("; ");
                }
                printf(" %ds", total_fg_time);
                printf("> " RESET);
            }
            else
            {
                printf(YELLOW "<%s@%s:~%s>" RESET " ", username, hostname, current_dir + strlen(shell_dir));
            }
        }
    }
    else
    {
        if (last_foreground_duration > -1)
        {
            printf(YELLOW "<%s@%s:~%s ", username, hostname, current_dir);
            for (int i = 0; i < fg_process_count; i++)
            {
                if (atoi(fg_processes[i].data) > 2)
                    printf("%s : %s", fg_processes[i].cmddata, fg_processes[i].data);
                if (i < fg_process_count - 1)
                    printf("; ");
            }
            printf(" %ds", total_fg_time);
            printf("> " RESET);
        }
        else
        {
            printf(YELLOW "<%s@%s:~%s>" RESET " ", username, hostname, current_dir);
        }
    }

    total_fg_time = 0;
    fg_process_count = 0;
    clear_fg_processes(); // Clear the list after displaying
}

void sigchld_handler()
{
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        // Update background process state
        for (int i = 0; i < bg_process_count; i++)
        {
            if (bg_processes[i].pid == pid)
            {
                if (WIFEXITED(status))
                {
                    printf("%d %s exitted normally\n", pid, bg_processes[i].cmd);
                    remove_bg_process(pid);
                    remove_process(pid);
                }

                else if (WIFSIGNALED(status))
                {
                    printf("%d %s exitted abnormally\n", pid, bg_processes[i].cmd);
                }
                else if (WIFSTOPPED(status))
                {
                    printf("Child process was stopped by SIGTSTP.\n");
                }

                break;
            }
        }

        for (int i = 0; i < process_count; i++)
        {
            if (processes[i].pid == pid)
            {
                if (WIFSTOPPED(status))
                {
                    printf("Process %d (%s) stopped due to signal %d\n", pid, processes[i].name, WSTOPSIG(status));
                }
                else if (WIFEXITED(status))
                {

                    printf("Process %d (%s) exited normally with status %d\n", pid, processes[i].name, WEXITSTATUS(status));
                    remove_process(pid);
                }
                else if (WIFSIGNALED(status))
                {
                    printf("Process %d (%s) exited abnormally due to signal %d\n", pid, processes[i].name, WTERMSIG(status));
                }

                break;
            }
        }
    }
}



void user_def(char *args[], int arg_count)
{
    if (strcmp(args[0], "hop") == 0)
    {
        insertAtStart(&dir_list, current_dir);
        hop_execut(args, arg_count);
    }
    else if (strcmp(args[0], "log") == 0)
    {
        insertAtStart(&dir_list, current_dir);
        log_execut(args, arg_count);
    }
    else if (strcmp(args[0], "reveal") == 0)
        rev_execut(args, arg_count);
    else if (strcmp(args[0], "seek") == 0)
    {
        insertAtStart(&dir_list, current_dir);
        seek_execut(args, arg_count);
    }
    else if (strcmp(args[0], "ping") == 0)
    {
        if (arg_count < 3)
        {
            fprintf(stderr, RED "Error: Invalid number of arguments for ping\n" RESET);
            return;
        }

        pid_t pid = atoi(args[1]);
        int signal = atoi(args[2]);

        handle_ping(pid, signal);
    }
    else if (strncmp("activities", args[0], 10) == 0)
    {
        // for (int i = 0; i < process_count; i++)
        //     printf("%d %d %s\n", i + 1, processes[i].pid, processes[i].name);
        list_processes(processes, process_count);
    }
    else if (strcmp(args[0], "iMan") == 0)
    {
        if (arg_count < 2)
            fprintf(stderr, RED "Error: Invalid number of arguments for iMan\n" RESET);
        else
            fetch_man_page(args[1]);
    }
    else if (strcmp(args[0], "neonate") == 0)
        neonate(arg_count, args);
    else if (strcmp(args[0], "bg") == 0)
    {
        if (arg_count != 2)
        {
            fprintf(stderr, RED "Invalid number of arguments for bg\n" RESET);
            return;
        }

        int p = atoi(args[1]);
        bg_command(p);
    }
    else if (strcmp(args[0], "fg") == 0)
    {
        if (arg_count != 2)
        {
            fprintf(stderr, RED "Invalid number of arguments for fg\n" RESET);
            return;
        }

        int p = atoi(args[1]);
        // fg_command(p);
        exec(p);
    }
    else // proclore
        proc_execut(arg_count, args);
}

void execute_command_bg(char *cmd, char *args[], int arg_count)
{

    pid_t pid;
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, RED "Fork failed\n" RESET);
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        signal(SIGINT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        if (strcmp(cmd, "exit") == 0)
        {
            printf("\n\n%d\n\n", getpid()); // Print the PID
            printf("Process exit with %d exited normally\n\n", getpid());
            exit(0);
        }
        else
        {
            if (strcmp("hop", cmd) == 0 || strcmp("log", cmd) == 0 || strcmp("reveal", cmd) == 0 || strcmp("seek", cmd) == 0 || strcmp("proclore", cmd) == 0 || strcmp("iMan", cmd) == 0)
                user_def(args, arg_count);
            else
            {

                if (execvp(cmd, args) == -1)
                {
                    fprintf(stderr, RED "'%s' is not a valid command\n" RESET,cmd);
                }
                exit(EXIT_FAILURE); // Exit if execvp fails
            }
        }
    }
    else
    {
        printf("\n\n%d\n\n", pid);
        if (bg_process_count < MAX_BG_PROCESSES)
        {
            bg_processes[bg_process_count].pid = pid;
            strncpy(bg_processes[bg_process_count].cmd, cmd, sizeof(bg_processes[bg_process_count].cmd) - 1);
            bg_processes[bg_process_count].status = atoi(args[1]);
            bg_processes[bg_process_count].cmd[sizeof(bg_processes[bg_process_count].cmd) - 1] = '\0';
            bg_process_count++;
        }
        add_process(pid, cmd);
    }
}


void execute_command_fg(char *cmd, char *args[], int arg_count)
{
    if (getcwd(current_dir, sizeof(current_dir)) == NULL)
    {
        fprintf(stderr, RED "Error: getcwd failed\n" RESET);
        return;
    }

    pid_t pid;
    int status;
    time_t start_time, end_time;

    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        signal(SIGINT, sigint_handler);
        signal(SIGTSTP, sigtstp_handler);
        if (strcmp(cmd, "exit") == 0)
        {
            exit(EXIT_SUCCESS);
        }
        else if (strcmp("hop", cmd) == 0 || strcmp("log", cmd) == 0 || strcmp("reveal", cmd) == 0 || strcmp("seek", cmd) == 0 || strcmp("proclore", cmd) == 0 || strcmp("ping", cmd) == 0 || strcmp("iMan", cmd) == 0 || strcmp("neonate", cmd) == 0 || strcmp("activities", cmd) == 0 || strcmp(cmd, "bg") == 0 || strcmp(args[0], "fg") == 0)
            user_def(args, arg_count);
        else
        {
            execvp(cmd, args);
            fprintf(stderr, RED "'%s' is not a valid command\n" RESET,cmd);
            exit(EXIT_FAILURE);
        }
    }

    else
    {
        foreground_pid = pid;
        start_time = time(NULL);
        waitpid(pid, &status, 0);
        end_time = time(NULL);
        double elapsed = difftime(end_time, start_time);
        int rounded_elapsed = (int)elapsed;
        char stpp[100];
        snprintf(stpp, sizeof(stpp), "%d", rounded_elapsed);

        if (fg_process_count < MAX_BG_PROCESSES)
        {
            fg_processes[fg_process_count].pid = pid;
            strncpy(fg_processes[fg_process_count].cmddata, cmd, sizeof(fg_processes[fg_process_count].cmddata) - 1);
            fg_processes[fg_process_count].cmddata[sizeof(fg_processes[fg_process_count].cmddata) - 1] = '\0';
            strncpy(fg_processes[fg_process_count].data, stpp, sizeof(fg_processes[fg_process_count].data) - 1);
            fg_processes[fg_process_count].data[sizeof(fg_processes[fg_process_count].data) - 1] = '\0';
            fg_process_count++;
        }
        foreground_pid = -1;

        if (elapsed > 2)
        {
            last_foreground_duration = rounded_elapsed;
            total_fg_time += rounded_elapsed;
            // printf("%d===\n", rounded_elapsed);
        }
        else
        {
            last_foreground_duration = -1;
        }
    }
}

void execute_commands(char *input)
{
    char *command, *command_ptr;
    char *commands = strdup(input); 
    if (commands == NULL)
    {
        fprintf(stderr, "strdup failed\n");
        return;
    }

    command = strtok_r(commands, ";", &command_ptr);
    while (command != NULL)
    {
        trim_whitespace2(command);

        char *tokens[MAX_ARGS];
        int token_count = 0;
        char *sub_token, *sub_token_ptr;

        // Tokenize each command by spaces and tabs
        sub_token = strtok_r(command, " \t", &sub_token_ptr);
        while (sub_token != NULL)
        {
            tokens[token_count++] = sub_token;
            sub_token = strtok_r(NULL, " \t", &sub_token_ptr);
        }

        char *args[MAX_ARGS];
        int arg_count = 0;
        int is_background = 0;

        for (int i = 0; i < token_count; i++)
        {
            if (strcmp(tokens[i], "&") == 0)
            {
                is_background = 1;
                if (arg_count > 0)
                {
                    args[arg_count] = NULL;
                    execute_command_bg(args[0], args, arg_count);
                    arg_count = 0;
                }
                is_background = 0;
            }
            else
            {
                args[arg_count++] = tokens[i];
            }
        }

        if (arg_count > 0)
        {
            args[arg_count] = NULL;
            if (is_background)
                execute_command_bg(args[0], args, arg_count);
            else
                execute_command_fg(args[0], args, arg_count);
        }

        command = strtok_r(NULL, ";", &command_ptr);
    }

    free(commands);
}
