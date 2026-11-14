#!/usr/bin/env python3
"""Generate the solution's .vcxproj files and gl.sln.

Usage: genproj.py <repo-root> [project ...] [gl.sln]

With no project names it writes every .vcxproj and gl.sln. With names it
writes only those projects' .vcxproj files, and gl.sln only if "gl.sln" is
one of the names, so a project changed by hand in Visual Studio is left
alone. Every project uses PlatformToolset v145 (VS 18), as the script
writes. gl's file list is every .h and .cpp in
gl/port/ and gl/port/stubs/, so adding a file there needs only a rerun:

    python3 tools/linux/genproj.py . gl

gl_tests.vcxproj and report_tests.vcxproj are hand-written (they compile
sources from other directories); the script only lists them in gl.sln.
"""
import sys, pathlib

ROOT = pathlib.Path(sys.argv[1])
ONLY = set(sys.argv[2:])


def gl_files(pattern):
    base = ROOT / "gl"
    # Sorted by the case-sensitive path so Windows (whose paths compare without
    # case) and Linux write the same order.
    def tree(sub):
        return sorted(base.glob(sub + pattern), key=lambda f: f.as_posix())
    found = tree("port/") + tree("port/reports/") + tree("port/stubs/")
    return [str(f.relative_to(base)).replace("/", "\\") for f in found]


VCXPROJ_TYPE = "{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}"

def guid(n):
    return "{A0C1C001-%04d-4001-A001-%012d}" % (n, n)

