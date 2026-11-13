// Linux syntax-check shim: host C/C++ runtime headers + MinGW-w64 Win32 API headers.
#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdarg>
#include <cwchar>
#include <cstdlib>
#include <cstring>
#define _INC_CORECRT
#define _INC_CRTDEFS
#define _INC_EXCPT
#define _CRTIMP
typedef int errno_t;
#define _SSIZE_T_DEFINED   // glibc already has ssize_t; rpcndr.h (COM, WIC, DWM headers) would redeclare it
#include <cwctype>
typedef long long __time64_t;
typedef enum _EXCEPTION_DISPOSITION { ExceptionContinueExecution, ExceptionContinueSearch, ExceptionNestedException, ExceptionCollidedUnwind } EXCEPTION_DISPOSITION;
#include_next <windows.h>
// MSVC CRT functions win32fx uses (declarations only).
extern "C" int _vscwprintf(const wchar_t*, va_list);
extern "C" int vswprintf_s(wchar_t*, size_t, const wchar_t*, va_list);
template <size_t N> int swprintf_s(wchar_t (&)[N], const wchar_t*, ...);
template <size_t N> int strcpy_s(char (&)[N], const char*);
