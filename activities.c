#include "initialise.h"

const char *get_process_state(pid_t pid)
{
    char path[256];
    char buf[1024];
    FILE *status_file;
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    status_file = fopen(path, "r");
    if (status_file == NULL)
    {
        return NULL; // Error in opening the file, default to "Stopped null"
    }
    while (fgets(buf, sizeof(buf), status_file))
    {
        if (strncmp(buf, "State:", 6) == 0)
        {
            if (strstr(buf, "T") != NULL)
            {
                return "Stopped";
            }
            else
            {
                return "Running";
            }
        }
    }
    fclose(status_file);
    return "Unknown";
}

void list_processes(Process processes[], int count)
{
    for (int i = 0; i < count; i++)
    {
        const char *state = get_process_state(processes[i].pid);
        if(state != NULL)
            printf("[%d] : %s - %s\n", processes[i].pid, processes[i].name, state);
    }
}
