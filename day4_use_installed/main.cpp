#include <iostream>
#include <my_math/my_math.h>   // 注意路径，因为安装时放在了 include/my_math/ 下

int main()
{
    std::cout << "Using installed library : 100 + 200 = " << add(100, 200) << std::endl;
    return 0;
}
