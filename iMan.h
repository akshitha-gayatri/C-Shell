#ifndef IMAN_H
#define IMAN_H

void fetch_man_page_for_section(const char *command, const char *section, int *found);
void fetch_man_page(const char *command);

#endif