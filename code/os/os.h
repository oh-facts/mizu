/* date = August 7th 2024 7:05 am */

#ifndef OS_H
#define OS_H

global u64 total_cmt;
global u64 total_res;

enum OS_Key
{
	OS_Key_A,
	OS_Key_B,
	OS_Key_C,
	OS_Key_D,
	OS_Key_E,
	OS_Key_F,
	OS_Key_G,
	OS_Key_H,
	OS_Key_I,
	OS_Key_J,
	OS_Key_K,
	OS_Key_L,
	OS_Key_M,
	OS_Key_N,
	OS_Key_O,
	OS_Key_P,
	OS_Key_Q,
	OS_Key_R,
	OS_Key_S,
	OS_Key_T,
	OS_Key_U,
	OS_Key_V,
	OS_Key_W,
	OS_Key_X,
	OS_Key_Y,
	OS_Key_Z,
	OS_Key_COUNT
};

enum OS_KeyMod
{
	OS_KeyMod_Ctrl = (1 << 0),
	OS_KeyMod_Shift = (1 << 1),
	OS_KeyMod_Alt = (1 << 2),
};

enum OS_EventKind
{
	OS_EventKind_MouseMove,
	OS_EventKind_MousePressed,
	OS_EventKind_MouseReleased,
	OS_EventKind_KeyPressed,
	OS_EventKind_KeyReleased,
	
	OS_EventKind_COUNT,
};

enum OS_MouseButton
{
	OS_MouseButton_Left,
	OS_MouseButton_Middle,
	OS_MouseButton_Right,
	OS_MouseButton_COUNT
};

struct OS_Event
{
	v2s mpos;
	OS_Key key;
	OS_MouseButton button;
	OS_KeyMod mods;
	OS_EventKind kind;
};

struct OS_Event_node
{
	OS_Event_node *next;
	OS_Event_node *prev;
	
	OS_Event v;
};

struct OS_Event_list
{
	OS_Event_node *first;
	OS_Event_node *last;
	
	u32 count;
};

function OS_Event *os_push_event(Arena *arena, OS_Event_list *list);
function void os_consume_event(OS_Event_list *list, OS_Event_node *node);
function b32 os_key_press(OS_Event_list *list, OS_Key key);
function v2s os_mouse_pos(OS_Event_list *list);
function b32 os_mouse_held(OS_MouseButton button);
function b32 os_mouse_pressed(OS_Event_list *list, OS_MouseButton button);
function b32 os_mouse_released(OS_Event_list *list, OS_MouseButton button);

struct OS_Window
{
	u64 handle;
};

function OS_Window os_window_open(Arena *arena, const char *title, s32 w, s32 h, b32 init_opengl);
function void os_window_close(OS_Window win);
function void os_window_toggle_fullscreen(OS_Window win){};

function u64 os_get_perf_counter();
function u64 os_get_perf_freq();
function void os_swap_buffers(OS_Window handle);
function void os_poll_events(Arena *arena, OS_Window handle, OS_Event_list *events);
function b32 os_window_is_closed(OS_Window win);
function void os_window_close(OS_Window win);
function v2s os_get_window_size(OS_Window handle);

function u64 os_get_page_size();
function Str8 os_get_app_dir(Arena *arena);

#if defined(OS_USE_DYNAMIC_HOOKS)

typedef void *(*os_reserve_fn)(u64 size);
typedef b32 (*os_commit_fn)(void *ptr, u64 size);
typedef void (*os_decommit_fn)(void *ptr, u64 size);
typedef void (*os_release_fn)(void *ptr, u64 size);

struct OS_Api
{
	os_reserve_fn os_reserve;
	os_commit_fn os_commit;
	os_decommit_fn os_decommit;
	os_release_fn os_release;
};

global os_reserve_fn os_reserve;
global os_commit_fn os_commit;
global os_decommit_fn os_decommit;
global os_release_fn os_release;

function void os_api_init(OS_Api *api);
function OS_Api os_get_api();

#else
function void *os_reserve(u64 size);
function b32 os_commit(void *ptr, u64 size);
function void os_decommit(void *ptr, u64 size);
function void os_release(void *ptr, u64 size);
#endif

#endif //OS_H
