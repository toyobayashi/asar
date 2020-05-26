#include <cstring>

#include "json/json.h"
#include "asar/AsarFileSystem.hpp"
#include "AsarError.hpp"

#include <cstddef>
#include <cstdio>

#include "toyo/path.hpp"
#include "toyo/fs.hpp"
#include "toyo/charset.hpp"

namespace asar {

AsarFileSystemNode::~AsarFileSystemNode() = default;

AsarFileSystemDirectoryNode::AsarFileSystemDirectoryNode(): AsarFileSystemNode() {
  this->json["files"] = Json::Value(Json::objectValue);
}

AsarFileSystemFileNode::AsarFileSystemFileNode(uint32_t size, uint64_t offset): AsarFileSystemNode() {
  this->json["size"] = size;
  this->json["offset"] = std::to_string(offset);
}

AsarFileSystemFileNode::AsarFileSystemFileNode(uint32_t size, uint64_t offset, bool unpacked): AsarFileSystemNode() {
  this->json["size"] = size;
  this->json["offset"] = std::to_string(offset);
  this->json["unpacked"] = unpacked;
}

AsarFileSystemFileNode::AsarFileSystemFileNode(uint32_t size, uint64_t offset, bool unpacked, bool executable): AsarFileSystemNode() {
  this->json["size"] = size;
  this->json["offset"] = std::to_string(offset);
  this->json["unpacked"] = unpacked;
  this->json["executable"] = executable;
}

AsarFileSystemFileNode::AsarFileSystemFileNode(const std::string& link): AsarFileSystemNode() {
  json["link"] = link;
}

std::vector<std::string> AsarFileSystem::split(const std::string& self, const std::string& separator, int limit) {
  std::string copy = self;
  char* copyBuf = new char[copy.size() + 1];
  memset(copyBuf, 0, (copy.size() + 1) * sizeof(char));
  strcpy(copyBuf, copy.c_str());
#ifdef _MSC_VER
  char* tokenPtr = strtok(copyBuf, separator.c_str());
#else
  char* buffer;
  char* tokenPtr = strtok_r(copyBuf, separator.c_str(), &buffer);
#endif
  std::vector<std::string> res;
  while (tokenPtr != NULL && (limit == -1 ? true : ((int)res.size()) < limit)) {
    res.push_back(tokenPtr);
#ifdef _MSC_VER
    tokenPtr = strtok(NULL, separator.c_str());
#else
    tokenPtr = strtok_r(NULL, separator.c_str(), &buffer);
#endif
  }
  if (copyBuf[copy.size() - 1] == '\0') {
    res.push_back("");
  }
  delete[] copyBuf;
  return res;
}

AsarFileSystem::AsarFileSystem(): header() {
  header["files"] = Json::Value(Json::objectValue);
}

AsarFileSystem::AsarFileSystem(const Json::Value& h): header() {
  if (!h.isObject()) throw AsarError(invalid_header, "Invalid header.");
  auto keys = h.getMemberNames();
  if (keys.size() != 1 || keys[0] != "files") {
    throw AsarError(invalid_header, "Invalid header.");
  }

  header = h;
}

const Json::Value& AsarFileSystem::get() const {
  return this->header;
}

void AsarFileSystem::insertNode(const std::string& path, const AsarFileSystemNode& node) {
  if (path == "") throw AsarError(invalid_path, "Cannot insert empty path.");
  std::string p = toyo::path::join(path);

  if (p[0] == '/' || p[0] == '\\') p = p.substr(1);
  if (p == "" || p == ".") throw AsarError(invalid_path, "Cannot insert root path.");

  auto paths = split(p, toyo::path::sep);
  Json::Value* pointer = &(this->header["files"]);

  std::string currentName;
  for (size_t i = 0; i < paths.size() - 1; i++) {
    currentName = paths[i];
    if (pointer->isMember(currentName)) {
      if (pointer->operator[](currentName).isMember("files")) {
        pointer = &(pointer->operator[](currentName)["files"]);
      } else {
        throw AsarError(invalid_path, "Invalid path: " + p);
      }
    } else {
      Json::Value json;
      json["files"] = Json::Value(Json::objectValue);
      pointer->operator[](currentName) = json;
      pointer = &(pointer->operator[](currentName)["files"]);
    }
  }

  std::string basename = paths[paths.size() - 1];
  if (pointer->isMember(basename)) {
    throw AsarError(invalid_path, "Existing path: " + p);
  }
  pointer->operator[](basename) = node.json;
}

void AsarFileSystem::removeNode(const std::string& path) {
  if (path == "") throw AsarError(invalid_path, "Cannot remove empty path.");
  std::string p = toyo::path::join(path);

  if (p[0] == '/' || p[0] == '\\') p = p.substr(1);
  if (p == "" || p == ".") {
    this->header["files"] = Json::Value(Json::objectValue);
    return;
  }

  auto paths = split(p, toyo::path::sep);
  Json::Value* pointer = &(this->header["files"]);

  std::string currentName;
  for (size_t i = 0; i < paths.size() - 1; i++) {
    currentName = paths[i];
    if (pointer->isMember(currentName)) {
      if (pointer->operator[](currentName).isMember("files")) {
        pointer = &(pointer->operator[](currentName)["files"]);
      } else {
        return;
      }
    } else {
      return;
    }
  }

  std::string basename = paths[paths.size() - 1];
  if (pointer->isMember(basename)) {
    pointer->removeMember(basename);
  }
}

Json::Value AsarFileSystem::getNode(const std::string& path) const {
  if (path == "") return Json::Value(Json::nullValue);
  std::string p = toyo::path::join(path);

  if (p[0] == '/' || p[0] == '\\') p = p.substr(1);
  if (p == "" || p == ".") return this->header;

  auto paths = split(p, toyo::path::sep);
  const Json::Value* pointer = &(this->header["files"]);

  std::string currentName;
  for (size_t i = 0; i < paths.size() - 1; i++) {
    currentName = paths[i];
    if (pointer->isMember(currentName)) {
      if (pointer->operator[](currentName).isMember("files")) {
        pointer = &(pointer->operator[](currentName)["files"]);
      } else {
        return Json::Value(Json::nullValue);
      }
    } else {
      return Json::Value(Json::nullValue);
    }
  }

  std::string basename = paths[paths.size() - 1];
  if (pointer->isMember(basename)) {
    return pointer->operator[](basename);
  }
  
  return Json::Value(Json::nullValue);
}

std::string AsarFileSystem::toJson(bool format) const {
  Json::StreamWriterBuilder wb;
  wb.settings_["emitUTF8"] = true;
  wb.settings_["indentation"] = format ? "  " : "";
  return Json::writeString(wb, this->header);
}

bool AsarFileSystem::exists(const std::string& path) const {
  Json::Value node = this->getNode(path);
  return !node.isNull();
}

std::vector<std::string> AsarFileSystem::readdir(const std::string& path) const {
  Json::Value node = this->getNode(path);
  if (node.isNull()) {
    throw AsarError(invalid_path, "No such directory: " + path);
  }
  if (!node.isMember("files")) {
    throw AsarError(invalid_path, "Not a directory: " + path);
  }
  return node.getMemberNames();
}

}
