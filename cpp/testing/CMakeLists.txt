CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT("armorup-testing")

MESSAGE(STATUS "Sub Project Name: ${PROJECT_NAME}")
MESSAGE(STATUS "Sub Project Source: ${PROJECT_SOURCE_DIR}")
MESSAGE(STATUS "Sub Project Binary: ${PROJECT_BINARY_DIR}")

INCLUDE_DIRECTORIES(".")

# Copy Regression Queries to Target Directory
FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/regression")
FILE(REMOVE_RECURSE DESTINATION "${CMAKE_BINARY_DIR}/regression/queries")
# FILE(COPY "${PROJECT_SOURCE_DIR}/queries" 
#   DESTINATION "${CMAKE_BINARY_DIR}/regression")
# FILE(REMOVE_RECURSE DESTINATION "${CMAKE_BINARY_DIR}/regression/test.sh")
# FILE(COPY "${PROJECT_SOURCE_DIR}/test.sh" 
#   DESTINATION "${CMAKE_BINARY_DIR}/regression")
# FILE(REMOVE_RECURSE DESTINATION "${CMAKE_BINARY_DIR}/regression/test_mhx.sh")
# FILE(COPY "${PROJECT_SOURCE_DIR}/test_mhx.sh" 
#   DESTINATION "${CMAKE_BINARY_DIR}/regression")


# Binary to Run Regression Tests
ADD_EXECUTABLE(regression regression.cc)
TARGET_LINK_LIBRARIES(regression helpers sqlite_loader binary_loader data_service)
SET_TARGET_PROPERTIES(regression PROPERTIES 
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/regression")

# This is an ALL target - will be invoked for every build.
ADD_CUSTOM_TARGET(regression_suite ALL)
ADD_CUSTOM_COMMAND(
  TARGET regression_suite POST_BUILD
  # MH4G regression tests.
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/regression/mh4g"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/queries/mh4g" "${CMAKE_BINARY_DIR}/regression/mh4g"
  # MHX regression tests.
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${CMAKE_BINARY_DIR}/regression/mhx"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/queries/mhx" "${CMAKE_BINARY_DIR}/regression/mhx"
  COMMENT "Copying regression queries.")
