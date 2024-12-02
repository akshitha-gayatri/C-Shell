#ifndef MY_H
#define MY_H

void add_alias(const char *name, const char *command);
void add_function(const char *name, const char *body);
void replace_argument(char *line, const char *arg);
void load_myshrc(const char *filename);
void process_input(char *line);

#endif