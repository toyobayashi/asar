#include "asar/asar.h"
#include "Asar.hpp"
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

int asar_is_open(asar_t* asar) {
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

int asar_pack(const char* src, const char* dest, const char* unpack, asar_transform_callback_t transform) {
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
