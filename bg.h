#ifndef BG_H
#define BG_H

int get_process_index_by_pid(pid_t pid);
void bg_command(pid_t pid);

#endif