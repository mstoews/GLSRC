//
// Stubs.cpp - the gl.exe globals the portable gl/port sources reference, for
// gl_tests. The real definitions (Globals.cpp, Company.cpp) need Win32.
//

#include "Company.h"

#include <stdexcept>

char szUser[11] = "TESTER";

// Company.cpp stamps "update"/"user"/"time" fields with the local time. None
// of the tables gl_tests writes so far has them (glaccts), so the real function
// returns false there too. A table that has them would need the real stamp,
// so the stub refuses rather than silently skipping it.
bool UpdateUserTime(Data4& data, const char*)
{
    if (Field4(data, "update").isValid() && Field4(data, "user").isValid() && Field4(data, "time").isValid())
        throw std::logic_error("UpdateUserTime stub called on a table with update/user/time fields");
    return false;
}
