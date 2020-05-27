file(GLOB_RECURSE LIB_SOURCE_FILES "src/lib/*.cpp" "src/lib/*.c")

if(CCPM_BUILD_DLL)
  if(WIN32 AND MSVC)
    set(NODEV_VERSIONINFO_RC "${CMAKE_CURRENT_BINARY_DIR}/asar.rc")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/lib/asar.rc.in" "${NODEV_VERSIONINFO_RC}")
  else()
    set(NODEV_VERSIONINFO_RC "")
  endif()

  add_library(${LIB_NAME} SHARED
    ${LIB_SOURCE_FILES}
    ${NODEV_VERSIONINFO_RC}
  )

  target_compile_definitions(${LIB_NAME} PRIVATE "CCPM_BUILD_DLL_${LIB_NAME}")

  if(NOT MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath=$ORIGIN")
  endif()
else()
  add_library(${LIB_NAME} STATIC
    ${LIB_SOURCE_FILES}
  )
endif()

set_target_properties(${LIB_NAME} PROPERTIES CXX_STANDARD 11)

set(BUILD_SHARED_LIBS OFF)
set(BUILD_STATIC_LIBS ON)
set(JSONCPP_WITH_TESTS OFF)
add_subdirectory(deps/jsoncpp)

target_link_libraries(${LIB_NAME} jsoncpp_lib)

# target_include_directories(${TEST_EXE_NAME} PRIVATE "deps/jsoncpp/include")

# set_target_properties(${LIB_NAME} PROPERTIES PREFIX "lib")

if(WIN32 AND MSVC)
  # set_target_properties(${LIB_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_compile_options(${LIB_NAME} PRIVATE /utf-8)
  target_compile_definitions(${LIB_NAME} PRIVATE
    _CRT_SECURE_NO_WARNINGS
    UNICODE
    _UNICODE
  )
else()
  if(CCPM_BUILD_DLL)
    target_compile_options(${LIB_NAME} PUBLIC -fPIC)
  endif()
endif()

target_include_directories(${LIB_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
