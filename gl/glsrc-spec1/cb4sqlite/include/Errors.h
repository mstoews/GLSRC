#pragma once
#include <stdexcept>
#include <string>

namespace cb4sqlite {

class Error : public std::runtime_error {
public:
    explicit Error(const std::string& msg) : std::runtime_error(msg) {}
};

} // namespace cb4sqlite
