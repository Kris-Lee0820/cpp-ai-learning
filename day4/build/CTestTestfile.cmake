# CMake generated Testfile for 
# Source directory: /home/kris-lee/cpp-ai-learning/day4
# Build directory: /home/kris-lee/cpp-ai-learning/day4/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(MathTest "/home/kris-lee/cpp-ai-learning/day4/build/test_math")
set_tests_properties(MathTest PROPERTIES  _BACKTRACE_TRIPLES "/home/kris-lee/cpp-ai-learning/day4/CMakeLists.txt;35;add_test;/home/kris-lee/cpp-ai-learning/day4/CMakeLists.txt;0;")
subdirs("math_lib")
subdirs("app")
