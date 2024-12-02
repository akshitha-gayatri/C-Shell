#ifndef SIG_H
#define SIG_H

void CtrlC(int sig);
void CtrlZ(int sig);
// void logout(pid_t pid);
void handle_ping(int pid, int sig_number);
// int is_bg(pid_t pid);


void handle_signal(int sig);
// void handle_ping(char *input);
void init_signal_handling();
void logout(pid_t pid);


void resume_process(pid_t pid);
#endif