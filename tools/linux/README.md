# Linux checking tools

These help develop the port on a machine without Visual Studio. They don't replace the Windows build. Every milestone still has to build with `/W4 /WX` and run on Windows. Nothing in `gl.sln` uses them.

Run everything from the repository root.

## `wincheck.sh` and `wincheck_all.sh`

`wincheck.sh` syntax- and type-checks one Win32 source file with the host `g++` and the MinGW-w64 Windows headers (Debian/Ubuntu package `mingw-w64-common`), with `-Wall -Wextra -Werror`. `winhdr/` holds the small overrides that let the MSVC-oriented code compile there.

```
tools/linux/wincheck.sh -Iwin32fx/include -Icb4sqlite/include -Iinclude -Igl/header -Igl/port -Igl/res gl/port/AccountList.cpp
tools/linux/wincheck_all.sh     # every gl.exe, win32fx and trivial_dialog source; exit code = failing files
```

It catches typos, wrong API names and signatures, and most of what `/W4` warns about. It doesn't link anything, and `long` is 64-bit there, unlike on Windows, so size-dependent mistakes can slip through.

## `genproj.py`

Writes the solution's `.vcxproj` files and `gl.sln`.

```
python3 tools/linux/genproj.py . gl            # after adding or removing a file in gl/port/
python3 tools/linux/genproj.py . gl win32fx    # several projects
python3 tools/linux/genproj.py . report gl.sln  # one project, and the solution
python3 tools/linux/genproj.py .               # every project and gl.sln
```

- **gl:** the file list is every `.h` and `.cpp` in `gl/port/` and `gl/port/stubs/`.
- **Other projects:** their file lists are written out in the script, so a new file there means editing the list.
- **Hand-written projects:** `gl_tests` and `report_tests` compile sources from other directories, so the script only lists them in `gl.sln`. Edit their `.vcxproj` files directly.
- **Naming projects:** when you give project names, only those `.vcxproj` files are written, so a project edited by hand in Visual Studio is left alone. Check `git diff` after running it.
- **Toolset:** every project uses PlatformToolset v145 (Visual Studio 18), and the script writes v145.

## Sanitizer builds of the tests

```
tools/linux/build_tests.sh --run            # build, then run both test programs
tools/linux/build_tests.sh [out-dir]        # build only
```

`build_tests.sh` builds `cb4sqlite_tests`, `gl_tests`, `report_tests` and `dbf2sqlite` with g++, AddressSanitizer and UBSan into `out-dir` (default `x64/linux-asan`, git-ignored). `--run` then runs the three test programs from the repository root, with their temporary databases in `out-dir/tmp` (or `$TMPDIR_FOR_TESTS`).

- **Rebuilds:** objects rebuild when their source is newer. Delete `out-dir` after a header change.
- **MSVC-only calls:** the sources call MSVC-only CRT functions (`sscanf_s`, `_dupenv_s`). `msvc_compat.h` supplies them, passed with `-include`.
- **Fixtures:** the tests find them by walking up from the working directory, or from `GL_REPO_ROOT`.
- **The `dbf2sqlite` it builds** can also migrate the dev fixture.

## `../rptdump.py`

Dumps the readable records of a Crystal Reports 3/4 `.RPT` file: title, tables and links, selection formulas, formula names and text, the field references in layout order, and text objects. Spec #3 §2.2's inventory was read with it, and each report milestone re-reads its `.RPT` the same way. Layout positions and fonts are binary and aren't decoded.

```
python3 -I tools/rptdump.py dbf/GLTB.RPT
```
