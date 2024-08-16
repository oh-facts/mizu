@echo off
setlocal
cd /D "%~dp0"

set debug_build="-O0 -g"
set release_build="-O3"
set build_type=""
for %%a in (%*) do set "%%a=1"

set common_flags="-std=c++17 -msse4.1 -fno-rtti -fno-exceptions -Wall -Wno-unused-function -Wno-writable-strings -Wno-comment -g"

if "%cloc%" == "1" cloc --exclude-list-file=.clocignore .\code\
if "%clean%" == "1" rmdir /s /q "out" && del "code\meta.h"
if "%debug%" == "1" echo [debug] && set build_type=%debug_build%
if "%release%" == "1" echo [release] && set build_type=%release_build%

:: literally doesnt work if I use "if not"
if exist out (
    REM folder exists
) else (
    mkdir out && echo [created build dir]
)
if exist out\SDL3.dll (
    REM file exists
) else (
    xcopy /s /y data\SDL3.dll out
)

if "%platform%" == "1" echo [platform] && clang "%common_flags%" "%build_type%" -I./code/ -o out/platform.exe code/game/main.cpp -luser32 -lkernel32 -lgdi32 -lcomdlg32 -lopengl32 -ldata\SDL3

if %errorlevel% neq 0 echo platform compilation failed && exit /b

if "%app%" == "1" echo [app] && del "out\yk.dll" && clang "%common_flags%" "%build_type%" -I./code/ code/game/game.cpp -shared -o out/yk.dll -lopengl32 -ldata\SDL3

if %errorlevel% neq 0 echo app compilation failed && exit /b

if "%run%" == "1" start cmd /c ".\out\platform.exe && echo [run]"
