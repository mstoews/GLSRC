//
// Relate4set.cpp - filtered iteration over the parent Data4.
//

#include "Relate4set.h"
#include "Internal.h"
#include "QueryExpr.h"

namespace cb4sqlite {

int Relate4set::querySet(const char* expression)
{
    if (!data_) return e4info;
    // A missing table has no fields, so only the DELETED() forms translate.
    filter_ = detail::translateQuery(data_->info_, expression ? expression : "");
    return r4success;
}

int Relate4set::top()        { return data_ ? data_->first(true, filter_) : e4info; }
int Relate4set::bottom()     { return data_ ? data_->first(false, filter_) : e4info; }
int Relate4set::skip(long n) { return data_ ? data_->step(n, filter_) : e4info; }

} // namespace cb4sqlite
