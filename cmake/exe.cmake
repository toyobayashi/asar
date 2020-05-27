file(GLOB_RECURSE EXE_SOURCE_FILES "src/bin/*.cpp" "src/bin/*.c")

if(WIN32 AND MSVC)
  set(NODEV_VERSIONINFO_RC "${CMAKE_CURRENT_BINARY_DIR}/asarcpp.rc")
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/bin/asarcpp.rc.in" "${NODEV_VERSIONINFO_RC}")
else()
  set(NODEV_VERSIONINFO_RC "")
endif()

add_executable(${EXE_NAME} ${EXE_SOURCE_FILES} ${NODEV_VERSIONINFO_RC})

set_target_properties(${EXE_NAME} PROPERTIES CXX_STANDARD 11)

target_compile_definitions(${EXE_NAME}
  PRIVATE ASAR_VERSION="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
)

if(LIB_NAME)
  target_link_libraries(${EXE_NAME} ${LIB_NAME})
endif()

if(WIN32 AND MSVC)
  # set_target_properties(${EXE_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_compile_options(${EXE_NAME} PRIVATE /utf-8)
  target_compile_definitions(${EXE_NAME} PRIVATE
    _CRT_SECURE_NO_WARNINGS
    UNICODE
    _UNICODE
  )
  if(CCPM_BUILD_DLL AND LIB_NAME)
    target_link_options(${EXE_NAME} PRIVATE /ignore:4199 /DELAYLOAD:${LIB_NAME}.dll)
  endif()
endif()
