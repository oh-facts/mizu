void d_init()
{
	Arena *arena = arena_create();
	d_state = push_struct(arena, D_State);
	d_state->arena = arena;
	u32 white_square[1] = {0xFFFFFFFF};
	d_state->white_square = r_alloc_texture(white_square, 1, 1, 4, &tiled_params);
}

void d_begin(Atlas *atlas, R_Handle *atlas_tex)
{
	d_state->temp = arena_temp_begin(d_state->arena);
	d_state->default_text_params =
	(D_Text_params){
		(v4f){{1,1,1,1}},
		FONT_SIZE,
		atlas,
		atlas_tex
	};
}

void d_end()
{
	arena_temp_end(&d_state->temp);
}

D_Bucket *d_bucket()
{
	D_Bucket *out = push_struct(d_state->arena, D_Bucket);
	return out;
}

void d_push_bucket(D_Bucket *bucket)
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

void d_pop_bucket()
{
	d_state->top = d_state->top->next;
}

void d_push_proj_view(m4f proj_view)
{
	D_Bucket *bucket = d_state->top;
	D_Proj_view_node *node = push_struct(d_state->arena, D_Proj_view_node);
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

void d_pop_proj_view()
{
	D_Bucket *bucket = d_state->top;
	bucket->proj_view_top = bucket->proj_view_top->next;
}

R_Rect *d_draw_img(Rect dst, Rect src, v4f color, R_Handle tex)
{
	D_Bucket *bucket = d_state->top;
	R_Pass *pass = r_push_pass(d_state->arena, &bucket->list, R_PASS_KIND_UI);
	R_Rect *out = r_push_batch(d_state->arena, &pass->rect_pass.rects, R_Rect);
	
	out->dst = dst;
	out->src = src;
	out->fade[Corner_00] = color;
	out->fade[Corner_01] = color;
	out->fade[Corner_10] = color;
	out->fade[Corner_11] = color;
	out->tex = tex;
	out->color = color;
	
	return out;
	//pass->out_pass.proj_view = bucket->proj_view_top->v;
}

R_Rect *d_draw_rect(Rect dst, v4f color)
{
	Rect src = {};
	src.tl.x = 0;
	src.tl.y = 0;
	src.br.x = 1;
	src.br.y = 1;
	
	return d_draw_img(dst, src, color, d_state->white_square);
}

void d_draw_text(Str8 text, v2f pos, D_Text_params *p)
{
	text_extent ex = 
		ui_text_spacing_stats(p->atlas->glyphs, text, p->scale);
	
	v2f text_pos = pos;
	text_pos.y += ex.tl.y;
	
	//f32 width = 0;
	for(u32 i = 0; i < text.len; i ++)
	{
		char c = text.c[i];
		
		Glyph *ch = glyph_from_codepoint(p->atlas, c);
		f32 xpos = text_pos.x + ch->bearing.x * p->scale;
		
		f32 ypos = text_pos.y + (ch->bearing.y - (ch->y1 + ch->y0)) * p->scale;
		
		f32 w = (ch->x1 - ch->x0) * p->scale;
		f32 h = (ch->y1 - ch->y0) * p->scale;
		
		//width += ch->advance * p->scale;
		
		if(c == ' ')
		{
			text_pos.x += ch->advance * p->scale;
			continue;
		}
		
		text_pos.x += ch->advance * p->scale;
		
		D_Bucket *bucket = d_state->top;
		R_Pass *pass = r_push_pass(d_state->arena, &bucket->list, R_PASS_KIND_UI);
		R_Rect *rect = r_push_batch(d_state->arena, &pass->rect_pass.rects, R_Rect);
		
		rect->dst.tl.x = xpos;
		rect->dst.tl.y = ypos;
		
		rect->dst.br.x = rect->dst.tl.x + w;
		rect->dst.br.y = rect->dst.tl.y + h;
		
		rect->src.tl.x = 0;
		rect->src.tl.y = 0;
		rect->src.br.x = 1;
		rect->src.br.y = 1;
		
		rect->fade[Corner_00] = p->color;;
		rect->fade[Corner_01] = p->color;;
		rect->fade[Corner_10] = p->color;;
		rect->fade[Corner_11] = p->color;;
		
		rect->tex = p->atlas_tex[(u32)c];
		rect->color = p->color;
		//pass->rect_pass.proj_view = bucket->proj_view_top->v;
	}
}