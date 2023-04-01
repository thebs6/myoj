# 设置编译目标
add_library(logger
        AsyncLogging.cpp
        FileUtil.cpp
        LogFile.cpp
        Logging.cc
        LogStream.cpp
        )

include_directories(.)
target_link_libraries(logger mymuduo)

# 指定头文件路径
target_include_directories(logger PUBLIC ${PROJECT_SOURCE_DIR}/http)
target_include_directories(logger PUBLIC ${PROJECT_SOURCE_DIR}/src)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
# 设置可执行文件
add_executable(logger_test test/logger_test.cpp)

# 链接库
target_link_libraries(logger_test logger)
