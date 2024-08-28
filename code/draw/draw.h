/* date = July 18th 2024 0:47 pm */

#ifndef DRAW_H
#define DRAW_H

#define D_COLOR_BLACK (v4f){{0,0,0,1}}
#define D_COLOR_WHITE (v4f){{1,1,1,1}}

#define D_COLOR_RED (v4f){{1,0,0,1}}
#define D_COLOR_GREEN (v4f){{0,1,0,1}}
#define D_COLOR_BLUE (v4f){{0,0,1,1}}

#define D_COLOR_YELLOW (v4f){{1,1,0,1}}
#define D_COLOR_MAGENTA (v4f){{1,0,1,1}}
#define D_COLOR_CYAN (v4f){{0,1,1,1}}

#define D_COLOR_THEME_1 (v4f){{0.66519, 0.37321, 0.12030,1}}
#define D_COLOR_THEME_2 (v4f){{0.03, 0.02, 0.03,1}}
#define D_COLOR_THEME_3 (v4f){{0.21044,0.02368,0.06198,1}}

struct D_Proj_view_node
{
	D_Proj_view_node *next;
	m4f v;
};

struct D_Text_params
{
	v4f color;
	f32 scale;
	Atlas *atlas;
	R_Handle *atlas_tex;
};

struct D_Bucket
{
	D_Bucket *next;
	R_Pass_list list;
	D_Proj_view_node *proj_view_top;
};

struct D_State
{
	Arena *arena;
	R_Handle white_square;
	D_Bucket *top;
	D_Text_params default_text_params;
	Arena_temp temp;
};

global D_State *d_state;

function void d_init();
function void d_begin(Atlas *atlas, R_Handle *atlas_tex);
function void d_end();

function D_Bucket *d_bucket();
function void d_push_bucket(D_Bucket *bucket);

function void d_pop_bucket();

function void d_push_proj_view(m4f proj_view);
function void d_pop_proj_view();

function R_Rect *d_draw_img(Rect dst, Rect src, v4f color, R_Handle tex);
function R_Rect *d_draw_rect(Rect dst, v4f color);
function void d_draw_text(Str8 text, v2f pos, D_Text_params *p);

#endif //DRAW_H
