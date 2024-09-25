#define _CRT_SECURE_NO_WARNINGS

#define ED_THEME_BG v4f{{0.14, 0.282, 0.286, 1}}
#define ED_THEME_BG_DARKER v4f{{0, 0, 0, 0.2}}

#define ED_THEME_TEXT v4f{{0.81, 0.46, 0.13, 1}}

#define FONT_SIZE 0.01

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
#include <draw.cpp>

#include <ui.cpp>

#include <editor.cpp>

#include <backends/render_opengl.cpp>

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
  
	// engine
	Atlas atlas;
	R_Handle atlas_tex[256];
};

typedef void (*update_and_render_fn)(void *, f32 delta);

extern "C"
{
  export_function void update_and_render(void *, f32 delta);
}

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
    
    ED_Window *game_win = ed_open_window(ED_WindowFlags_HasSurface | ED_WindowFlags_ChildrenSum, v2f{{480, 46}}, v2f{{960, 540}});
    ED_Panel *main_panel = ed_open_panel(game_win, Axis2_X, 1);
    
    ED_Tab *ts_viewer = ed_open_tab(main_panel, ED_TabKind_TileSetViewer);
    ED_Tab *insp = ed_open_tab(main_panel, ED_TabKind_Inspector);
    insp->hsva = {{1,0,1,1}};
    ts_viewer->inspector = insp;
    
    ed_open_tab(main_panel, ED_TabKind_Game);
    
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
			
			'&', '.', '?', ',', '-', ':', '!', '%', '\\', '/',
			
			'(', ')', '[', ']', '{', '}',
			
			' ', '\n'
		};
		
		Arena_temp temp = arena_temp_begin(state->trans);
		
		Str8 font_path = str8_join(state->trans, state->app_dir, str8_lit("../data/assets/fonts/arial.ttf"));
		Glyph *temp_font = make_bmp_font(font_path.c, codepoints, ARRAY_LEN(codepoints), state->trans);
		
		for(u32 i = 0; i < ARRAY_LEN(codepoints); i ++)
		{
			u32 c = codepoints[i];
			
			if(c != '\n' && c != ' ')
			{
				state->atlas_tex[c] = r_alloc_texture(temp_font[i].bmp, temp_font[i].w, temp_font[i].h, 1, &font_params);
			}
			
			state->atlas.glyphs[c].bearing = temp_font[i].bearing;
			state->atlas.glyphs[c].advance = temp_font[i].advance;
			state->atlas.glyphs[c].x0 = temp_font[i].x0;
			state->atlas.glyphs[c].x1 = temp_font[i].x1;
			state->atlas.glyphs[c].y0 = temp_font[i].y0;
			state->atlas.glyphs[c].y1 = temp_font[i].y1;
		}
		
    //os_window_close(temp_win);
    
		arena_temp_end(&temp);
	}
	
	BEGIN_TIMED_BLOCK(UPDATE_AND_RENDER);
	Arena_temp temp = arena_temp_begin(trans);
	
	os_poll_events();
	d_begin(&state->atlas, state->atlas_tex);
	
  f32 aspect = 16 / 9.f;
  f32 zoom = 1;
  
  D_Bucket *draw = d_bucket();
	d_push_bucket(draw);
	d_push_proj_view(m4f_ortho(-aspect * zoom, aspect * zoom, -zoom, zoom, -1.001, 1000).fwd);
	
  //gltf_load_mesh(temp.arena, str8_lit("gltf_test/asuka/scene.gltf"));
  
  ed_update(&state->atlas, delta);
  
  for(s32 i = 0; i < ed_state->num_windows; i++)
	{
		ED_Window *window = ed_state->windows + i;
		r_submit(window->win, &window->bucket->list);
	}
	
  d_pop_proj_view();
	d_pop_bucket();
	d_end();
	
	/*
		if(os_key_press(&state->events, state->win, OS_Key_F))
	{
		printf("Toggle Fullscreen\n");
	}
	
		if(os_key_press(&state->events, OS_Key_Q))
		{
			os_window_close(state->win);
		}
		*/
  
  if(os_key_press(ed_state->main_window->win, SDLK_R) || get_file_last_modified_time((char*)state->hr.path.c) > state->hr.reloaded_time)
  {
    state->hr.state = HotReloadState_Requested;
  }
  
	arena_temp_end(&temp);
	
  for(s32 i = 0; i < ed_state->num_windows; i++)
	{
    OS_Window *window = ed_state->windows[i].win;
		
    for(s32 j = 0; j < 8; j++)
    {
      window->mdown_old[j] = window->mdown[j];
    }
  }
	
	END_TIMED_BLOCK(UPDATE_AND_RENDER);
	
	tcxt_process_debug_counters();
}

void update_game(R_Handle game_tex)
{
	
}