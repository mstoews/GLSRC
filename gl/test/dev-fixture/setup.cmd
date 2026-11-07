@echo off
rem setup.cmd [Debug|Release] - put the dev-fixture company next to gl.exe.
rem Migrates dbf\ into x64\<Config>\dev.db with dbf2sqlite and copies GLDR.INI
rem (DIR=dev.db) and GL.INI ([ACCESS] PSW=T) beside it. Build gl.sln first.
rem Overwrites dev.db, GLDR.INI and GL.INI in that directory. See gl\README.md.
setlocal
set CFG=%~1
if "%CFG%"=="" set CFG=Debug
set OUT=%~dp0..\..\..\x64\%CFG%
if not exist "%OUT%\dbf2sqlite.exe" (
  echo %OUT%\dbf2sqlite.exe not found: build gl.sln %CFG%^|x64 first.
  exit /b 1
)
if exist "%OUT%\dev.db" del "%OUT%\dev.db" || exit /b 1
"%OUT%\dbf2sqlite.exe" --src "%~dp0dbf" --out "%OUT%\dev.db" || exit /b 1
copy /y "%~dp0GLDR.INI" "%OUT%\GLDR.INI" >nul || exit /b 1
copy /y "%~dp0GL.INI" "%OUT%\GL.INI" >nul || exit /b 1
echo.
echo Ready: %OUT%\gl.exe opens dev.db. Log in as ADMIN / ADMIN, or CLERK / clerk.
