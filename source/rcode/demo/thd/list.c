#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

Node *list_add(Node *head, Node *node)
{
    Node *cur;

    node->next = NULL;
    
    if (head == NULL) {
        return node;
    }

    for (cur = head; cur->next != NULL; cur = cur->next) {};
    cur->next = node;

    return head;
}

int list_len(Node *head)
{
    Node *cur;
    int cnt = 0;

    for (cur = head; cur != NULL; cur = cur->next, cnt++) {};

    return cnt;
}

Node *list_travl(Node *head, Node *last)
{
    if (last == NULL) {
        return head;
    }
    return last->next;
}

typedef struct {
    Node node;
    char *val;
} StrNode;

Node *slist_new(const char *s)
{
    StrNode *snode = (StrNode *)malloc(sizeof(StrNode));
    snode->val = strdup(s);
    return (Node *)snode;
}

void list_test()
{
    const char *sarr[] = {
        "s1",
        "s2",
        "s3",
        "s4",
        "s5"
    };
    
    Node *shead = NULL;
    int i;

    for (i = 0; i < ARR_LEN(sarr); i++) {
        shead = list_add(shead, slist_new(sarr[i]));
    }

    printf("%d\n", list_len(shead));

    StrNode *cur = NULL;
    for (;;) {
        cur = (StrNode *)list_travl(shead, (Node *)cur);
        if (cur == NULL) {
            break;
        }

        printf("%s\n", cur->val);
    }
}

int main()
{
    list_test();

    return 0;
}
