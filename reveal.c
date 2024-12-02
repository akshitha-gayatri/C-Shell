#include "initialise.h"
#include "reveal.h"
#include "list.h"

char shell_dir[4096];

char *get_parent_directory(const char *current_dir)
{
    static char parent_dir[4096];
    strcpy(parent_dir, current_dir);

    // Find the last slash
    char *last_slash = strrchr(parent_dir, '/');
    if (last_slash == NULL)
    {
        // No slash found, current_dir is the root directory
        strcpy(parent_dir, ".");
        return parent_dir;
    }

    // If the last slash is the only character or the directory is "/"
    if (last_slash == parent_dir)
    {
        // This is the root directory
        strcpy(parent_dir, "/");
        return parent_dir;
    }

    // Terminate the string before the last slash
    *(last_slash) = '\0';
    return parent_dir;
}

char *find_path(const char *input_path)
{
    static char path[4096];
    if (strcmp(input_path, ".") == 0)
        return getcwd(path, sizeof(path));
    else if (strcmp(input_path, "..") == 0)
    {
        char cwd[4096];
        if (getcwd(cwd, sizeof(cwd)) == NULL)
        {
            printf(RED "getcwd error" RESET "\n");
            return NULL;
        }

        char *parent = get_parent_directory(cwd);
        if (parent == NULL)
        {
            printf(RED "getcwd error" RESET "\n");
            return NULL;
        }
        return realpath(parent, path);
    }
    else if (strncmp(input_path, "~", 1) == 0)
    {
        if (strlen(input_path) == 1)
        {
            // printf("char **** path is %s\n", path);
            return realpath(shell_dir, path);
        }
        else
        {

            snprintf(path, sizeof(path), "%s%s", shell_dir, input_path + 1);
            // printf("char * path is %s\n", path);
            return path;
        }
    }
    else if (strcmp(input_path, "-") == 0)
    {
        // printf("input path is : %s\n", input_path);
        if (dir_list != NULL && dir_list->next != NULL && is_directory_change == 1)
        {
            return realpath(dir_list->next->data, path);
        }

        else
        {
            printf(RED "Error: no previous directory" RESET "\n");
            return NULL;
        }
    }

    else
    {
        char *pat = (char *)input_path;
        // printf("char *455 path is %s\n", pat);
        return pat;
    }
}

void print_file_details(const char *full_path, const char *name)
{
    struct stat st;
    if (stat(full_path, &st) == -1)
    {
        printf(RED "stat error for %s" RESET "\n", full_path);
        return;
    }

    char permissions[11];
    snprintf(permissions, sizeof(permissions), "%s%s%s",
             (S_ISDIR(st.st_mode)) ? "d" : "-",
             (st.st_mode & S_IRUSR) ? "r" : "-",
             (st.st_mode & S_IWUSR) ? "w" : "-");
    snprintf(permissions + 3, sizeof(permissions) - 3, "%s%s%s",
             (st.st_mode & S_IXUSR) ? "x" : "-",
             (st.st_mode & S_IRGRP) ? "r" : "-",
             (st.st_mode & S_IWGRP) ? "w" : "-");
    snprintf(permissions + 6, sizeof(permissions) - 6, "%s%s%s",
             (st.st_mode & S_IXGRP) ? "x" : "-",
             (st.st_mode & S_IROTH) ? "r" : "-",
             (st.st_mode & S_IWOTH) ? "w" : "-");
    snprintf(permissions + 9, sizeof(permissions) - 9, "%s",
             (st.st_mode & S_IXOTH) ? "x" : "-");

    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);

    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%b %d %Y %H:%M", localtime(&st.st_mtime));

    // Determine color based on file type
    const char *color;
    if (S_ISDIR(st.st_mode))
        color = BLUE;
    else if (st.st_mode & S_IXUSR)
        color = GREEN;
    else
        color = WHITE;

    // Print detailed file information
    printf("%s%s  %s  %s  %ld  %s  %s" RESET "\n",
           color,
           permissions,
           pw ? pw->pw_name : "UNKNOWN",
           gr ? gr->gr_name : "UNKNOWN",
           (long)st.st_size,
           timebuf,
           name);
}

