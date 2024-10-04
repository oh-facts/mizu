// TODO(mizu): Major: widget focus and metaprogram implicit parameter stacks.
// Minor : Refactor pass. Transitions.
#define UI_DeferLoop(begin, end) for(int _i_ = ((begin), 0); !_i_; _i_ += 1, (end))

enum Axis2
{
	Axis2_0 = -1,
	Axis2_X = 0,
	Axis2_Y,
	Axis2_COUNT
};

enum UI_AlignKind
{
	UI_AlignKind_Left,
	UI_AlignKind_Right,
};

enum UI_SizeKind
{
	UI_SizeKind_Null,
	UI_SizeKind_Pixels,
	UI_SizeKind_TextContent,
	UI_SizeKind_PercentOfParent,
	UI_SizeKind_ChildrenSum,
};

struct UI_Size
{
	UI_SizeKind kind;
	f32 value;
	f32 strictness;
};

enum UI_Flags
{
	UI_Flags_has_text = 1 << 0,
	UI_Flags_has_bg = 1 << 1,
	UI_Flags_clickable = 1 << 2,
	UI_Flags_has_custom_draw = 1 << 3,
	UI_Flags_has_scroll = 1 << 4,
	UI_Flags_is_floating_x = 1 << 5,
	UI_Flags_is_floating_y = 1 << 6,
};

struct UI_Widget;

struct UI_Signal
{
	UI_Widget *widget;
	b32 hot;
	b32 active;
	b32 toggle;
};

#define UI_CUSTOM_DRAW(name) void name(struct UI_Widget *widget, void *user_data)
typedef UI_CUSTOM_DRAW(UI_WidgetCustomDrawFunctionType);

struct UI_Widget
{
	UI_Widget *first;
	UI_Widget *last;
	UI_Widget *next;
	UI_Widget *prev;
	UI_Widget *parent;
	u64 num_child;
	
	UI_Widget *hash_next;
	UI_Widget *hash_prev;
	
	u64 last_frame_touched_index;
	
	u32 id;
	u64 hash;
	
	UI_Flags flags;
	R_Handle img;
	Str8 text;
	UI_Size pref_size[Axis2_COUNT];
	
	v4f color;
	v4f bg_color;
	v4f hover_color;
  v4f press_color;
  v4f border_color;
	
	f32 border_thickness;
	f32 radius;
	
  Axis2 child_layout_axis;
	UI_AlignKind alignKind;
  v2f fixed_position;
	UI_WidgetCustomDrawFunctionType *custom_draw;
	void *custom_draw_data;
	
	// calculated after hierearchy pass
	f32 computed_rel_position[Axis2_COUNT];
	f32 computed_size[Axis2_COUNT];
	
	// persistant
	v2f pos;
	v2f size;
	
	b32 hot;
	b32 active;
	b32 toggle;
};

#include <generated/ui_styles.h>

struct UI_Hash_slot
{
	UI_Widget *first;
	UI_Widget *last;
};

struct UI_Context
{
	// perm arena
	Arena *arena;
	
	// reset completely every frame. Non zeroed.
	Arena *frame_arena;
	
	OS_Window *win;
	
	u64 hash_table_size;
	UI_Hash_slot *hash_slots;
	
	u64 frames;
	
	UI_Widget *widget_free_list;
	
	UI_STYLE_STACKS;
	
	u32 num;
};

#include <generated/ui_styles.cpp>

function UI_Signal ui_signal(UI_Context *cxt, UI_Widget *widget)
{
	UI_Signal out = {};
	
	v2f tl = {};
	tl.x = widget->pos.x;
	tl.y = widget->pos.y;
	
	v2f br = {};
	br.x = tl.x + widget->size.x;
	br.y = tl.y + widget->size.y;
	
	v2f mpos = cxt->win->mpos;
	if(mpos.x > tl.x && mpos.x < br.x && mpos.y > tl.y && mpos.y < br.y)
	{
		out.hot = true;
	}
	
	widget->hot = out.hot;
	
	out.active = widget->active;
	out.toggle = widget->toggle;
	out.widget = widget;
	
	return out;
}

function UI_Widget *ui_alloc_widget(UI_Context *cxt)
{
	UI_Widget *out = cxt->widget_free_list;
	
	if(out)
	{
		cxt->widget_free_list= cxt->widget_free_list->next;
		memset(out, 0, sizeof(*out));
	}
	else
	{
		out = push_struct(cxt->arena, UI_Widget);
	}
	
	return out;
}

function void ui_free_widget(UI_Context *cxt, UI_Widget *node)
{
	node->next = cxt->widget_free_list;
	cxt->widget_free_list = node;
}