PROJECTS = [
    # name, dir, guid#, type, subsystem, headers, sources, refs, extra
    dict(name="cb4sqlite", dir="cb4sqlite", n=1, type="StaticLibrary", sub=None,
         headers=["include\\cb4sqlite.h", "include\\cb4sqlite_compat.h",
                  "include\\legacy_headers_begin.h", "include\\legacy_headers_end.h",
                  "include\\Code4.h", "include\\Data4.h", "include\\Field4.h",
                  "include\\Date4.h", "include\\Relate4set.h", "include\\Errors.h",
                  "include\\Schema.h", "include\\Query.h", "src\\Internal.h", "src\\QueryExpr.h"],
         sources=["src\\Internal.cpp", "src\\Schema.cpp", "src\\Code4.cpp", "src\\Data4.cpp",
                  "src\\Field4.cpp", "src\\Field4_win32.cpp", "src\\Date4.cpp", "src\\Relate4set.cpp", "src\\QueryExpr.cpp",
                  "src\\Query.cpp"],
         refs=[], extra="sqlite"),
    dict(name="win32fx", dir="win32fx", n=2, type="StaticLibrary", sub=None,
         headers=["include\\win32fx.h", "include\\Window.h", "include\\Dialog.h", "include\\MsgMap.h",
                  "include\\HandleGuard.h", "include\\StringUtil.h", "include\\Verify.h",
                  "include\\MessageLoop.h", "include\\MdiFrame.h", "include\\Toolbar.h",
                  "include\\StatusBar.h",
                  "include\\controls\\CustomControls.h", "include\\controls\\Grid.h", "src\\Internal.h",
                  "src\\controls\\ControlsInternal.h", "src\\controls\\EditRules.h"],
         sources=["src\\Window.cpp", "src\\Dialog.cpp", "src\\MessageLoop.cpp", "src\\StringUtil.cpp",
                  "src\\Verify.cpp", "src\\MdiFrame.cpp", "src\\Toolbar.cpp",
                  "src\\StatusBar.cpp", "src\\controls\\Register.cpp", "src\\controls\\EditRules.cpp",
                  "src\\controls\\EditBase.cpp", "src\\controls\\PicEdit.cpp", "src\\controls\\DateEdit.cpp",
                  "src\\controls\\FloatEdit.cpp", "src\\controls\\IntEdit.cpp", "src\\controls\\SuperBtn.cpp",
                  "src\\controls\\Grid.cpp", "src\\controls\\StatusPane.cpp"],
         refs=[], extra=None),
    dict(name="dbf2sqlite_core", dir="dbf2sqlite_core", n=3, type="StaticLibrary", sub=None,
         headers=["include\\Migrator.h", "src\\Source.h"],
         sources=["src\\DbfFile.cpp", "src\\MdxFile.cpp", "src\\Encoding.cpp", "src\\KeyExpr.cpp",
                  "src\\MemoFile.cpp", "src\\Migrator.cpp", "src\\HeaderGen.cpp"],
         refs=[], extra="shapelib"),
    dict(name="dbf2sqlite", dir="dbf2sqlite", n=4, type="Application", sub="Console",
         headers=[], sources=["main.cpp"], refs=["dbf2sqlite_core", "cb4sqlite"], extra=None),
    dict(name="cb4sqlite_tests", dir="cb4sqlite_tests", n=5, type="Application", sub="Console",
         headers=["TestDb.h"], sources=["..\\cb4sqlite\\test\\parse_smoke.cpp", "m0_smoke_test.cpp",
                  "test_code4_schema.cpp", "test_field4.cpp", "test_field4_win32.cpp",
                  "test_data4.cpp", "test_relate4set.cpp", "test_legacy_headers.cpp",
                  "test_dbf2sqlite.cpp", "test_queryexpr.cpp", "test_transaction.cpp", "test_backup.cpp",
                  "test_query.cpp"],
         refs=["cb4sqlite", "dbf2sqlite_core"], extra="catch2"),
    dict(name="gl", dir="gl", n=6, type="Application", sub="Windows",
         headers=gl_files("*.h") + ["res\\resource.h"],
         sources=gl_files("*.cpp"),
         refs=["win32fx", "cb4sqlite", "report"], extra=None,
         res=["res\\gl.rc"], manifest="res\\manifest.xml"),
    # Hand-written .vcxproj (it compiles sources from gl\port and cb4sqlite_tests): only listed in gl.sln.
    dict(name="gl_tests", dir="gl_tests", n=7, hand=True),
    # The report library (spec #3 4): the portable core, and the Windows half in src\win32 (S3-M1).
    dict(name="report", dir="report", n=8, type="StaticLibrary", sub=None,
         headers=["include\\report\\Report.h", "include\\report\\Csv.h", "include\\report\\Format.h",
                  "include\\report\\Layout.h", "include\\report\\TextRender.h", "include\\report\\win32\\Gdi.h",
                  "include\\report\\win32\\PrinterSetup.h", "include\\report\\win32\\Print.h",
                  "include\\report\\win32\\Preview.h"],
         sources=["src\\Csv.cpp", "src\\Format.cpp", "src\\Layout.cpp", "src\\TextRender.cpp", "src\\win32\\GdiMeasure.cpp",
                  "src\\win32\\PrinterSetup.cpp", "src\\win32\\Print.cpp", "src\\win32\\Preview.cpp"],
         refs=[], extra=None),
    # Hand-written, as gl_tests (Catch2 from cb4sqlite_tests\vendor).
    dict(name="report_tests", dir="report_tests", n=9, hand=True),
]
BYNAME = {p["name"]: p for p in PROJECTS}

SQLITE = r"""  <!--
    Vendored SQLite amalgamation (plan M0 task 3). See VENDORED.md. Compiled as C with the spec 7.1 defines;
    third-party code is exempt from /W4 /WX.
  -->
  <ItemGroup>
    <ClInclude Include="sqlite3.h" />
    <ClCompile Include="sqlite3.c">
      <CompileAs>CompileAsC</CompileAs>
      <WarningLevel>Level3</WarningLevel>
      <TreatWarningAsError>false</TreatWarningAsError>
      <SDLCheck>false</SDLCheck>
      <PreprocessorDefinitions>SQLITE_THREADSAFE=0;SQLITE_OMIT_LOAD_EXTENSION=1;SQLITE_DEFAULT_FOREIGN_KEYS=1;%(PreprocessorDefinitions)</PreprocessorDefinitions>
    </ClCompile>
  </ItemGroup>
"""

