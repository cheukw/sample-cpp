#include <stdio.h>
#include <stdlib.h>

void test()
{
    int *x = malloc(10 * sizeof(int));
    x[9] = 10;
    x[10] = 999;
}

int main() 
{
    test();
    return 0;
}