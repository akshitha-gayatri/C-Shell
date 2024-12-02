#include "initialise.h"

int prefix_match(const char *name, const char *prefix)
{
    size_t name_len = strlen(name);
    size_t prefix_len = strlen(prefix);

    if (prefix_len > name_len)
    {
        return 0; // Prefix cannot be longer than the name
    }

    return strncmp(name, prefix, prefix_len) == 0;
}

void print_result(const char *path, const char *base_dir, int is_dir)
{
    char real_base_dir[PATH_MAX];
    if (realpath(base_dir, real_base_dir) == NULL)
    {
        printf(RED "realpath error: %s" RESET "\n", strerror(errno));
        return;
    }
    // printf("real base : %s\n",real_base_dir);
    // printf("base dir : %s\n",base_dir);
    char relative_path[PATH_MAX];

    // Calculate the relative path if the path starts with the base directory
    if (strncmp(path, base_dir, strlen(base_dir)) == 0)
    {
        snprintf(relative_path, sizeof(relative_path), "%s", path + strlen(base_dir));
    }
    else
    {
        snprintf(relative_path, sizeof(relative_path), "%s", path);
    }

    // Handle leading slash in the relative path
    if (relative_path[0] == '/')
    {
        snprintf(relative_path, sizeof(relative_path), "%s", relative_path + 1);
    }

    // Print the relative path with color coding for directories and files
    if (is_dir)
    {
        printf(BLUE "./%s" RESET "\n", relative_path);
    }
    else
    {
        printf(GREEN "./%s" RESET "\n", relative_path);
    }
}

void search_files(const char *dir_path, const char *search, int exact_match, int *file_count, char *single_match, int *error_count, const char *base_dir)
{
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char path[MAX_PATH];

    if ((dir = opendir(dir_path)) == NULL)
    {
        (*error_count)++;
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        if (stat(path, &statbuf) != 0)
        {
            (*error_count)++;
            continue;
        }

        if (S_ISREG(statbuf.st_mode)) // Check if it's a regular file
        {
            if (prefix_match(entry->d_name, search))
            {
                if (exact_match && access(path, R_OK) != 0)
                {
                    printf(RED "Missing permissions for task!" RESET "\n");
                    (*error_count)++;
                    continue;
                }

                (*file_count)++;
                strcpy(single_match, path);
                if (!exact_match) // Print file only if -e is not set
                    print_result(path, base_dir, 0);
            }
        }
        else if (S_ISDIR(statbuf.st_mode)) // Check if it's a directory
        {
            search_files(path, search, exact_match, file_count, single_match, error_count, base_dir);
        }
    }

    closedir(dir);
}

void search_directory(const char *dir_path, const char *search, int only_dirs, int only_files, int exact_match, int *dir_count, int *file_count, char *single_match, int *error_count, const char *base_dir)
{
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char path[MAX_PATH];

    if ((dir = opendir(dir_path)) == NULL)
    {
        (*error_count)++;
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        if (stat(path, &statbuf) != 0)
        {
            (*error_count)++;
            continue;
        }

        if (S_ISDIR(statbuf.st_mode))
        {
            if (!only_files && prefix_match(entry->d_name, search))
            {
                if (exact_match && access(path, X_OK) != 0)
                {
                    printf(RED "Missing permissions for task!" RESET "\n");
                    (*error_count)++;
                    continue;
                }
                (*dir_count)++;
                strcpy(single_match, path);
                if (!exact_match) // Print directory only if -e is not set
                    print_result(path, base_dir, 1);
            }

            // Recurse into subdirectories
            search_directory(path, search, only_dirs, only_files, exact_match, dir_count, file_count, single_match, error_count, base_dir);
        }
        else if (S_ISREG(statbuf.st_mode) && !only_dirs) // Check if it's a regular file
        {
            if (prefix_match(entry->d_name, search))
            {
                if (exact_match && access(path, R_OK) != 0)
                {
                    printf(RED "Missing permissions for task!" RESET "\n");
                    (*error_count)++;
                    continue;
                }
                (*file_count)++;
                strcpy(single_match, path);
                if (!exact_match) // Print file only if -e is not set
                    print_result(path, base_dir, 0);
            }
        }
    }

    closedir(dir);
}

void seek_execut(char *argv[], int argc)
{
    int only_dirs = 0, only_files = 0, exact_match = 0;
    char *search = NULL, *target_dir = "."; // Default to current directory
    char single_match[MAX_PATH] = "";
    int dir_count = 0, file_count = 0;
    int error_count = 0;

    if (handle_flags(argc, argv, &search, &target_dir, &only_dirs, &only_files, &exact_match) != 0)
    {
        return;
    }

    search_directory(target_dir, search, only_dirs, only_files, exact_match, &dir_count, &file_count, single_match, &error_count, target_dir);

    if ((dir_count == 0 && file_count == 0) || (dir_count == 0 && only_dirs == 1) || (file_count == 0 && only_files == 1))
    {
        printf(RED "No match found" RESET "\n");
        return;
    }

    if (exact_match)
    {
        if (dir_count == 1 && file_count == 0)
        {
            hop(single_match);
            printf("Directory changed to %s\n", single_match);
        }
        else if (dir_count == 0 && file_count == 1)
        {
            // Single file found, print file contents
            FILE *file = fopen(single_match, "r");
            if (file)
            {
                char buffer[MAX_PATH];
                while (fgets(buffer, sizeof(buffer), file))
                {
                    printf("%s", buffer);
                }
                fclose(file);
            }
            else
            {
                printf(RED "fopen error" RESET "\n");
            }
        }
    }
}

int handle_flags(int argc, char *argv[], char **search, char **target_dir, int *only_dirs, int *only_files, int *exact_match)
{
    int search_set = 0;

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (strcmp(argv[i], "-d") == 0)
            {
                *only_dirs = 1;
            }
            else if (strcmp(argv[i], "-f") == 0)
            {
                *only_files = 1;
            }
            else if (strcmp(argv[i], "-e") == 0)
            {
                *exact_match = 1;
            }
            else
            {
                printf(RED "Invalid flags!" RESET "\n");
                return 1;
            }
        }
        else
        {
            if (!search_set)
            {
                *search = argv[i];
                search_set = 1;
            }
            else if (*target_dir == NULL || strcmp(*target_dir, ".") == 0)
            {
                *target_dir = find_path(argv[argc - 1]);
            }
            else
            {
                printf(RED "Too many arguments!" RESET "\n");
                return 1;
            }
        }
    }

    if (*only_dirs && *only_files)
    {
        printf(RED "Invalid flags: cannot use -d and -f together!" RESET "\n");
        return 1;
    }

    if (*search == NULL)
    {
        printf(RED "Search term not provided!" RESET "\n");
        return 1;
    }

    return 0;
}