CATCH2 = r"""  <!--
    Vendored Catch2 v3 amalgamation (plan M0 task 3). See VENDORED.md.
    Provides main(); exempt from /W4 /WX.
  -->
  <ItemGroup>
    <ClInclude Include="vendor\catch2\catch_amalgamated.hpp" />
    <ClCompile Include="vendor\catch2\catch_amalgamated.cpp">
      <WarningLevel>Level3</WarningLevel>
      <TreatWarningAsError>false</TreatWarningAsError>
    </ClCompile>
  </ItemGroup>
"""

SHAPELIB = r"""  <!--
    Vendored shapelib DBF reader (spec 10 Q3 fallback; MIT licence option). See VENDORED.md.
    Compiled as C; exempt from /W4 /WX. Only the .dbf API is used.
  -->
  <ItemGroup>
    <ClInclude Include="vendor\shapelib\shapefil.h" />
    <ClInclude Include="vendor\shapelib\shapefil_private.h" />
    <ClCompile Include="vendor\shapelib\dbfopen.c">
      <CompileAs>CompileAsC</CompileAs>
      <WarningLevel>Level3</WarningLevel>
      <TreatWarningAsError>false</TreatWarningAsError>
      <SDLCheck>false</SDLCheck>
    </ClCompile>
    <ClCompile Include="vendor\shapelib\safileio.c">
      <CompileAs>CompileAsC</CompileAs>
      <WarningLevel>Level3</WarningLevel>
      <TreatWarningAsError>false</TreatWarningAsError>
      <SDLCheck>false</SDLCheck>
    </ClCompile>
  </ItemGroup>
"""

def vcxproj(p):
    cfg = []
    for c in ("Debug", "Release"):
        wpo = "\n    <WholeProgramOptimization>true</WholeProgramOptimization>" if c == "Release" else ""
        cfg.append(f"""  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='{c}|x64'" Label="Configuration">
    <ConfigurationType>{p['type']}</ConfigurationType>
    <UseDebugLibraries>{'true' if c == 'Debug' else 'false'}</UseDebugLibraries>
    <PlatformToolset>v145</PlatformToolset>
    <CharacterSet>Unicode</CharacterSet>{wpo}
  </PropertyGroup>""")
    out = [f"""<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|x64">
      <Configuration>Debug</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Release|x64">
      <Configuration>Release</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
  </ItemGroup>
  <PropertyGroup Label="Globals">
    <VCProjectVersion>17.0</VCProjectVersion>
    <ProjectGuid>{guid(p['n'])}</ProjectGuid>
    <RootNamespace>{p['name']}</RootNamespace>
    <WindowsTargetPlatformVersion>10.0</WindowsTargetPlatformVersion>
    <Keyword>Win32Proj</Keyword>
  </PropertyGroup>
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.Default.props" />
""" + "\n".join(cfg) + """
  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.props" />
  <ImportGroup Label="PropertySheets">
    <Import Project="$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props" Condition="exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')" Label="LocalAppDataPlatform" />
    <Import Project="$(MSBuildThisFileDirectory)..\\common.props" />
  </ImportGroup>
"""]
    if p["extra"] == "catch2":
        out.append("""  <PropertyGroup>
    <IncludePath>$(ProjectDir)vendor\\catch2;$(GlRoot)cb4sqlite;$(IncludePath)</IncludePath>
  </PropertyGroup>
""")
    if p["sub"]:
        out.append(f"""  <ItemDefinitionGroup>
    <Link>
      <SubSystem>{p['sub']}</SubSystem>
    </Link>
  </ItemDefinitionGroup>
""")
    if p["headers"]:
        out.append("  <ItemGroup>\n" + "".join(f'    <ClInclude Include="{h}" />\n' for h in p["headers"]) + "  </ItemGroup>\n")
    out.append("  <ItemGroup>\n" + "".join(f'    <ClCompile Include="{s}" />\n' for s in p["sources"]) + "  </ItemGroup>\n")
    if p.get("res"):
        # res\ on the rc include path: resource.h and the bitmaps/ and icons/ files resolve from there.
        out.append("  <ItemGroup>\n" + "".join(
            f'    <ResourceCompile Include="{r}">\n'
            f'      <AdditionalIncludeDirectories>$(ProjectDir)res;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>\n'
            f'    </ResourceCompile>\n' for r in p["res"]) + "  </ItemGroup>\n")
    if p.get("manifest"):
        out.append("  <!-- Common controls v6 + per-monitor DPI v2 (spec 7.1); mt.exe merges it into the linker's manifest. -->\n"
                   f'  <ItemGroup>\n    <Manifest Include="{p["manifest"]}" />\n  </ItemGroup>\n')
    if p["extra"] == "sqlite":
        out.append(SQLITE)
    if p["extra"] == "catch2":
        out.append(CATCH2)
    if p["extra"] == "shapelib":
        out.append(SHAPELIB)
    if p["refs"]:
        out.append("  <ItemGroup>\n")
        for r in p["refs"]:
            rp = BYNAME[r]
            out.append(f"""    <ProjectReference Include="..\\{rp['dir']}\\{rp['name']}.vcxproj">
      <Project>{guid(rp['n'])}</Project>
    </ProjectReference>
""")
        out.append("  </ItemGroup>\n")
    out.append("""  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" />
</Project>
""")
    return "".join(out)

