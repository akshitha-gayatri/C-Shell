#ifndef REVEAL
#define REVEAL

#include <dirent.h>
#include <grp.h>
#include <time.h>
#include <limits.h>


#define GREEN "\033[92m"
#define BLUE "\033[34m"
#define WHITE "\033[97m"
#define RESET "\033[0m"

char *find_path(const char *input);
void print_file_details(const char *full_path, const char *name);
void list_directory(const char *dir_path, int show_all, int show_details);
void process_reveal_command(char *flags, char *path);
void rev_execut(char *args[4096],int arg_count);

#endif