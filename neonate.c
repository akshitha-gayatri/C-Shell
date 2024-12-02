#include "initialise.h"

volatile sig_atomic_t keep_running = 1;

// Function to enable raw mode
void enablerawmode()
{
    struct termios tty;

    // Get current terminal attributes
    tcgetattr(STDIN_FILENO, &tty);

    // Disable canonical mode and echo
    tty.c_lflag &= ~(ICANON | ECHO);

    // Set the new attributes immediately
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

// Function to disable raw mode
void disablerawmode()
{
    struct termios tty;

    // Get current terminal attributes
    tcgetattr(STDIN_FILENO, &tty);

    // Enable canonical mode and echo
    tty.c_lflag |= (ICANON | ECHO);

    // Set the new attributes immediately
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

// Function to check for keypress in non-blocking mode
int check_keypress()
{
    char ch;
    struct termios tty;

    // Get current terminal attributes
    tcgetattr(STDIN_FILENO, &tty);

    // Set to non-blocking
    tty.c_cc[VTIME] = 0; 
    tty.c_cc[VMIN] = 0;  
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);

    int result = read(STDIN_FILENO, &ch, 1);

    // Restore the terminal settings
    tty.c_cc[VTIME] = 1; // Restore default
    tty.c_cc[VMIN] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);

    if (result > 0)
    {
        return ch;
    }

    return -1; // No input
}

int get_most_recent_pid()
{
    DIR *proc_dir;
    struct dirent *entry;
    int most_recent_pid = -1;

    proc_dir = opendir("/proc");
    if (proc_dir == NULL)
    {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(proc_dir)) != NULL)
    {
        // Check if the directory name is a number (PID)
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0]))
        {
            int pid = atoi(entry->d_name);
            if (pid > most_recent_pid)
            {
                most_recent_pid = pid;
            }
        }
    }

    closedir(proc_dir);
    return most_recent_pid;
}

void neonate(int argc, char *argv[])
{
    if (argc != 3 || strcmp(argv[1], "-n") != 0)
    {
        fprintf(stderr,RED "Usage: %s -n <time_arg>\n" RESET, argv[0]);
        return ;
    }

    int interval = atoi(argv[2]);
    if (interval < 0)
    {
        fprintf(stderr,RED "Error: <time_arg> must be a positive integer.\n" RESET);
        return ;
    }

    enablerawmode(); // Enable raw mode for terminal input

    while (keep_running)
    {
        int pid = get_most_recent_pid();
        if (pid != -1)
        {
            printf("%d\n", pid);
        }
        else
        {
            printf("No processes found\n");
        }

        sleep(interval);

        // Check for keypress
        int ch = check_keypress();
        if (ch == 'x')
        {
            keep_running = 0;
        }
    }

    disablerawmode(); // Disable raw mode and restore terminal settings
    return ;
}

