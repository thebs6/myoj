cmake_minimum_required(VERSION 3.5)#最低CMake版本

project (mymuduo)# 工程名


file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cc)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

add_executable(main ${SOURCES})#用所有的源文件生成一个可执行文件，因为这里定义了SOURCE变量，所以就不需要罗列cpp文件了
#等价于命令：     add_executable(hello_headers src/Hello.cpp src/main.cpp)

# target_link_libraries(main pthread)

target_include_directories(main
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
)#设置这个可执行文件hello_headers需要包含的库的路径