def sln():
    s = ["Microsoft Visual Studio Solution File, Format Version 12.00\n",
         "# Visual Studio Version 17\n",
         "VisualStudioVersion = 17.0.31903.59\n",
         "MinimumVisualStudioVersion = 10.0.40219.1\n"]
    for p in PROJECTS:
        s.append(f'Project("{VCXPROJ_TYPE}") = "{p["name"]}", "{p["dir"]}\\{p["name"]}.vcxproj", "{guid(p["n"])}"\nEndProject\n')
    s.append("Global\n\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n"
             "\t\tDebug|x64 = Debug|x64\n\t\tRelease|x64 = Release|x64\n\tEndGlobalSection\n"
             "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n")
    for p in PROJECTS:
        g = guid(p["n"])
        for c in ("Debug", "Release"):
            s.append(f"\t\t{g}.{c}|x64.ActiveCfg = {c}|x64\n\t\t{g}.{c}|x64.Build.0 = {c}|x64\n")
    s.append("\tEndGlobalSection\n\tGlobalSection(SolutionProperties) = preSolution\n"
             "\t\tHideSolutionNode = FALSE\n\tEndGlobalSection\nEndGlobal\n")
    return "".join(s)

def write(path, text):
    path.parent.mkdir(parents=True, exist_ok=True)
    # Visual Studio's native line ending for .sln/.vcxproj is CRLF.
    path.write_bytes(text.replace("\r\n", "\n").replace("\n", "\r\n").encode("utf-8"))

unknown = ONLY - set(BYNAME) - {"gl.sln"}
if unknown:
    sys.exit("unknown project(s): " + ", ".join(sorted(unknown)))
for p in PROJECTS:
    if p.get("hand"):
        if p["name"] in ONLY:
            sys.exit(p["name"] + ".vcxproj is hand-written; edit it directly")
        continue
    if not ONLY or p["name"] in ONLY:
        write(ROOT / p["dir"] / f"{p['name']}.vcxproj", vcxproj(p))
if not ONLY or "gl.sln" in ONLY:
    write(ROOT / "gl.sln", "﻿" + sln())
print("ok")
