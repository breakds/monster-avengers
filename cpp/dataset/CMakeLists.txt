CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT("armorup-dataset")

MESSAGE(STATUS "Sub Project Name: ${PROJECT_NAME}")
MESSAGE(STATUS "Sub Project Source: ${PROJECT_SOURCE_DIR}")
MESSAGE(STATUS "Sub Project Binary: ${PROJECT_BINARY_DIR}")

INCLUDE_DIRECTORIES(".")

ADD_LIBRARY(properties STATIC base/properties.cc)
TARGET_LINK_LIBRARIES(properties helpers)

ADD_EXECUTABLE(properties_test base/properties_test.cc)
TARGET_LINK_LIBRARIES(properties_test properties)

ADD_EXECUTABLE(reindexed_table_test base/reindexed_table_test.cc)
TARGET_LINK_LIBRARIES(reindexed_table_test helpers)

ADD_LIBRARY(dataset STATIC dataset.cc)

ADD_LIBRARY(binary_loader STATIC loader/binary_loader.cc)
TARGET_LINK_LIBRARIES(binary_loader dataset helpers)

ADD_LIBRARY(sqlite_loader STATIC loader/sqlite_loader.cc)
TARGET_LINK_LIBRARIES(sqlite_loader dataset helpers -lsqlite3)

ADD_LIBRARY(data_service STATIC data_service.cc)
TARGET_LINK_LIBRARIES(data_service properties dataset helpers -lsqlite3)

ADD_EXECUTABLE(dataset_test dataset_test.cc)
TARGET_LINK_LIBRARIES(dataset_test dataset helpers data_service sqlite_loader binary_loader)

