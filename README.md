<p>
<h1 align="center">Mizu Mizu Game Engine 1 million</h2>
<p align="center">C++ game engine</p>
<p align="center">
<img width="350"src="https://github.com/user-attachments/assets/7ee7d368-0b47-4dda-9650-a31f11ec7557">
</p>
</p>

## Table of Contents

1. [About](#about)
2. [Build](#build)
   - [Windows](#windows)
   - [Linux / Mac](#linux--mac)
3. [Contributions](#contributions)
4. [Project Structure](#project-structure)
## About
Hi, this is my game engine. Nice to meet you. Goodnight.

<p>
<img width="400"src="https://github.com/oh-facts/mizu/blob/main/data/misc/screenshot.png">
</p>
8/30/24

## Build
If `main` doesn't work, `stable` probably will
Arguments: `clean`, `platform`, `release`, `debug`, `app`, `cloc`, `run`

### Windows
```
./build.bat release platform app
```
### Linux / Mac
**SDL3** needs to be installed. Also, this doesn't compile on linux/mac yet. I am fixing the compiler errors one by one. I get bored sometimes. Give it a crack if you want. Its all platform related.
`clang` and `gcc` are also valid args. 
```
./build.bat release clang platform app
```

## Contributions
Open issue and make pr. Off the top of my head, nothing comes to mind.

## Project Structure
- `asset_cache.cpp` : texture cache and asset system
- `base.cpp` : common utility functions.
- `draw.cpp` : high level renderer layer
- `editor.cpp` : engine editor layer
- `entity.cpp` : entity system
- `context_cracking.cpp` : platform / compiler context
- `game.cpp` : game stuff
- `third_party/` : libraries, currently SDL, cgltf, glad and stb 
- `os_core.cpp` : Core OS utility, like memory function
- `os_gfx.cpp` : Graphics related OS utility, like opening a window
- `backends/` : opengl, win32 and linux backend code
- `ui.cpp` : immediate mode UI layer. Used for game and engine tools
