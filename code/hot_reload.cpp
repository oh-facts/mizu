enum HotReloadState
{
	HotReloadState_Null,
	HotReloadState_Requested,
	HotReloadState_Completed,
};

struct HotReload
{
	HotReloadState state;
	void *entry;
	void *game_dll;
	Str8 path;
	Str8 cloned_path;
	time_t reloaded_time;
};

function void load_game_dll(HotReload *reload, const char *entry_name)
{
	while(!clone_file((char*)reload->path.c, (char*)reload->cloned_path.c))
	{
		_sleep(10);
	}
	
	reload->game_dll = SDL_LoadObject((char*)reload->cloned_path.c);
	if (!reload->game_dll)
	{
		printf("dll not found, reload failed\n\r");
		INVALID_CODE_PATH();
	}
	
	reload->entry = (void*)SDL_LoadFunction(reload->game_dll, entry_name);
	reload->reloaded_time = get_file_last_modified_time((char*)reload->path.c);
}

function void hot_reload(HotReload *reload, const char *entry_name)
{
	SDL_UnloadObject(reload->game_dll);
	load_game_dll(reload, entry_name);
}