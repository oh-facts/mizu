// TODO(mizu): Make an OS gfx state, and core OS.
// I am sick of abstracting over SDL
// inside OS gfx state, just call sdl stuff directly. And let other files call sdl
// stuff directly, rather than wrapping it over, since I have very little intention
// of abstracting windows and linux and mac for gfx stuff.

// put memory functions inside OS core
// And see if you can remove source / header files co-existance. It is crap design.
// Working on the editor has gotten easier now that I don't have to update function
// declarations. C is so crappy to write in. But there is too much inertia 
// (thus I won't use better alternatives like odin) and until I get around to making
// my own language / language editor, I will need to use this crap. C is for Crap.
// C++ is Crap ++
// Also, consider raylib.
// I have spent an insane amt of time wrapping over sdl so that in the future, I can
// potentially use other libraries. Not worth it. I am clearly targetting 64 bit desktop 
// OS (I am reserving insane amounts of memory) so I am not going to put any effort in
// abstracting stuff. Once I have hundreds of programmers, I will make them rewrite the OS
// backend so we can do SDL for desktop, and other OS apis for other platforms.
// I can't imagine nintendo, ps or xbox would send me their devkits, so SDL is really fine
// making an OS gfx abstraction is a waste of time (unless you wan't to learn, then its a very
// good reason). I don't want to learn (I have mangled w/ msdn enough), I want to ship a game.

function void os_api_init(OS_Api *api)
{
  os_reserve = api->os_reserve;
	os_commit = api->os_commit;
	os_decommit = api->os_decommit;
	os_release = api->os_release;
}

function OS_Api os_get_api()
{
#if defined(OS_WIN32)
  return os_win32_get_api();
#else
  return os_linux_get_api();
#endif
}

function u64 os_get_page_size()
{
#if defined(OS_WIN32)
  return os_win32_get_page_size();
#else
  return os_linux_get_page_size();
#endif
}

function Str8 os_get_app_dir(Arena *arena)
{
#if defined(OS_WIN32)
  return os_win32_get_app_dir(arena);
#else
  return os_linux_get_app_dir(arena);
#endif
}

function u64 os_get_perf_counter()
{
	return SDL_GetPerformanceCounter();
}

function u64 os_get_perf_freq()
{
	return SDL_GetPerformanceFrequency();
}