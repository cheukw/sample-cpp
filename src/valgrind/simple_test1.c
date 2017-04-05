#include <stdio.h>
#include <stdlib.h>

void test()
{
    int *x = malloc(10 * sizeof(int));

}

int main() 
{
    char* c = malloc(10);

    test();
    return 0;
}