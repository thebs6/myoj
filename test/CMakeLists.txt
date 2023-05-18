# 加载子目录
add_executable(echoServer echoServer.cc)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/test)

target_link_libraries(echoServer mymuduo)

