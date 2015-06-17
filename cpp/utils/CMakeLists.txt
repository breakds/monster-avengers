CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT("armorup-utils")

MESSAGE(STATUS "Sub Project Name: ${PROJECT_NAME}")
MESSAGE(STATUS "Sub Project Source: ${PROJECT_SOURCE_DIR}")
MESSAGE(STATUS "Sub Project Binary: ${PROJECT_BINARY_DIR}")

INCLUDE_DIRECTORIES(".")

ADD_EXECUTABLE(query_test query_test.cc)
TARGET_LINK_LIBRARIES(query_test helpers properties dataset sqlite_loader)

ADD_EXECUTABLE(jewels_query_test jewels_query_test.cc)
TARGET_LINK_LIBRARIES(jewels_query_test helpers properties dataset sqlite_loader)

ADD_EXECUTABLE(signature_test jewels_query_test.cc)
TARGET_LINK_LIBRARIES(signature_test helpers properties dataset sqlite_loader)


