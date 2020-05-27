#include "asar/AsarError.hpp"

namespace asar {
  AsarError::~AsarError() {}

  AsarError::AsarError(asar_status code, const std::string& message)
    :_code(code), _message(message) {}
  
  const char* AsarError::what() const noexcept { return _message.c_str(); }

  asar_status AsarError::code() const { return _code; }
}
