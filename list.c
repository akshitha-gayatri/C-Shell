#include "initialise.h"
#include "list.h"

void insertAtStart(struct Node **head, const char *newData)
{
    struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
    if (newNode == NULL)
    {
        fprintf(stderr, RED "Error: Failed to allocate memory for new node\n" RESET);
        return;
    }

    newNode->data = strdup(newData);
    if (newNode->data == NULL)
    {
        free(newNode);
        fprintf(stderr, RED "Error: Failed to allocate memory for node data\n" RESET);
        return;
    }

    newNode->next = *head;
    *head = newNode;
}



void deletelist(struct Node **head)
{
    struct Node *cur = *head;
    struct Node *temp;

    while (cur != NULL)
    {
        temp = cur->next;
        free(cur->data);
        free(cur);
        cur = temp;
    }

    *head = NULL;
}

