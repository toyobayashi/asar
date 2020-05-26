#ifndef __ASAR_FILE_SYSTEM_HPP__
#define __ASAR_FILE_SYSTEM_HPP__

#include <string>
#include <vector>
#include <cstdint>

#include "json/json.h"

namespace asar {

class AsarFileSystemNode {
 public:
  Json::Value json;
  virtual ~AsarFileSystemNode();
};

class AsarFileSystemFileNode : public AsarFileSystemNode {
 public:
  AsarFileSystemFileNode(uint32_t size, uint64_t offset);
  AsarFileSystemFileNode(uint32_t size, uint64_t offset, bool unpacked);
  AsarFileSystemFileNode(uint32_t size, uint64_t offset, bool unpacked, bool executable);
  AsarFileSystemFileNode(const std::string& link);
};

class AsarFileSystemDirectoryNode : public AsarFileSystemNode {
 public:
  AsarFileSystemDirectoryNode();
};

typedef std::vector<uint8_t> (*TransformCallback)(const std::string& path, const uint8_t* buffer, size_t size, bool* doTransform);

class AsarFileSystem {
 public:
  AsarFileSystem();
  AsarFileSystem(const Json::Value&);
  void insertNode(const std::string& path, const AsarFileSystemNode& node);
  void removeNode(const std::string& path);
  Json::Value getNode(const std::string& path) const;

  std::string toJson(bool format = false) const;

  bool exists(const std::string& path) const;
  std::vector<std::string> readdir(const std::string& path) const;
  const Json::Value& get() const;
 private:
  Json::Value header;

  static std::vector<std::string> split(const std::string& self, const std::string& separator, int limit = -1);
};

}

#endif