#define ui_push_size_kind(cxt, kind) ui_push_size_kind_x(cxt, kind); \
ui_push_size_kind_y(cxt, kind);

#define ui_pop_size_kind(cxt) ui_pop_size_kind_x(cxt); \
ui_pop_size_kind_y(cxt);

function UI_Context *ui_alloc_cxt()
{
	Arena *arena = arena_create();
	
	UI_Context *cxt = push_struct(arena, UI_Context);
	
	cxt->arena = arena;
	
	cxt->hash_table_size = 1024;
	cxt->hash_slots = push_array(arena, UI_Hash_slot, cxt->hash_table_size);
	cxt->frame_arena = arena_create();
	
	ui_push_text_color(cxt, D_COLOR_WHITE);
	ui_push_bg_color(cxt, D_COLOR_WHITE);
	ui_push_hover_color(cxt, D_COLOR_WHITE);
  ui_push_press_color(cxt, D_COLOR_WHITE);
  ui_push_border_color(cxt, (v4f{{}}));
  ui_push_border_thickness(cxt, 0);
  ui_push_radius(cxt, 0);
  
  ui_push_pref_width(cxt, 0);
	ui_push_pref_height(cxt, 0);
	ui_push_fixed_pos(cxt, v2f{{0,0}});
	ui_push_size_kind(cxt, UI_SizeKind_Null);
	ui_push_align_kind_x(cxt, UI_AlignKind_Left);
	
	cxt->frames = 0;
	
	return cxt;
}

// djb2
function unsigned long ui_hash(Str8 str)
{
	unsigned long hash = 5381;
	int c;
	
	for(u32 i = 0; i < str.len; i++)
	{
		c = str.c[i];
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}
	
	return hash;
}

function UI_Widget *ui_widget_from_hash(UI_Context *cxt, u64 hash)
{
	UI_Widget *widget = 0;
	
	u64 slot =  hash % cxt->hash_table_size;
	
	UI_Widget *cur = cxt->hash_slots[slot].first;
	while(cur)
	{
		if(hash == cur->hash)
		{
			widget = cur;
			break;
		}
		
		cur = cur->hash_next;
	}
	
	return widget;
}

function UI_Widget *ui_make_widget(UI_Context *cxt, Str8 text)
{
	u64 hash = ui_hash(text);
	UI_Widget *widget = 0;
	
	if(hash == ui_hash(str8_lit("")))
	{
		widget = push_struct(cxt->frame_arena, UI_Widget);
		*widget = {};
	}
	else
	{
		widget = ui_widget_from_hash(cxt, hash);
		
		if(widget)
		{
			if(widget->hot && os_mouse_pressed(cxt->win, SDL_BUTTON_LEFT))
			{
				widget->toggle = !widget->toggle;
				widget->active = 1;
			}
			else
			{
				widget->active = 0;
			}
			
			widget->prev = 0;
			widget->last = 0;
			widget->next = 0;
			widget->last_frame_touched_index = cxt->frames;
			widget->computed_size[0] = 0;
			widget->computed_size[1] = 0;
			
			widget->computed_rel_position[0] = 0;
			widget->computed_rel_position[1] = 0;
		}
		else
		{
			widget = ui_alloc_widget(cxt);
			
			u64 slot = hash % cxt->hash_table_size;
			
			if(cxt->hash_slots[slot].last)
			{
				cxt->hash_slots[slot].last = cxt->hash_slots[slot].last->hash_next = widget;
			}
			else
			{
				cxt->hash_slots[slot].last = cxt->hash_slots[slot].first = widget;
			}
			//cxt->hash_slots[slot].first = widget;
			widget->last_frame_touched_index = cxt->frames;
			widget->id = cxt->num++;
		}
	}
	
	widget->hash = hash;
	widget->text.c = push_array(cxt->frame_arena, u8, text.len);
	widget->text.len = text.len;
	str8_cpy(&widget->text, &text);
	
	if(cxt->parent_stack.top)
	{
		UI_Widget *parent = cxt->parent_stack.top->v;
		parent->num_child++;
		widget->parent = parent;
		
		if(parent->last)
		{
			widget->prev = parent->last;
			parent->last = parent->last->next = widget;
		}
		else
		{
			parent->last = parent->first = widget;
		}
	}
	
  widget->alignKind = cxt->align_kind_x_stack.top->v;
	widget->color = cxt->text_color_stack.top->v;
	widget->bg_color = cxt->bg_color_stack.top->v;
	widget->hover_color = cxt->hover_color_stack.top->v;
  widget->press_color = cxt->press_color_stack.top->v;
  widget->border_color = cxt->border_color_stack.top->v;
	widget->border_thickness = cxt->border_thickness_stack.top->v;
  widget->radius = cxt->radius_stack.top->v;
  
  Rect extent = ui_text_spacing_stats(font->atlas.glyphs, text, FONT_SIZE);
	
	widget->pref_size[Axis2_X].kind = cxt->size_kind_x_stack.top->v;
	
	switch(widget->pref_size[Axis2_X].kind)
	{
		default:
		case UI_SizeKind_Null: {}break;
		case UI_SizeKind_ChildrenSum:
		{
			widget->pref_size[Axis2_X].value = 0;
		}break;
		case UI_SizeKind_PercentOfParent:
		case UI_SizeKind_Pixels:
		{
			widget->pref_size[Axis2_X].value = cxt->pref_width_stack.top->v;
		}break;
		case UI_SizeKind_TextContent:
		{
			widget->pref_size[Axis2_X].value = extent.br.x;
		}break;
	}
	
	widget->pref_size[Axis2_Y].kind = cxt->size_kind_y_stack.top->v;
	
	switch(widget->pref_size[Axis2_Y].kind)
	{
		default:
		case UI_SizeKind_Null: {}break;
		
		case UI_SizeKind_ChildrenSum:
		{
			widget->pref_size[Axis2_Y].value = 0;
		}break;
		case UI_SizeKind_PercentOfParent:
		case UI_SizeKind_Pixels:
		{
			widget->pref_size[Axis2_Y].value = cxt->pref_height_stack.top->v;
		}break;
		case UI_SizeKind_TextContent:
		{
			widget->pref_size[Axis2_Y].value = extent.tl.y - extent.br.y;
		}break;
	}
	
	widget->fixed_position = cxt->fixed_pos_stack.top->v;
	
	if(cxt->child_layout_axis_stack.auto_pop)
	{
		widget->child_layout_axis = cxt->child_layout_axis_stack.top->v;
		cxt->child_layout_axis_stack.auto_pop = 0;
		ui_pop_child_layout_axis(cxt);
	}
	return widget;
}

