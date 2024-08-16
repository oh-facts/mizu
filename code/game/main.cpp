#include <game/game.h>

int main(int argc, char **argv)
{
	OS_Api os_api = os_get_api();
	os_api_init(&os_api);
	
	Arena *arena = arena_create();
	void *memory = push_struct(arena, State);
	State *state = (State*)memory;
	state->argc = argc;
	state->argv = argv;
	
	state->os_api = os_api;
	
	Str8 app_dir = os_get_app_dir(arena);
	
	state->app_dir = app_dir;
	
	Str8 dll_rel_path = str8_lit(GAME_DLL);
	Str8 dll_clone_rel_path = str8_lit(GAME_DLL_CLONED);
	
	state->hr.path = str8_join(arena, app_dir, dll_rel_path);
	state->hr.cloned_path = str8_join(arena, app_dir, dll_clone_rel_path);
	
	load_game_dll(&state->hr);
	
	u64 start = os_get_perf_counter();
	u64 freq = os_get_perf_freq();
	
	f64 time_elapsed = 0;
	f64 delta = 0;
	
	do
	{
		if(state->hr.state == HotReloadState_Requested)
		{
			hot_reload(&state->hr);
			state->hr.state = HotReloadState_Completed;
		}
		
		Arena_temp temp = arena_temp_begin(arena);
		f64 time_since_last = time_elapsed;
		
		state->res = total_res;
		state->cmt = total_cmt;
		
		((update_and_render_fn)state->hr.entry)(memory, delta);
		
		u64 end = os_get_perf_counter();
		time_elapsed = (double)(end - start) / freq;
		
		delta = time_elapsed - time_since_last;
		arena_temp_end(&temp);
	}while(!os_window_is_closed(state->win));
	
	return 0;
}