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

asar_t* asar_open(const char* asar_path);
void asar_close(asar_t*);
int asar_is_open(asar_t*);
const char* asar_get_temp_dir(asar_t*);
const char* asar_get_src(asar_t*);
uint32_t asar_get_header_size(asar_t*);
uint64_t asar_get_file_size(asar_t*);

typedef void (*asar_transform_callback_t)(const char* src, const char* tmp_path);

typedef enum asar_status {
  ok,
  unknown,
  invalid_header,
  invalid_path,
  invalid_asar,
  not_dir,
  not_exists,
  not_file,
  file_error
} asar_status;

ASAR_API asar_status asar_get_last_error_code();

ASAR_API const char* asar_get_last_error_message();

ASAR_API int asar_pack(const char* src, const char* dest, const char* unpack, asar_transform_callback_t transform);

EXTERN_C_END

#endif