function UI_Signal ui_begin_named_rowf(UI_Context *cxt, char *fmt, ...)
{
	Arena_temp temp = scratch_begin(0,0);
	va_list args;
	va_start(args, fmt);
	Str8 text = push_str8fv(temp.arena, fmt, args);
	va_end(args);
	
	ui_set_next_child_layout_axis(cxt, Axis2_X);
	UI_Widget *widget = ui_make_widget(cxt, text);
	ui_push_parent(cxt, widget);
	arena_temp_end(&temp);
	
	UI_Signal out = ui_signal(cxt, widget);
	
	return out;
}

function void ui_end_row(UI_Context *cxt)
{
	ui_pop_parent(cxt);
}

// TODO(mizu): make it so that transient nodes are allocated from the frame arena and aren't added to the free list when popped

function UI_Signal ui_begin_row(UI_Context *cxt)
{
	return ui_begin_named_rowf(cxt, "");
}

function UI_Signal ui_begin_named_colf(UI_Context *cxt, char *fmt, ...)
{
	Arena_temp temp = scratch_begin(0,0);
	va_list args;
	va_start(args, fmt);
	Str8 text = push_str8fv(temp.arena, fmt, args);
	va_end(args);
	
	ui_set_next_child_layout_axis(cxt, Axis2_Y);
	UI_Widget *widget = ui_make_widget(cxt, text);
	ui_push_parent(cxt, widget);
	arena_temp_end(&temp);
	
	UI_Signal out = ui_signal(cxt, widget);
	
	return out;
}

function UI_Signal ui_begin_col(UI_Context *cxt)
{
	return ui_begin_named_colf(cxt, "");
}

function void ui_end_col(UI_Context *cxt)
{
	ui_pop_parent(cxt);
}

function UI_Signal ui_label(UI_Context *cxt, Str8 text)
{
	UI_Widget *widget = ui_make_widget(cxt, text);
	widget->flags = UI_Flags_has_text;
	
	UI_Signal out = ui_signal(cxt, widget);
	
	return out;
}

function UI_Signal ui_labelf(UI_Context *cxt, char *fmt, ...)
{
	Arena_temp temp = scratch_begin(0,0);
	va_list args;
	va_start(args, fmt);
	Str8 text = push_str8fv(temp.arena, fmt, args);
	va_end(args);
	
	UI_Signal out = ui_label(cxt, text); 
	arena_temp_end(&temp);
	return out;
}

