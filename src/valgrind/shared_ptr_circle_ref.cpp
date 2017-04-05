#include <memory>
#include <string>
#include <thread>
#include <chrono>

class Bar;
class Foo: public std::enable_shared_from_this<Foo>
{
public:
    Foo(const std::shared_ptr<Bar>& bar): bar_(bar){}

    std::shared_ptr<Bar> bar_;
};

class Bar: public std::enable_shared_from_this<Bar>
{
public:
    Bar():foo_(nullptr){}
    void SetFoo(const std::shared_ptr<Foo>& foo){foo_ = foo; }
    std::shared_ptr<Foo> foo_;
};

void test()
{
    auto bar = std::make_shared<Bar>();
    auto foo = std::make_shared<Foo>(bar);
    bar->SetFoo(foo);
}
int main()
{
    test();
    while(1){
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
    return 0;
}