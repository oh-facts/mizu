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

R_Pass *d_push_pass(Arena *arena, D_Bucket *bucket, R_PASS_KIND kind)
{
	R_Pass_node *node = bucket->list.last;
	R_Pass *pass = 0;
	
  if(!node || node->pass.kind != kind || bucket->stack_gen != bucket->last_stack_gen)
	{
		pass = r_push_pass_list(arena, &bucket->list, kind);
    
    if(bucket->target_top)
    {
      pass->rect_pass.target = bucket->target_top->v;
    }
  }
	else
	{
		pass = &node->pass;
	}
	
	return pass;
}

R_Rect *d_rect(Rect dst, v4f color)
{
	D_Bucket *bucket = d_state->top;
	
  R_Pass *pass = d_push_pass(d_state->arena, bucket, R_PASS_KIND_UI);
	
  R_Rect *out = r_push_batch(d_state->arena, &pass->rect_pass.rects, R_Rect);
	
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

void d_draw_text(Str8 text, v2f pos, D_Text_params *p)
{
	D_Bucket *bucket = d_state->top;
  
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
		
		R_Pass *pass = d_push_pass(d_state->arena, bucket, R_PASS_KIND_UI);
		R_Rect *rect = r_push_batch(d_state->arena, &pass->rect_pass.rects, R_Rect);
		
		rect->dst.tl.x = xpos;
		rect->dst.tl.y = ypos;
		
		rect->dst.br.x = rect->dst.tl.x + w;
		rect->dst.br.y = rect->dst.tl.y + h;
		
		rect->src.tl.x = 0;
		rect->src.tl.y = 0;
		rect->src.br.x = 1;
		rect->src.br.y = 1;
		
		rect->fade[Corner_00] = p->color;
		rect->fade[Corner_01] = p->color;
		rect->fade[Corner_10] = p->color;
		rect->fade[Corner_11] = p->color;
		
		rect->tex = p->atlas_tex[(u32)c];
		
    rect->border_color = p->color;
    rect->radius = 0;
    rect->border_thickness = -2;
    //pass->rect_pass.proj_view = bucket->proj_view_top->v;
	}
  
  bucket->last_stack_gen = bucket->stack_gen;
}