function v3f hsv_to_rgb(v3f hsv)
{
	float h = hsv.x;
	float s = hsv.y;
	float v = hsv.z;
	
	float c = v * s;
	float x = c * (1 - fabsf(fmodf(h / 60.0f, 2) - 1));
	float m = v - c;
	
	float r, g, b;
	if (h >= 0 && h < 60) {
		r = c; g = x; b = 0;
	} else if (h >= 60 && h < 120) {
		r = x; g = c; b = 0;
	} else if (h >= 120 && h < 180) {
		r = 0; g = c; b = x;
	} else if (h >= 180 && h < 240) {
		r = 0; g = x; b = c;
	} else if (h >= 240 && h < 300) {
		r = x; g = 0; b = c;
	} else {
		r = c; g = 0; b = x;
	}
	
	v3f rgb = {{ (r + m), (g + m), (b + m) }};
	return rgb;
}

function v4f hsva_to_rgba(v4f hsva)
{
	v3f hsv = hsv_to_rgb(hsva.xyz);
	v4f out = {
		.x = hsv.x,
		.y = hsv.y,
		.z = hsv.z,
		.w = hsva.w,
	};
	
	return out;
}

struct UI_SatPickerDrawData
{
	s32 hue;
	f32 sat;
	f32 val;
};

function UI_CUSTOM_DRAW(ui_sat_picker_draw)
{
	UI_SatPickerDrawData *draw_data = (UI_SatPickerDrawData *)user_data;
	
	Rect w_rect = rect(widget->pos, widget->size);
	
	{
		//static f32 red = 0;
		//red += delta * 100;
		v3f rgb = hsv_to_rgb({{draw_data->hue * 1.f, 1, 1}});;
		
		v4f col = {{}};
		col.x = rgb.x;
		col.y = rgb.y;
		col.z = rgb.z;
		col.w = 1;
		
		R_Rect *recty = d_rect(w_rect, {{0,0,0,0.1}});
		
		recty->fade[Corner_00] = v4f{{1, 1, 1, 1}};
		recty->fade[Corner_01] = v4f{{1, 1, 1, 1}};
		recty->fade[Corner_10] = col;
		recty->fade[Corner_11] = col;
    recty->radius = 10;
    
  }
	
	{
		R_Rect *recty = d_rect(w_rect, {{0,0,0,0}});
		
		recty->fade[Corner_00] = v4f{{0,0,0,0}};
		recty->fade[Corner_01] = v4f{{0, 0, 0, 1}};
		recty->fade[Corner_10] = v4f{{0,0,0,0}};
		recty->fade[Corner_11] = v4f{{0, 0, 0, 1}};
    recty->radius = 10;
  }
	
	// indicator
	{
		v2f size = {.x = 20, .y = 20};
		v2f pos = widget->pos;
		pos.x += draw_data->sat * widget->size.x;
		pos.y += (1 - draw_data->val) * widget->size.y;
		
		pos -= size/2;
		
		Rect recty = rect(pos, size);
		
		v4f color = D_COLOR_WHITE;
		
		if(draw_data->val > 0.5)
		{
			color = D_COLOR_BLACK;
		}
		
		R_Rect *indi = d_rect(recty, {});
    indi->radius = 5;
    indi->border_color = color;
    indi->border_thickness = 5;
	}
	
}

function UI_Signal ui_sat_picker(UI_Context *cxt, s32 hue, f32 *sat, f32 *val, Str8 text)
{
	UI_Widget *widget = ui_make_widget(cxt, text);
	widget->flags = UI_Flags_has_custom_draw;
	
	UI_SatPickerDrawData *draw_data = push_struct(cxt->frame_arena, UI_SatPickerDrawData);
	draw_data->hue = hue;
	draw_data->sat = *sat;
	draw_data->val = *val;
	
	widget->custom_draw = ui_sat_picker_draw;
	widget->custom_draw_data = draw_data;
	
	if(widget->hot && os_mouse_held(cxt->win, SDL_BUTTON_LEFT))
	{
		v2f mpos = cxt->win->mpos;
		f32 _sat, _val;
		_sat = (mpos.x - widget->pos.x) / widget->size.x;
		_val = 1 - (mpos.y - widget->pos.y) / widget->size.y;
		
		_sat = ClampTop(_sat, 1);
		_sat = ClampBot(_sat, 0);
		
		_val = ClampTop(_val, 1);
		_val = ClampBot(_val, 0);
		
		*sat = _sat;
		*val = _val;
	}
	
	UI_Signal out = ui_signal(cxt, widget);
	
	return out;
}

