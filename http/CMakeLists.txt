include_directories(${PROJECT_SOURCE_DIR}/src)

# 定义可执行文件的源文件
set(EXE_SRC httpTest.cc)

# 添加可执行文件
add_executable(httpTest ${EXE_SRC})

# 链接动态库
target_link_libraries(httpTest mymuduo pthread)
