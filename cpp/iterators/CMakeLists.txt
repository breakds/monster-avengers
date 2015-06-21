CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT("armorup-iterators")

MESSAGE(STATUS "Sub Project Name: ${PROJECT_NAME}")
MESSAGE(STATUS "Sub Project Source: ${PROJECT_SOURCE_DIR}")
MESSAGE(STATUS "Sub Project Binary: ${PROJECT_BINARY_DIR}")

INCLUDE_DIRECTORIES(".")

ADD_EXECUTABLE(iterator_base_test base_test.cc)
TARGET_LINK_LIBRARIES(iterator_base_test helpers)