function UI_Signal ui_sat_pickerf(UI_Context *cxt, s32 hue, f32 *sat, f32 *val, char *fmt, ...)
{
	Arena_temp temp = scratch_begin(0,0);
	va_list args;
	va_start(args, fmt);
	Str8 text = push_str8fv(temp.arena, fmt, args);
	va_end(args);
	
	UI_Signal out = ui_sat_picker(cxt, hue, sat, val, text);
	
	arena_temp_end(&temp);
	
	return out;
}

struct UI_AlphaPickerDrawData
{
	v3f hsv;
	f32 alpha;
};

function v4f v4f_from_v3f(v3f a, f32 b)
{
	v4f out = {};
	out.x = a.x;
	out.y = a.y;
	out.z = a.z;
	out.w = b;
	return out;
}

function UI_CUSTOM_DRAW(ui_alpha_picker_draw)
{
	UI_AlphaPickerDrawData *draw_data = (UI_AlphaPickerDrawData *)user_data;
	
	Rect w_rect = rect(widget->pos, widget->size);
	
	// checker pattern
	{
		v2f dim = size_from_rect(w_rect);
		R_Rect *checker = d_rect(w_rect, D_COLOR_WHITE);
    checker->src = rect(0, 0, dim.x / 10, dim.y / 10);
    checker->tex = a_get_alpha_bg_tex();
  }
	
	// alpha fade
	{
		R_Rect *recty = d_rect(w_rect, {{}});
		
		recty->fade[Corner_00] = v4f{{0,0,0,0}};
		recty->fade[Corner_01] = v4f{{0, 0, 0, 0}};
		
		v4f col = v4f_from_v3f(hsv_to_rgb(draw_data->hsv), 1);
		recty->fade[Corner_10] = col;
		recty->fade[Corner_11] = col;
	}
	
	// indicator
	{
		v2f size = {.x = 20, .y = 20};
		v2f pos = widget->pos;
		pos.x += draw_data->alpha * widget->size.x;
		pos.y += widget->size.y / 2;
		
		pos -= size/2;
		
		Rect recty = rect(pos, size);
		
		v4f color = D_COLOR_WHITE;
		
    
		R_Rect *indi = d_rect(recty, {});
    indi->radius = 5;
    indi->border_color = color;
    indi->border_thickness = 5;
	}
	
}

function UI_Signal ui_alpha_picker(UI_Context *cxt, v3f hsv, f32 *alpha, Str8 text)
{
	UI_Widget *widget = ui_make_widget(cxt, text);
	widget->flags = UI_Flags_has_custom_draw;
	
	UI_AlphaPickerDrawData *draw_data = push_struct(cxt->frame_arena, UI_AlphaPickerDrawData);
	draw_data->hsv = hsv;
	draw_data->alpha = *alpha;
	
	widget->custom_draw = ui_alpha_picker_draw;
	widget->custom_draw_data = draw_data;
	
	if(widget->hot && os_mouse_held(cxt->win, SDL_BUTTON_LEFT))
	{
		v2f mpos = cxt->win->mpos;
		
		f32 _alpha;
		_alpha = (mpos.x - widget->pos.x) / widget->size.x;
		_alpha = ClampTop(_alpha, 1);
		_alpha = ClampBot(_alpha, 0);
		
		*alpha = _alpha;
	}
	
	UI_Signal out = ui_signal(cxt, widget);
	
	return out;
}

function UI_Signal ui_alpha_pickerf(UI_Context *cxt, v3f hsv, f32 *alpha, char *fmt, ...)
{
	Arena_temp temp = scratch_begin(0,0);
	va_list args;
	va_start(args, fmt);
	Str8 text = push_str8fv(temp.arena, fmt, args);
	va_end(args);
	
	UI_Signal out = ui_alpha_picker(cxt, hsv, alpha, text);
	
	arena_temp_end(&temp);
	
	return out;
}

struct UI_HuePickerDrawData
{
	f32 hue;
};

