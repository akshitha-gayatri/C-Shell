#include "initialise.h"

const char *home_dir;
int foreground_pid = -1;
int is_directory_change;
pid_t shell_pid;
pid_t shell_pgid;
struct termios shell_tmodes;

int main()
{
    load_myshrc("myshrc");
    

    shell_pid = getpid();
    shell_pgid = getpgrp();
    tcgetattr(STDIN_FILENO, &shell_tmodes); // Save terminal modes for shell

    struct sigaction sa_chld, sa_tstp;
    sa_chld.sa_handler = sigchld_handler;
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&sa_chld.sa_mask);
    sigaction(SIGCHLD, &sa_chld, NULL);
    
    signal(SIGINT, CtrlC);

    // init_signal_handling();
    if (getcwd(shell_dir, sizeof(shell_dir)) == NULL)
    {
        perror(RED "Error getting current working directory" RESET);
    }

    init_log_filename();
    init_log();

    is_directory_change = 0;

    char input[4096];
    print_prompt();
    while (1)
    {
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            if (feof(stdin))
            {
                logout(shell_pid);
                printf("\nExiting shell\n");
                break;
            }
            continue;
        }

        input[strcspn(input, "\n")] = '\0'; // Remove trailing newline

        if (strlen(input) == 0 || strspn(input, " \t") == strlen(input))
        {
            print_prompt();
            continue; // Skip empty or whitespace-only input
        }
        add_to_log(input);
        if (strncmp(input, "exit",4) == 0)
        {
            add_to_log(input);
            logout(shell_pid);
        }
        else if (strstr(input, ">") != NULL || strstr(input, "<") != NULL || strstr(input, "|") != NULL)
        {
            execute_commands_2(input);
        }
        else
            process_input(input);

        if (last_foreground_duration > -1 && fg_process_count != 0)
        {
            print_prompt_in_fg();
        }
        else
        {
            print_prompt();
        }
    }

    deletelist(&dir_list);
    return 0;
}
