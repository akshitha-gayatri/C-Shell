#include "initialise.h"

#define BUFFER_SIZE 10000
#define READ_END 0
#define WRITE_END 1

void tokenize_command(char *command, char **args, int *arg_count)
{

    *arg_count = 0;
    char *token = command;
    int in_token = 0;

    for (int i = 0; token[i] != '\0'; i++)
    {
        if (token[i] == ' ' || token[i] == '\t')
        {
            if (in_token)
            {
                token[i] = '\0'; // End of the token
                in_token = 0;
            }
        }
        else
        {
            if (!in_token)
            {
                args[*arg_count] = &token[i]; // Start of a new token
                (*arg_count)++;
                in_token = 1;
            }
        }
    }
    args[*arg_count] = NULL;
}

int has_invalid_pipes(char *input)
{

    char *buffer = strdup(input);
    if (buffer == NULL)
    {
        perror("strdup");
        return -1;
    }

    char *token, *saveptr;
    int prev_was_pipe = 0;

    token = strtok_r(buffer, "|", &saveptr);
    while (token != NULL)
    {

        token = trim_whitespace2(token);

        // Check for invalid pipe usage
        if (strlen(token) == 0)
        {
            if (prev_was_pipe)
            {
                fprintf(stderr, RED "Invalid use of pipe\n" RESET);
                return -1; // Error due to invalid pipe usage
            }
        }

        prev_was_pipe = 0;
        token = strtok_r(NULL, "|", &saveptr);
        if (token != NULL)
        {
            prev_was_pipe = 1;
        }
    }

    if (prev_was_pipe)
    {
        fprintf(stderr, RED "Invalid use of pipe\n" RESET);
        return -1;
    }

    return 0; // No errors found
}

char *trim_whitespace2(char *str)
{
    char *end;

    // Trim leading space
    while (*str == ' ' || *str == '\t')
        str++;

    // Trim trailing space
    if (*str == 0)
        return str;

    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t'))
        end--;

    *(end + 1) = '\0';

    return str;
}

// Function to tokenize the command by pipe
int split_by_pipe(char *input, char *commands[])
{
    char *cmd;
    char *saveptr;
    int count = 0;

    cmd = strtok_r(input, "|", &saveptr);
    while (cmd != NULL)
    {
        commands[count++] = strdup(trim_whitespace2(cmd)); // Trim and store command
        cmd = strtok_r(NULL, "|", &saveptr);
    }
    commands[count] = NULL;
    return count;
}

// Function to handle completed background processes
void check_background_processes()
{
    for (int i = 0; i < bg_process_count; i++)
    {
        int status;
        pid_t pid = waitpid(bg_processes[i].pid, &status, WNOHANG);
        if (pid > 0)
        { // If the process has completed
            if (WIFEXITED(status))
            {
                printf("Background process with PID %d exited normally with status %d\n", pid, WEXITSTATUS(status));
            }
            else if (WIFSIGNALED(status))
            {
                printf("Background process with PID %d was terminated by signal %d\n", pid, WTERMSIG(status));
            }
            // Remove the process from the list
            for (int j = i; j < bg_process_count - 1; j++)
            {
                bg_processes[j] = bg_processes[j + 1];
            }
            bg_process_count--; // Decrease count
            i--;                // Adjust index since we removed an element
        }
    }
}