function UI_CUSTOM_DRAW(ui_hue_picker_draw)
{
	Rect box_rect = rect(widget->pos, widget->size);
	f32 samples = 360;
	
	f32 segment = size_from_rect(box_rect).x / samples;
	Rect segment_rect = box_rect;
	segment_rect.br.x = segment_rect.tl.x + segment;
	
	for(s32 i = 0; i < samples; i++)
	{
		f32 hue = i;
		f32 hue2 = (i+1);
		v3f rgb = hsv_to_rgb(v3f{{hue, 1, 1}});
		v3f rgb2 = hsv_to_rgb(v3f{{hue2, 1, 1}});
    
    v4f rgba = {.xyz = rgb, .aw = 1}; 
		v4f rgba2 = {.xyz = rgb2, .aw = 1};
		
		R_Rect *recty = d_rect(segment_rect, {});
		
		recty->fade[Corner_00] = rgba;
		recty->fade[Corner_01] = rgba;
		recty->fade[Corner_10] = rgba2;
		recty->fade[Corner_11] = rgba2;
		
		segment_rect.tl.x += segment;
		segment_rect.br.x += segment;
	}
	
	UI_HuePickerDrawData *draw_data = (UI_HuePickerDrawData*)user_data;
	
	// indicator
	{
		f32 size = 20;
		v2f pos = widget->pos;
		pos.x += ((draw_data->hue * 1.f - size/2) * widget->size.x) / 360;
		pos.y += widget->size.y / 2 - 10;
		
		Rect recty = rect(pos, {{size,size}});
		
    
		R_Rect *indi = d_rect(recty, {});
    indi->radius = 5;
    indi->border_color = D_COLOR_BLACK;
    indi->border_thickness = 5;
	}
	
}

function UI_Signal ui_hue_picker(UI_Context *cxt, f32 *hue, Str8 text)
{
	UI_Widget *widget = ui_make_widget(cxt, text);
	widget->flags = UI_Flags_has_custom_draw;
	
	UI_HuePickerDrawData *draw_data = push_struct(cxt->frame_arena, UI_HuePickerDrawData);
	draw_data->hue = *hue;
	
	widget->custom_draw = ui_hue_picker_draw;
	widget->custom_draw_data = draw_data;
	
	if(widget->hot && os_mouse_held(cxt->win, SDL_BUTTON_LEFT))
	{
		v2f mpos = cxt->win->mpos;
		
		f32 _hue;
		_hue = ((mpos.x - widget->pos.x) / widget->size.x) * 360;
		_hue = ClampTop(_hue, 360);
		_hue = ClampBot(_hue, 0);
		
		*hue = _hue;
	}
	
	UI_Signal out = ui_signal(cxt, widget);
	
	return out;
}

function UI_Signal ui_hue_pickerf(UI_Context *cxt, f32 *hue, char *fmt, ...)
{
	Arena_temp temp = scratch_begin(0,0);
	va_list args;
	va_start(args, fmt);
	Str8 text = push_str8fv(temp.arena, fmt, args);
	va_end(args);
	
	UI_Signal out = ui_hue_picker(cxt, hue, text);
	
	arena_temp_end(&temp);
	
	return out;
}

struct UI_ImageDrawData
{
	R_Handle img;
	Rect src;
	v4f color;
};

function UI_CUSTOM_DRAW(ui_image_draw)
{
	UI_ImageDrawData *draw_data = (UI_ImageDrawData *)user_data;
	
	v4f color = draw_data->color;
	if(widget->hot)
	{
		color = widget->hover_color;
	}
	
	R_Rect *img = d_rect(rect(widget->pos, widget->size), color);
  img->src = draw_data->src;
  img->tex = draw_data->img;
  img->border_thickness = widget->border_thickness;
  img->border_color = widget->border_color;
  img->radius = widget->radius;
}

function UI_Signal ui_image(UI_Context *cxt, R_Handle img, Rect src, v4f color, Str8 text)
{
	UI_Widget *widget = ui_make_widget(cxt, text);
	widget->flags = UI_Flags_has_custom_draw;
	
	UI_ImageDrawData *draw_data = push_struct(cxt->frame_arena, UI_ImageDrawData);
	draw_data->img = img;
	draw_data->src = src;
	draw_data->color = color;
	
	widget->custom_draw = ui_image_draw;
	widget->custom_draw_data = draw_data;
	
	UI_Signal out = ui_signal(cxt, widget);
	
	return out;
}

function UI_Signal ui_imagef(UI_Context *cxt, R_Handle img, Rect src, v4f color, char *fmt, ...)
{
	Arena_temp temp = scratch_begin(0,0);
	va_list args;
	va_start(args, fmt);
	Str8 text = push_str8fv(temp.arena, fmt, args);
	va_end(args);
	
	UI_Signal out = ui_image(cxt, img, src, color, text);
	
	arena_temp_end(&temp);
	
	return out;
}

function UI_Signal ui_named_spacer(UI_Context *cxt, Str8 text)
{
	UI_Widget *widget = ui_make_widget(cxt, text);
	
	UI_Signal out = ui_signal(cxt, widget);
	
	return out;
}

