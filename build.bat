@echo off

set BUILD_DIR=.\build
set EXECUTABLE=asteroids.exe

set SDL_PATH=.\dependencies\SDL3-3.4.2

echo Compiling to %BUILD_DIR%\%EXECUTABLE%
mkdir build 2>NUL
clang src\main.c ^
    -I%SDL_PATH%\include ^
    -L%SDL_PATH%\lib\x64 ^
    -lSDL3 ^
    -Xlinker /subsystem:console ^
    -pedantic ^
    -o %BUILD_DIR%\%EXECUTABLE%

echo Copy SDL3.dll to %BUILD_DIR%
xcopy /q /y %SDL_PATH%\lib\x64\SDL3.dll %BUILD_DIR%\ 1>NUL
