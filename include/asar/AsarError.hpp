#ifndef __ASAR_ERROR_HPP__
#define __ASAR_ERROR_HPP__

#include <string>
#include <exception>

#include "asar/asar.h"

namespace asar {
  class AsarError : public std::exception {
   private:
    asar_status _code;
    std::string _message;
   public:
    virtual ~AsarError();
    AsarError(asar_status, const std::string&);
    const char* what() const noexcept;
    asar_status code() const;
  };
}

#endif
