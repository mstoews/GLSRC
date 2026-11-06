//
// Date4.cpp
//

#include "Date4.h"

#include <cstdio>
#include <cstring>
#include <ctime>

namespace cb4sqlite {

Date4::Date4() noexcept
{
    std::memset(dt_, ' ', 8);
    dt_[8] = '\0';
}

void Date4::today() noexcept
{
    const std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::snprintf(dt_, sizeof dt_, "%04d%02d%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

void Date4::assign(const char* yyyymmdd) noexcept
{
    std::memset(dt_, ' ', 8);
    dt_[8] = '\0';
    if (!yyyymmdd) return;
    for (int i = 0; i < 8 && yyyymmdd[i]; ++i)
        dt_[i] = yyyymmdd[i];
}

static int digits(const char* p, int n) noexcept
{
    int v = 0;
    for (int i = 0; i < n; ++i) {
        if (p[i] < '0' || p[i] > '9') return 0;
        v = v * 10 + (p[i] - '0');
    }
    return v;
}

int Date4::year()  const noexcept { return digits(dt_, 4); }
int Date4::month() const noexcept { return digits(dt_ + 4, 2); }
int Date4::day()   const noexcept { return digits(dt_ + 6, 2); }

} // namespace cb4sqlite
