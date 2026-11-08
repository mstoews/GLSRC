// msvc_compat.h - Linux stand-ins for the MSVC-only CRT calls in the Windows
// sources, so the test builds in build_tests.sh compile with g++ unchanged.
// Passed with -include; never used by the Visual Studio build.
#pragma once
#ifndef _WIN32
#include <cstdio>
#include <cstdlib>
#include <cstring>

// dbf2sqlite_core/src/DbfFile.cpp reads "LDID/%d" with sscanf_s; with only
// numeric conversions it behaves as sscanf.
#define sscanf_s sscanf

// cb4sqlite_tests/test_dbf2sqlite.cpp reads GL_REPO_ROOT with _dupenv_s.
inline int _dupenv_s(char** buffer, size_t* size, const char* name)
{
    *buffer = nullptr;
    if (size) *size = 0;
    const char* v = std::getenv(name);
    if (!v) return 0;
    const size_t n = std::strlen(v) + 1;
    *buffer = static_cast<char*>(std::malloc(n));
    if (!*buffer) return 12;   // ENOMEM
    std::memcpy(*buffer, v, n);
    if (size) *size = n;
    return 0;
}
#endif
