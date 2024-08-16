/* date = July 18th 2024 0:47 pm */

#ifndef DRAW_H
#define DRAW_H

#define D_COLOR_BLACK (v4f){{0,0,0,1}}
#define D_COLOR_WHITE (v4f){{1,1,1,1}}
#define D_COLOR_RED (v4f){{1,0,0,1}}
#define D_COLOR_GREEN (v4f){{0,1,0,1}}
#define D_COLOR_BLUE (v4f){{0,0,1,1}}
#define D_COLOR_THEME_1 (v4f){{0.66519, 0.37321, 0.12030,1}}
#define D_COLOR_THEME_2 (v4f){{0.03, 0.02, 0.03,1}}
#define D_COLOR_THEME_3 (v4f){{0.21044,0.02368,0.06198,1}}

struct D_State
{
	Arena *arena;
	R_Handle white_square;
};

global D_State *d_state;

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
	Arena *arena;
	R_Pass_list list;
	
	D_Proj_view_node *proj_view_top;
	
	D_Text_params default_text_params;
};

function void d_init()
{
	Arena *arena = arena_create();
	d_state = push_struct(arena, D_State);
	d_state->arena = arena;
	u32 white_square[1] = {0xFFFFFFFF};
	d_state->white_square = r_alloc_texture(white_square, 1, 1, 4, &tiled_params);
}

function void d_begin(D_Bucket *draw, Arena *arena, Atlas *atlas, R_Handle *atlas_tex)
{
	memset(draw, 0, sizeof(*draw));
	draw->arena = arena;
	draw->default_text_params =
	(D_Text_params){
		(v4f){{1,1,1,1}},
		0.00007,
		atlas,
		atlas_tex
	};
}

function void d_push_proj_view(D_Bucket *bucket, m4f proj_view)
{
	D_Proj_view_node *node = push_struct(bucket->arena, D_Proj_view_node);
	node->v = proj_view;
	
	if(bucket->proj_view_top)
	{
		node->next = bucket->proj_view_top;
		bucket->proj_view_top = node;
	}
	else
	{
		bucket->proj_view_top = node;
	}
}

function void d_pop_proj_view(D_Bucket *bucket)
{
	bucket->proj_view_top = bucket->proj_view_top->next;
}

function R_Pass *d_pass_from_bucket(D_Bucket *bucket, R_PASS_KIND kind)
{
	R_Pass_node *node = bucket->list.last;
	R_Pass *pass = 0;
	if(!node || node->pass.kind != kind)
	{
		pass = r_push_pass_list(bucket->arena, &bucket->list, kind);
	}
	else
	{
		pass = &node->pass;
	}
	
	return pass;
}

function void d_draw_img(D_Bucket *bucket, v2f pos, v2f scale, v4f color, R_Handle tex)
{
	R_Pass *pass = d_pass_from_bucket(bucket, R_PASS_KIND_UI);
	R_Rect *rect = r_push_batch(bucket->arena, &pass->rect_pass.rects, R_Rect);
	
	rect->tl.x = pos.x;
	rect->tl.y = pos.y;
	
	rect->br.x = rect->tl.x + scale.x;
	rect->br.y = rect->tl.y - scale.y;
	
	rect->tex = tex;
	rect->color = color;
	pass->rect_pass.proj_view = bucket->proj_view_top->v;
}

function void d_draw_rect(D_Bucket *bucket, v2f pos, v2f scale, v4f color)
{
	d_draw_img(bucket, pos, scale, color, d_state->white_square);
}

function void d_draw_text(D_Bucket *bucket, Str8 text, v2f pos, D_Text_params *p)
{
	v2f text_pos = pos;
	f32 width = 0;
	for(u32 i = 0; i < text.len; i ++)
	{
		char c = text.c[i];
		
		Glyph *ch = glyph_from_codepoint(p->atlas, c);
		f32 xpos = text_pos.x + ch->bearing.x * p->scale;
		f32 ypos = text_pos.y + ch->bearing.y * p->scale;
		f32 w = (ch->x1 - ch->x0) * p->scale;
		f32 h = (ch->y1 - ch->y0) * p->scale;
		
		width += ch->advance * p->scale;
		
		if(c == ' ')
		{
			text_pos.x += ch->advance * p->scale;
			continue;
		}
		s32 max_w = 10000;
		if(width + w > max_w)
		{
			width = 0;
			text_pos.x = pos.x;
			text_pos.y -= 0.08;
		}
		else
		{
			text_pos.x += ch->advance * p->scale;
		}
		
		R_Pass *pass = d_pass_from_bucket(bucket, R_PASS_KIND_UI);
		R_Rect *rect = r_push_batch(bucket->arena, &pass->rect_pass.rects, R_Rect);
		
		rect->tl.x = xpos;
		rect->tl.y = ypos - (0.0504f - h);
		
		rect->br.x = rect->tl.x + w;
		rect->br.y = rect->tl.y - h;
		
		rect->tex = p->atlas_tex[(u32)c];
		rect->color = p->color;
		pass->rect_pass.proj_view = bucket->proj_view_top->v;
	}
	
}

#endif //DRAW_H
