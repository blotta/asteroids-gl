@echo off

set BUILD_DIR=.\build
set EXECUTABLE=asteroids.exe

set SDL_PATH=.\dependencies\SDL3-3.4.2
set GLAD_PATH=.\dependencies\glad
set HHM_PATH=.\dependencies\HandmadeMath\

echo Compiling to %BUILD_DIR%\%EXECUTABLE%
mkdir build 2>NUL
clang src\main.c ^
    %GLAD_PATH%\src\glad.c ^
    -I%SDL_PATH%\include ^
    -L%SDL_PATH%\lib\x64 ^
    -I%GLAD_PATH%\include ^
    -I%HHM_PATH% ^
    -lSDL3 ^
    -lopengl32 ^
    -Wall -Werror -Wextra -pedantic ^
    -std=c11 ^
    -Xlinker /subsystem:console ^
    -o %BUILD_DIR%\%EXECUTABLE%

echo Copy SDL3.dll to %BUILD_DIR%
xcopy /q /y %SDL_PATH%\lib\x64\SDL3.dll %BUILD_DIR%\ 1>NUL
