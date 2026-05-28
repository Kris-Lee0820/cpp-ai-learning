#include <iostream>
#include <memory>

#ifdef _WIN32
#include <cstdlib>  // 为 system()
#endif

class MyClass {
    public:
        MyClass() {
            std::cout << "构建" << std::endl;
        }
        ~MyClass() {
            std::cout << "析构" << std::endl;
        }
        void hello() {
            std::cout << "Hello" << std::endl;
        }
};

int main(int, char**){
#ifdef _WIN32
    system("chcp 65001 > nul");  // 切换到 UTF-8 代码页，并隐藏多余输出
#endif

    auto p = std::make_unique<MyClass>();
    p->hello();

    //auto p2 = p; // 错误：unique_ptr不允许复制
    auto p2 = std::move(p); // 转移所有权
    if (!p) {
        std::cout << "已被转移, 无法访问" << std::endl;
    }
    p2->hello();
    MyClass *ptr = p2.release(); // 释放资源但不删除对象
    ptr->hello();
    delete ptr; // 手动删除对象
    ptr = nullptr; // 防止悬空指针
    if(!ptr) {
        std::cout << "对象已手动删除" << std::endl;
    }
    if(!p2) {
        std::cout << "资源已释放但对象未删除" << std::endl;
    }
    p2.reset(); // 显式释放资源
    if(!p2) {
        std::cout << "资源已释放" << std::endl;
    }

    auto sp1 = std::make_shared<MyClass>();
    auto sp2 = sp1; // 共享所有权
    std::cout << "引用计数: " << sp1.use_count() << std::endl; // 输出引用计数
    
    std::weak_ptr<MyClass> wp = sp1; // 创建弱引用
    std::cout << "弱引用后的计数 :" << sp1.use_count() << std::endl; // 计数不变 
    if (auto locked = wp.lock()) { // 尝试获取共享所有权
        std::cout << "成功获取共享所有权" << std::endl;
        locked->hello();
    } else {
        std::cout << "对象已被销毁" << std::endl;
    }
    return 0;
}
