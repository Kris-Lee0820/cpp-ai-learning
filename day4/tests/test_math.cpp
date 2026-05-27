#include <iostream>
#include "my_math.h"

int main()
{
    int result = add(2, 3);
    if (result == 5) {
        std::cout << "test_add : PASSED" << std::endl;
	return 0;
    } else {
	std::cout << "test_add : FAILED (got " << result << ")" << std::endl;
	return 1;
    }
}