int compare_entries(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

void list_directory(const char *dir_path, int show_all, int show_details)
{
    DIR *dir = opendir(dir_path);
    if (!dir)
    {
        printf(RED "opendir error for %s" RESET "\n", dir_path);
        return;
    }

    struct dirent *entry;
    char *entries[4096]; // Array to hold directory entries
    int count = 0;
    off_t total_blocks = 0; // Total number of blocks

    // Collect entries
    while ((entry = readdir(dir)) != NULL)
    {
        if (!show_all && entry->d_name[0] == '.')
        {
            continue; // Skip hidden files if -a is not specified
        }
        if (count < (int)(sizeof(entries) / sizeof(entries[0])))
        {
            entries[count] = strdup(entry->d_name);
            if (entries[count] == NULL)
            {
                printf(RED "Memory allocation error" RESET "\n");
                closedir(dir);
                return;
            }
            count++;
        }
    }
    closedir(dir);

    // Sort entries lexicographically
    qsort(entries, count, sizeof(char *), compare_entries);

    if (show_details)
    {
        // Print total number of blocks
        for (int i = 0; i < count; i++)
        {
            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entries[i]);

            struct stat st;
            if (stat(full_path, &st) == -1)
            {
                printf(RED "stat error for %s" RESET "\n", full_path);
                continue;
            }

            total_blocks += st.st_blocks;
        }

        printf("total %lld\n", (long long)total_blocks);

        // Print details for each file/directory
        for (int i = 0; i < count; i++)
        {
            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entries[i]);

            print_file_details(full_path, entries[i]);
        }
    }
    else
    {
        // Print entries
        for (int i = 0; i < count; i++)
        {
            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entries[i]);

            struct stat st;
            if (stat(full_path, &st) == -1)
            {
                printf(RED "stat error for %s" RESET "\n", full_path);
                continue;
            }

            if (S_ISDIR(st.st_mode))
                printf(BLUE "%s/" RESET "\n", entries[i]);
            else if (st.st_mode & S_IXUSR)
                printf(GREEN "%s" RESET "\n", entries[i]);
            else
                printf(WHITE "%s" RESET "\n", entries[i]);

            free(entries[i]); // Free memory allocated by strdup
        }
    }
}

void process_reveal_command(char *flags, char *path)
{
    int show_all = 0;
    int show_details = 0;

    // Check for flags
    if (strchr(flags, 'a'))
        show_all = 1;
    if (strchr(flags, 'l'))
        show_details = 1;
    // printf("before prc path is : %s\n", path);

    path = find_path(path);
    // printf("In prc path is : %s\n", path);
    if (path == NULL)
    {
        printf(RED "Error path not found" RESET "\n");
        return;
    }

    struct stat st;
    if (stat(path, &st) == -1)
    {
        printf(RED "stat error for %s" RESET "\n", path);
        return;
    }

    if (S_ISDIR(st.st_mode))
    {
        // for directory
        list_directory(path, show_all, show_details);
    }
    else
    {
        // for file
        if (show_details)
            print_file_details(path, strrchr(path, '/') ? strrchr(path, '/') + 1 : path);
        else
        {
            // Print file name
            printf("%s\n", strrchr(path, '/') ? strrchr(path, '/') + 1 : path);
        }
    }
}

void rev_execut(char *args[4096], int arg_count)
{
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror(RED "getcwd error" RESET "\n");
        return;
    }

    int show_all = 0;
    int show_details = 0;
    char *path = cwd;   // Default to current directory if no path is specified
    char flags[3] = ""; // Buffer to hold the combined flags string

    if (arg_count == 2 && strcmp(args[1], "-") == 0)
    {
        process_reveal_command("l", "-");
    }
    else
    {
        for (int i = 1; i < arg_count; i++)
        {

            if (strchr(args[i], 'a') && strchr(args[i], '-'))
                show_all = 1;
            if (strchr(args[i], 'l') && strchr(args[i], '-'))
                show_details = 1;
            else
            {
                if (strncmp(args[i], "-", 1) != 0)
                    path = args[i];
            }
        }

        if (show_all)
            strcat(flags, "a");
        if (show_details)
            strcat(flags, "l");

        path = find_path(path);
        if (path == NULL)
        {
            printf(RED "Error resolving path" RESET "\n");
            return;
        }
        // printf("paaaath is  :%s\n", path);
        process_reveal_command(flags, path);
    }
}
