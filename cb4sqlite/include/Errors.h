#pragma once
#include <stdexcept>
#include <string>

namespace cb4sqlite {

// Thrown for real failures (SQLite errors, misuse that CodeBase would have
// reported as an error, features not implemented in Spec #1). Ported screens
// let it propagate to the win32fx dispatch-boundary catch (spec 7.2).
// Expected outcomes (seek miss, eof, missing table/field) are return codes or
// isValid() == false, never exceptions.
class Error : public std::runtime_error {
public:
    Error(const char* method, const std::string& detail)
        : std::runtime_error(std::string("cb4sqlite: ") + method + ": " + detail) {}
};

[[noreturn]] inline void notImplemented(const char* method, const std::string& what = {})
{
    throw Error(method, "not implemented" + (what.empty() ? std::string() : ": " + what));
}

} // namespace cb4sqlite
