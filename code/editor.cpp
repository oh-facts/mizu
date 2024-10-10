// TODO(mizu): If a window is focussed, other windows come to the top too. Also, add a minimize and maximize and close button. And work on making the panels better to interact with.

// TODO(mizu): Then work on panels and then on tabs

// Also make it so that if a window is ontop of main window, it moves with the main window
// 
// oh yeah, and make parent sibling relationships for the windows
// Main window meaning parent window. child windows always display on top of main window
// and if they are inside the parent window's rect, then they should move with it too (?).

// And give the debug panel lots of functionality to control all other windows's features.

// padding in ui
// UI_SizeKind_PercentOfParent

// DOTO(mizu): And ofc, make the main engine window also one of these ui windows. 
// Call panels windows, because thats what they are. 
// rects with borders.
// rects with round edges
// render game to an image, then render that as a ui element
// make a render parameter - output frame buffer. used by submit maybe? If 0, renders to default
// fb, otherwise, renders to the passed framebuffer
// make text look proper
// make a transition_time so when you press on ui, you can see it as a certain color for
// some time

#define ED_MAX_WINDOWS 10

typedef s32 ED_WindowFlags;

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

typedef int ED_TabFlags;

struct ED_Window;
struct ED_Panel;
struct ED_Tab;

#define ED_CUSTOM_TAB(name) void name(ED_Window *window, ED_Tab *tab, f32 delta, void *user_data)
typedef ED_CUSTOM_TAB(ED_CustomTab);

struct ED_Tab
{
	ED_Tab *next;
	ED_Tab *prev;
	ED_Panel *parent;
	
	Str8 name;
	
	ED_Tab *inspector;
	
	f32 update_timer;
	
	UI_Widget *selected_slot;
	Str8 selected_tile;
	Rect selected_tile_rect;
	
	f32 cc;
	f32 ft;
	
	v4f hsva;
	
	R_Handle target;
	
	ED_CustomTab *custom_draw;
	void *custom_drawData;
};

struct ED_Panel
{
	ED_Panel *next;
	ED_Panel *prev;
	
	ED_Tab *active_tab;
	ED_Tab *first_tab;
	ED_Tab *last_tab;
	
	ED_Window *parent;
	
	Axis2 axis;
	f32 pct_of_parent;
};

struct ED_Window
{
	ED_WindowFlags flags;
	OS_Window *win;
	
	UI_Widget *root;
	UI_Context *cxt;
	
	UI_Widget *menu_bar;
	
	v2f pos;
	v2f size;
	
	v2f old_pos;
	
	v4f color;
	
	ED_Panel *first_panel;
	ED_Panel *last_panel;
	
	// per frame artifacts
	D_Bucket *bucket;
};

struct ED_State
{
	Arena *arena;
	
	ED_Window *main_window;
	ED_Tab *free_tab;
	ED_Panel *free_panel;
	
	ED_Window windows[ED_MAX_WINDOWS];
	s32 num_windows;
};

global ED_State *ed_state;

function void ed_drawSpritesheet(ED_Tab *tab, f32 x, f32 y, Str8 path)
{
	ED_Panel *panel = tab->parent;
	ED_Window *window = panel->parent;
	
	ui_radius(window->cxt, 5)
		ui_border_thickness(window->cxt, 5)
		ui_border_color(window->cxt, (v4f{{0, 0, 0, 0.3}}))
		ui_press_color(window->cxt, D_COLOR_RED)
		ui_hover_color(window->cxt, D_COLOR_BLUE)
		ui_col(window->cxt)
	{
		ui_size_kind(window->cxt, UI_SizeKind_Pixels)
			ui_pref_height(window->cxt, 15)
		{
			ui_spacer(window->cxt);
		}
		
		ui_size_kind(window->cxt, UI_SizeKind_TextContent)
		{
			ui_label(window->cxt, path);
		}
		
		A_Key key = a_keyFromPath(path, pixel_tiled_params);
		R_Handle img = a_handleFromKey(key);
		
		f32 width = 1.f/x;
		f32 height = 1.f/y;
		
		f32 advance_y = 1 - height;
		
		ui_size_kind(window->cxt, UI_SizeKind_ChildrenSum)
			ui_col(window->cxt)
		{
			for(u32 i = 0; i < y; i ++)
			{
				f32 advance_x = 0;
				ui_row(window->cxt)
				{
					ui_size_kind(window->cxt, UI_SizeKind_Pixels)
						ui_pref_size(window->cxt, 60)
					{
						for(u32 j = 0; j < x; j++)
						{
							Rect recty = rect(advance_x, advance_y, advance_x + width, advance_y + height);
							
							if(ui_imagef(window->cxt, img, recty, D_COLOR_WHITE, "%.*s%d%d", str8_varg(path), i, j).active)
							{
								tab->inspector->selected_tile = path;
								tab->inspector->selected_tile_rect = recty;
							}
							advance_x += width;
						}
					}
					
				}
				advance_y -= height;
			}
		}
	}
}

