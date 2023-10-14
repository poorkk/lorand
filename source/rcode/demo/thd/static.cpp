#include <stdio.h>

int get_cipher(int num)
{
    int a = num;
    printf("call get cipher");
    return a;
}

void func(int num)
{
    static int sn = get_cipher(num);
    printf("%d\n", sn);
}

int main()
{
    int i;

    for (int i = 0; i < 5; i++) {
        func(i);
    }

    return 0;
}