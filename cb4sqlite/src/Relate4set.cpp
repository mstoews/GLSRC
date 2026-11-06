//
// Relate4set.cpp - filtered iteration over the parent Data4.
//

#include "Relate4set.h"
#include "Internal.h"

#include <cctype>

namespace cb4sqlite {

int Relate4set::querySet(const char* expression)
{
    std::string norm;
    for (const char* p = expression ? expression : ""; *p; ++p)
        if (!std::isspace(static_cast<unsigned char>(*p)))
            norm += static_cast<char>(std::toupper(static_cast<unsigned char>(*p)));

    if (norm.empty())
        filter_.clear();
    else if (norm == ".NOT.DELETED()" || norm == "!DELETED()")
        filter_ = "_deleted = 0";
    else
        notImplemented("Relate4set::querySet", std::string("expression \"") + expression + "\"");
    return r4success;
}

int Relate4set::top()        { return data_ ? data_->first(true, filter_) : e4info; }
int Relate4set::bottom()     { return data_ ? data_->first(false, filter_) : e4info; }
int Relate4set::skip(long n) { return data_ ? data_->step(n, filter_) : e4info; }

} // namespace cb4sqlite
