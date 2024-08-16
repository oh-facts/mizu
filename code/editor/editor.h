/* date = August 15th 2024 2:41 pm */

#ifndef EDITOR_H
#define EDITOR_H

struct ED_Panel
{
	v2f size;
};

struct ED_State
{
	b32 initialized;
	UI_Context *cxt;
	ED_Panel panels[3];
};

struct State;

function void ed_update(State *state, OS_Event_list *events, f32 delta);
#endif //EDITOR_H
