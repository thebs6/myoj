link_directories(${CMAKE_SOURCE_DIR}/lib)

include_directories(/usr/local/include/mongocxx/v_noabi /usr/local/include/libmongoc-1.0)
include_directories(/usr/local/include/bsoncxx/v_noabi /usr/local/include/libbson-1.0)
include_directories(hiredis sw)


set(SRC_JUDGE ${PROJECT_SOURCE_DIR}/oj/judge/logger/logger.c
${PROJECT_SOURCE_DIR}/oj/judge/logger/logger.h
${PROJECT_SOURCE_DIR}/oj/judge/common/common.c
${PROJECT_SOURCE_DIR}/oj/judge/common/common.h
${PROJECT_SOURCE_DIR}/oj/judge/time/time.c
${PROJECT_SOURCE_DIR}/oj/judge/time/time.h
${PROJECT_SOURCE_DIR}/oj/judge/system/system.c
${PROJECT_SOURCE_DIR}/oj/judge/system/system.h
${PROJECT_SOURCE_DIR}/oj/judge/child/child.c
${PROJECT_SOURCE_DIR}/oj/judge/child/child.h
${PROJECT_SOURCE_DIR}/oj/judge/judge/judge.c
${PROJECT_SOURCE_DIR}/oj/judge/judge/judge.h
${PROJECT_SOURCE_DIR}/oj/judge/guard/guard.c
${PROJECT_SOURCE_DIR}/oj/judge/guard/guard.h)

aux_source_directory(${PROJECT_SOURCE_DIR}/oj/src SRC_BASE)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/http/ HTTP_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/oj/model MODEL_SRC)
include_directories(${PROJECT_SOURCE_DIR}/oj/src)
include_directories(${PROJECT_SOURCE_DIR}/include/http)

add_executable(OJ ${SRC_BASE} ${HTTP_SRC} ${MODEL_SRC} ${SRC_JUDGE})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/oj)

target_link_libraries(OJ hiredis redis++ seccomp pthread mongocxx bsoncxx mymuduo)