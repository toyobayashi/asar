#ifndef __ASAR_HPP__
#define __ASAR_HPP__


#include "asar.h"
#include "AsarFileSystem.hpp"
#include <cstddef>
#include <cstdio>

namespace toyo {
  namespace fs {
    std::vector<std::string> readdir(const std::string&);
  }

  namespace path {
    template <typename... Args>
    std::string join(Args... args);

    class env_paths;
  }
}

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
  bool isOpen() const;
  const std::string& getTempDir() const;
  std::string getTempPath(const std::string&) const;
  const std::string& getSrc() const;
  Json::Value getNode(const std::string&) const;
  uint64_t getFileSize() const;
  uint32_t getHeaderSize() const;
  std::string getHeaderJsonString(bool format = false) const;
  bool exists(const std::string&) const;
  std::vector<std::string> readdir(const std::string& path) const;
  std::vector<uint8_t> readFile(const std::string& path) const;
  std::vector<std::string> list() const;
  void extract(const std::string&, const std::string&) const;
  void extractTemp(const std::string&) const;
 private:
  
  template <typename Callable>
  void walk(const Json::Value& node, const Callable& callback, const std::string& path = "") const {
    if (callback(node, path)) {
      if (node.isMember("files")) {
        std::vector<std::string> keys = node["files"].getMemberNames();
        for (const std::string& name : keys) {
          this->walk(node["files"][name], callback, toyo::path::join(path, name));
        }
      }
    }
  }

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
