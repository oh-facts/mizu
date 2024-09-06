/* date = August 15th 2024 2:41 pm */

#ifndef EDITOR_H
#define EDITOR_H

#define ED_THEME_BG v4f{{0.14, 0.282, 0.286, 0.8}}

#define ED_THEME_TEXT v4f{{0.81, 0.46, 0.13, 1}}

enum ED_PanelKind
{
	ED_PanelKind_TileSetViewer,
	ED_PanelKind_Inspector,
	ED_PanelKind_Debug,
	ED_PanelKind_COUNT,
};

struct ED_Panel
{
	OS_Window win;
	
	UI_Widget *root;
	UI_Context *cxt;
	
	Str8 name;
	b32 hide;
	b32 floating;
	v2f pos;
	v2f scale;
	b32 grabbed;
	f32 update_timer;
	f32 cc;
	f32 ft;
	v4f color;
	
	Str8 selected_tile;
	Rect selected_tile_rect;
	UI_Widget *selected_slot;
	
	v4f hsva;
	
	v2f old_pos;
	
	D_Bucket *bucket;
};

struct ED_State
{
	Arena *arena;
	b32 initialized;
	ED_Panel panels[ED_PanelKind_COUNT];
};

struct State;

function void ed_update(State *state, OS_Event_list *events, f32 delta);
function void ed_draw_spritesheet(ED_Panel *panel, f32 x, f32 y, Str8 path);
function void ed_draw_panel(ED_Panel *panel);
function void ed_draw_children(ED_Panel *panel, UI_Widget *root);

#endif //EDITOR_H
