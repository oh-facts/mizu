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

typedef int ED_WindowFlags;

enum
{
  ED_WindowFlags_HasSurface = 1 << 0,
  ED_WindowFlags_ChildrenSum = 1 << 1,
  ED_WindowFlags_FixedSize = 1 << 2,
  ED_WindowFlags_Hidden = 1 << 3,
  ED_WindowFlags_Minimized = 1 << 4,
  ED_WindowFlags_Maximized = 1 << 5,
  ED_WindowFlags_Floating = 1 << 6,
  ED_WindowFlags_Grabbed = 1 << 7,
  ED_WindowFlags_Tab = 1 << 8,
};

enum ED_PanelKind
{
	ED_PanelKind_Game,
  ED_PanelKind_TileSetViewer,
	ED_PanelKind_Inspector,
	ED_PanelKind_Debug,
	ED_PanelKind_COUNT,
};

global char *panel_names[ED_PanelKind_COUNT] =
{
  "Alfia",
  "tileset",
  "inspector",
  "debug"
};

struct ED_Window;

struct ED_Panel
{
  ED_Panel *next;
  ED_Panel *prev;
  ED_Window *parent;
  
  ED_PanelKind kind;
	Str8 name;
	
  ED_Panel *inspector;
  
  f32 update_timer;
	
	UI_Widget *selected_slot;
	Str8 selected_tile;
	Rect selected_tile_rect;
	
	f32 cc;
	f32 ft;
	
	v4f hsva;
  
  R_Handle target;
};

struct ED_Window
{
  ED_WindowFlags flags;
  OS_Window win;
	
	UI_Widget *root;
	UI_Context *cxt;
	
  UI_Widget *menu_bar;
  
  v2f pos;
	v2f size;
	
	v2f old_pos;
	
  ED_Panel *active_tab;
  ED_Panel *first_tab;
  ED_Panel *last_tab;
  
  v4f color;
	
  // per frame artifacts
  D_Bucket *bucket;
};

struct State;

function ED_Window *ed_open_window(State *state, ED_WindowFlags flags, v2f pos, v2f size);
function void ed_update(State *state, OS_Event_list *events, f32 delta);
function void ed_draw_spritesheet(ED_Panel *panel, f32 x, f32 y, Str8 path);
function void ed_draw_window(ED_Window *window);
function void ed_draw_children(ED_Window *window, UI_Widget *root);

#endif //EDITOR_H