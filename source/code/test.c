#include <stdio.h>
#include <string.h>

/* for执行顺序：for (s1; s2; s4) {s3} */
void _for()
{
    int i;
    for (i = 0; i < 5; i++, printf("%d\n", i)) {
        printf("-- %d\n", i);
    }
    printf("++ %d\n", i);
}

int main()
{
    _for();

    return 0;
}