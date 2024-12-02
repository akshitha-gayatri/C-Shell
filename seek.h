#ifndef SEEK_H
#define SEEK_H

#define MAX_PATH 4096

// void print_result(const char *path, int is_dir);
// int search_directory(const char *dir_path, const char *search, int only_dirs, int only_files, int exact_match, int *found_any);
int handle_flags(int argc, char *argv[], char **search, char **target_dir, int *only_dirs, int *only_files, int *exact_match);
void seek_execut(char *argv[], int argc);
int prefix_match(const char *name, const char *prefix);
// void search_directory(const char *dir_path, const char *search, int only_dirs, int only_files, int exact_match, int *dir_count, int *file_count, char *single_match, int *error_count);
// void search_files(const char *dir_path, const char *search, int exact_match, int *file_count, char *single_match, int *error_count);
void search_files(const char *dir_path, const char *search, int exact_match, int *file_count, char *single_match, int *error_count, const char *target_dir);
void search_directory(const char *dir_path, const char *search, int only_dirs, int only_files, int exact_match, int *dir_count, int *file_count, char *single_match, int *error_count, const char *target_dir);
void print_result(const char *path, const char *base_dir, int is_dir);

#endif // SEEK_H

