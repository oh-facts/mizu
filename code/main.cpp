#include <engine.cpp>

int main(int argc, char **argv)
{
	OS_Api os_api = os_get_api();
	os_api_init(&os_api);
	
  Arena *arena = arena_create();
	void *memory = push_struct(arena, State);
	State *state = (State*)memory;
	state->argc = argc;
	state->argv = argv;
	state->arena = arena;
	state->trans = arena_create();
	state->os_api = os_api;
	
	Str8 app_dir = os_get_app_dir(arena);
	
	state->app_dir = app_dir;
	
	Str8 dll_rel_path = str8_lit(GAME_DLL);
	Str8 dll_clone_rel_path = str8_lit(GAME_DLL_CLONED);
	
	state->hr.path = str8_join(arena, app_dir, dll_rel_path);
	state->hr.cloned_path = str8_join(arena, app_dir, dll_clone_rel_path);
	
	load_game_dll(&state->hr, "update_and_render");
	
	u64 start = os_get_perf_counter();
	u64 freq = os_get_perf_freq();
	
	f64 time_elapsed = 0;
	f64 delta = 0;
	
	for(;;)
	{
		if(state->hr.state == HotReloadState_Requested)
		{
			hot_reload(&state->hr, "update_and_render");
			state->hr.state = HotReloadState_Completed;
		}
		
		f64 time_since_last = time_elapsed;
		
		state->res = total_res;
		state->cmt = total_cmt;
		
		((update_and_render_fn)state->hr.entry)(memory, delta);
		
		u64 end = os_get_perf_counter();
		time_elapsed = (double)(end - start) / freq;
		
		delta = time_elapsed - time_since_last;
    
    if(state->ed_state->main_window->win->close_requested)
    {
      return 0;
    }
  }
	
	return 0;
}