#ifndef NEO_H
#define NEO_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <termios.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>

void enablerawmode();
void disablerawmode();
int check_keypress();
int get_most_recent_pid();
void neonate(int argc, char *argv[]);

#endif 