# 加载子目录
set(HTTP_SRCS
  HttpServer.cc
  HttpResponse.cc
  HttpContext.cc
  main.cc
)

# set(OJ
#   HttpServer.cc
#   HttpResponse.cc
#   HttpContext.cc
#   oj.cc
# )

add_executable(HttpServer ${HTTP_SRCS})
# add_executable(OJ ${OJ})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/http)

# target_link_libraries(OJ mymuduo)
target_link_libraries(HttpServer mymuduo)

