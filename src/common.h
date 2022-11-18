#ifndef COMMON_HPP
#define COMMON_HPP

#include <algorithm>
#include <stdexcept>
#include <cstdint>
#include <vector>
#include <mutex>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "platform/platform.h"
#include "interapp/ctmgr.h"


#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
#endif

#define SAFE_CLOSE(s) platform::vclose(s); s = NULL

class wsa_error : public std::exception {
  public:
    using _Mybase = std::exception;
    explicit wsa_error(const std::string& _Message) : _Mybase(_Message.c_str()) {}
    explicit wsa_error(const char* _Message) : _Mybase(_Message) {}
};
class win_error : public std::exception {
  public:
    using _Mybase = std::exception;
    explicit win_error(const std::string& _Message) : _Mybase(_Message.c_str()) {}
    explicit win_error(const char* _Message) : _Mybase(_Message) {}
};

#endif /* COMMON_HPP */
