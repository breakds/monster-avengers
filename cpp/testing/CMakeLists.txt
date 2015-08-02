CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT("armorup-testing")

MESSAGE(STATUS "Sub Project Name: ${PROJECT_NAME}")
MESSAGE(STATUS "Sub Project Source: ${PROJECT_SOURCE_DIR}")
MESSAGE(STATUS "Sub Project Binary: ${PROJECT_BINARY_DIR}")

INCLUDE_DIRECTORIES(".")

# Copy Regression Queries to Target Directory
FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/regression")
FILE(COPY "${PROJECT_SOURCE_DIR}/queries" 
  DESTINATION "${CMAKE_BINARY_DIR}/regression")

# Binary to Run Regression Tests
ADD_EXECUTABLE(regression regression.cc)
TARGET_LINK_LIBRARIES(regression helpers sqlite_loader data_service)
SET_TARGET_PROPERTIES(regression PROPERTIES 
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/regression")