function UI_Signal ui_named_spacerf(UI_Context *cxt, char *fmt, ...)
{
	Arena_temp temp = scratch_begin(0,0);
	va_list args;
	va_start(args, fmt);
	Str8 text = push_str8fv(temp.arena, fmt, args);
	va_end(args);
	
	UI_Signal out = ui_named_spacer(cxt, text); 
	arena_temp_end(&temp);
	return out;
}

function UI_Signal ui_spacer(UI_Context *cxt)
{
	return ui_named_spacer(cxt, str8_lit(""));
}

#define ui_named_rowf(v,...) UI_DeferLoop(ui_begin_named_rowf(v,__VA_ARGS__), ui_end_row(v))
#define ui_named_colf(v,...) UI_DeferLoop(ui_begin_named_colf(v,__VA_ARGS__), ui_end_col(v))

#define ui_row(v) UI_DeferLoop(ui_begin_row(v), ui_end_row(v))
#define ui_col(v) UI_DeferLoop(ui_begin_col(v), ui_end_col(v))

#define ui_parent(cxt, v) UI_DeferLoop(ui_push_parent(cxt, v), ui_pop_parent(cxt))

#define ui_fixed_pos(cxt, v) UI_DeferLoop(ui_push_fixed_pos(cxt, v), ui_pop_fixed_pos(cxt))
#define ui_text_color(cxt, v) UI_DeferLoop(ui_push_text_color(cxt, v), ui_pop_text_color(cxt))
#define ui_bg_color(cxt, v) UI_DeferLoop(ui_push_bg_color(cxt, v), ui_pop_bg_color(cxt))
#define ui_hover_color(cxt, v) UI_DeferLoop(ui_push_hover_color(cxt, v), ui_pop_hover_color(cxt))
#define ui_press_color(cxt, v) UI_DeferLoop(ui_push_press_color(cxt, v), ui_pop_press_color(cxt))

#define ui_border_color(cxt, v) UI_DeferLoop(ui_push_border_color(cxt, v), ui_pop_border_color(cxt))

#define ui_border_thickness(cxt, v) UI_DeferLoop(ui_push_border_thickness(cxt, v), ui_pop_border_thickness(cxt))

#define ui_radius(cxt, v) UI_DeferLoop(ui_push_radius(cxt, v), ui_pop_radius(cxt))

#define ui_size_kind_x(cxt, v) UI_DeferLoop(ui_push_size_kind_x(cxt, v), ui_pop_size_kind_x(cxt))

#define ui_size_kind_y(cxt, v) UI_DeferLoop(ui_push_size_kind_y(cxt, v), ui_pop_size_kind_y(cxt))


#define ui_size_kind(cxt, v) ui_size_kind_x(cxt, v) ui_size_kind_y(cxt, v)

#define ui_pref_width(cxt, v) UI_DeferLoop(ui_push_pref_width(cxt, v), ui_pop_pref_width(cxt))
#define ui_pref_height(cxt, v) UI_DeferLoop(ui_push_pref_height(cxt, v), ui_pop_pref_height(cxt))

#define ui_pref_size(cxt, v) ui_pref_width(cxt, v) ui_pref_height(cxt, v)

#define ui_align_kind_x(cxt, v) UI_DeferLoop(ui_push_align_kind_x(cxt, v), ui_pop_align_kind_x(cxt))

function void ui_layout_fixed_size(UI_Widget *root, Axis2 axis)
{
	for(UI_Widget *child = root->first; child; child = child->next)
	{
		ui_layout_fixed_size(child, axis);
	}
	
	switch(root->pref_size[axis].kind)
	{
		default:
		case UI_SizeKind_Null:
		break;
		case UI_SizeKind_TextContent:
		case UI_SizeKind_Pixels:
		{
			root->computed_size[axis] = root->pref_size[axis].value;
		}break;
	}
}

function void ui_layout_upward_dependent(UI_Widget *root, Axis2 axis)
{
	if(root->pref_size[axis].kind == UI_SizeKind_PercentOfParent)
	{
		if(root->parent->pref_size[axis].kind != UI_SizeKind_ChildrenSum)
		{
			//printf("%s\n", root->text.c);
		}
	}
	
	for(UI_Widget *child = root->first; child; child = child->next)
	{
		ui_layout_upward_dependent(child, axis);
	}
}

function void ui_layout_downward_dependent(UI_Widget *root, Axis2 axis)
{
	for(UI_Widget *child = root->first; child; child = child->next)
	{
		ui_layout_downward_dependent(child, axis);
	}
	UI_Widget *parent = root->parent;
	if(parent)
	{
		if(parent->pref_size[axis].kind == UI_SizeKind_ChildrenSum)
		{
			if(parent->child_layout_axis == axis)
			{
				parent->computed_size[axis] += root->computed_size[axis];
			}
			else
			{
				if(parent->computed_size[axis] < root->computed_size[axis])
				{
					parent->computed_size[axis] = root->computed_size[axis];
				}
			}
		}
	}
}

