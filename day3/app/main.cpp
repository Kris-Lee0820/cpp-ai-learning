#include <iostream>
#include "my_math.h"
#include <fmt/core.h> //包含fmt库的头文件

int main()
{
    int a = 10, b = 5;
    //使用fmt库进行格式化输出
    fmt::print("CMake Day 3 挑战:使用第三方库 {}\n", "fmt");
    std::cout << "CMake Day 3 : 模块化构建成功！" << std::endl;
    std::cout << a << " + " << b << " = " << add(a, b) << std::endl;
    std::cout << a << " - " << b << " = " << subtract(a, b) << std::endl;
    std::cout << a << " * " << b << " = " << multiply(a, b) << std::endl;
    return 0;
}
