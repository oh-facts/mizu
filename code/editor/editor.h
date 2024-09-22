/* date = August 15th 2024 2:41 pm */

#ifndef EDITOR_H
#define EDITOR_H

// TODO(mizu): If a window is focussed, other windows come to the top too. Also, add a minimize and maximize and close button. And work on making the panels better to interact with.

// Also fix the focus stealing thing where if a window is being interacted with
// but it hovers over another window, other window and current window fight for focus.

// TODO(mizu): Then work on panels and then on tabs

// Also make it so that if a window is ontop of main window, it moves with the main window
// 
// oh yeah, and make parent sibling relationships for the windows
// Main window meaning parent window. child windows always display on top of main window
// and if they are inside the parent window's rect, then they should move with it too (?).

// And give the debug panel lots of functionality to control all other windows's features.

// padding in ui
// make text look proper
// UI_SizeKind_PercentOfParent
// make a transition_time so when you press on ui, you can see it as a certain color for
// some time

// DOTO(mizu): And ofc, make the main engine window also one of these ui windows. 
// Call panels windows, because thats what they are. 
// rects with borders.
// rects with round edges
// render game to an image, then render that as a ui element
// make a render parameter - output frame buffer. used by submit maybe? If 0, renders to default
// fb, otherwise, renders to the passed framebuffer

enum ED_SizeKind
{
  ED_SizeKind_ChildrenSum,
  ED_SizeKind_FixedSize
};

enum ED_WindowKind
{
	ED_WindowKind_Game,
  ED_WindowKind_TileSetViewer,
	ED_WindowKind_Inspector,
	ED_WindowKind_Debug,
	ED_WindowKind_COUNT,
};

global char *ed_window_str[ED_WindowKind_COUNT] = 
{
  "GAME",
  "tile set viewer",
  "inspector",
  "debug"
};

struct ED_Window
{
  ED_WindowKind kind;
	ED_SizeKind sizeKind;
  
  OS_Window win;
	
	UI_Widget *root;
	UI_Context *cxt;
	
  UI_Widget *menu_bar;
  
	Str8 name;
	
  // NOTE(mizu): at some point, make these flags?
  b32 hide;
	b32 minimize;
  b32 maximize;
  b32 floating;
	b32 grabbed;
	
  v2f pos;
	v2f size;
	f32 update_timer;
	
	UI_Widget *selected_slot;
	
	Str8 selected_tile;
	Rect selected_tile_rect;
	
	f32 cc;
	f32 ft;
	v4f color;
	
	v4f hsva;
	
	v2f old_pos;
	
  R_Handle target;
  
  ED_Window *inspector;
  
	D_Bucket *bucket;
};

#define ED_MAX_WINDOWS 10

struct ED_State
{
	Arena *arena;
	ED_Window windows[ED_MAX_WINDOWS];
  s32 num_windows;
};

struct State;

function void ed_init(State *state);
function ED_Window *ed_open_window(ED_State *ed_state, ED_WindowKind kind, ED_SizeKind sizeKind, v2f pos, v2f size);
function void ed_update(State *state, OS_Event_list *events, f32 delta);
function void ed_draw_spritesheet(ED_Window *window, f32 x, f32 y, Str8 path);
function void ed_draw_window(ED_Window *window);
function void ed_draw_children(ED_Window *window, UI_Widget *root);

#endif //EDITOR_H