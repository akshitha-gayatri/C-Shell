#ifndef LIST
#define LIST

struct Node
{
    char *data;
    struct Node *next;
};

void insertAtStart(struct Node **head, const char *newData);
void deletelist(struct Node **head);
extern struct Node *dir_list;

#endif