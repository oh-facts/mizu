@echo off
setlocal
cd /D "%~dp0"

if not exist out mkdir out

for %%a in (%*) do set "%%a=1"

if "%meta%" == "1" clang code\meta\draw_gen.cpp -o out\draw_gen.exe && out\draw_gen.exe > code\generated\draw_styles.cpp && clang code\meta\ui_gen_h.cpp -o out\ui_gen_h.exe && out\ui_gen_h.exe > code\generated\ui_styles.h && clang code\meta\ui_gen_src.cpp -o out\ui_gen_src.exe && out\ui_gen_src.exe > code\generated\ui_styles.cpp

set debug_build="-O0 -g"
set release_build="-O3"
set build_type=""

set common_flags="-msse4.1 -fno-rtti -fno-exceptions -Wall -Wno-unused-function -Wno-writable-strings -Wno-comment -g -ferror-limit=10000"

if not exist out\SDL3.dll xcopy /s /y data\bin\SDL3.dll out

if "%clean%" == "1" rmdir /s /q "out" && echo [deleted out/]
if "%cloc%" == "1" cloc --exclude-list-file=.clocignore .\code\
if "%debug%" == "1" echo [debug] && set build_type=%debug_build%
if "%release%" == "1" echo [release] && set build_type=%release_build%

if "%platform%" == "1" echo [platform] && clang "%common_flags%" "%build_type%" -I./code/ -I./code/third_party/ -o out/platform.exe code/main.cpp -luser32 -lkernel32 -lgdi32 -lcomdlg32 -lopengl32 -ldata\bin\SDL3

if %errorlevel% neq 0 echo platform compilation failed && exit /b

if "%app%" == "1" echo [app] && del "out\yk.dll" && clang "%common_flags%" "%build_type%" -I./code/third_party/ -I./code/ code/engine.cpp -shared -o out/yk.dll -lopengl32 -ldata\bin\SDL3

if %errorlevel% neq 0 echo app compilation failed && exit /b
   
if "%run%" == "1" start cmd /c ".\out\platform.exe && echo [run]"