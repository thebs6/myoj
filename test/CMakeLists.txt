include_directories(${PROJECT_SOURCE_DIR}/src)

# 定义可执行文件的源文件
set(EXE_SRC test22.cc)

# 添加可执行文件
add_executable(test22 ${EXE_SRC})

# 链接动态库
target_link_libraries(test22 mymuduo pthread)