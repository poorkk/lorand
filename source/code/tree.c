#include <stdlib.h>
#include <stdio.h>

typedef char bool;
#define true 1
#define false 0

typedef struct Node {
    int val;
    struct Node *left;
    struct Node *right;
} Node;

int tree_max_depth(Node *root)
{
    int left_depth;
    int right_depth;

    if (root == NULL) {
        return 0;
    }

    left_depth = tree_max_depth(root->left);
    right_depth = tree_max_depth(root->right);

    return left_depth > right_depth ? left_depth + 1 : right_depth + 1;
}

/*
根节点到叶子节点：
        3
    9       20
        15      7
*/
int tree_min_depth(Node *root)
{
    int left_depth;
    int right_depth;

    if (root == NULL) {
        return 0;
    }

    left_depth = tree_min_depth(root->left);
    right_depth = tree_min_depth(root->right);

    if (left_depth == 0 && right_depth == 0)  {
        return 1;
    } else if (left_depth == 0) {
        return right_depth + 1;
    } else if (right_depth == 0) {
        return left_depth + 1;
    } else {
        return left_depth < right_depth ? left_depth + 1 : right_depth + 1;
    }
}

Node *tree_rotate(Node *root)
{
    if (root == NULL) {
        return NULL;
    }

    tree_rotate(root->left);
    tree_rotate(root->right);

    Node *tmp = root->left;
    root->left = root->right;
    root->right = tmp;

    return root;
}

bool tree_same(Node *p, Node *q)
{
    if (p == NULL && q == NULL) {
        return true;
    } else if (p == NULL || q == NULL) {
        return false;
    } else if (p->val != q->val) {
        return false;
    }
    return (tree_same(p->left, q->left) && tree_same(p->right, q->right));
}

// bool tree_symm(Node *t1, Node *t2)
// {
//     if (t1 == NULL && t2 == NULL) {
//         return true;
//     } else if (t1 == NULL || t2 == NULL) {
//         return false;
//     } else if (t1->val )
// }

bool tree_symm(Node *root)
{
    return true;
    // if (root == NULL) {
    //     return true;
    // }

    // Node *left = root->left;
    // Node *right  = root->right;

    // if (left == NULL && root->right == NULL) {
    //     return true;
    // } else if (root->left == NULL || root->right == NULL) {
    //     return false;
    // } else if (root->left->val != root->right->val) {
    //     return false;
    // } else {
    //     return (tree_symm(root->left) && tree_symm(root->right));
    // }
}

bool tree_path_sum(Node *root, int sum)
{
#define IS_LEAF(node) ((node)->left == NULL && (node)->right == NULL)
    if (root == NULL) {
        return false;
    }

    if (IS_LEAF(root)) {
        return root->val == sum;
    }

    int childsum = sum - root->val;
    return (tree_path_sum(root->left, childsum) || tree_path_sum(root->right, childsum));
}

int tree_left_sum(Node *root)
{
#define IS_LEAF(node) ((node)->left == NULL && (node)->right == NULL)
    int sum = 0;
    
    if (root == NULL) {
        return 0;
    }

    if (root->left != NULL && IS_LEAF(root->left)) {
        sum = root->left->val;
    }

    sum += tree_left_sum(root->left);
    sum += tree_left_sum(root->right);

    return sum;
}

/* 1 3 5 7 2 4 6 */
Node *tree_build(int *arr, int arrsz)
{
#define IS_NULL(num) ((num) == -1)

    Node *root;
    if (arrsz == 0 || IS_NULL(arr[0])) {
        return NULL;
    }

    root = malloc(sizeof(Node));
    if (root == NULL) {
        return NULL;
    }
    root->val = arr[0];

    int childsz = (arrsz - 1) / 2;
    root->left = tree_build(arr + 1, childsz);
    root->right = tree_build(arr + 1 + childsz, childsz);

    return root;
}