function void ed_init()
{
	Arena *arena = arenaAlloc();
	ed_state = push_struct(arena, ED_State);
	ed_state->arena = arena;
}

function ED_Window *ed_openWindow(ED_WindowFlags flags, v2f pos, v2f size)
{
	ED_Window *out = ed_state->windows + ed_state->num_windows++;
	out->flags = flags;
	out->size = size;
	out->pos = pos;
	out->cxt = ui_allocCxt();
	
	out->win = os_windowOpen("alfia", out->size.x, out->size.y);
	
	os_setWindowPos(out->win, out->pos);
	
	if(ed_state->num_windows == 1)
	{
		ed_state->main_window = out;
	}
	
	return out;
}

function v2f ed_sizeOfPanel(ED_Panel *panel)
{
	v2f out = {};
	out.x = 960;
	out.y = 540;
	return out;
}

function ED_Panel *ed_openPanel(ED_Window *window, Axis2 axis, f32 pct_of_parent)
{
	ED_Panel *out = push_struct(ed_state->arena, ED_Panel);
	*out = {};
	
	if(window->last_panel)
	{
		out->prev = window->last_panel;
		window->last_panel = window->last_panel->next = out;
	}
	else
	{
		window->last_panel = window->first_panel = out;
	}
	
	out->parent = window;
	out->axis = axis;
	out->pct_of_parent = pct_of_parent;
	
	return out;
}

function ED_Tab *ed_openTab(ED_Panel *panel, char *name, v2f size = {{960, 540}})
{
	ED_Tab *out = push_struct(ed_state->arena, ED_Tab);
	*out = {};
	
	if(panel->last_tab)
	{
		out->prev = panel->last_tab;
		panel->last_tab = panel->last_tab->next = out;
	}
	else
	{
		panel->last_tab = panel->first_tab = out;
	}
	
	out->parent = panel;
	
	//v2f size = ed_size_of_panel(panel);
	out->target = r_allocFramebuffer(size.x, size.y);
	
	out->name = push_str8f(ed_state->arena, name);
	
	panel->active_tab = out;
	
	return out;
}

function void ed_drawChildren(ED_Panel *panel, UI_Widget *root)
{
	ED_Window *window = panel->parent;
	root->pos.x = root->computed_rel_position[0];
	root->pos.y = root->computed_rel_position[1];
	root->size.x = root->computed_size[0];
	root->size.y = root->computed_size[1];
	
	if(window->flags & ED_WindowFlags_Floating)
	{
		root->pos.x += root->fixed_position.x;
		root->pos.y += root->fixed_position.y;
	}
	
	if(root->flags & UI_Flags_has_text)
	{
		v4f color = {};
		if(root->timer > 0)
		{
			color = root->color - root->press_color * (root->timer / 3);
			color.w = 1;
		}
		else if(root->hot)
		{
			color = root->hover_color;
		}
		else
		{
			color = root->color;
		}
		
		v2f pos = root->pos;
		
#if 0
		R_Rect *bg = d_rect(rect(root->pos, root->size - v2f{.y = 2}), color * 0.1);
		bg->radius = 4;
		bg->border_color = color;
		bg->border_thickness = 4;
		
		v2f size = root->size;
		
		Rect top_left = rect(pos, size);
		Rect text_size = ui_text_spacing_stats(font->atlas.glyphs, root->text, FONT_SIZE);
		
		top_left.tl += text_size.tl / 2;
		top_left.br += text_size.br / 2;
		
		pos.y = top_left.tl.y;
		pos.x = top_left.tl.x + 4;
#endif
		d_text(root->text, pos, color, root->scale);
	}
	
	ED_Tab *tab = panel->active_tab;
	if(root->flags & UI_Flags_has_custom_draw)
	{
		if(tab->selected_slot == root)
		{
			Rect selected_slot_rect = rect(tab->selected_slot->pos, tab->selected_slot->size);
			R_Rect *slot = d_rect(selected_slot_rect, D_COLOR_WHITE);
			slot->src = rect(0, 0, 2, 2);
			slot->tex = a_getAlphaBGTex();
		}
		
		root->custom_draw(root, root->custom_draw_data);
	}
	
	for(UI_Widget *child = root->first; child; child = child->next)
	{
		ed_drawChildren(panel, child);
	}
}

