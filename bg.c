#include "initialise.h"

int get_process_index_by_pid(pid_t pid)
{
    for (int i = 0; i < process_count; i++)
    {
        if (processes[i].pid == pid)
        {
            return i;
        }
    }
    return -1;
}

void bg_command(pid_t pid)
{
    int process_index = get_process_index_by_pid(pid);

    if (process_index == -1)
    {
        printf(RED "No such process found\n" RESET);
        return;
    }
    const char *state = get_process_state(processes[process_index].pid);
    // printf("status : %s\n",state);
    if (strcmp(state, "Stopped") != 0)
    {
        printf(RED "Process [%d] is not in a stopped state\n" RESET, pid);
        return;
    }

    if (kill(pid, SIGCONT) == 0)
    {
        printf(PURPLE "Process [%d] %s resumed in the background\n" RESET, pid, processes[process_index].name);
    }
    else
    {
        fprintf(stderr,RED "Failed to resume process [%d]: %s\n" RESET, pid, strerror(errno));
    }
}
