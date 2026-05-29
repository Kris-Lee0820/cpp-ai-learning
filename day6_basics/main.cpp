#include <iostream>

#ifdef DEBUG_MODE
#define LOG(msg) std::cerr << "[DEBUG]" << msg << std::endl;
#else
#define LOG(msg) ((void)0)
#endif

int main()
{
    std::cout << "Hello, CMake config!" << std::endl;
    LOG("This is a debug message");
    return 0;
}
