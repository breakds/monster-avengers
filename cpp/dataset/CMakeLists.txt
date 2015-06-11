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
TARGET_LINK_LIBRARIES(dataset helpers)

ADD_LIBRARY(dataset_full STATIC dataset_full.cc)
TARGET_LINK_LIBRARIES(dataset_full helpers -lsqlite3)

ADD_EXECUTABLE(dataset_full_test dataset_full_test)
TARGET_LINK_LIBRARIES(dataset_full_test helpers dataset_full)

