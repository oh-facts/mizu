#include "game.h"

void update_and_render(void *memory, f32 delta)
{
	State *state = (State*)memory;
	Arena *arena = state->arena;
	Arena *trans = state->trans;
	
	if(state->hr.state == HotReloadState_Completed)
	{
		os_api_init(&state->os_api);
		
		opengl_load_functions();
		glEnable(GL_FRAMEBUFFER_SRGB);
		
		tcxt = state->tcxt;
		d_state = state->d_state;
		r_opengl_state = state->r_opengl_state;
		a_asset_cache = state->a_asset_cache;
		
		state->hr.state = HotReloadState_Null;
	}
	
	if(!state->initialized)
	{
		state->initialized = 1;
		os_api_init(&state->os_api);
		
		tcxt_init();
		
		state->win = os_window_open(arena, "window", 960, 540, 1);
		
		//glEnable(GL_FRAMEBUFFER_SRGB);
		r_opengl_init();
		d_init();
		a_init();
		
		state->tcxt = tcxt;
		state->d_state = d_state;
		state->r_opengl_state = r_opengl_state;
		state->a_asset_cache = a_asset_cache;
		
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
		
		Str8 font_path = str8_join(state->trans, state->app_dir, str8_lit("../data/assets/fonts/delius.ttf"));
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
		
		state->entities = push_array(state->arena, Entity, max_entities);
		for(u32 i = 0; i < max_entities; i++)
		{
			Entity *e = state->entities + i;
			
			e->tex = push_str8f(state->arena, "debug/numbers%u.png", i+1);
			e->name = str8_lit("player");
		}
		
		arena_temp_end(&temp);
	}
	
	BEGIN_TIMED_BLOCK(UPDATE_AND_RENDER);
	
	Arena_temp temp = arena_temp_begin(trans);
	
	d_begin(&state->atlas, state->atlas_tex);
	
	//f32 zoom = 2;
	
	//f32 aspect = (win_size.x * 1.f)/ win_size.y;
	
	D_Bucket *draw = d_bucket();
	d_push_bucket(draw);
	//d_push_proj_view(m4f_ortho(-aspect * zoom, aspect * zoom, -zoom, zoom, -1.001, 1000).fwd);
	R_Handle bg = a_handle_from_path(str8_lit("debug/clouds.jpg"));
	d_draw_img(rect(0,0,1920,1080), rect(0,0,1,1), D_COLOR_WHITE, bg);
	//d_pop_proj_view();
	
	r_submit(state->win, &draw->list);
	
	d_pop_bucket();
	d_end();
	
	ed_update(state, delta);
	
	
	/*
		if(os_key_press(&state->events, OS_Key_F))
		{
			printf("Toggle Fullscreen\n");
		}
		
		if(os_key_press(&state->events, OS_Key_R) || get_file_last_modified_time((char*)state->hr.path.c) > state->hr.reloaded_time)
		{
			state->hr.state = HotReloadState_Requested;
		}
		
		if(os_key_press(&state->events, OS_Key_Q))
		{
			os_window_close(state->win);
		}
		*/
	
	//state->events.first = state->events.last = 0;
	//state->events.count = 0; 
	arena_temp_end(&temp);
	
	END_TIMED_BLOCK(UPDATE_AND_RENDER);
	
	tcxt_process_debug_counters();
}

void update_game(R_Handle game_tex)
{
	
}