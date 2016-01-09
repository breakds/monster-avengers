CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT("armorup-server")

MESSAGE(STATUS "Sub Project Name: ${PROJECT_NAME}")
MESSAGE(STATUS "Sub Project Source: ${PROJECT_SOURCE_DIR}")
MESSAGE(STATUS "Sub Project Binary: ${PROJECT_BINARY_DIR}")

INCLUDE_DIRECTORIES(".")

ADD_EXECUTABLE(armor_up_server armor_up_server.cc)
TARGET_LINK_LIBRARIES(armor_up_server helpers sqlite_loader binary_loader data_service -lmicrohttpd)

