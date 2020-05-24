cmake_minimum_required(VERSION 3.6)

project(asar)

set(LIB_NAME asar)
set(EXE_NAME asarmain)
set(TEST_EXE_NAME asartest)

# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(cmake/lib.cmake)
include(cmake/exe.cmake)
include(cmake/npm.cmake)

dp_require("@ccpm/oid")
dp_require("@ccpm/toyo")
dp_require("@ccpm/pickle")
target_link_libraries(${LIB_NAME} oid toyo pickle)

if(CCPM_BUILD_TEST)
  include(cmake/test.cmake)
endif()

