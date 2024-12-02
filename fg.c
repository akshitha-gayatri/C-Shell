#include "initialise.h"
void fg_command(pid_t pid)
{
    int status;
    int found = 0;

    for (int i = 0; i < bg_process_count; i++)
    {
        if (pid == bg_processes[i].pid)
        {
            remove_bg_process(pid);
            found = 1;
            break;
        }

    }
    if (found)
    {
        if (tcsetpgrp(STDIN_FILENO, pid) == -1)
        {
            perror("tcsetpgrp");
            exit(EXIT_FAILURE);
        }
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);

            if (kill(pid, SIGCONT) == -1)
            {
                perror("SIGCONT");
                exit(EXIT_FAILURE);
            }
        waitpid(pid, &status, WUNTRACED) ;
        tcsetpgrp(STDIN_FILENO, getpgrp());
    }
    if (!found)
    {
        printf("No such process found\n");
    }
}

// #include "initialise.h"
// void fg_command(pid_t pid)
// {
//     int status;
//     int found = 0;
//     for (int i = 0; i < bg_process_count; i++)
//     {
//         if (pid == bg_processes[i].pid)
//         {
//             remove_bg_process(pid);
//             break;
//         }
//     }
//     kill(pid, SIGCONT);

//     waitpid(pid, &status, WUNTRACED);
// }