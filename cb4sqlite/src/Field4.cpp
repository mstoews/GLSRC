//
// Field4.cpp - field accessor over the Data4 record buffer.
//

#include "Field4.h"
#include "Internal.h"

#include <string>

namespace cb4sqlite {

namespace {
const FieldDef kNoField{};
}

int Field4::init(Data4& d, int fieldNo) noexcept
{
    data_ = nullptr;
    field_ = -1;
    if (!d.isValid() || fieldNo < 1 || fieldNo > d.numFields())
        return -1;
    data_ = &d;
    field_ = fieldNo - 1;
    return 0;
}

int Field4::init(Data4& d, const char* name) noexcept
{
    const int n = d.isValid() ? d.fieldNumber(name) : 0;
    if (n == 0) {
        data_ = nullptr;
        field_ = -1;
        return -1;
    }
    return init(d, n);
}

const FieldDef& Field4::def() const noexcept
{
    return data_ ? data_->info_.fields[static_cast<size_t>(field_)] : kNoField;
}

const char* Field4::str() const noexcept
{
    return data_ ? data_->value(field_).c_str() : "";
}

unsigned Field4::len() const noexcept      { return static_cast<unsigned>(def().len); }
int      Field4::decimals() const noexcept { return def().dec; }
char     Field4::type() const noexcept     { return data_ ? def().type : '\0'; }
const char* Field4::name() const noexcept  { return def().name.c_str(); }

int Field4::assign(const char* value)
{
    if (!data_) return e4fieldName;
    data_->setValue(field_, detail::imageFromText(def(), value ? value : ""));
    return r4success;
}

void Field4::assignDouble(double value, int, int nDec)
{
    if (!data_) return;
    const FieldDef& f = def();
    data_->setValue(field_, detail::imageFromDouble(f, value, nDec >= 0 ? nDec : f.dec));
}

void Field4::assignLong(long value, int, int zerosInFront)
{
    if (!data_) return;
    data_->setValue(field_, detail::imageFromLong(def(), value, zerosInFront != 0));
}

void Field4::trim()
{
    if (data_) data_->trimValue(field_);
}

unsigned Field4memo::len() const noexcept
{
    if (type() != r4memo) return Field4::len();
    return static_cast<unsigned>(std::char_traits<char>::length(str()));
}

} // namespace cb4sqlite