// pre order
function void ui_layout_pos(UI_Widget *root)
{
	if(root->parent)
	{
		root->computed_rel_position[0] = root->parent->computed_rel_position[0];
		root->computed_rel_position[1] = root->parent->computed_rel_position[1];
	}
	
	// TODO(mizu): Note how this just adds / subtracts instead of cumulatively adding 
	// /subtracting. I get this weird distance bug where the distance is accounted for twice when I remove it. I will record the commit so its easier to understand / demonstrate. 
	
	if(root->prev)
	{
		if(root->parent->child_layout_axis == Axis2_X)
		{
			if(!(root->flags & UI_Flags_is_floating_x))
			{
				root->computed_rel_position[Axis2_X] = root->prev->computed_rel_position[Axis2_X] + root->prev->computed_size[Axis2_X];
        // NOTE(mizu): working on this rn
        //root->computed_rel_position[Axis2_X] += root->parent->computed_size[Axis2_X];// * 0.5;
      }
		}
		else if(root->parent->child_layout_axis == Axis2_Y)
		{
			if(!(root->flags & UI_Flags_is_floating_y))
			{
				root->computed_rel_position[Axis2_Y] = root->prev->computed_rel_position[Axis2_Y] + root->prev->computed_size[Axis2_Y];
      }
		}
	}
	
	for(UI_Widget *child = root->first; child; child = child->next)
	{
		ui_layout_pos(child);
	}
}

// post order dfs
function void ui_print_nodes_post_order(UI_Widget *root, s32 depth)
{
	for(UI_Widget *child = root->first; child; child = child->next)
	{
		ui_print_nodes_post_order(child, depth + 1);
	}
	
	for(s32 i = 0; i < depth; ++i) 
	{
		printf("-");
	}
	
	printf("%s [%.2f , %.2f] [%.2f , %.2f] \n", root->text.c, root->computed_size[0], root->computed_size[1], root->computed_rel_position[0], root->computed_rel_position[1]);
}

function void ui_print_nodes_pre_order(UI_Widget *root, s32 depth)
{
	
	for(s32 i = 0; i < depth; ++i) 
	{
		printf("-");
	}
	
	printf("%s [%.2f , %.2f] [%.2f , %.2f] \n", root->text.c, root->computed_size[0], root->computed_size[1], root->computed_rel_position[0], root->computed_rel_position[1]);
	
	for(UI_Widget *child = root->first; child; child = child->next)
	{
		ui_print_nodes_pre_order(child, depth + 1);
	}
}

function void ui_layout(UI_Widget *root)
{
	for(Axis2 axis = (Axis2)0; axis < Axis2_COUNT; axis = (Axis2)(axis + 1))
	{
		ui_layout_fixed_size(root, axis);
		ui_layout_upward_dependent(root, axis);
		ui_layout_downward_dependent(root, axis);
	}
	ui_layout_pos(root);
	//ui_print_nodes_pre_order(root, 0);
	//printf("\n");
}

function void ui_begin(UI_Context *cxt, OS_Window *win)
{
	cxt->win = win;
	
	cxt->frame_arena->used = ARENA_HEADER_SIZE;
	
	cxt->frames++;
}

function void ui_end(UI_Context *cxt)
{
	//ui_pop_parent(cxt);
	
	for(u32 i = 0; i < cxt->hash_table_size; i++)
	{
		UI_Widget *first_hash = (cxt->hash_slots + i)->first;
		if(!first_hash)
		{
			continue;
		}
		if(first_hash)
		{
			UI_Widget *cur = first_hash;
			UI_Widget *prev = 0;
			while(cur)
			{
				if(cur->last_frame_touched_index != cxt->frames)
				{
					//printf("pruned %.*s\n", str8_varg(cur->text));
					
					if(prev)
					{
						prev->hash_next = cur->hash_next;
						if (!cur->hash_next)
						{
							(cxt->hash_slots + i)->last = prev;
						}
					}
					else
					{
						(cxt->hash_slots + i)->first = cur->hash_next;
						if (!cur->hash_next)
						{
							(cxt->hash_slots + i)->last = 0;
						}
					}
					
					UI_Widget *to_free = cur;
					cur = cur->hash_next;
					ui_free_widget(cxt, to_free);
				}
				else
				{
					prev = cur;
					cur = cur->hash_next;
				}
			}
		}
	}
	//printf("\n");
}