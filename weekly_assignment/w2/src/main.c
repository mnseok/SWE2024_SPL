#include <stdio.h>

int plus(int, int);
int minus(int, int);

int main()
{
    int a, b;
    scanf("%d %d", &a, &b);

    printf("%d %d\n", plus(a, b), minus(a, b));
}
