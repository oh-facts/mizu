/* date = August 15th 2024 2:41 pm */

#ifndef EDITOR_H
#define EDITOR_H

struct ED_Panel
{
	v2f pos;
	v2f scale;
	b32 grabbed;
};

struct ED_State
{
	b32 initialized;
	UI_Context *cxt;
	ED_Panel panels[3];
	v2f old_pos;
};

struct State;

function void ed_update(State *state, OS_Event_list *events, f32 delta);
function void ed_draw_panel(UI_Widget *root);
function void ed_draw_children(UI_Widget *root);

#endif //EDITOR_H
