#include "hop.h"
#include "initialise.h"
#include "list.h"

void hop_execut(char *args[4096], int arg_count)
{
    if (arg_count == 1)
        hop(shell_dir);
    else
    {
        for (int j = 1; j < arg_count; j++)
            hop(args[j]);
    }
}

void hop(char *path)
{
    char new_dir[4096];

    if (getcwd(current_dir, sizeof(current_dir)) == NULL)
    {
        fprintf(stderr, RED "Error: getcwd failed\n" RESET);
        return;
    }

    if (path == NULL || strcmp(path, "") == 0)
    {
        if (chdir(shell_dir) != 0)
        {
            fprintf(stderr, RED "Error: chdir to home directory failed\n" RESET);
            return;
        }
    }
    else if (strcmp(path, ".") == 0)
    {
        if (getcwd(current_dir, sizeof(current_dir)) == NULL)
        {
            fprintf(stderr, RED "Error: getcwd failed\n" RESET);
            return;
        }

        if (chdir(".") != 0)
        {
            fprintf(stderr, RED "Error: chdir to current directory failed\n" RESET);
            return;
        }
    }

    else if (strcmp(path, "..") == 0)
    {
        is_directory_change = 1;

        insertAtStart(&dir_list, current_dir);
        if (chdir("..") != 0)
        {
            fprintf(stderr, RED "Error: chdir to parent directory failed\n" RESET);
            return;
        }
    }
    else if (strcmp(path, "~") == 0)
    {
        // "~" refers to the shell directory
        if (chdir(shell_dir) != 0)
        {
            fprintf(stderr, RED "Error: chdir to home directory failed\n" RESET);
            return;
        }
    }
    else if (strcmp(path, "-") == 0)
    {
        // printf("%d===\n", is_directory_change);

        if (dir_list->next == NULL)
        {
            printf(RED "Error: No previous directory\n" RESET);
            return;
        }

        else
        {
            // printf("%s====\n", dir_list->next->data);
            if (is_directory_change == 1)
            {
                if (chdir(dir_list->next->data) != 0)
                {
                    fprintf(stderr, RED "Error: chdir to previous directory failed\n" RESET);
                    return;
                }
            }

            else
            {
                printf(RED "Error: No previous directory\n" RESET);
                return;
            }
        }
    }
    else
    {
        if (path[0] == '~')
        {
            is_directory_change = 1;
            snprintf(new_dir, sizeof(new_dir), "%s%s", shell_dir, path + 1);
            if (chdir(new_dir) != 0)
            {
                fprintf(stderr, RED "Error: chdir to home-relative path failed\n" RESET);
                return;
            }
        }
        else
        {
            // absolute path
            is_directory_change = 1;
            if (chdir(path) != 0)
            {
                fprintf(stderr, RED "Error: chdir to specified path failed\n" RESET);
                return;
            }
        }
    }

    if (getcwd(current_dir, sizeof(current_dir)) != NULL)
        printf(PURPLE "%s" RESET "\n", current_dir);

    else
        fprintf(stderr, RED "Error: getcwd failed\n" RESET);
}
