#include <stdio.h>

#include "asar/asar.h"
// #include "toyo/path.hpp"

int main() {
  int r = asar_pack("C:\\Projects\\cpp\\asar\\test\\pkg", "C:\\Projects\\cpp\\asar\\test\\pkg.asar", NULL, NULL);
  printf("%s\n", asar_get_last_error_message());

  asar_t* asar = asar_open("C:\\Projects\\cpp\\asar\\test\\pkg.asar");
  asar_close(asar);
  printf("%s\n", asar_get_last_error_message());

  // asar::AsarFileSystem fs;
  // fs.insertNode("a/b/ccc/ddd.exe", asar::AsarFileSystemFileNode(375, 0, false, true));
  // fs.insertNode("a/b/ccc/x.txt", asar::AsarFileSystemFileNode(184, 375));
  // std::cout << fs.exists("a/b/ccc/ddd") << std::endl;
  // std::cout << fs.exists("a/b/ccc/x.txt") << std::endl;
  // std::cout << fs.toJson() << std::endl;
  return 0;
}
