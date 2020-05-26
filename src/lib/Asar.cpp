#include "Asar.hpp"
#include "AsarError.hpp"
#include "AsarFileSystem.hpp"

#include "toyo/process.hpp"
#include "toyo/charset.hpp"
#include "pickle/pickle.hpp"
#include "oid/oid.hpp"

#include <regex>
#include <fstream>

namespace asar {

toyo::path::env_paths Asar::envpaths = toyo::path::env_paths::create("libasar");

void Asar::copyDirectory(const std::string s, const std::string& d, asar_transform_callback_t transform) {
  std::string source = toyo::path::resolve(s);
  std::string dest = toyo::path::resolve(d);

  if (source == dest) {
    return;
  }

  toyo::fs::stats stat = toyo::fs::lstat(source);

  if (stat.is_directory()) {
    if (toyo::path::relative(s, d).find("..") != 0) {
      throw AsarError(invalid_path, std::string("Cannot copy a directory into itself. copy \"") + s + "\" -> \"" + d + "\"");
    }
    toyo::fs::mkdirs(dest);
    std::vector<std::string> items = toyo::fs::readdir(source);
    for (size_t i = 0; i < items.size(); i++) {
      toyo::fs::copy(toyo::path::join(source, items[i]), toyo::path::join(dest, items[i]), transform);
    }
  } else {
    Asar::copyFile(source, dest, transform);
  }
}

void Asar::copyFile(const std::string s, const std::string& d, asar_transform_callback_t transform) {
  std::string source = toyo::path::resolve(s);
  std::string dest = toyo::path::resolve(d);
  std::string errmessage = "copy \"" + s + "\" -> \"" + d + "\"";

  if (source == dest) {
    return;
  }

  int mode;
  try {
    mode = toyo::fs::stat(source).mode;
  } catch (const std::exception&) {
    throw AsarError(invalid_path, errmessage);
  }

#ifdef _WIN32
  FILE* sf = ::_wfopen(toyo::charset::a2w(source).c_str(), L"rb+");
#else
  FILE* sf = ::fopen(source.c_str(), "rb+");
#endif
  if (!sf) {
    throw AsarError(invalid_path, errmessage);
  }
#ifdef _WIN32
  FILE* df = ::_wfopen(toyo::charset::a2w(dest).c_str(), L"wb+");
#else
  FILE* df = ::fopen(dest.c_str(), "wb+");
#endif
  if (!df) {
    ::fclose(sf);
    throw AsarError(invalid_path, errmessage);
  }
  uint8_t buf[128 * 1024];
  size_t read;
  while ((read = ::fread(buf, sizeof(uint8_t), 128 * 1024, sf)) > 0) {
    ::fwrite(buf, sizeof(uint8_t), read, df);
    ::fflush(df);
  }
  ::fclose(sf);
  ::fclose(df);

  toyo::fs::chmod(dest, mode);

  if (transform != nullptr) {
    transform(source.c_str(), dest.c_str());
  }
}

Asar::HeaderInfo Asar::createHeaderInfo(
  const std::string& dir,
  const char* unpack,
  uint64_t* offset,
  uint64_t* totalSize,
  std::vector<FileInfo>* files,
  const std::string& root) {
  
  std::vector<FileInfo> flist;
  uint64_t poffset = 0;
  uint64_t ptotalSize = 0;
  if (offset == nullptr) {
    offset = &poffset;
  }
  if (totalSize == nullptr) {
    totalSize = &ptotalSize;
  }
  if (files == nullptr) {
    files = &flist;
  }

  AsarFileSystem asarFs;

  std::string rootDir = root == "" ? dir : root;

  auto walkHandler = [&](const std::string& path) {
    std::regex re("\\\\");
    
    std::string pathInAsar = std::regex_replace(toyo::path::sep + toyo::path::relative(dir, path), re, "/");
    std::string pathInAsarFull = std::regex_replace(toyo::path::sep + toyo::path::relative(rootDir, path), re, "/");
    auto stat = toyo::fs::lstat(path);

    if (stat.is_directory()) {
      AsarFileSystemDirectoryNode node;
      node.json = Asar::createHeaderInfo(path, unpack, offset, totalSize, files, rootDir).fs.get();
      asarFs.insertNode(pathInAsar, node);
    } else {
      Json::Value node;
      node["size"] = stat.size;

      if (stat.is_symbolic_link()) {
        auto link = toyo::path::relative(toyo::fs::realpath(rootDir), toyo::fs::realpath(path));
        if (link.substr(0, 2) == "..") {
          throw AsarError(invalid_path, link + ": file links out of the package");
        }
        node.removeMember("size");
        node["link"] = link;
      }

      if (unpack != nullptr) {
        std::smatch sm;
        std::regex unpackRegex(unpack);
        if (std::regex_match(pathInAsarFull, sm, unpackRegex)) {
          node["unpacked"] = true;
        }
      }

      if (!node.isMember("link") && ((toyo::process::platform() == "win32" && toyo::path::extname(path) == ".exe") || (toyo::process::platform() != "win32" && (stat.mode & 0100)))) {
        node["executable"] = true;
      }

      if (!node.isMember("unpacked") && !node.isMember("link")) {
        node["offset"] = std::to_string(*offset);
        *offset = *offset + stat.size;
      }
      AsarFileSystemNode asarnode;
      asarnode.json = node;
      asarFs.insertNode(pathInAsar, asarnode);
      totalSize += (node.isMember("link") ? 0 : stat.size);

      Asar::FileInfo fileinfo;
      fileinfo.path = path;
      fileinfo.size = stat.size;
      fileinfo.unpacked = node.isMember("unpacked") ? node["unpacked"].asBool() : false;
      fileinfo.symlink = node.isMember("link") ? (node["link"].asString() != "") : false;

      files->push_back(fileinfo);
    }
  };

  walkDir(dir, walkHandler);

  Asar::HeaderInfo info;
  info.fs = asarFs;
  info.files = *files;
  info.size = *totalSize;
  return info;
}

void Asar::pack(
  const std::string& src,
  const std::string& dest,
  const char* unpack,
  asar_transform_callback_t transform
) {
  std::string tmpsrc = toyo::path::join(envpaths.temp, ObjectId().toHexString());

  Asar::copyDirectory(src, tmpsrc, transform);

  auto info = createHeaderInfo(tmpsrc, unpack);
  std::string headerString = info.fs.toJson();

  Pickle headerPickle;
  headerPickle.WriteString(headerString);

  Pickle sizePickle;
  sizePickle.WriteUInt32(headerPickle.size());

  toyo::fs::mkdirs(toyo::path::dirname(dest));

  std::ofstream out;
#ifdef _WIN32
  out.open(toyo::charset::a2w(dest), std::wios::binary | std::wios::out | std::wios::trunc);
#else
  out.open(dest, std::ios::binary | std::ios::out | std::ios::trunc);
#endif

  if (!out.is_open()) {
    throw AsarError(file_error, "Open file failed.");
  }

  out.write(reinterpret_cast<const char*>(sizePickle.data()), sizePickle.size());
  out.write(reinterpret_cast<const char*>(headerPickle.data()), headerPickle.size());

  for (size_t i = 0; i < info.files.size(); i++) {
    const auto& file = info.files[i];
    if (!file.unpacked) {
      if (file.symlink) {
        continue;
      }

      std::ifstream in;
#ifdef _WIN32
      in.open(toyo::charset::a2w(file.path), std::wios::binary | std::wios::in);
#else
      in.open(file.path, std::ios::binary | std::ios::in);
#endif
      if (!in.is_open()) {
        out.close();
        throw AsarError(file_error, "Open file failed.");
      }

      char buf[128 * 1024];

      while (!in.eof()) {
        in.read(buf, 128 * 1024);
        std::streamsize readCount = in.gcount();
        out.write(buf, readCount);
        out.flush();
      }
      in.close();
    } else {
      std::string target = toyo::path::join(dest + ".unpacked", toyo::path::relative(tmpsrc, file.path));
      toyo::fs::mkdirs(toyo::path::dirname(target));
      toyo::fs::copy_file(file.path, target);
    }
  }

  out.close();
}

Asar::~Asar() {
  if (this->_fd != nullptr) {
    ::fclose(this->_fd);
    this->_fd = nullptr;
  }
  if (this->_tmp != "") {
    try {
      toyo::fs::remove(this->_tmp);
    } catch (const std::exception&) {}
  }
}

Asar::Asar() {
  this->_init();
}

void Asar::open(const std::string& asarPath) {
  if (this->_fd != nullptr) {
    this->close();
  }

#ifdef _WIN32
  this->_fd = ::_wfopen(toyo::charset::a2w(asarPath).c_str(), L"rb+");
#else
  this->_fd = ::fopen(asarPath.c_str(), "rb+");
#endif
  // ::fseek(this->_fd, 0, SEEK_SET);
  if (this->_fd == NULL) {
    throw AsarError(file_error, "Open asar file failed: " + asarPath);
  }

  this->_src = asarPath;

  this->_tmp = toyo::path::join(envpaths.temp, toyo::path::basename(asarPath) + "_" + ObjectId().toHexString());

  this->_readInfo();
}

void Asar::_readInfo() {
  // this->_asarCheck();
  bool r = false;
  char headerSize[8];
  ::fread(headerSize, 1, 8, this->_fd);
  Pickle pickle(headerSize, 8);
  PickleIterator it(pickle);

  uint32_t uHeaderSize = 0;
  r = pickle.ReadUInt32(&it, &uHeaderSize);
  this->_headerSize = uHeaderSize;

  if (!r) {
    throw AsarError(invalid_asar, "Invalid asar file. Read header size failed.");
  }

  char* headerString = new char[uHeaderSize];
  memset(headerString, 0, uHeaderSize);
  ::fread(headerString, 1, uHeaderSize, this->_fd);
  // _begin = ::ftell(this->_fd);

  Pickle pickle2(headerString, uHeaderSize);
  PickleIterator it2(pickle2);
  std::string strHeader;
  it2.ReadString(&strHeader);
  delete[] headerString;

  std::istringstream is(strHeader);
  Json::Value headerJson;
  is >> headerJson;
  this->_fs = AsarFileSystem(headerJson);

  try {
    this->_fileSize = toyo::fs::lstat(this->_src).size;
  } catch (const std::exception&) {
    throw AsarError(invalid_asar, "Read file size failed.");
  }
}

void Asar::close() {
  this->~Asar();
  this->_init();
}

void Asar::_init(const std::string& src, uint32_t headerSize, uint64_t fileSize, AsarFileSystem* fs, const std::string& tmp) {
  this->_fd = nullptr;
  this->_src = src;
  this->_headerSize = headerSize;
  this->_fileSize = fileSize;
  if (fs != nullptr) this->_fs = *fs;
  this->_tmp = tmp;
}

bool Asar::isOpen() const {
  return this->_fd != nullptr;
}

std::string Asar::getTempDir() const {
  return this->_tmp;
}
std::string Asar::getTempPath(const std::string& path) const {
  return toyo::path::join(this->_tmp, path);
}
std::string Asar::getSrc() const {
  return this->_src;
}

uint64_t Asar::getFileSize() const {
  return this->_fileSize;
}

uint32_t Asar::getHeaderSize() const {
  return this->_headerSize;
}

std::string Asar::getHeaderJsonString(bool format) const {
  return this->_fs.toJson(format);
}

bool Asar::exists(const std::string& path) const {
  return this->_fs.exists(path);
}

std::vector<std::string> Asar::readdir(const std::string& path) const {
  return this->_fs.readdir(path);
}

Json::Value Asar::getNode(const std::string& path) const {
  return this->_fs.getNode(path);
}

std::vector<uint8_t> Asar::readFile(const std::string& path) const {
  Json::Value node = this->_fs.getNode(path);
  if (node.isNull()) {
    throw AsarError(invalid_path, "No such file or directory: " + toyo::path::join(this->_src, path));
  }

  if (node.isMember("files")) {
    throw AsarError(invalid_path, "Illegal operation on a directory: " + toyo::path::join(this->_src, path));
  }

  if (node.isMember("unpacked")) {
    return toyo::fs::read_file(toyo::path::join(this->_src + ".unpacked", path));
  }
  uint32_t size = node["size"].asUInt();
  uint64_t offset = 8 + this->_headerSize + std::strtoull(node["offset"].asString().c_str(), nullptr, 10);
  uint8_t* buf = new uint8_t[size];
  long curpos = ::ftell(this->_fd);
  ::fseek(this->_fd, offset, SEEK_SET);
  ::fread(buf, 1, size, this->_fd);
  ::fseek(this->_fd, curpos, SEEK_SET);
  std::vector<uint8_t> res(buf, buf + size);
  delete buf;
  return res;
}

std::vector<std::string> Asar::list() const {
  std::vector<std::string> res;
  std::regex re("\\\\");
  this->walk(this->_fs.get(), [&](const Json::Value&, const std::string& name) {
    res.push_back(std::regex_replace(name, re, "/"));
    return true;
  });
  return res;
}

void Asar::extract(const std::string& p, const std::string& dest) const {
  std::regex re("\\\\");
  std::string path = std::regex_replace(p, re, "/");

  Json::Value node = this->_fs.getNode(path);
  if (node.isNull()) {
    throw AsarError(invalid_path, "No such file or directory: " + toyo::path::join(this->_src, path));
  }

  std::string target = toyo::path::join(dest, toyo::path::basename(path));

  if (node.isMember("files")) {
    auto files = node["files"].getMemberNames();
    for (const std::string& name : files) {
      this->extract(toyo::path::join(path, name), target);
    }
    return;
  }

  auto dir = toyo::path::dirname(target);
  if (!toyo::fs::exists(dir)) toyo::fs::mkdirs(dir);

  if (node.isMember("unpacked")) {
    toyo::fs::copy_file(toyo::path::join(this->_src + ".unpacked", path), target);
    return;
  }

  if (node.isMember("link")) {
    auto link = node["link"].asString();
    if (toyo::process::platform() == "win32") {
      this->extract(link, dest);
      toyo::fs::rename(toyo::path::join(dest, toyo::path::basename(link)), target);
      return;
    }

    auto destFilename = toyo::path::join(dest, path);

    auto linkSrcPath = toyo::path::dirname(toyo::path::join(dest, link));
    auto linkDestPath = toyo::path::dirname(destFilename);
    auto relativePath = toyo::path::relative(linkDestPath, linkSrcPath);

    toyo::fs::unlink(destFilename);
    auto linkTo = toyo::path::join(relativePath, toyo::path::basename(link));

    toyo::fs::symlink(linkTo, destFilename);
    return;
  }

#ifdef _WIN32
  FILE* df = ::_wfopen(toyo::charset::a2w(target).c_str(), L"wb+");
#else
  FILE* df = ::fopen(target.c_str(), "wb+");
#endif
  if (!df) {
    throw AsarError(invalid_path, "Cannot write target file.");
  }

  uint8_t buf[128 * 1024];
  long curpos = ::ftell(this->_fd);
  size_t read;
  uint32_t size = node["size"].asUInt();
  uint64_t offset = 8 + this->_headerSize + std::strtoull(node["offset"].asString().c_str(), nullptr, 10);
  ::fseek(this->_fd, offset, SEEK_SET);
  size_t total = 0;
  while ((read = ::fread(buf, sizeof(uint8_t), 128 * 1024, this->_fd)) > 0) {
    total += read;

    if (total >= size) {
      ::fwrite(buf, sizeof(uint8_t), read - (total - size), df);
      ::fflush(df);
    } else {
      ::fwrite(buf, sizeof(uint8_t), read, df);
      ::fflush(df);
    }
  }

  ::fclose(df);
}

} // namespace asar
