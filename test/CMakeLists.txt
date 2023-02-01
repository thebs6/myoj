# for each "src/x.cpp", generate target "x"
file(GLOB_RECURSE all_srcs CONFIGURE_DEPENDS *.cc)
# message(!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!${all_srcs})
foreach(v ${all_srcs})
    string(REGEX MATCH "test/.*" relative_path ${v})
    string(REGEX REPLACE "test/" "" target_name ${relative_path})
    string(REGEX REPLACE "\\.cc" "" target_name ${target_name})
    add_executable(${target_name} ${v})
    target_include_directories(${target_name} PRIVATE ${PROJECT_SOURCE_DIR}/src)#设置这个可执行文件hello_headers需要包含的库的路径
endforeach()



