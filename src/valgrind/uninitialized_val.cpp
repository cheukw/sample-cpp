#include <stdio.h>
#include <stdlib.h>

void test(int x)
{
    if(x > 0){
        x += 9;
    }
}

class Foo
{
public:
    Foo(){}
    int GetVal(){return val_;}
    int val_;
};
int main() 
{
    int x;
    test(x);
    Foo f;
   int a = f.GetVal();

   a += 1;
    return 0;
}