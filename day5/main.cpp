#include <iostream>
#include <memory>

class MyClass {
public:
    MyClass() {
	std::cout << "构建" << std::endl;
    }
    ~MyClass() {
	std::cout << "析构" << std::endl;
    }
    void hello() {
	std::cout << "hello" << std::endl;
    }
};

int main()
{
    auto p = std::make_unique<MyClass>();
    p->hello();

    // auto p2 = p; //错误！不能拷贝
    auto p2 = std::move(p); //可以移动，p为空
    if (!p) std::cout << "p为空" << std::endl;
    p2->hello();

    //不需要手动delete，离开作用域自动析构

    auto sp1 = std::make_shared<MyClass>();
    auto sp2 = sp1;
    std::cout << "计数：" << sp1.use_count() << std::endl;

    std::weak_ptr<MyClass> wp = sp1;
    if (auto locked = wp.lock()) { //尝试提升为shared_ptr
	locked->hello();
    } else {
	std::cout << "对象已经被销毁" << std::endl;
    }
    return 0;
}
