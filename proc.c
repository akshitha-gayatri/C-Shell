#include "initialise.h"

void print_process_info(pid_t pid)
{
    char path[4096];
    char status[256];
    char exe_path[6000];
    FILE *file;
    int status_value;
    int tty_nr, shell_tty_nr;
    char proc_state;
    pid_t session_id, tty_pgrp;

    // Path to the process stat file
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    // Open the process stat file
    file = fopen(path, "r");
    if (!file)
    {
        fprintf(stderr, RED "Process with pid %d not found\n" RESET, pid);
        return;
    }

    // Read fields from the /proc/[pid]/stat file, especially tty_nr and proc state
    fscanf(file, "%*d %*s %c %*d %*d %d %d %d", &proc_state, &session_id, &tty_nr, &tty_pgrp);
    fclose(file);

    // Get the tty_nr of the custom shell (shell_pid)
    snprintf(path, sizeof(path), "/proc/%d/stat", shell_pid);
    file = fopen(path, "r");
    if (!file)
    {
        fprintf(stderr, RED "Shell with pid %d not found\n" RESET, shell_pid);
        return;
    }

    // Read tty_nr of the shell
    fscanf(file, "%*d %*s %*c %*d %*d %*d %d", &shell_tty_nr);
    fclose(file);
    int found = 0;
    // Determine whether process is in the foreground or background
    if (tty_nr != 0 && tty_nr == shell_tty_nr)
    {
        // Foreground process: append '+' to the state
        for (int i = 0; i < bg_process_count; i++)
        {
            if (pid == bg_processes[i].pid)
            {
                found = 1;
                break;
            }
        }
        if (!found)
            printf("Process Status : %c+\n", proc_state);
        else
            printf("Process Status : %c\n", proc_state);
    }
    else
    {
        printf("Process Status : %c\n", proc_state);
    }

    // Additional details: pid, process group, etc.
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    file = fopen(path, "r");
    if (!file)
    {
        fprintf(stderr, RED "Process with pid %d not found\n" RESET, pid);
        return;
    }

    while (fgets(status, sizeof(status), file))
    {
        if (strncmp(status, "Pid:", 4) == 0)
        {
            printf("pid : %s", status + 5);
        }
        if (strncmp(status, "Tgid:", 5) == 0)
        {
            printf("Process Group : %s", status + 6);
        }
    }
    fclose(file);

    // Virtual memory usage
    snprintf(path, sizeof(path), "/proc/%d/statm", pid);
    file = fopen(path, "r");
    if (!file)
    {
        fprintf(stderr, RED "Process with pid %d not found\n" RESET, pid);
        return;
    }

    if (fscanf(file, "%*s %d", &status_value) == 1)
    {
        printf("Virtual memory : %d\n", status_value * getpagesize());
    }
    fclose(file);

    // Get executable path
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);
    if (readlink(path, exe_path, sizeof(exe_path) - 1) != -1)
    {
        exe_path[6000 - 1] = '\0';
        printf("Executable path : %s\n", exe_path);
    }
    else
    {
        perror("readlink");
    }
}

void proc_execut(int argc, char *argv[])
{
    pid_t pid;
    if (argc == 1)
        pid = getpid(); 
    else if (argc == 2)
        pid = atoi(argv[1]); // Convert argument to PID
    else
    {
        fprintf(stderr, RED "Invalid number of arguments\n" RESET);
        return;
    }

    print_process_info(pid);
}
