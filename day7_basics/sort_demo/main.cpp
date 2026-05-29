#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    std::vector<int> nums = {5, 2, 8, 1, 9};

    // 升序排序（默认）
    std::sort(nums.begin(), nums.end());

    // 降序排序（使用lambda）
    std::sort(nums.begin(), nums.end(), [](int a, int b) { return a > b; });

    // 按绝对值降序排列
    std::sort(nums.begin(), nums.end(), [](int a, int b) { return std::abs(a) > std::abs(b); });

    // 打印
    for (int n : nums) std::cout << n << " ";
    std::cout << "\n";
    return 0;
}
