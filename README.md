unique_ptr的relese是释放资源但不会删除对象，返回一个当前智能指针类型的原始指针，后续需要手动delete掉这块空间，否则会造成内存泄漏。
			reset会释放资源并且释放对象。
			不能够拷贝，只能移动。

===============================================================================================================================

智能指针 + OpenCV入门
🧱 基础模块：C++ 智能指针（预计45分钟）
1. 为什么需要智能指针？
传统 new/delete 容易忘记释放内存，或异常时跳过 delete，导致内存泄漏。智能指针是对象，离开作用域时会自动调用析构函数释放资源。

2. std::unique_ptr – 独占所有权
不能拷贝，只能移动（std::move）。

适用于明确的单一所有者。

常用函数：make_unique（C++14）、release()、reset()。

核心代码示例：

cpp
#include <memory>
#include <iostream>

class MyClass {
public:
    MyClass() { std::cout << "构造\n"; }
    ~MyClass() { std::cout << "析构\n"; }
    void hello() { std::cout << "hello\n"; }
};

int main() {
    auto p = std::make_unique<MyClass>(); // C++14
    p->hello();

    // auto p2 = p; // 错误！不能拷贝
    auto p2 = std::move(p); // 可以移动，p 变为空
    if (!p) std::cout << "p 为空\n";
    p2->hello();

    // 不需要手动 delete，离开作用域自动析构
}
3. std::shared_ptr – 共享所有权
引用计数，多个指针指向同一对象。

最后一个 shared_ptr 销毁时释放对象。

常用函数：make_shared、use_count()。

示例：

cpp
auto sp1 = std::make_shared<MyClass>();
auto sp2 = sp1; // 引用计数 +1
std::cout << "计数: " << sp1.use_count() << std::endl; // 2
// 离开作用域，计数减到0时自动析构
4. std::weak_ptr – 打破循环引用
不增加引用计数，需要时 lock() 获得 shared_ptr。

用于观察者模式或缓存。

示例：

cpp
std::weak_ptr<MyClass> wp = sp1;
if (auto locked = wp.lock()) { // 尝试提升为 shared_ptr
    locked->hello();
} else {
    std::cout << "对象已销毁";
}
5. 常见坑点
不要用 new 初始化智能指针：shared_ptr<MyClass>(new MyClass) 不如 make_shared 安全（异常时可能泄漏）。

不要用 get() 获取原始指针后再 delete。

循环引用：两个 shared_ptr 互相持有对方，导致内存泄漏。用 weak_ptr 打破。