function void ed_drawPanel(ED_Window *window, UI_Widget *root)
{
	for(ED_Panel *panel = window->first_panel; panel; panel = panel->next)
	{
		ed_drawChildren(panel, root);
	}
}

function void ed_drawWindow(ED_Window *window)
{
	UI_Widget *parent = window->root;
	
	parent->pos.x = parent->computed_rel_position[0];
	parent->pos.y = parent->computed_rel_position[1];
	
	if(window->flags & ED_WindowFlags_Floating)
	{
		parent->pos.x +=  window->pos.x;
		parent->pos.y +=  window->pos.y;
	}
	else
	{
		//window->pos = window->root->pos;
	}
	
	parent->size.x = parent->computed_size[0];
	parent->size.y = parent->computed_size[1];
	
	d_rect(rect({}, parent->size), window->color);
	
	d_rect(rect(window->menu_bar->pos, window->menu_bar->size), {{0.902, 0.902, 0.902, 1}});
	
	if((window->flags & ED_WindowFlags_ChildrenSum) || (window->flags & ED_WindowFlags_Hidden))
	{
		os_setWindowSize(window->win, parent->size);
	}
	else
	{
		os_setWindowSize(window->win, window->size);
	}
	ed_drawPanel(window, parent);
}

function void ed_update(f32 delta)
{
	for(u32 i = 0; i < ed_state->num_windows; i++)
	{
		ED_Window *window = ed_state->windows + i;
		
		window->bucket = d_bucket();
		d_pushBucket(window->bucket);
		d_push_proj_view(m4f_identity());
		
		ui_begin(window->cxt, window->win);
		
		ui_set_next_child_layout_axis(window->cxt, Axis2_X);
		UI_Widget *dad = ui_makeWidget(window->cxt, str8_lit(""));
		ui_parent(window->cxt, dad)
		{
			ui_size_kind(window->cxt, UI_SizeKind_ChildrenSum)
			{
				window->root = ui_makeWidget(window->cxt, str8_lit("bridget"));
				
				window->color = ED_THEME_BG;
				
				if(window->flags & ED_WindowFlags_Floating)
				{
					window->root->flags = (UI_Flags)(UI_Flags_is_floating_x | UI_Flags_is_floating_y);
				}
				
				// NOTE(mizu):  title bar, dragging and hiding
				ui_parent(window->cxt, window->root)
					ui_text_color(window->cxt, ED_THEME_TEXT)
					//ui_fixed_pos(window->cxt, (window->pos))
					ui_named_colf(window->cxt, "jones")
				{
					ui_named_rowf(window->cxt, "%d editor title bar")
					{
						window->menu_bar = window->cxt->parent_stack.top->v;
						A_Key key = a_keyFromPath(str8_lit("editor/xp_titlebar.png"), font_params);
						R_Handle titlebar_img = a_handleFromKey(key);
						
						// hide button
						ui_hover_color(window->cxt, (v4f{{0.4, 0.4, 0.4, 1}}))
							ui_pref_size(window->cxt, 32)
							ui_size_kind(window->cxt, UI_SizeKind_Pixels)
						{
							if(ui_imagef(window->cxt, titlebar_img, rect(0, 0, 0.25, 1), ED_THEME_IMG, "hide img %d", i).active)
							{
								window->flags ^= ED_WindowFlags_Hidden;
							}
						}
						
						// drag titlebar region (entire titlebar - labels/buttons + title)
						ui_size_kind_x(window->cxt, UI_SizeKind_Pixels)
							// NOTE(mizu): bandaid fix until I get aligning done
							ui_pref_width(window->cxt, window->size.x - 32 * 4)
							ui_named_rowf(window->cxt, "menu bar %d", i)
						{
							UI_Widget *menu_bar = window->cxt->parent_stack.top->v;
							
							if(ui_signal(window->cxt, menu_bar).active)
							{
								window->flags ^= ED_WindowFlags_Grabbed;
							}
							
							ui_col(window->cxt)
							{
								ui_size_kind(window->cxt, UI_SizeKind_Pixels)
									ui_pref_height(window->cxt, 8)
								{
									ui_spacer(window->cxt);
								}
								
								// window title
								ui_text_color(window->cxt, D_COLOR_BLACK)
									ui_size_kind(window->cxt, UI_SizeKind_TextContent)
								{
									ui_label(window->cxt, str8_lit("Mizu Mizu Game Engine 1 million"));
								}
								
							}
						}
						
						// minimize, maximize, close 
						
						ui_hover_color(window->cxt, (v4f{{0.4, 0.4, 0.4, 1}}))
							ui_pref_size(window->cxt, 32)
							ui_size_kind(window->cxt, UI_SizeKind_Pixels)
						{
							
							if(ui_imagef(window->cxt, titlebar_img, rect(0.25, 0, 0.5, 1), ED_THEME_IMG, "minimize img %d", i).active)
							{
								window->flags ^= ED_WindowFlags_Minimized;
							}
							
							if(ui_imagef(window->cxt, titlebar_img, rect(0.5, 0, 0.75, 1), ED_THEME_IMG, "maximize img %d", i).active)
							{
								window->flags ^= ED_WindowFlags_Maximized;
							}
							
							if(ui_imagef(window->cxt, titlebar_img, rect(0.75, 0, 1, 1), ED_THEME_IMG, "close img %d", i).active)
							{
								window->win->close_requested = 1;
							}
							
							
						}
						
						if(os_mouseHeld(window->win, SDL_BUTTON_LEFT) && (window->flags & ED_WindowFlags_Grabbed))
						{
							f32 x, y;
							SDL_GetGlobalMouseState(&x, &y);
							window->pos += v2f{{x, y}} - window->old_pos;
							os_setWindowPos(window->win, window->pos);
						}
						else
						{
							window->flags &= ~ED_WindowFlags_Grabbed;
						}
					}
					
					//printf("%d\n", window->grabbed);
					
					if(!(window->flags & ED_WindowFlags_Hidden))
					{
						// tab list
						for(ED_Panel *panel = window->first_panel; panel; panel = panel->next)
						{
							ui_row(window->cxt)
								ui_size_kind(window->cxt, UI_SizeKind_TextContent)
							{
								for(ED_Tab *tab = panel->first_tab; tab; tab = tab->next)
								{
									if(ui_label(window->cxt, tab->name).active)
									{
										panel->active_tab = tab;
									}
									ui_pref_width(window->cxt, 10)
										ui_size_kind(window->cxt, UI_SizeKind_Pixels)
										ui_spacer(window->cxt);
								}
							}
							
							ED_Tab *tab = panel->active_tab;
							
							tab->custom_draw(window, tab, delta, tab->custom_drawData);
						}
					}
				}
			}
		}
		
		SDL_GetGlobalMouseState(&window->old_pos.x, &window->old_pos.y);
		
		ui_layout(dad);
		ed_drawWindow(window);
		ui_end(window->cxt);
		
		d_pop_proj_view();
		d_popBucket();
		
	}
}

function void ed_submit()
{
	for(s32 i = 0; i < ed_state->num_windows; i++)
	{
		ED_Window *window = ed_state->windows + i;
		r_submit(window->win, &window->bucket->list);
	}
}