@echo off
setlocal
cd /D "%~dp0"

for %%a in (%*) do set "%%a=1"

set debug_build="-O0 -g"
set release_build="-O3"
set build_type=""

set common_flags="-std=c++17 -msse4.1 -fno-rtti -fno-exceptions -Wall -Wno-unused-function -Wno-writable-strings -Wno-comment -g -ferror-limit=10000"

if not exist out mkdir out
if not exist out\SDL3.dll xcopy /s /y data\bin\SDL3.dll out

if "%clean%" == "1" rmdir /s /q "out"
if "%cloc%" == "1" cloc --exclude-list-file=.clocignore .\code\
if "%debug%" == "1" echo [debug] && set build_type=%debug_build%
if "%release%" == "1" echo [release] && set build_type=%release_build%

if "%platform%" == "1" echo [platform] && clang "%common_flags%" "%build_type%" -I./code/ -I./code/third_party/ -o out/platform.exe code/main.cpp -luser32 -lkernel32 -lgdi32 -lcomdlg32 -lopengl32 -ldata\bin\SDL3

if %errorlevel% neq 0 echo platform compilation failed && exit /b

if "%app%" == "1" echo [app] && del "out\yk.dll" && clang "%common_flags%" "%build_type%" -I./code/third_party/ -I./code/ code/engine.cpp -shared -o out/yk.dll -lopengl32 -ldata\bin\SDL3

if %errorlevel% neq 0 echo app compilation failed && exit /b
   
if "%run%" == "1" start cmd /c ".\out\platform.exe && echo [run]"

if "%meta%" == "1" clang code\draw\draw_gen.cpp -o out\draw_gen.exe && out\draw_gen.exe > code\draw\draw_styles.cpp