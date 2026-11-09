#!/bin/bash
# build_tests.sh - Linux g++ builds of the portable test programs, with
# AddressSanitizer and UndefinedBehaviorSanitizer (README.md, "Sanitizer
# builds of the tests"). Not a Windows build: Win32-only sources are left out.
#
#   tools/linux/build_tests.sh [--run] [out-dir]
#
# Builds into out-dir (default x64/linux-asan, git-ignored):
#   cb4sqlite_tests   cb4sqlite + dbf2sqlite_core layer-1 tests
#   gl_tests          the portable gl/port sources on the migrated dev fixture
#   dbf2sqlite        the migration CLI (for the dev fixture: --src gl/test/dev-fixture/dbf)
# --run then runs both test programs from the repository root and stops at the
# first failure. Objects are rebuilt only when their source is newer, so after
# a header change delete out-dir.
#
# MSVC-only CRT calls in the sources (sscanf_s, _dupenv_s) are supplied by
# msvc_compat.h, passed with -include.
set -euo pipefail

ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
RUN=0
if [ "${1:-}" = "--run" ]; then RUN=1; shift; fi
OUT=${1:-$ROOT/x64/linux-asan}
mkdir -p "$OUT"

SAN="-fsanitize=address,undefined -fno-omit-frame-pointer"
CXXFLAGS="-std=c++20 -g -O1 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror $SAN
  -include $ROOT/tools/linux/msvc_compat.h
  -I$ROOT/cb4sqlite/include -I$ROOT/dbf2sqlite_core/include -I$ROOT/include -I$ROOT/gl/header"
CATCH="-I$ROOT/cb4sqlite -I$ROOT/cb4sqlite_tests/vendor/catch2"

# compile <source> <object> [extra flags...]: skipped when the object is newer.
compile() {
  local src=$1 obj=$2; shift 2
  if [ ! -f "$obj" ] || [ "$src" -nt "$obj" ]; then
    echo "  $(basename "$src")"
    # shellcheck disable=SC2086
    g++ $CXXFLAGS "$@" -c "$src" -o "$obj"
  fi
}

echo "third-party"
[ -f "$OUT/sqlite3.o" ] || gcc -O1 -c "$ROOT/cb4sqlite/sqlite3.c" -o "$OUT/sqlite3.o" \
  -DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION=1 -DSQLITE_DEFAULT_FOREIGN_KEYS=1
for f in dbfopen safileio; do
  [ -f "$OUT/$f.o" ] || gcc -O1 -c "$ROOT/dbf2sqlite_core/vendor/shapelib/$f.c" -o "$OUT/$f.o"
done
[ -f "$OUT/catch.o" ] || g++ -std=c++20 -O1 -c "$ROOT/cb4sqlite_tests/vendor/catch2/catch_amalgamated.cpp" \
  -I"$ROOT/cb4sqlite_tests/vendor/catch2" -o "$OUT/catch.o"
THIRD="$OUT/sqlite3.o $OUT/dbfopen.o $OUT/safileio.o"

echo "libraries"
LIB=""
for f in "$ROOT"/cb4sqlite/src/*.cpp "$ROOT"/dbf2sqlite_core/src/*.cpp; do
  o="$OUT/lib_$(basename "$f" .cpp).o"; compile "$f" "$o"; LIB="$LIB $o"
done

echo "dbf2sqlite"
compile "$ROOT/dbf2sqlite/main.cpp" "$OUT/cli_main.o"
# shellcheck disable=SC2086
g++ $SAN -o "$OUT/dbf2sqlite" "$OUT/cli_main.o" $LIB $THIRD -lm

echo "cb4sqlite_tests"
T=""
for f in "$ROOT"/cb4sqlite/test/parse_smoke.cpp "$ROOT"/cb4sqlite_tests/*.cpp; do
  o="$OUT/t_$(basename "$f" .cpp).o"; compile "$f" "$o" $CATCH; T="$T $o"
done
# shellcheck disable=SC2086
g++ $SAN -o "$OUT/cb4sqlite_tests" $T $LIB $THIRD "$OUT/catch.o" -lm

echo "gl_tests"
G=""
# The portable gl/port sources gl_tests links (keep in step with gl_tests.vcxproj).
for f in "$ROOT"/gl_tests/*.cpp "$ROOT"/gl/port/Accounts.cpp; do
  o="$OUT/g_$(basename "$f" .cpp).o"; compile "$f" "$o" $CATCH -I"$ROOT/gl/port" -I"$ROOT/gl_tests"; G="$G $o"
done
# shellcheck disable=SC2086
g++ $SAN -o "$OUT/gl_tests" $G $LIB $THIRD "$OUT/catch.o" -lm

echo "built in $OUT"
if [ $RUN = 1 ]; then
  cd "$ROOT"
  "$OUT/cb4sqlite_tests"
  "$OUT/gl_tests"
fi
