#include "initialise.h"

void handle_signal(int sig)
{
    if (foreground_pid > 0)
    {
        if (kill(foreground_pid, sig) == -1)
        {
            perror("Error sending signal");
        }
    }
}

void CtrlC(int sig)
{
    if (foreground_pid != -1)
    {
        kill(foreground_pid, SIGINT);
        foreground_pid = -1;
    }

    else
        fprintf(stderr,RED "No foreground process to interrupt\n" RESET);
}

void handle_ping(int pid, int sig_number)
{
    int signal = sig_number % 32;

    if (kill(pid, signal) == -1)
    {
        if (errno == ESRCH)
            fprintf(stderr, RED "No such process found\n" RESET);
        else
            fprintf(stderr, RED "Error sending signal\n" RESET);
        return;
    }

    printf(PURPLE "Sent signal %d to process with pid %d\n" RESET, signal, pid);
}

void init_signal_handling()
{
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
}

void logout(pid_t pid)
{
    // to send termination signals to all processes
    kill(-pid, SIGTERM);
}

void CtrlZ(int sig)
{
    if (foreground_pid > 0)
    {
        kill(foreground_pid, SIGTSTP);

        printf(PURPLE "\nProcess with PID %d stopped and moved to background.\n" RESET, foreground_pid);

        foreground_pid = -1;
    }
    else
    {
        printf(RED "\nNo foreground process running.\n" RESET);
    }
}

void resume_process(pid_t pid)
{
    // Send SIGCONT to the process to resume it
    if (kill(pid, SIGCONT) == -1)
    {
        if (errno == ESRCH)
        {
            // ESRCH means no such process
            printf("No such process found\n");
        }
        else
        {
            // Other errors
            fprintf(stderr, "Failed to send SIGCONT: %s\n", strerror(errno));
        }
    }
    else
    {
        printf("Process %d is now running in the background\n", pid);
    }
}