#!/bin/bash
cd "${0%/*}"

for arg in "$@"; do eval "$arg=1"; done

debug_build="-O0 -g"
release_build="-O3"

common_flags="-std=c++17 -msse4.1 -fno-rtti -fno-exceptions -Wall -Wno-unused-function -Wno-writable-strings -Wno-write-strings -Wno-comment -Wno-misleading-indentation -Wno-unused-result"

mkdir -p out

[ "$cloc" == "1" ] && cloc --exclude-list-file=.clocignore "code/" && exit 0
[ "$debug" == "1" ] && build_type="$debug_build"
[ "$release" == "1" ] && build_type="$release_build"
[ "$clang" == "1" ] && compiler_type="clang" && echo "[clang]"
[ "$gcc" == "1" ] && compiler_type="gcc" && echo "[gcc]"

[ "$clean" == "1" ] && echo "deleted /out" && rm -rf "out/"

[ "$build_type" == "$debug_build" ] && echo "[debug build]"
[ "$build_type" == "$release_build" ] && echo "[release build]"

[ "$platform" == "1" ] && $compiler_type $common_flags $build_type -I./code/ -o out/platform code/game/main.cpp -lopengl32 -lSDL3 -lm -ldata/bin/SDL3 && echo "sandbox"

[ "$app" == "1" ] && $compiler_type $common_flags $build_type -I./code/ code/game/game.cpp -fPIC -shared -lopengl32 -lSDL3 -lm -o out/libyk.so

[ "$run" == "1" ] && ./out/platform