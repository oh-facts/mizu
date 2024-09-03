/* date = August 13th 2024 7:13 am */

#ifndef GAME_H
#define GAME_H
#define FONT_SIZE 0.03
#define STB_SPRINTF_IMPLEMENTATION
#include "stb/stb_sprintf.h"

#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

#include <stdint.h>

#include <stdio.h>

#include <SDL3/SDL.h>
#include <base/base_context_cracking.h>
#if defined(OS_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/mman.h>
#endif

#include <base/base_core.h>
#include <base/base_string.h>
#include <base/base_tcxt.h>
#include <base/base_math.h>
#define OS_USE_DYNAMIC_HOOKS
#include <os/os.h>
#include <file/file_handler.h>
#include <render/render.h>
#include <asset/asset_cache.h>
#include <draw/draw.h>
#include <ui/ui.h>
#include <editor/editor.h>
#include <entity/entity.h>

#include <base/base_core.cpp>
#include <base/base_string.cpp>
#include <base/base_tcxt.cpp>
#include <base/base_math.cpp>
#include <os/os.cpp>
#include <render/render.cpp>
#include <asset/asset_cache.cpp>
#include <draw/draw.cpp>

#if defined(OS_WIN32)
#include <os/os_win32.cpp>
#else
#include <os/os_linux.cpp>
#endif

#include <GL/gl.h>

#if defined(OS_WIN32)
#include <opengl/opengl_khr_platform.h>
#include <opengl/opengl_win32_platform.h>
#else
#include <GL/glx.h>
#include <opengl/opengl_glxext.h>
#include <opengl/opengl_win32_platform.h>

#endif

#include <os/os_sdl.cpp>
#include <render/render_opengl.h>
#include <render/render_opengl.cpp>

#if defined(OS_WIN32)
#define GAME_DLL "yk.dll"
#define GAME_DLL_CLONED "yk_clone.dll"
#elif defined(OS_LINUX)
#define GAME_DLL "libyk.so"
#define GAME_DLL_CLONED "libyk_clone.so"
#elif defined(OS_APPLE)
#define GAME_DLL "libyk.dylib"
#define GAME_DLL_CLONED "libyk_clone.dylib"
#endif

#define max_entities 10

struct State
{
	b32 initialized;
	Arena *arena;
	Arena *trans;
	
	// platform
	int argc;
	char **argv;
	Str8 app_dir;
	OS_Api os_api;
	HotReload hr;
	time_t modified_time;
	u64 res;
	u64 cmt;
	
	// static globals
	TCXT *tcxt;
	D_State *d_state;
	R_Opengl_state *r_opengl_state;
	A_AssetCache *a_asset_cache;
	
	// engine
	OS_Window win;
	OS_Event_list events;
	Atlas atlas;
	R_Handle atlas_tex[256];
	R_Handle fb;
	ED_State ed_state;
	
	// game
	Entity *entities;
	u32 num_entities;
};

#include <editor/editor.cpp>

typedef void (*update_and_render_fn)(void *, f32 delta);

extern "C"
{
	export_function void update_and_render(void *, f32 delta);
}

#endif //GAME_H