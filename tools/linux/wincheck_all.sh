#!/bin/bash
# Syntax/type check of every Win32 source of gl.exe, win32fx and the
# trivial_dialog example with wincheck.sh. Run from the repository root:
#     tools/linux/wincheck_all.sh
# Prints FAIL and the first errors for each file that doesn't pass; the exit
# code is the number of failing files.
cd "$(dirname "$0")/../.." || exit 100
INC="-Iwin32fx/include -Iwin32fx/src -Icb4sqlite/include -Iinclude -Igl/header -Igl/port -Igl/res"
fails=0
for f in gl/port/*.cpp gl/port/stubs/*.cpp win32fx/src/*.cpp win32fx/src/controls/*.cpp \
         win32fx/examples/trivial_dialog/trivial_dialog.cpp; do
    if ! out=$(tools/linux/wincheck.sh $INC "$f" 2>&1); then
        echo "FAIL $f"
        echo "$out" | head -20
        fails=$((fails + 1))
    fi
done
echo "wincheck: $fails file(s) failed"
exit $fails
