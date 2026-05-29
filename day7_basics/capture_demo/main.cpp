#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    int threshold = 5;
    std::vector<int> nums = {1, 7, 3, 9, 2};

    // 值捕获 threshold
    auto count_greater = [threshold](int val) { return val > threshold; };
    int cnt = std::count_if(nums.begin(), nums.end(), count_greater);
    std::cout << "大于 " << threshold << " 的个数：" << cnt << std::endl;

    // 引用捕获，可以修改外部变量
    int total = 0;
    auto accumulate = [&total](int val) { total += val; };
    std::for_each(nums.begin(), nums.end(), accumulate);
    std::cout << "总和：" << total << std::endl;

    return 0;
}
