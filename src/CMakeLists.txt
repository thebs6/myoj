# # for each "src/x.cpp", generate target "x"
# file(GLOB_RECURSE all_srcs CONFIGURE_DEPENDS *.cc)
# foreach(v ${all_srcs})
#     string(REGEX MATCH "src/.*" relative_path ${v})
#     # message(${relative_path})
#     string(REGEX REPLACE "src/" "" target_name ${relative_path})
#     string(REGEX REPLACE "\\.cc" "" target_name ${target_name})
#     add_executable(${target_name} ${v})
# endforeach()

file(GLOB SRC_FILES ./*.cc ./*.h)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC")

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 定义一个动态库
add_library(mymuduo SHARED ${SRC_FILES})

# set_target_properties(mymuduo PROPERTIES VERSION 1.0 SOVERSION 1)
# 定义动态库的版本号
# message(${PROJECT_SOURCE_DIR})
# 定义库的安装路径
# install(TARGETS mymuduo DESTINATION ${PROJECT_SOURCE_DIR}/lib)