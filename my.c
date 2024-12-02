#include "initialise.h"

#define MAX_LINE_LENGTH 1024
#define MAX_ALIAS_NAME 100
#define MAX_ALIAS_CMD 1024
#define MAX_FUNCTION_NAME 100
#define MAX_FUNCTION_BODY 1024

typedef struct Alias
{
    char name[MAX_ALIAS_NAME];
    char command[MAX_ALIAS_CMD];
    struct Alias *next;
} Alias;

typedef struct Function
{
    char name[MAX_FUNCTION_NAME];
    char body[MAX_FUNCTION_BODY];
    struct Function *next;
} Function;

Alias *alias_head = NULL;
Function *function_head = NULL;

void add_alias(const char *name, const char *command)
{
    Alias *new_alias = (Alias *)malloc(sizeof(Alias));
    strncpy(new_alias->name, name, MAX_ALIAS_NAME);
    strncpy(new_alias->command, command, MAX_ALIAS_CMD);
    new_alias->next = alias_head;
    alias_head = new_alias;
}

void add_function(const char *name, const char *body)
{
    Function *new_function = (Function *)malloc(sizeof(Function));
    strncpy(new_function->name, name, MAX_FUNCTION_NAME);
    strncpy(new_function->body, body, MAX_FUNCTION_BODY);
    new_function->next = function_head;
    function_head = new_function;
}

void replace_argument(char *line, const char *arg)
{
    char buffer[MAX_FUNCTION_BODY];
    char *pos;

    while ((pos = strstr(line, "\"$1\"")) != NULL)
    {
        strncpy(buffer, line, pos - line);
        buffer[pos - line] = '\0';
        strcat(buffer, arg);
        strcat(buffer, pos + 4);
        strncpy(line, buffer, MAX_FUNCTION_BODY);
    }
}


void load_myshrc(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("fopen");
        return;
    }

    char line[MAX_LINE_LENGTH];
    int in_function = 0;
    char function_name[MAX_FUNCTION_NAME];
    char function_body[MAX_FUNCTION_BODY];
    function_body[0] = '\0'; 

    while (fgets(line, sizeof(line), file))
    {
        // Remove comments
        char *pos = strchr(line, '#');
        if (pos)
        {
            *pos = '\0';
        }

        // Remove trailing newline
        pos = strchr(line, '\n');
        if (pos)
        {
            *pos = '\0';
        }

        // Handle aliases
        if (strncmp(line, "alias ", 6) == 0)
        {
            char *alias_name = strtok(line + 6, "=");
            char *alias_command = strtok(NULL, "=");
            if (alias_name && alias_command)
            {
                add_alias(alias_name, alias_command);
            }
        }
        // Start of function
        else if (strchr(line, '(') && strchr(line, ')') && strchr(line, '{'))
        {
            in_function = 1;
            char *function_name_ptr = strtok(line, " (){");
            if (function_name_ptr)
            {
                strncpy(function_name, function_name_ptr, MAX_FUNCTION_NAME);
            }
        }
        // Inside function body
        else if (in_function)
        {
            if (strchr(line, '}'))
            {
                in_function = 0; // End of function
                add_function(function_name, function_body);
                function_body[0] = '\0'; // Reset function body for the next function
            }
            else
            {
                strncat(function_body, line, sizeof(function_body) - strlen(function_body) - 1);
                strncat(function_body, "\n", sizeof(function_body) - strlen(function_body) - 1);
            }
        }
    }

    fclose(file);
}

void process_input(char *line)
{
    if (strncmp(line, "exit",4) == 0)
    {
        exit(0); 
    }
    else
    {
        Alias *alias = alias_head;
        while (alias)
        {
            if (strcmp(line, alias->name) == 0)
            {
                printf("Executing alias: %s -> %s\n", alias->name, alias->command);
                execute_commands(alias->command);
                return;
            }
            alias = alias->next;
        }

        Function *function = function_head;
        while (function)
        {
            if (strstr(line, function->name) == line)
            {
                printf("Executing function: %s\n", function->name);
                char function_body[MAX_FUNCTION_BODY];
                strncpy(function_body, function->body, MAX_FUNCTION_BODY);

                char *arg = strchr(line, ' ');
                if (arg)
                {
                    replace_argument(function_body, arg + 1);
                }

                char *line_in_function = strtok(function_body, "\n");
                while (line_in_function)
                {
                    execute_commands(line_in_function);
                    line_in_function = strtok(NULL, "\n");
                }
                return;
            }
            function = function->next;
        }

        // If no alias or function matched, execute the command
        // printf("%s==\n",line);
        execute_commands(line);
    }
}

