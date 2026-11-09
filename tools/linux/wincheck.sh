#!/bin/bash
# Syntax/type check of Win32 C++ on Linux: host g++ + MinGW-w64 Windows headers.
# Not a real Windows build (long is 64-bit here, unlike Windows): a typo/API check only.
D=$(dirname "$0")
exec g++ -std=c++20 -fsyntax-only -fshort-wchar -Wall -Wextra -Werror -Wno-attributes -Wno-unknown-pragmas \
  -D_WIN32 -D_WIN64 -DWIN32 -D__MINGW64__ -DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DNOMINMAX \
  -D__cdecl= -D__stdcall= -D__fastcall= -D__thiscall= -D'__declspec(x)=__attribute__((x))' \
  -D_MSC_VER_CHECK_ONLY -I"$D/winhdr" -idirafter /usr/x86_64-w64-mingw32/include "$@"
