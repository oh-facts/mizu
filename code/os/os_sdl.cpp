// TODO(mizu): OS Layer todo. Call it OS_Handle and use OS_Window instead of S_Window. Make
// other changes as neccessary. Also use linked list

struct S_Window
{
	const char *title;
	
	SDL_Window *raw;
	SDL_GLContext gl_cxt;
	s32 w;
	s32 h;
	b32 closed;
	b32 fullscreen;
	
	v2s mpos;
	b32 mdown[8];
	
	OS_Event_list events;
};

global S_Window **s_windows;
global u32 s_window_num;

function S_Window *sdl_win_from_event(SDL_Event *event)
{
	SDL_Window *sdl_win = SDL_GetWindowFromID(event->window.windowID);
	
	S_Window *out = 0;
	
	for(u32 i = 0; i < s_window_num; i++)
	{
		S_Window *s_win = *(s_windows + i);
		if(s_win->raw == sdl_win)
		{
			out = s_win;
			break;
		}
	}
	
	return out;
}

function S_Window *sdl_win_from_os_win(OS_Window win)
{
	return (S_Window*)win.handle;
}

void os_window_set_mouse_pos(OS_Window win, v2s pos)
{
	S_Window *s_win = sdl_win_from_os_win(win);
	s_win->mpos = pos;
}

v2s os_window_get_mouse_pos(OS_Window win)
{
	S_Window *s_win = sdl_win_from_os_win(win);
	return s_win->mpos;
}

void os_window_set_mouse_state(OS_Window win, OS_MouseButton button, b32 state)
{
	S_Window *s_win = sdl_win_from_os_win(win);
	s_win->mdown[button] = state;
}

b32 os_window_get_mouse_state(OS_Window win, OS_MouseButton button)
{
	S_Window *s_win = sdl_win_from_os_win(win);
	return s_win->mdown[button];
}

void os_set_window_size(OS_Window handle, v2f size)
{
	S_Window *s_win = sdl_win_from_os_win(handle);
	SDL_SetWindowSize(s_win->raw, size.x, size.y);
}

void os_set_window_pos(OS_Window handle, v2f pos)
{
	S_Window *s_win = sdl_win_from_os_win(handle);
	SDL_SetWindowPosition(s_win->raw, pos.x, pos.y);
}

v2f os_get_window_pos(OS_Window handle)
{
	S_Window *s_win = sdl_win_from_os_win(handle);
	
	v2f out = {};
	
	v2s wtf = {};
	SDL_GetWindowPosition(s_win->raw, &wtf.x, &wtf.y);
	
	out.x = wtf.x;
	out.y = wtf.y;
	
	return out;
}

void os_window_close(OS_Window win)
{
	sdl_win_from_os_win(win)->closed = 1;
}

u64 os_get_perf_counter()
{
	return SDL_GetPerformanceCounter();
}

u64 os_get_perf_freq()
{
	return SDL_GetPerformanceFrequency();
}

void os_swap_buffers(OS_Window handle)
{
	SDL_GL_SwapWindow(sdl_win_from_os_win(handle)->raw);
}

OS_Event_list *os_event_list_from_window(OS_Window win)
{
	S_Window *s_win = sdl_win_from_os_win(win);
	OS_Event_list *out = &s_win->events;
	
	return out;
}

void os_poll_events(Arena *arena)
{
	SDL_Event sdl_event;
	while (SDL_PollEvent(&sdl_event)) 
	{
		S_Window *win = sdl_win_from_event(&sdl_event);
		OS_Event_list *events = &win->events;
		switch (sdl_event.type) 
		{
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			{
				win->closed = 1;
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
				
				local_persist OS_Key key_shift[256] = {};
				local_persist b32 initialized = 0;
				
				if(!initialized)
				{
					for(u32 i = 0; i < 26; i++)
					{
						key_shift[SDLK_A + i] = (OS_Key)(OS_Key_A + i);
					}
					
					initialized = 1;
				}
				
				OS_Event *event = os_push_event(arena, events);
				event->kind = pressed ? OS_EventKind_KeyPressed : OS_EventKind_KeyReleased;
				
				if(key >= SDLK_A && key <= SDLK_Z)
				{
					event->key = key_shift[key];
				}
				/*
				event->mods = (OS_KeyMod)((mod & mod_shift[SDL_KMOD_CTRL]) | 
																	(mod & mod_shift[SDL_KMOD_SHIFT]) |
																	(mod & mod_shift[SDL_KMOD_ALT]));
				*/
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
				OS_Event *event = os_push_event(arena, events);
				event->kind = OS_EventKind_MouseMove;
				
				event->mpos.x = sdl_event.motion.x;
				event->mpos.y = sdl_event.motion.y;
				/*
				if(sdl_event.motion.x < win->w && sdl_event.motion.x >= 0 && sdl_event.motion.y >= 0 && sdl_event.motion.y < win->h)
				{
					
				}*/
			}break;
			
			case SDL_EVENT_MOUSE_BUTTON_UP:
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			{
				b32 pressed = sdl_event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? 1 : 0;
				
				local_persist b32 initialized = 0;
				local_persist OS_MouseButton button_table_shift[8] = {};
				
				if(!initialized)
				{
					initialized = 1;
					button_table_shift[1] = OS_MouseButton_Left;
					button_table_shift[2] = OS_MouseButton_Middle;
					button_table_shift[3] = OS_MouseButton_Right;
				};
				
				OS_Event *event = os_push_event(arena, events);
				event->kind = pressed ? OS_EventKind_MousePressed : OS_EventKind_MouseReleased;
				event->button = button_table_shift[sdl_event.button.button];
				
				//printf("hi %d %s\n", pressed, win->title);
				
				
			}break;
		}
	}
	
}

b32 os_window_is_closed(OS_Window win)
{
	return sdl_win_from_os_win(win)->closed;
}

v2s os_get_window_size(OS_Window handle)
{
	S_Window *win = sdl_win_from_os_win(handle);
	
	v2s out = {
		.x = win->w,
		.y = win->h,
	};
	
	return out;
}

OS_Window os_window_open(Arena *arena, const char *title, s32 w, s32 h, OS_WindowKind flags)
{
	S_Window *win = push_struct(arena, S_Window);
	win->w = w;
	win->h = h;
	win->closed = 0;
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("%s\n\r", SDL_GetError());
		printf("[Quitting Fatal]\n\r");
		INVALID_CODE_PATH();
	}
	
	if(flags & OS_WindowKind_Opengl)
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
		
		if(flags & OS_WindowKind_Undecorate)
		{
			win->raw = SDL_CreateWindow(title, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE);
		}
		else
		{
			win->raw = SDL_CreateWindow(title, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		}
		
		win->gl_cxt = SDL_GL_CreateContext(win->raw);
		SDL_GL_MakeCurrent(win->raw, win->gl_cxt);
		
		opengl_load_functions();
		
		GLuint default_rubbish_bs_vao;
		glCreateVertexArrays(1,&default_rubbish_bs_vao);
		glBindVertexArray(default_rubbish_bs_vao);
		
		if(!win->gl_cxt)
		{
			printf("%s\n\r", SDL_GetError());
			printf("[Quitting Fatal]\n\r");
			INVALID_CODE_PATH();
		}
		
		SDL_GL_SetSwapInterval(1);
	}
	else
	{
		win->raw = SDL_CreateWindow(title, w, h, 0);
	}
	
	if(!s_windows)
	{
		s_windows = push_array(arena, S_Window*, 10);
	}
	
	s_windows[s_window_num++] = win;
	
	win->title = title;
	
	OS_Window out = {};
	out.handle = (u64)win;
	return out;
}