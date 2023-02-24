# 顶层 CMakeLists.txt

# 指定 CMake 的最低版本
cmake_minimum_required(VERSION 3.0)

# 定义项目名称
project(mymuduo)

# 包含子目录 src
add_subdirectory(src)

# 包含子目录 test
add_subdirectory(test)

add_subdirectory(http)