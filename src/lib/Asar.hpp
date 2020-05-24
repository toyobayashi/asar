#ifndef __ASAR_HPP__
#define __ASAR_HPP__

#include <string>
#include <vector>
#include <regex>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include "json/json.h"
#include "toyo/fs.hpp"
#include "toyo/path.hpp"
#include "AsarFileSystem.hpp"
#include "asar/asar.h"

namespace asar {

class Asar {
 private:
  FILE* _fd;
  std::string _src;
  uint32_t _headerSize;
  uint64_t _fileSize;
  AsarFileSystem _fs;
  std::string _tmp;

  void _init(const std::string& src = "", uint32_t headerSize = 0, uint64_t fileSize = 0, AsarFileSystem* fs = nullptr, const std::string& tmp = "");
 public:
  ~Asar();
  Asar();
  Asar(const Asar&) = delete;
  Asar(Asar&&) = default;
  Asar& operator=(const Asar&) = delete;
  Asar& operator=(Asar&&) = default;
  void open(const std::string& asarPath);
  void close();
 private:
  static toyo::path::env_paths envpaths;

  struct FileInfo {
    std::string path;
    uint64_t size;
    bool unpacked;
    bool symlink;
  };
  class HeaderInfo {
   public:
    AsarFileSystem fs;
    std::vector<FileInfo> files;
    uint64_t size;
  };

  template <typename Callable>
  static void walkDir(const std::string& dir, const Callable& callback) {
    const auto& items = toyo::fs::readdir(dir);
    for (size_t i = 0; i < items.size(); i++) {
      if (items[i] != "." && items[i] != "..") {
        std::string full = toyo::path::join(dir, items[i]);
        callback(full);
      }
    }
  }

  static void copyDirectory(const std::string src, const std::string& dest, asar_transform_callback_t transform = nullptr);
  static void copyFile(const std::string src, const std::string& dest, asar_transform_callback_t transform = nullptr);

  static HeaderInfo createHeaderInfo(
    const std::string& dir,
    const char* unpack = nullptr,
    uint64_t* offset = nullptr,
    uint64_t* totalSize = nullptr,
    std::vector<FileInfo>* files = nullptr,
    const std::string& rootDir = "");
  
  void _readInfo();
 public:
  static void pack(
    const std::string& src,
    const std::string& dest,
    const char* unpack = nullptr,
    asar_transform_callback_t transform = nullptr
  );
};

}

#endif
