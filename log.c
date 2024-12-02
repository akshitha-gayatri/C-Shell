#include "initialise.h"

char LOG_FILE[10000];

void init_log()
{
    FILE *file = fopen(LOG_FILE, "a"); // Open in append mode to create the file if it doesn't exist
    if (!file)
    {
        perror(RED "Error initializing log file" RESET);
        exit(EXIT_FAILURE);
    }
    fclose(file);
}

void init_log_filename()
{
    snprintf(LOG_FILE, sizeof(LOG_FILE), "%s/command.txt", shell_dir);
}

int contains_command(const char *command)
{
    FILE *file = fopen(LOG_FILE, "r");
    if (!file)
    {
        fprintf(stderr, RED "Error opening log file: %s" RESET "\n", strerror(errno));
        return 0;
    }

    char line[MAX_COMMAND_LENGTH];
    char most_recent_command[MAX_COMMAND_LENGTH] = {0};

    // to get the most recent command
    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\n\t")] = '\0';
        strncpy(most_recent_command, line, MAX_COMMAND_LENGTH - 1);
        most_recent_command[MAX_COMMAND_LENGTH - 1] = '\0';
    }

    fclose(file);

    return (strcmp(most_recent_command, command) == 0);
}

int count_commands()
{
    FILE *file = fopen(LOG_FILE, "r");
    if (!file)
    {
        fprintf(stderr, RED "Error opening log file: %s" RESET "\n", strerror(errno));
        return 0;
    }

    int count = 0;
    char line[MAX_COMMAND_LENGTH];
    while (fgets(line, sizeof(line), file))
        count++;

    fclose(file);
    return count;
}

void add_to_log(const char *command)
{
    if (strstr(command, "log") != NULL || contains_command(command))
        return;

    int num_commands = count_commands();

    if (num_commands >= MAX_COMMANDS)
    {
        FILE *temp_file = fopen("temp.txt", "w");
        FILE *log_file = fopen(LOG_FILE, "r");

        if (!temp_file || !log_file)
        {
            perror(RED "Error opening file" RESET);
            exit(EXIT_FAILURE);
        }

        char line[MAX_COMMAND_LENGTH];
        fgets(line, sizeof(line), log_file); // Skip the oldest command

        while (fgets(line, sizeof(line), log_file))
            fputs(line, temp_file);

        fprintf(temp_file, "%s\n", command);

        fclose(log_file);
        fclose(temp_file);

        remove(LOG_FILE);
        rename("temp.txt", LOG_FILE);
    }

    else
    {
        FILE *log_file = fopen(LOG_FILE, "a");
        if (!log_file)
        {
            perror(RED "Error opening log file" RESET);
            exit(EXIT_FAILURE);
        }
        fprintf(log_file, "%s\n", command);
        fclose(log_file);
    }
}

void print_log()
{
    FILE *file = fopen(LOG_FILE, "r");
    if (!file)
    {
        printf(RED "Log is empty or cannot be opened." RESET "\n");
        return;
    }

    fseek(file, 0, SEEK_END);

    // Variables to store lines
    long file_size = ftell(file);
    char *line = NULL;
    size_t line_len = 0;
    // ssize_t read_len;

    // Array of strings to store lines
    char **lines = NULL;
    size_t line_count = 0;

    // Reading the file backwards
    for (long i = file_size - 1; i >= 0; i--)
    {
        fseek(file, i, SEEK_SET);
        int ch = fgetc(file);

        if (ch == '\n' || i == 0)
        {
            if (i == 0)
            {
                fseek(file, i, SEEK_SET); // Move to the start for the last line
            }

            getline(&line, &line_len, file); // Read the line

            // Store the line
            lines = realloc(lines, sizeof(char *) * (line_count + 1));
            lines[line_count] = malloc(strlen(line) + 1);
            strcpy(lines[line_count], line);
            line_count++;
        }
    }

    // Print lines in reverse order
    for (long i = line_count - 1; i > 0; i--)
    {
        printf("%ld %s", line_count - i, lines[i]);
        free(lines[i]); // Free the allocated memory for each line
    }

    free(lines); // Free the array of pointers
    free(line);  // Free the buffer used by getline
}

void purge_log()
{
    FILE *file = fopen(LOG_FILE, "w");
    if (!file)
    {
        printf(RED "Error: Could not open log file for purging." RESET "\n");
        return;
    }
    fclose(file);
}

void execute_log_command(int index)
{
    FILE *file = fopen(LOG_FILE, "r");
    if (!file)
    {
        printf(RED "Log is empty or cannot be opened." RESET "\n");
        return;
    }

    int num_cmd = count_commands();

    // Validate the index
    if (index < 1 || index > num_cmd)
    {
        printf(RED "Invalid index. Please provide an index between 1 and %d." RESET "\n", num_cmd);
        fclose(file);
        return;
    }

    char line[MAX_COMMAND_LENGTH];
    int current_index = 0;

    // Loop to find the required command
    while (fgets(line, sizeof(line), file))
    {
        current_index++;

        size_t len = strcspn(line, "\r\n"); // To remove '\r' or '\n'
        line[len] = '\0';                   // Replace the end-of-line character with null terminator

        if (current_index == num_cmd - index + 1)
        {
            // printf("Executing command %d: %s", current_index, line); // debug statement for checking command
            if (index > 1)
                add_to_log(line);
            if (strstr(line, ">") != NULL || strstr(line, "<") != NULL || strstr(line, "|") != NULL)
            {
                execute_commands_2(line);
            }
            else
                execute_commands(line);
            fclose(file);
            return;
        }
    }

    printf(RED "Command not found in the log." RESET "\n");
    fclose(file);
}

void log_execut(char *args[4096], int arg_count)
{
    init_log();
    if (arg_count == 1 && strcmp(args[0], "log") == 0)
        print_log();
    else if (arg_count == 2 && strcmp(args[1], "purge") == 0)
        purge_log();
    else if (arg_count == 3 && strcmp(args[1], "execute") == 0)
    {
        int index = atoi(args[2]);
        execute_log_command(index);
    }
    else
        printf(RED "Invalid log command" RESET "\n");
}