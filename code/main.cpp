#include <engine.cpp>

int main(int argc, char **argv)
{
	OS_Api os_api = os_getApi();
	os_api_init(&os_api);
	
	Arena *arena = arenaAlloc();
	void *memory = push_struct(arena, State);
	State *state = (State *)memory;
	state->argc = argc;
	state->argv = argv;
	state->arena = arena;
	state->trans = arenaAlloc();
	state->os_api = os_api;
	
	Str8 app_dir = os_getAppDir(arena);
	
	state->app_dir = app_dir;
	
	Str8 dll_rel_path = str8_lit(GAME_DLL);
	Str8 dll_clone_rel_path = str8_lit(GAME_DLL_CLONED);
	
	state->hr.path = str8_join(arena, app_dir, dll_rel_path);
	push_struct(arena, u8);
	state->hr.cloned_path = str8_join(arena, app_dir, dll_clone_rel_path);
	push_struct(arena, u8);
	
	load_game_dll(&state->hr, "update_and_render");
	
	u64 start = os_getPerfCounter();
	u64 freq = os_getPerfFreq();
	
	f64 time_elapsed = 0;
	f64 delta = 0;
	
	for (;;)
	{
		if (state->hr.state == HotReloadState_Requested)
		{
			hot_reload(&state->hr, "update_and_render");
			state->hr.state = HotReloadState_Completed;
		}
		
		f64 time_since_last = time_elapsed;
		
		state->res = total_res;
		state->cmt = total_cmt;
		
		((update_and_render_fn)state->hr.entry)(memory, delta);
		
		u64 end = os_getPerfounter();
		time_elapsed = (double)(end - start) / freq;
		
		delta = time_elapsed - time_since_last;
		
		if (state->ed_state->main_window->win->close_requested)
		{
			return 0;
		}
	}
	
	return 0;
}

/*
loop()
{

}

a : 3 : int;
a : 10 : int;

a : 3;

 a :: 3;
a = 3;

: constant decl
:: mut decl

= assignment

b8
b16
b32
b64

f32
f64

str8
str16
str32

x : 10 -> b8;
y : 5 -> b8;

z : x - y;
z : y - x; 1000 0101 ; 1 + 4 + 128

print("%s", z);
print("%u", z);

update :: (a : int) -> int, float, double
 {

}

  a : (b b32) : int
  {
  return b;
}

  import math/fm as fast_math
  import radio/fm as fm_radio

  use game.players[10].pos as pos;

same as #define pos game.players[10]

different from
v2 pos = game.players[10].pos

or

v2 *pos = &game.players[10].pos

@compile
@inline
@export

iterate through any structure, union, enum
Full introspection
*/