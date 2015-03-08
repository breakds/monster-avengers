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

SET(CMAKE_CXX_FLAGS "${OPENCV_FLAGS} -std=c++11")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -DNDEBUG -O3")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_CXX_FLAGS_GPROF "-O1 -pg")


IF(BUILD_TESTS)
  ADD_EXECUTABLE(test core/test.cc)
  ADD_EXECUTABLE(signature_test utils/signature_test.cc)
ENDIF(BUILD_TESTS)

ADD_EXECUTABLE(serve_query serve_query.cc)


