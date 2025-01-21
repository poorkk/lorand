// 本题为考试多行输入输出规范示例，无需提交，不计分。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * mid:  left-child   root   right-child
 * post: left-child   right-child   root
 */
void tree_get_pre_order(char *mid, int midlen, char *post, int postlen, char *pre, int *prelen)
{
    if (midlen <= 0) {
        return;
    }
    
    /* find root */
    char root = post[postlen - 1];
    pre[(*prelen)++] = root;

    /* get the lenth of left child */
    int i;
    for (i = 0; i < midlen; i++) {
        if (mid[i] == root) {
            break;
        }
    }

    /* caculate the lenth of right child */
    int leftlen = i;
    int righlen = midlen - leftlen - 1;

    /* get the left child */
    tree_get_pre_order(mid, leftlen, post, leftlen, pre, prelen);

    /* get the right child */
    tree_get_pre_order(mid + leftlen + 1 , righlen, post + leftlen, righlen, pre, prelen);
}

int main()
{
    char mid[4096] = {0};
    char post[4096] = {0};
    scanf("%s\n%s", post, mid);

    if (strlen(mid) != strlen(post)) {
        printf("invalid input\n");
        exit(-1);
    }

    int len = (int)strlen(mid);
    if (len == 0) {
        exit(0);
    }

    char pre[4096] = {0};
    int prelen = 0;

    tree_get_pre_order(mid, len, post, len, pre, &prelen);
    if (prelen != len) {
        printf("invalid output: %s\n", pre);
        exit(-1);
    }

    printf("pre order: %s\n", pre);

    return 0;
}