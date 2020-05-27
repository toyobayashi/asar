#ifndef __ASAR_H__
#define __ASAR_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
  #define EXTERN_C_START extern "C" {
  #define EXTERN_C_END }
#else
  #define EXTERN_C_START
  #define EXTERN_C_END
#endif

#ifdef _WIN32
  #ifdef CCPM_BUILD_DLL_asar
  #define ASAR_API __declspec(dllexport)
  #else
  // #define ASAR_API __declspec(dllimport)
  #define ASAR_API
  #endif
#else
  #ifdef CCPM_BUILD_DLL_asar
  #define ASAR_API __attribute__((visibility("default")))
  #else
  #define ASAR_API
  #endif
#endif

EXTERN_C_START

struct __asar_context;
typedef struct __asar_context asar_t;

typedef int boolean_t;
typedef struct asar_node_struct {
  boolean_t is_directory;
  uint32_t size;
  uint64_t offset;
  boolean_t unpacked;
  boolean_t executable;
  char link[260];
} asar_node_t;

typedef enum asar_status {
  ok,
  unknown,
  invalid_header,
  invalid_path,
  invalid_asar,
  not_dir,
  not_exists,
  not_file,
  null_node,
  file_error
} asar_status;

ASAR_API asar_t* asar_open(const char* asar_path);
ASAR_API void asar_close(asar_t*);
ASAR_API boolean_t asar_is_open(asar_t*);
ASAR_API const char* asar_get_temp_dir(asar_t*);
ASAR_API const char* asar_get_src(asar_t*);
ASAR_API uint32_t asar_get_header_size(asar_t*);
ASAR_API uint64_t asar_get_file_size(asar_t*);
ASAR_API int asar_get_header_json_string(asar_t*, boolean_t, char*, size_t);
ASAR_API asar_status asar_get_node(asar_t*, const char*, asar_node_t*); 
ASAR_API boolean_t asar_exists(asar_t*, const char*);
ASAR_API int asar_read_file(asar_t*, const char*, char*, size_t);
ASAR_API void asar_list(asar_t*);
ASAR_API asar_status asar_extract(asar_t*, const char*, const char*);
ASAR_API asar_status asar_extract_temp(asar_t*, const char*);

typedef void (*asar_transform_callback_t)(const char* src, const char* tmp_path);

ASAR_API asar_status asar_get_last_error_code();

ASAR_API const char* asar_get_last_error_message();

ASAR_API asar_status asar_pack(const char* src, const char* dest, const char* unpack, asar_transform_callback_t transform);

EXTERN_C_END

#endif
