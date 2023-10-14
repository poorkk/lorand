#ifndef LIST_H_
#define LIST_H_

#define ARR_LEN(arr) (sizeof((arr)) / sizeof((arr)[0]))

typedef struct Node {
    struct Node *next;
} Node;

Node *list_add(Node *head, Node *node);
int list_len(Node *head);
Node *list_travl(Node *head, Node *last);

#endif