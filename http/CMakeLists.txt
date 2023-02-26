# 设置编译目标
add_library(http
  HttpContext.cc
  HttpRequest.cc
  HttpResponse.cc
  HttpServer.cc
)

target_link_libraries(http mymuduo)

# 指定头文件路径
target_include_directories(http PUBLIC ${PROJECT_SOURCE_DIR}/http)
target_include_directories(http PUBLIC ${PROJECT_SOURCE_DIR}/src)

# 设置可执行文件
add_executable(httpTest test/httpTest.cc)

# 链接库
target_link_libraries(httpTest http)
