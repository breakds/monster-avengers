CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(Comparison)

SET(CMAKE_CXX_COMPILER "/usr/bin/clang++")

MESSAGE(STATUS "Project Name:           ${PROJECT_NAME}")
MESSAGE(STATUS "Project Source Dir:     ${PROJECT_SOURCE_DIR}")
MESSAGE(STATUS "Kernel:                 ${CMAKE_SYSTEM}")
EXECUTE_PROCESS(COMMAND hostname OUTPUT_VARIABLE HOSTNAME)
MESSAGE(STATUS "Hostname:               ${HOSTNAME}")
MESSAGE(STATUS "Build Type:             ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "GCC:                    ${CMAKE_CXX_COMPILER}")

include_directories("~/pf/projects" ".")

option (BUILD_TESTS "build executables in purpose of unittest." ON)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -DNDEBUG -O3")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_CXX_FLAGS_GPROF "-O1 -pg")

IF(BUILD_TESTS)
  ADD_EXECUTABLE(test core/test.cc)
  TARGET_LINK_LIBRARIES(test -lsqlite3)
  ADD_EXECUTABLE(explore_test core/explore_test.cc)
  TARGET_LINK_LIBRARIES(explore_test -lsqlite3)
  ADD_EXECUTABLE(signature_test utils/signature_test.cc)
  TARGET_LINK_LIBRARIES(signature_test -lsqlite3)
ENDIF(BUILD_TESTS)

ADD_EXECUTABLE(serve_query serve_query.cc)
TARGET_LINK_LIBRARIES(serve_query -lsqlite3)

ADD_EXECUTABLE(serve_explore serve_explore.cc)
TARGET_LINK_LIBRARIES(serve_explore -lsqlite3)

ADD_EXECUTABLE(armor_up_server server/armor_up_server.cc)
TARGET_LINK_LIBRARIES(armor_up_server -lmicrohttpd -lsqlite3)




