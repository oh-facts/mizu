struct OS_Window
{
	b32 close_requested;
	
	// read only
	const char *title;
	SDL_Window *raw;
	SDL_GLContext gl_cxt;
	s32 w;
	s32 h;
	b32 fullscreen;
	v2f mpos;
	v2f mpos_old;
	b32 mdown[8];
	b32 mdown_old[8];
	b32 keys[256];
	b32 keys_old[256];
};

#define OS_MAX_WINDOWS 10

struct OS_GfxState
{
	Arena *arena;
	OS_Window windows[OS_MAX_WINDOWS];
	u32 window_num;
};

global OS_GfxState *os_gfx_state;

function void os_init()
{
		if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("%s\n\r", SDL_GetError());
		printf("[Quitting Fatal]\n\r");
		INVALID_CODE_PATH();
	}
	
	Arena *arena = arenaAlloc();
	os_gfx_state = push_struct(arena, OS_GfxState);
	os_gfx_state->arena = arena;
}

function OS_Window *os_winFromEvent(SDL_Event *event)
{
	SDL_Window *sdl_win = SDL_GetWindowFromID(event->window.windowID);
	
	OS_Window *out = 0;
	
	for(u32 i = 0; i < os_gfx_state->window_num; i++)
	{
		OS_Window *win = os_gfx_state->windows + i;
		if(win->raw == sdl_win)
		{
			out = win;
			break;
		}
	}
	
	return out;
}

function void os_setWindowSize(OS_Window *win, v2f size)
{
		SDL_SetWindowSize(win->raw, size.x, size.y);
}

function void os_setWindowPos(OS_Window *win, v2f pos)
{
		SDL_SetWindowPosition(win->raw, pos.x, pos.y);
}

function b32 os_mouseHeld(OS_Window *win, u32 key)
{
		//printf("%d\n", win->mdown[key]);
		return win->mdown[key];
}

function b32 os_mousePressed(OS_Window *win, u32 key)
{
		//printf("%d, %d\n", win->mdown[key] , win->mdown_old[key]);
		return win->mdown[key] && !win->mdown_old[key];
}

function b32 os_mouseReleased(OS_Window *win, u32 key)
{
		return !win->mdown[key] && win->mdown_old[key];
}

function b32 os_keyPress(OS_Window *win, u32 key)
{
		return win->keys[key];
}

function void os_pollEvents()
{
	for(s32 i = 0; i < os_gfx_state->window_num; i++)
	{
		OS_Window *window = os_gfx_state->windows + i;
		for(s32 j = 0; j < 8; j++)
		{
			window->mdown_old[j] = window->mdown[j];
		}
	}
	
	SDL_Event sdl_event;
	while (SDL_PollEvent(&sdl_event)) 
	{
		OS_Window *win = os_winFromEvent(&sdl_event);
		
		switch (sdl_event.type) 
		{
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			{
				win->close_requested = 1;
			}break;
			
			case SDL_EVENT_WINDOW_RESIZED:
			{
				win->w = sdl_event.window.data1;
				win->h = sdl_event.window.data2;
			}break;
			
			case SDL_EVENT_KEY_UP:
			case SDL_EVENT_KEY_DOWN:
			{
				b32 pressed = sdl_event.type == SDL_EVENT_KEY_DOWN ? 1 : 0;
				SDL_Keycode key = sdl_event.key.key;
				SDL_Keymod mod = SDL_GetModState();
				
				if(key >= SDLK_A && key <= SDLK_Z)
				{
					win->keys[key] = pressed;;
				}
				
				if ((mod & SDL_KMOD_CTRL) && key == SDLK_F && pressed) 
				{
					if(win->fullscreen)
					{
						SDL_SetWindowFullscreen(win->raw, SDL_FALSE);
						SDL_SetWindowSize(win->raw, 960, 540);
					}
					else
					{
						SDL_SetWindowFullscreen(win->raw, SDL_TRUE);
					}
					
					win->fullscreen = !win->fullscreen;
				} 
				
			}break;
			
			case SDL_EVENT_WINDOW_MOUSE_ENTER:
			{
				SDL_RaiseWindow(win->raw);
			}break;
			
			case SDL_EVENT_MOUSE_MOTION:
			{
				win->mpos.x = sdl_event.motion.x;
				win->mpos.y = sdl_event.motion.y;
				/*
				if(sdl_event.motion.x < win->w && sdl_event.motion.x >= 0 && sdl_event.motion.y >= 0 && sdl_event.motion.y < win->h)
				{
					
				}*/
			}break;
			
			case SDL_EVENT_MOUSE_BUTTON_UP:
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			{
				b32 pressed = sdl_event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? 1 : 0;
				win->mdown[sdl_event.button.button] = pressed;
								
				//printf("hi %d %s\n", pressed, win->title);
				
			}break;
		}
	}
}

function OS_Window *os_windowOpen(const char *title, s32 w, s32 h)
{
	OS_Window *out = os_gfx_state->windows + os_gfx_state->window_num++;
	*out = {};
		
	out->w = w;
	out->h = h;
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
	
	out->raw = SDL_CreateWindow(title, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE);
	out->gl_cxt = SDL_GL_CreateContext(out->raw);
	SDL_GL_MakeCurrent(out->raw, out->gl_cxt);
	
	gladLoadGL();
	
	GLuint default_rubbish_bs_vao;
	glCreateVertexArrays(1,&default_rubbish_bs_vao);
	glBindVertexArray(default_rubbish_bs_vao);
	
	if(!out->gl_cxt)
	{
		printf("%s\n\r", SDL_GetError());
		printf("[Quitting Fatal]\n\r");
		INVALID_CODE_PATH();
	}
	
	out->title = title;
	
	return out;
}