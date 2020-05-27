cmake_minimum_required(VERSION 3.6)

project(asar VERSION 1.0.0)

set(LIB_NAME asar)
set(EXE_NAME asarcpp)
set(TEST_EXE_NAME asartest)

# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(cmake/lib.cmake)
include(cmake/exe.cmake)
include(cmake/npm.cmake)

set(ORIGINAL_DLL ${CCPM_BUILD_DLL})
set(CCPM_BUILD_DLL false)
dp_require("@ccpm/oid")
set(CCPM_BUILD_DLL ${ORIGINAL_DLL})

dp_require("@ccpm/toyo")
dp_require("@ccpm/pickle")

if(CCPM_BUILD_DLL AND NOT MSVC)
  target_compile_options(toyo PRIVATE -fPIC)
  target_compile_options(pickle PRIVATE -fPIC)
endif()

target_link_libraries(${LIB_NAME} oid toyo pickle)

if(CCPM_BUILD_TEST)
  include(cmake/test.cmake)
endif()
