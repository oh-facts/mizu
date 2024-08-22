<p>
<h1 align="center">Mizu Mizu Game Engine 1 million</h2>
<p align="center">C++ game engine</p>
<p align="center">
<img width="400"src="https://github.com/user-attachments/assets/7ee7d368-0b47-4dda-9650-a31f11ec7557">
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
8/22/24

## Build

Arguments: `clean`, `platform`, `release`, `debug`, `app`, `cloc`, `run`
### Windows
```
./build.bat release platform app
```

### Linux / Mac
`clang` and `gcc` are also valid. **SDL3** needs to be installed. Also, this doesn't compile on linux/mac yet. I am fixing the compiler errors one by one. I get bored sometimes. Give it a crack if you want. Its all platform related.

```
./build.bat release clang platform app
```

## Contributions
Make pr.

## Project Structure
- `asset/` : texture cache and asset system
- `base/` : common util libraries
- `draw/` : high level renderer layer
- `editor/` : engine editor layer
- `entity/` : entity system
- `file/` : file handling
- `game/` : game and entry point
- `opengl/` : opengl 
- `os/` : os layer. win32 / linux / sdl abstraction
- `render/` : low level renderer layer
- `SDL3/` : sdl3 include headers
- `stb/` : Sean barrett's stb_image, stb_truetype and stb_sprintf
- `ui/` : immediate mode UI layer. Used for game and engine tools