void execute_command_2(char *input, int is_background)
{
    int saved_stdout = dup(STDOUT_FILENO);
    char *commands[BUFFER_SIZE];
    int num_commands = split_by_pipe(input, commands);

    int pipe_fd[2], input_fd = -1, output_fd = -1;
    pid_t pid;

    for (int i = 0; i < num_commands; i++)
    {
        // Tokenize the current command and handle redirection
        char *args[BUFFER_SIZE];
        int arg_count = 0;
        char *input_file = NULL, *output_file = NULL;
        int append_mode = 0;

        tokenize_command(commands[i], args, &arg_count);

        // Detect input/output redirection
        for (int j = 0; j < arg_count; j++)
        {
            if (strcmp(args[j], "<") == 0 && j + 1 < arg_count)
            {
                input_file = args[j + 1];
                args[j] = NULL; // Remove redirection operator and file
            }
            else if (strcmp(args[j], ">") == 0 && j + 1 < arg_count)
            {
                output_file = args[j + 1];
                append_mode = 0;
                args[j] = NULL;
            }
            else if (strcmp(args[j], ">>") == 0 && j + 1 < arg_count)
            {
                output_file = args[j + 1];
                append_mode = 1;
                args[j] = NULL;
            }
        }

        // Create pipe if not the last command
        if (i < num_commands - 1)
        {
            if (pipe(pipe_fd) < 0)
            {
                perror("Pipe failed");
                exit(EXIT_FAILURE);
            }
        }

        // Fork the process
        pid = fork();
        if (pid < 0)
        {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) // Child process
        {
            // Input redirection (from file or pipe)
            if (i == 0 && input_file) // First command with input redirection
            {
                input_fd = open(input_file, O_RDONLY);
                if (input_fd < 0)
                {
                    fprintf(stderr, RED "No such input file found!\n" RESET);
                    exit(EXIT_FAILURE);
                }
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }
            else if (i > 0) // Middle or last command in pipeline
            {
                dup2(input_fd, STDIN_FILENO); // Read from previous pipe
                close(input_fd);
            }

            // Output redirection (to file or pipe)
            if (output_file) // Last command with output redirection
            {
                if (append_mode)
                    output_fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                else
                    output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                if (output_fd < 0)
                {
                    perror("Error opening output file");
                    exit(EXIT_FAILURE);
                }
                dup2(output_fd, STDOUT_FILENO);
                close(output_fd);
            }
            else if (i < num_commands - 1) // Middle or first command in pipeline
            {
                dup2(pipe_fd[WRITE_END], STDOUT_FILENO); // Write to pipe
                close(pipe_fd[WRITE_END]);
                close(pipe_fd[READ_END]);
            }

            if (strcmp("hop", args[0]) == 0 || strcmp("log", args[0]) == 0 || strcmp("reveal", args[0]) == 0 || strcmp("seek", args[0]) == 0 || strcmp("proclore", args[0]) == 0 || strcmp("ping", args[0]) == 0 || strcmp("iMan", args[0]) == 0 || strcmp("neonate", args[0]) == 0 || strcmp("activities", args[0]) == 0 || strcmp(args[0], "bg") == 0 || strcmp(args[0], "fg") == 0)
                user_def(args, arg_count - 2);
            else
            {
                
                execvp(args[0], args);
                perror("execvp failed");
                exit(EXIT_FAILURE);
            }
        }
        else 
        {
            if (i < num_commands - 1)
            {
                close(pipe_fd[WRITE_END]);
                input_fd = pipe_fd[READ_END]; 
            }

            if (i == num_commands - 1)
            {
                if (is_background)
                {
                    printf("\n\n%d\n\n", pid);

                    if (bg_process_count < MAX_BG_PROCESSES)
                    {
                        bg_processes[bg_process_count].pid = pid;
                        strncpy(bg_processes[bg_process_count].cmd, args[0], sizeof(bg_processes[bg_process_count].cmd) - 1);
                        bg_processes[bg_process_count].cmd[sizeof(bg_processes[bg_process_count].cmd) - 1] = '\0';
                        bg_process_count++;
                    }
                }
                else
                {
                    // Foreground process: wait for it to finish
                    waitpid(pid, NULL, 0);
                }
            }
            else
            {
                waitpid(pid, NULL, 0);
            }
        }
    }
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);

    // Check for any completed background processes
    check_background_processes();
}

void execute_commands_2(char *input)
{
    if (has_invalid_pipes(input) == -1)
        return;
    char *command, *command_ptr;
    char *commands = strdup(input); // Duplicate the input string
    if (commands == NULL)
    {
        fprintf(stderr, "strdup failed\n");
        return;
    }

    // Tokenize the input by semicolons first
    command = strtok_r(commands, ";", &command_ptr);
    while (command != NULL)
    {
        trim_whitespace2(command);

        int length = strlen(command);
        char subcommand[4096];
        int sub_idx = 0;
        int is_background = 0;

        for (int i = 0; i <= length; i++)
        {
            is_background = 0;
            if (command[i] == '&' || command[i] == '\0')
            {
                // End of subcommand or end of the full command
                subcommand[sub_idx] = '\0'; // Null-terminate the subcommand
                trim_whitespace2(subcommand);
                if (command[i] == '&')
                {
                    is_background = 1;
                }
                if (strlen(subcommand) > 0)
                {
                    // printf("bg == %d\n", is_background);
                    execute_command_2(subcommand, is_background);
                }

                sub_idx = 0; // Reset subcommand index for the next subcommand
            }
            else
            {
                subcommand[sub_idx++] = command[i]; // Build the subcommand character by character
            }
        }

        command = strtok_r(NULL, ";", &command_ptr);
    }

    free(commands);
}