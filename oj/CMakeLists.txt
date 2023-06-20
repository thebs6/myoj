link_directories(${CMAKE_SOURCE_DIR}/lib)

include_directories(/usr/local/include/mongocxx/v_noabi /usr/local/include/libmongoc-1.0)
include_directories(/usr/local/include/bsoncxx/v_noabi /usr/local/include/libbson-1.0)
include_directories(hiredis sw)

aux_source_directory(${PROJECT_SOURCE_DIR}/oj/src SRC_BASE)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/http HTTP_SRC)
aux_source_directory(${PROJECT_SOURCE_DIR}/oj/model MODEL_SRC)
include_directories(${PROJECT_SOURCE_DIR}/oj/src)
include_directories(${PROJECT_SOURCE_DIR}/include/http)

add_executable(OJ ${SRC_BASE} ${HTTP_SRC} ${MODEL_SRC})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/oj)

target_link_libraries(OJ hiredis redis++ pthread judger mongocxx bsoncxx mymuduo)