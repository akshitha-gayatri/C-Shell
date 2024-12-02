#ifndef ACT_H
#define ACT_H

typedef struct
{
    pid_t pid;
    char name[4096];
} Process;

const char *get_process_state(pid_t pid);
void list_processes(Process processes[], int count);

#endif