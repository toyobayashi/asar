file(GLOB_RECURSE TEST_SOURCE_FILES "test/*.c" "test/*.cpp")

add_executable(${TEST_EXE_NAME}
  ${TEST_SOURCE_FILES}
)

set_target_properties(${TEST_EXE_NAME} PROPERTIES CXX_STANDARD 11)

target_link_libraries(${TEST_EXE_NAME} ${LIB_NAME})

target_compile_definitions(${TEST_EXE_NAME} PRIVATE
  ASAR_INPUT_1="${CMAKE_CURRENT_SOURCE_DIR}/test/input/packthis"
  ASAR_OUTPUT_1="${CMAKE_CURRENT_SOURCE_DIR}/test/output/packthis.asar"
  ASAR_OUTPUT_2="${CMAKE_CURRENT_SOURCE_DIR}/test/output/packthis-unpack.asar"
  ASAR_OUTPUT_3="${CMAKE_CURRENT_SOURCE_DIR}/test/output/packthis-transformed.asar"
  ASAR_EXTRACT_1="${CMAKE_CURRENT_SOURCE_DIR}/test/output/unpack"
)

if(WIN32 AND MSVC)
  set_directory_properties(PROPERTIES VS_STARTUP_PROJECT ${TEST_EXE_NAME})
  # set_target_properties(${TEST_EXE_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_compile_options(${TEST_EXE_NAME} PRIVATE /utf-8)
  target_compile_definitions(${TEST_EXE_NAME} PRIVATE
    _CRT_SECURE_NO_WARNINGS
    UNICODE
    _UNICODE
  )
  if(CCPM_BUILD_DLL)
    target_link_options(${TEST_EXE_NAME} PRIVATE /ignore:4199 /DELAYLOAD:${LIB_NAME}.dll)
  endif()
endif()
