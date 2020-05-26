#include "asar/asar.h"
#include "asar/Asar.hpp"
#include "AsarError.hpp"

#include <cstring>
#include <exception>

static asar_status code = ok;
static char msg[256] = { 0 };

static void asar__set_last_error(const asar::AsarError& err) {
  code = err.code();
  memset(msg, 0, sizeof(msg));
  strcpy(msg, err.what());
}

struct __asar_context {
  asar::Asar* impl;
};

asar_t* asar_open(const char* asar_path) {
  asar_t* asar = new asar_t;
  asar->impl = new asar::Asar;
  try {
    asar->impl->open(asar_path);
  } catch (const asar::AsarError& err) {
    asar__set_last_error(err);
    delete asar->impl;
    asar->impl = NULL;
    delete asar;
    return NULL;
  } catch (const std::exception& stdexpt) {
    code = unknown;
    memset(msg, 0, sizeof(msg));
    strcpy(msg, stdexpt.what());
    delete asar->impl;
    asar->impl = NULL;
    delete asar;
    return NULL;
  }
  return asar;
}

void asar_close(asar_t* asar) {
  if (asar == NULL) {
    return;
  }

  if (asar->impl != NULL) {
    delete asar->impl;
    asar->impl = NULL;
  }
  delete asar;
}

boolean_t asar_is_open(asar_t* asar) {
  return asar->impl->isOpen() ? 1 : 0;
}

const char* asar_get_temp_dir(asar_t* asar) {
  return asar->impl->getTempDir().c_str();
}

const char* asar_get_src(asar_t* asar) {
  return asar->impl->getSrc().c_str();
}

uint32_t asar_get_header_size(asar_t* asar) {
  return asar->impl->getHeaderSize();
}

uint64_t asar_get_file_size(asar_t* asar) {
  return asar->impl->getFileSize();
}

int asar_get_header_json_string(asar_t* asar, boolean_t format, char* out, size_t len) {
  std::string header = asar->impl->getHeaderJsonString(static_cast<bool>(format));
  size_t headerLength = header.size();
  if (out == nullptr) {
    return headerLength;
  }
  const char* headerCString = header.c_str();
  if (headerLength > len - 1) {
    strncpy(out, headerCString, len - 1);
    *(out + (len - 1)) = '\0';
    return len - 1;
  }
  strcpy(out, headerCString);
  return headerLength;
}

asar_status asar_get_node(asar_t* asar, const char* path, asar_node_t* out) {
  Json::Value node = asar->impl->getNode(path);
  if (node.isNull()) {
    code = not_exists;
    memset(msg, 0, sizeof(msg));
    strcpy(msg, ("Cannot get node: " + std::string(path)).c_str());
    return code;
  }

  if (node.isMember("files")) {
    out->is_directory = 1;
    out->size = 0;
    out->offset = 0;
    out->unpacked = 0;
    out->executable = 0;
    memset(out->link, 0, sizeof(out->link));
    return ok;
  }

  out->is_directory = 0;

  if (node.isMember("link")) {
    out->size = 0;
    out->offset = 0;
    out->unpacked = 0;
    out->executable = 0;
    memset(out->link, 0, sizeof(out->link));
    strcpy(out->link, node["link"].asString().c_str());
  } else {
    out->size = node.isMember("size") ? node["size"].asUInt() : 0;
    out->offset = node.isMember("offset") ? std::strtoull(node["offset"].asString().c_str(), nullptr, 10) : 0;
    out->unpacked = node.isMember("unpacked") ? (node["unpacked"].asBool() ? 1 : 0) : 0;
    out->executable = node.isMember("executable") ? (node["executable"].asBool() ? 1 : 0) : 0;
    memset(out->link, 0, sizeof(out->link));
  }

  return ok;
}

boolean_t asar_exists(asar_t* asar, const char* path) {
  return asar->impl->exists(path) ? 1 : 0;
}

int asar_read_file(asar_t* asar, const char* path, char* out, size_t len) {
  Json::Value node = asar->impl->getNode(path);
  if (node.isNull()) {
    code = not_exists;
    memset(msg, 0, sizeof(msg));
    strcpy(msg, ("Cannot get node: " + std::string(path)).c_str());
    return 0;
  }
  if (node.isMember("files")) {
    code = not_file;
    memset(msg, 0, sizeof(msg));
    strcpy(msg, ("Not a file: " + std::string(path)).c_str());
    return 0;
  }
  if (out == nullptr) {
    return node["size"].asInt();
  }
  auto buffer = asar->impl->readFile(path);
  auto size = buffer.size();
  if (size > len) {
    memcpy(out, buffer.data(), len);
    return len;
  }
  memcpy(out, buffer.data(), size);
  return size;
}

void asar_list(asar_t* asar) {
  auto ls = asar->impl->list();
  for (const auto& p : ls) {
    printf("%s\n", p.c_str());
  }
}

asar_status asar_extract(asar_t* asar, const char* path, const char* dest) {
  try {
    asar->impl->extract(path, dest);
  } catch (const asar::AsarError& err) {
    asar__set_last_error(err);
    return code;
  } catch (const std::exception& stdexpt) {
    code = unknown;
    memset(msg, 0, sizeof(msg));
    strcpy(msg, stdexpt.what());
    return code;
  }

  return ok;
}

asar_status asar_extract_temp(asar_t* asar, const char* path) {
  try {
    asar->impl->extractTemp(path);
  } catch (const asar::AsarError& err) {
    asar__set_last_error(err);
    return code;
  } catch (const std::exception& stdexpt) {
    code = unknown;
    memset(msg, 0, sizeof(msg));
    strcpy(msg, stdexpt.what());
    return code;
  }

  return ok;
}

asar_status asar_get_last_error_code() {
  return code;
}

const char* asar_get_last_error_message() {
  if (code == ok) {
    memset(msg, 0, sizeof(msg));
    strcpy(msg, "");
  }
  return msg;
}

asar_status asar_pack(const char* src, const char* dest, const char* unpack, asar_transform_callback_t transform) {
  try {
    asar::Asar::pack(src, dest, unpack, transform);
  } catch (const asar::AsarError& err) {
    asar__set_last_error(err);
    return code;
  } catch (const std::exception& stdexpt) {
    code = unknown;
    memset(msg, 0, sizeof(msg));
    strcpy(msg, stdexpt.what());
    return code;
  }

  return ok;
}
