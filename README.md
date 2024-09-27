
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
                                                                                            
                                                                                            I am making a demo with it. As the demo requires more features, I will add them to the engine. Until then, standalone engine builds / user documentation isn't reasonable to maintain because the api will be very unstable until demo release.
                                                                                            
                                                                                            <p>
                                                                                            <img width="400"src="https://github.com/oh-facts/mizu/blob/main/data/misc/screenshot.png">
                                                                                            </p>
                                                                                            8/30/24
                                                                                            
                                                                                            ## Build
                                                                                            
                                                                                            Build `stable` branch since `main` is not `stable`.
                                                                                            
                                                                                            As of today, I have revoked the asset directory because it has proprietary art. I will make an alternate directory that uses placeholder stuff. Fonts still work and everywhere you expect to see art, you'll see checkerboard textures.
                                                                                            
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
                                                                                            Open issue and make pr. Off the top of my head, a linux / mac port would be nice. Or you could work on the renderer to allow borders for the rects. But work on whatever. I am working on the editor rn.
                                                                                            
                                                                                            ## Project Structure
                                                                                            - `asset/` : texture cache and asset system
                                                                                            - `base/` : common utility functions.
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
                                                                                            