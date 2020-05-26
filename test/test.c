#ifdef _WIN32
#include <Windows.h>
#include <wchar.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "asar/asar.h"

static void transform(const char* src, const char* tmp_path) {
  printf("src: %s\n", src);
  printf("tmp: %s\n", tmp_path);
#ifdef _WIN32
  wchar_t w[260];
  MultiByteToWideChar(CP_UTF8, 0, tmp_path, -1, w, 260);
  FILE* sf = _wfopen(w, L"rb+");
#else
  FILE* sf = fopen(tmp_path, "rb+");
#endif
  fseek(sf, 0, SEEK_END);
  fwrite("append", 1, 6, sf);
  fflush(sf);
  fclose(sf);
}

int main() {
  // asar_pack(ASAR_INPUT_1, ASAR_OUTPUT_1, NULL, NULL);
  asar_pack(ASAR_INPUT_1, ASAR_OUTPUT_2, "^.*\\.png$", NULL);
  // asar_pack(ASAR_INPUT_1, ASAR_OUTPUT_3, NULL, transform);

  asar_t* asar = asar_open(ASAR_OUTPUT_2);
  uint32_t header_size = asar_get_header_size(asar);
  int jsonlen = asar_get_header_json_string(asar, 1, NULL, 0);
  char* buf = (char*)malloc(jsonlen + 1);
  memset(buf, 0, jsonlen + 1);
  asar_get_header_json_string(asar, 1, buf, jsonlen + 1);
  printf("header:\n%s\n", buf);
  free(buf);
  asar_list(asar);
  asar_extract(asar, "/", ASAR_EXTRACT_1);
  asar_close(asar);
  return 0;
}
