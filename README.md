https://github.com/user-attachments/assets/dcc2fac6-cb4c-42fc-887a-feeb844eef7d


<p>
<h1 align="center">Mizu Mizu Game Engine 1 million</h2>
<p align="center">C++ game engine</p>
<p align="center">
<img width="350"src="https://github.com/user-attachments/assets/7ee7d368-0b47-4dda-9650-a31f11ec7557">
</p>
</p>

## Table of Contents

- [Table of Contents](#table-of-contents)
- [About](#about)
- [Build](#build)
  - [Windows](#windows)
- [Contributions](#contributions)
- [Project Structure](#project-structure)
## About
Hi, this is my game engine. Nice to meet you. Goodnight.

<p>
<img width="400"src="https://github.com/oh-facts/mizu/blob/main/data/misc/screenshot.png">
</p>
8/30/24

## Build
If `main` doesn't work, `stable` probably will

Arguments: `clean`, `platform`, `release`, `debug`, `app`, `cloc`, `run`, `meta`

### Windows
```
./build.bat meta debug platform app
```
### Linux / Mac
**SDL3** and **box2d 3.1** need to be installed. Also, this doesn't compile on linux/mac yet. I am fixing the compiler errors one by one. I get bored sometimes. Give it a crack if you want. Its all platform related.
`clang` and `gcc` are also valid args. 
```
./build.sh meta debug clang platform app
```

## Contributions
Open issue and make pr for anything. Off the top of my head, nothing comes to mind. If you want, you can attempt a linux / mac port. Linux port will require making sure it compiles and updating `build.sh` to match the current `build.bat`. For a mac port, you'd likely have to make an opengl 3.3 backend for the renderer. This would be quite a bit of work since the 4.5 backend is about 900 loc.

## Project Structure
- `backends/` : opengl, win32 and linux backend code
- `generated/` : Generated code. Usually implicit params and templates.
- `meta/` : Metaprograms
- `third_party/` : external libraries, currently SDL, glad, box2d and stb 
- `base.cpp` : utility functions.
- `context.cpp` : libc replacements + platform context cracking
- `draw.cpp` : high level renderer layer. Handles batching.
- `editor.cpp` : ui abstraction to make gfx tools
- `main.cpp` : entry + game
- `os.cpp` : OS abstraction. Mainly windowing and memory.
- `render.cpp` : low level renderer layer
- `texture.cpp` : texture cache
- `ui.cpp` : immediate mode UI layer.
