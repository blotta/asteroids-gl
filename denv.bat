@echo off
set "VS_PATH=C:\Program Files\Microsoft Visual Studio\18\Community"

call "%VS_PATH%\VC\Auxiliary\Build\vcvarsall.bat" x64

set "PROJECT_DIR=%~dp0"
cd /d "%PROJECT_DIR%"

start zed .
