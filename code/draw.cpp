
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

// TODO(mizu): make the gen produce a header and src file
struct D_Proj_view_node;
struct D_Target_node;
struct D_Viewport_node;

struct D_Bucket
{
	D_Bucket *next;
	R_PassList list;
	
	u64 stack_gen;
	u64 last_stack_gen;
	
	D_Proj_view_node *proj_view_top;
	D_Target_node *target_top;
	D_Viewport_node *viewport_top;
};

struct D_State
{
	Arena *arena;
	R_Handle white_square;
	D_Bucket *top;
	ArenaTemp temp;
};

global D_State *d_state;

#include <generated/draw_styles.cpp>

function void d_init()
{
	Arena *arena = arenaAlloc();
	d_state = push_struct(arena, D_State);
	d_state->arena = arena;
	u32 white_square[1] = {0xFFFFFFFF};
	d_state->white_square = r_allocTexture(white_square, 1, 1, 4, &tiled_params);
}

function void d_begin()
{
	d_state->temp = arenaTempBegin(d_state->arena);
}

function void d_end()
{
	arenaTempEnd(&d_state->temp);
}

function D_Bucket *d_bucket()
{
	D_Bucket *out = push_struct(d_state->arena, D_Bucket);
	return out;
}

function void d_pushBucket(D_Bucket *bucket)
{
	if(!d_state->top)
	{
		d_state->top = bucket;
	}
	else
	{
		bucket->next = d_state->top;
		d_state->top = bucket;
	}
}

function void d_popBucket()
{
	d_state->top = d_state->top->next;
}

function R_Pass *d_pushPass(Arena *arena, D_Bucket *bucket, R_PASS_KIND kind)
{
	R_PassNode *node = bucket->list.last;
	R_Pass *pass = 0;
	
	if(!node || node->pass.kind != kind || bucket->stack_gen != bucket->last_stack_gen)
	{
		pass = r_pushPassList(arena, &bucket->list, kind);
		
		if(bucket->target_top)
		{
			pass->rect_pass.target = bucket->target_top->v;
		}
		if(bucket->proj_view_top)
		{
			pass->rect_pass.proj_view = bucket->proj_view_top->v;
		}
	}
	else
	{
		pass = &node->pass;
	}
	
	return pass;
}

function R_Rect *d_rect(Rect dst, v4f color)
{
	D_Bucket *bucket = d_state->top;
	
	R_Pass *pass = d_pushPass(d_state->arena, bucket, R_PASS_KIND_UI);
	
	R_Rect *out = r_pushBatch(d_state->arena, &pass->rect_pass.rects, R_Rect);
	
	out->dst = dst;
	out->src = rect(0, 0, 1, 1);
	out->fade[Corner_00] = color;
	out->fade[Corner_01] = color;
	out->fade[Corner_10] = color;
	out->fade[Corner_11] = color;
	out->tex = d_state->white_square;
	
	// NOTE(mizu): figure out why there is a 1px ghost outline even when thickness is 0
	out->border_color = color;
	out->radius = {};
	out->border_thickness = {};
	bucket->last_stack_gen = bucket->stack_gen;
	return out;
}

function R_Sprite *d_sprite(Rect dst, v4f color)
{
	D_Bucket *bucket = d_state->top;
	
	R_Pass *pass = d_pushPass(d_state->arena, bucket, R_PASS_KIND_SPRITE);
	
	R_Sprite *out = r_pushBatch(d_state->arena, &pass->sprite_pass.sprites, R_Sprite);
	
	out->dst = dst;
	out->src = rect(0, 0, 1, 1);
	out->fade[Corner_00] = color;
	out->fade[Corner_01] = color;
	out->fade[Corner_10] = color;
	out->fade[Corner_11] = color;
	out->tex = d_state->white_square;
	
	// NOTE(mizu): figure out why there is a 1px ghost outline even when thickness is 0
	out->border_color = color;
	out->radius = {};
	out->border_thickness = {};
	bucket->last_stack_gen = bucket->stack_gen;
	return out;
}

function inline R_Sprite *d_spriteCenter(v2f pos, v2f size, v4f color)
{
	return d_sprite(rect(pos - size / 2, size), color);
}

function void d_text(Str8 text, v2f pos, v4f color, f32 scale)
{
	D_Bucket *bucket = d_state->top;
	
	Rect ex = rectFromString(text, scale);
	
	v2f text_pos = pos;
	text_pos.y += ex.tl.y;
	
	//f32 width = 0;
	for(u32 i = 0; i < text.len; i ++)
	{
		char c = text.c[i];
		
		Glyph *ch = glyphFromCodepoint(&font->atlas, c);
		f32 xpos = text_pos.x + ch->bearing.x * scale;
		
		f32 ypos = text_pos.y + (ch->bearing.y - (ch->y1 + ch->y0)) * scale;
		
		f32 w = (ch->x1 - ch->x0) * scale;
		f32 h = (ch->y1 - ch->y0) * scale;
		
		//width += ch->advance * p->scale;
		
		if(c == ' ')
		{
			text_pos.x += ch->advance * scale;
			continue;
		}
		
		text_pos.x += ch->advance * scale;
		
		R_Pass *pass = d_pushPass(d_state->arena, bucket, R_PASS_KIND_UI);
		R_Rect *rect = r_pushBatch(d_state->arena, &pass->rect_pass.rects, R_Rect);
		
		rect->dst.tl.x = xpos;
		rect->dst.tl.y = ypos;
		
		rect->dst.br.x = rect->dst.tl.x + w;
		rect->dst.br.y = rect->dst.tl.y + h;
		
		rect->src.tl.x = 0;
		rect->src.tl.y = 0;
		rect->src.br.x = 1;
		rect->src.br.y = 1;
		
		rect->fade[Corner_00] = color;
		rect->fade[Corner_01] = color;
		rect->fade[Corner_10] = color;
		rect->fade[Corner_11] = color;
		
		rect->tex = font->atlas_tex[(u32)c];
		
		rect->border_color = color;
		rect->radius = 0;
		rect->border_thickness = -2;
	}
	
	bucket->last_stack_gen = bucket->stack_gen;
}