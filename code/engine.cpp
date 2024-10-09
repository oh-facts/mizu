#define _CRT_SECURE_NO_WARNINGS

#define ED_THEME_BG v4f{{0.643, 0, 0.357, 1}}
#define ED_THEME_BG_DARKER v4f{{0, 0, 0, 0.2}}

#define ED_THEME_TEXT v4f{{1, 0.576, 0.141, 1}}
#define ED_THEME_IMG D_COLOR_WHITE

// must be a way to map these to px sizes so I don't have to do this
// good for arial
//#define FONT_SIZE 0.013986

// good for genesis
//#define FONT_SIZE 0.025

// good for delius
#define FONT_SIZE 0.029

#define STB_SPRINTF_IMPLEMENTATION
#include "third_party/stb/stb_sprintf.h"

#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "third_party/stb/stb_truetype.h"

#define CGLTF_IMPLEMENTATION
#include <third_party/cgltf/cgltf.h>

#include <stdint.h>

#include <stdio.h>

#include <third_party/SDL3/SDL.h>

#include <third_party/KHR/khrplatform.h>
#include <third_party/glad/glad.h>
#include <third_party/glad/glad.c>
#include <third_party/box2d/box2d.h>

#include <context_cracking.cpp>
#if defined(OS_WIN32)
#define WIN32_LEAN_AND_MEAN
#undef function
#include <windows.h>
#define function static
#else
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/mman.h>
#endif

#include <base.cpp>

#if defined(OS_WIN32)
#include <backends/os_win32.cpp>
#else
#include <backends/os_linux.cpp>
#endif

#include <os_core.cpp>

#include <os_gfx.cpp>
#include <render.cpp>
#include <asset_cache.cpp>
#include <gltf_loader.cpp>

// TODO(mizu): NEEDS TO GO. I want to spend time on the game, so I will do this later
// The goal is to make it part of asset cache, so I can do text("cascadia", "hi")
struct Font
{
	Atlas atlas;
	R_Handle atlas_tex[256];
};

global Font *font;

#include <draw.cpp>
#include <ui.cpp>

#include <backends/render_opengl.cpp>

#include <editor.cpp>

#include <game.cpp>

#include <hot_reload.cpp>

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
	A_State *a_state;
	ED_State *ed_state;
	OS_GfxState *os_gfx_state;
	Font *font;
};

typedef void (*update_and_render_fn)(void *, f32 delta);

extern "C"
{
	export_function void update_and_render(void *, f32 delta);
}

// NOTE(mizu): Idea : pass arena. Keep State as global? That way I won't need to compile
// main everytime. If State is 0, means need to reassign / possibly uninitialized
// Also, font cache please. I will have an iffy one at first (works like tex cache. Unique tex for each glyph. Then I will do proper font packing.

function void update_and_render(void *memory, f32 delta)
{
	State *state = (State*)memory;
	//Arena *arena = state->arena;
	Arena *trans = state->trans;
	
	if(state->hr.state == HotReloadState_Completed)
	{
		os_api_init(&state->os_api);
		
		gladLoadGL();
		//glEnable(GL_FRAMEBUFFER_SRGB);
		
		tcxt = state->tcxt;
		d_state = state->d_state;
		r_opengl_state = state->r_opengl_state;
		a_state = state->a_state;
		ed_state = state->ed_state;
		os_gfx_state = state->os_gfx_state;
		font = state->font;
		
		state->hr.state = HotReloadState_Null;
	}
	
	if(!state->initialized)
	{
		state->initialized = 1;
		os_api_init(&state->os_api);
		
		tcxt_init();
		ed_state = ed_alloc();
		//glEnable(GL_FRAMEBUFFER_SRGB);
		os_init();
		state->os_gfx_state = os_gfx_state;
		
		SDL_GL_SetSwapInterval(1);
		ED_Window *game_win = ed_openWindow(ED_WindowFlags_HasSurface | ED_WindowFlags_ChildrenSum, v2f{{251,50}}, v2f{{960, 540}});
		
		ED_Panel *main_panel = ed_openPanel(game_win, Axis2_X, 1);
		
		ed_openTab(main_panel, ED_TabKind_ModelViewer);
		
		ED_Tab *ts_viewer = ed_openTab(main_panel, ED_TabKind_TileSetViewer);
		ED_Tab *insp = ed_openTab(main_panel, ED_TabKind_Inspector);
		insp->hsva = {{1,0,1,1}};
		ts_viewer->inspector = insp;
		
		ED_Tab *game = ed_openTab(main_panel, ED_TabKind_Custom);
		game->custom_draw = game_update_and_render;
		game->custom_drawData = push_struct(state->arena, Game);
		
		r_opengl_init();
		
		d_init();
		a_init();
		
		state->tcxt = tcxt;
		state->d_state = d_state;
		state->r_opengl_state = r_opengl_state;
		state->a_state = a_state;
		state->ed_state = ed_state;
		
		char codepoints[] =
		{
			'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 
			's', 't', 'u', 'v', 'w', 'x', 'y','z',
			
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
			'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
			
			'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
			
			'&', '.', '?', ',', '-', ':', '!', '%', '#',
			
			'(', ')', '[', ']', '{', '}',
			
			' ', '\n', '\\', '/'
		};
		
		ArenaTemp temp = arenaTempBegin(state->trans);
		
		state->font = push_struct(state->arena, Font);
		
		Str8 font_path = str8_join(state->trans, state->app_dir, str8_lit("../data/assets/fonts/delius.ttf"));
		Glyph *temp_font = make_bmp_font(font_path.c, codepoints, ARRAY_LEN(codepoints), state->trans);
		
		for(u32 i = 0; i < ARRAY_LEN(codepoints); i ++)
		{
			u32 c = codepoints[i];
			
			if(c != '\n' && c != ' ')
			{
				state->font->atlas_tex[c] = r_allocTexture(temp_font[i].bmp, temp_font[i].w, temp_font[i].h, 1, &font_params);
			}
			
			state->font->atlas.glyphs[c].bearing = temp_font[i].bearing;
			state->font->atlas.glyphs[c].advance = temp_font[i].advance;
			state->font->atlas.glyphs[c].x0 = temp_font[i].x0;
			state->font->atlas.glyphs[c].x1 = temp_font[i].x1;
			state->font->atlas.glyphs[c].y0 = temp_font[i].y0;
			state->font->atlas.glyphs[c].y1 = temp_font[i].y1;
		}
		
		font = state->font;
		
		arenaTempEnd(&temp);
	}
	
	total_res = state->res;
	total_cmt = state->cmt;
	
	BEGIN_TIMED_BLOCK(UPDATE_AND_RENDER);
	ArenaTemp temp = arenaTempBegin(trans);
	
	os_pollEvents();
	d_begin();
	
	ed_update(delta);
	ed_submit();
	
	d_end();
	
	if(os_keyPress(ed_state->main_window->win, SDLK_R) || get_file_last_modified_time((char*)state->hr.path.c) > state->hr.reloaded_time)
	{
		state->hr.state = HotReloadState_Requested;
	}
	
	arenaTempEnd(&temp);
	
	END_TIMED_BLOCK(UPDATE_AND_RENDER);
	
	tcxt_process_debug_counters();
}