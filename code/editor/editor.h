/* date = August 15th 2024 2:41 pm */

#ifndef EDITOR_H
#define EDITOR_H

#define ED_THEME_BG v4f{{0.14, 0.282, 0.286, 1}}
#define ED_THEME_TEXT v4f{{0.81, 0.46, 0.13, 1}}

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

// rects with borders.
// padding in ui
// make text look proper
// UI_SizeKind_PercentOfParent

// DOTO(mizu): And ofc, make the main engine window also one of these ui windows. 
// Call panels windows, because thats what they are. 


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
  "Game",
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