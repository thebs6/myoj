# # # 加载子目录
# set(HTTP_SRCS
#     ${PROJECT_SOURCE_DIR}/src/http/HttpResponse.cc
#     ${PROJECT_SOURCE_DIR}/src/http/HttpContext.cc
#     ${PROJECT_SOURCE_DIR}/src/http/HttpRequest.cc
#     ${PROJECT_SOURCE_DIR}/include/http/HttpContext.h
#     ${PROJECT_SOURCE_DIR}/include/http/HttpResponse.h
#     ${PROJECT_SOURCE_DIR}/include/http/HttpRequest.h
# )

# # include_directories(src/include)

# # set(SRC
# #   test/test.cc
# #   src/server/LCServer.cc
# #   src/controller/UserController.cc
# #   src/include/server/LCServer.h
# #   src/include/controller/UserController.h
# #   src/include/controller/ControllerBase.h
# # )

# # message(${SRC})

# # aux_source_directory(utils DIR_SRCS_UTILS)

# # add_executable(OJ ${SRC} ${HTTP_SRCS} ${DIR_SRCS_UTILS})

# # set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/oj)

# # target_link_libraries(OJ mymuduo pthread mysqlclient hiredis)
# set(SRC
#   test/test.cc
#   src/include/server/LCServer.h
#   src/server/LCServer.cc
#   src/include/controller/UserController.h
#   src/controller/UserController.cc
#   src/include/controller/ControllerBase.h
# )
# include_directories(${PROJECT_SOURCE_DIR}/thirdparty)


# add_executable(OJ ${SRC} ${HTTP_SRCS})

# target_link_libraries(OJ mymuduo pthread mysqlclient hiredis)


# include_directories(src/include)

include_directories(${PROJECT_SOURCE_DIR}/thirdparty)
include_directories(src/include/server)
include_directories(src/include/db)
include_directories(src/include/model)
include_directories(src/include/redis)
include_directories(src/include/service)
include_directories(src/include/controller)
include_directories(src/include/server)

aux_source_directory(./test SRC_LIST)
aux_source_directory(./src/db DB_LIST)
aux_source_directory(./src/model MODEL_LIST)
aux_source_directory(./src/redis REDIS_LIST)
aux_source_directory(./src/controller CONTROLLER_LIST)
aux_source_directory(./src/server SERVER_LIST)


add_executable(OJ ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${REDIS_LIST} ${CONTROLLER_LIST} ${SERVER_LIST})

target_link_libraries(OJ mymuduo pthread mysqlclient hiredis)