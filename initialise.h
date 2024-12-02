#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/utsname.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdbool.h>
#include <termios.h>
#include <netinet/in.h>


#include "proc.h"
#include "hop.h"
#include "log.h"
#include "list.h"
#include "reveal.h"
#include "seek.h"
#include "bgfg.h"
#include "pipe_redir.h"
#include "signal.h"
#include "iMan.h"
#include "neonate.h"
#include "bg.h"
#include "fg.h"
#include "my.h"

#define GREEN "\033[92m"
#define BLUE "\033[34m"
#define WHITE "\033[97m"
#define RESET "\033[0m"
#define RED "\033[31m"
#define YELLOW "\033[1;36m"
#define PURPLE "\033[1;35m"

typedef struct
{
    pid_t pid;
    char cmd[MAX_CMD_LEN];
    int status;
} BackgroundProcess;

typedef struct
{
    pid_t pid;
    char cmddata[4096];
    char data[4096]; // Store extra info like duration or other details
} ForegroundProcess;

typedef struct
{
    pid_t pid;
    char name[4096];
} Process;

const char *get_process_state(pid_t pid);
void list_processes(Process processes[], int count);

extern volatile sig_atomic_t Status[4096];
extern Process processes[4096];

extern int process_count;
extern ForegroundProcess fg_processes[4096];
extern BackgroundProcess bg_processes[MAX_BG_PROCESSES];
extern int is_directory_change;
extern char current_dir[4096];
extern char username[4096];
extern char hostname[4096];
extern char shell_dir[4096];
extern int last_foreground_duration;
extern int foreground_pid;
extern int bg_process_count;
extern int fg_process_count;


extern pid_t shell_pid;     
extern pid_t shell_pgid;          
extern struct termios shell_tmodes;      
extern int shell_terminal;               

// #define PATH_MAX 5000
#endif