# 加载子目录
set(HTTP_SRCS
    ${PROJECT_SOURCE_DIR}/src/http/HttpServer.cc
    ${PROJECT_SOURCE_DIR}/src/http/HttpResponse.cc
    ${PROJECT_SOURCE_DIR}/src/http/HttpContext.cc
    ${PROJECT_SOURCE_DIR}/include/http/HttpContext.h
    ${PROJECT_SOURCE_DIR}/include/http/HttpResponse.h
    ${PROJECT_SOURCE_DIR}/include/http/HttpServer.h
    ${PROJECT_SOURCE_DIR}/include/http/HttpRequest.h
)

set(OJ
  test.cc
)



add_executable(OJ ${OJ} ${HTTP_SRCS})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/OJ)

target_link_libraries(OJ mymuduo)

