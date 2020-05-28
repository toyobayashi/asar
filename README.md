# asar

C++ implemention of [asar](https://github.com/electron/asar).

## Usage

```
Usage: asarcpp [options] [command]

Manipulate asar archive files

Options:
  -V, --version                             output the version number
  -h, --help                                display help for command

Commands:
  pack|p [-u <glob>] <dir> <output>         create asar archive
  list|l <archive>                          list files of asar archive
  extract|e [-p <path>] <archive> <dest>    extract files from archive
```

## Build

Require Node.js, CMake, VC++ / GCC

Windows:

``` cmd
> npm install
> .\build.bat Win32 Release static
```

Linux / macOS:

``` bash
$ npm install
$ ./build.sh Release
```

## Example

C API:

``` cpp
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
  char output[] = "./test/output/packthis-unpack.asar";
  asar_pack("./test/input/packthis", output, "*.png", transform);

  asar_t* asar = asar_open(output);
  uint32_t header_size = asar_get_header_size(asar);
  int jsonlen = asar_get_header_json_string(asar, 1, NULL, 0);
  char* buf = (char*)malloc(jsonlen + 1);
  memset(buf, 0, jsonlen + 1);
  asar_get_header_json_string(asar, 1, buf, jsonlen + 1);
  printf("header:\n%s\n", buf);
  free(buf);
  asar_list(asar);
  asar_extract(asar, "/", "./test/output/unpack");
  asar_close(asar);
  return 0;
}
```

C++ API

``` cpp
#include "asar/Asar.hpp"
#include "asar/AsarError.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <exception>

int main() {
  std::string output = "./test/output/packthis-unpack.asar";
  try {
    asar::Asar::pack("./test/input/packthis", output, "*.png", transform);

    asar::Asar asar;
    asar.open(output);
    uint32_t header_size = asar.getHeaderSize();
    std::cout << asar.getHeaderJsonString(true) << "\n";
    std::vector<std::string> ls = asar.list();
    asar.extract("/", "./test/output/unpack");
    asar.close();
  } catch (const asar::AsarError& e) {
    // ...
    return 1;
  } catch (const std::exception& err) {
    // ...
    return 1;
  }
  
  return 0;
}
```
