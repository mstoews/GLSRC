//
// Company.cpp - see Company.h.
//
#include "Company.h"

#include "Globals.h"
#include "Tables.h"

#include "win32fx.h"

#include <cstdio>
#include <cstring>

std::wstring GetCompany()
{
    // The original opened its own Code4 in the company directory; the port
    // has one session, the global cb.
    GL_PROF gl_prof(cb);
    if (!gl_prof.isValid()) return {};
    gl_prof.top();
    std::wstring name;
    if (!gl_prof.eof()) name = fx::Trim(fx::Utf8ToWide(gl_prof.company.str()));
    gl_prof.close();
    return name;
}

std::wstring GetCurrentPeriod()
{
    GL_PDS gl_pds(cb);
    if (!gl_pds.isValid()) return {};
    gl_pds.SetIndex("Period");
    gl_pds.bottom();
    std::wstring period;
    // As szGetPeriod: walk backwards; the last open period seen (the lowest) wins.
    // bottom() on an empty table is eof; skip(-1) from eof would go to the last record.
    while (!gl_pds.eof() && !gl_pds.bof()) {
        if (std::strcmp(gl_pds.status.str(), "0") == 0) period = fx::Trim(fx::Utf8ToWide(gl_pds.pd_desc.str()));
        if (gl_pds.skip(-1) == r4bof) break;
    }
    gl_pds.close();
    return period;
}

bool UpdateUserTime(Data4& data, const char* user)
{
    Field4 fDate(data, "update");
    if (!fDate.isValid()) return false;
    Field4 fUser(data, "user");
    if (!fUser.isValid()) return false;
    Field4 fTime(data, "time");
    if (!fTime.isValid()) return false;

    Date4 d;
    d.today();
    // asctime(localtime(...)) without its '\n': "Fri Nov 06 11:30:00 2026".
    static const char* const kDays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char* const kMonths[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                          "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    SYSTEMTIME st{};
    GetLocalTime(&st);
    char stamp[32] = {};
    std::snprintf(stamp, sizeof stamp, "%s %s %02u %02u:%02u:%02u %u", kDays[st.wDayOfWeek % 7],
                  kMonths[(st.wMonth + 11) % 12], unsigned{st.wDay}, unsigned{st.wHour}, unsigned{st.wMinute},
                  unsigned{st.wSecond}, unsigned{st.wYear});

    fDate.assign(d.str());
    fUser.assign(user);
    fTime.assign(stamp);
    return true;
}
