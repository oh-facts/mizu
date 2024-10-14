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
	ED_WindowFlags_Minimized = 1 << 4,
	ED_WindowFlags_Maximized = 1 << 5,
	ED_WindowFlags_Floating = 1 << 6,
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
	
	v2f pos;
	v2f old_pos;
	
	v2f size;
	
	b32 close;
	b32 grab;
	b32 hide;
	b32 maximize;
	
	Str8 name;
	
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

// child window for surface less window seems best idea

struct ED_Window
{
	ED_WindowFlags flags;
	OS_Window *win;
	
	UI_Widget *root;
	UI_Context *cxt;
	
	UI_Widget *menu_bar;
	
	v2f pos;
	v2f size;
	b32 hide;
	b32 close;
	b32 grab;
	b32 maximize;
	
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
	
	if(ed_state->num_windows == 1)
	{
		out->win = os_windowOpen("alfia", out->size.x, out->size.y);
		os_setWindowPos(out->win, out->pos);
		
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

function ED_Tab *ed_openTab(ED_Panel *panel, char *name)
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
	
	v2f size = ed_sizeOfPanel(panel);
	out->target = r_allocFramebuffer(size.x, size.y);
	
	out->name = push_str8f(ed_state->arena, name);
	
	panel->active_tab = out;
	
	return out;
}

function ED_Tab *ed_openFloatingTab(ED_Panel *panel, char *name, v2f pos, v2f size)
{
	ED_Tab *out = push_struct(ed_state->arena, ED_Tab);
	*out = {};
	
	out->parent = panel;
	
	//v2f size = ed_size_of_panel(panel);
	
	out->name = push_str8f(ed_state->arena, name);
	out->pos = pos;
	out->size = size;
	return out;
}

function void ed_drawChildren(ED_Panel *panel, UI_Widget *root)
{
	if(root->flags & UI_Flags_has_bg)
	{
		R_Rect *bg = d_rect(root->dst, root->bg_color);
		bg->radius = root->radius;
		bg->border_color = ED_THEME_TITLEBAR;
		bg->border_thickness = 4;
	}
	
	if(root->flags & UI_Flags_draw_border)
	{
		R_Rect *border = d_rect(root->dst, {{}});
		border->radius = root->radius;
		border->border_color = root->border_color;
		border->border_thickness = 4;
	}
	
	if(root->flags & UI_Flags_has_text)
	{
		v4f color = {};
		v2f pos = {};
		f32 scale = root->scale;
		
		if(root->timer > 0)
		{
			color = root->color - root->press_color * (root->timer / 3);
			color.w = 1;
			//scale *= 1.1;
		}
		else if(root->hot)
		{
			color = root->hover_color;
			//scale *= 1.1;
		}
		else
		{
			color = root->color;
		}
		
		if(root->flags & UI_Flags_text_centered)
		{
			Rect box_extent = root->dst;
			Rect text_extent = rectFromString(root->text, root->scale);
			
			v2f box_size = size_from_rect(box_extent);
			v2f text_size = size_from_rect(text_extent);
			
			pos.x += box_extent.tl.x + (box_size.x - text_size.x) / 2.f;
			pos.y += box_extent.tl.y + (box_size.y - text_size.y) / 4.f;
		}
		else
		{
			pos += root->dst.tl;
		}
		
		d_text(root->text, pos, color, scale);
	}
	
	if(root->flags & UI_Flags_clickable)
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
		
		R_Rect *bg = d_rect(root->dst, {});
		bg->radius = 4;
		bg->border_color = color;
		bg->border_thickness = 4;
	}
	
	if(root->flags & UI_Flags_has_custom_draw)
	{
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

function UI_Widget *ed_titlebar(Str8 title, s32 i, UI_Context *cxt, v2f size, b32 *close, b32 *maximize, b32 *hide, b32 *grab)
{
	UI_Widget *menu_bar = 0;
	ui_named_rowf(cxt, "%d editor title bar", i)
	{
		menu_bar = cxt->parent_stack.top->v;
		menu_bar->flags |= UI_Flags_has_bg | UI_Flags_rounded_corners;;
		menu_bar->bg_color = ED_THEME_TITLEBAR;
		menu_bar->radius = 15 / 1.8f;
		TEX_Handle key = a_keyFromPath(str8_lit("editor/xp_titlebar.png"), font_params);
		R_Handle titlebar_img = a_handleFromKey(key);
		
		// hide button
		ui_hover_color(cxt, (v4f{{0.4, 0.4, 0.4, 1}}))
			ui_pref_size(cxt, 32)
			ui_size_kind(cxt, UI_SizeKind_Pixels)
		{
			if(ui_imagef(cxt, titlebar_img, rect(0, 0, 0.25, 1), ED_THEME_IMG, "hide img %d", i).active)
			{
				*hide = !(*hide);
				//window->flags ^= ED_WindowFlags_Hidden;
			}
		}
		
		// drag titlebar region (entire titlebar - labels/buttons + title)
		ui_size_kind_x(cxt, UI_SizeKind_Pixels)
			// NOTE(mizu): bandaid fix until I get aligning done
			ui_pref_width(cxt, size.x - 32 * 4)
			ui_named_rowf(cxt, "menu bar %d", i)
		{
			UI_Widget *menu_bar = cxt->parent_stack.top->v;
			
			if(ui_signal(cxt, menu_bar).active)
			{
				*grab = !(*grab);
				//window->flags ^= ED_WindowFlags_Grabbed;
			}
			
			ui_col(cxt)
			{
				ui_size_kind(cxt, UI_SizeKind_Pixels)
					ui_pref_height(cxt, 8)
				{
					ui_spacer(cxt);
				}
				
				// window title
				ui_text_color(cxt, D_COLOR_BLACK)
					ui_size_kind(cxt, UI_SizeKind_TextContent)
				{
					ui_label(cxt, title);
				}
				
			}
		}
		
		// minimize, maximize, close 
		
		ui_hover_color(cxt, (v4f{{0.4, 0.4, 0.4, 1}}))
			ui_pref_size(cxt, 32)
			ui_size_kind(cxt, UI_SizeKind_Pixels)
		{
			if(ui_imagef(cxt, titlebar_img, rect(0.25, 0, 0.5, 1), ED_THEME_IMG, "minimize img %d", i).active)
			{
				//window->flags ^= ED_WindowFlags_Minimized;
			}
			
			*maximize = ui_imagef(cxt, titlebar_img, rect(0.5, 0, 0.75, 1), ED_THEME_IMG, "maximize img %d", i).active;
			
			if(ui_imagef(cxt, titlebar_img, rect(0.75, 0, 1, 1), ED_THEME_IMG, "close img %d", i).active)
			{
				*close = !(*close);
				//window->win->close_requested = 1;
			}
		}
	}
	return menu_bar;
}

function void ed_floatingTab(f32 delta, Str8 name, ED_Tab *tab)
{
	u64 hash = ui_hash(name);
	
	ED_Window *window = tab->parent->parent;
	
	UI_Widget *floating = ui_makeWidget(window->cxt, str8_lit(""));
	floating->flags = UI_Flags_is_floating | UI_Flags_has_bg | UI_Flags_draw_border | UI_Flags_rounded_corners;
	floating->computed_rel_position[0] = tab->pos.x;
	floating->computed_rel_position[1] = tab->pos.y;
	floating->bg_color = ED_THEME_BG_FROSTED;
	floating->border_color = ED_THEME_TITLEBAR;
	floating->radius = 15 / 1.8f;
	
	ui_scale(window->cxt, FONT_SIZE * 0.8)
		ui_parent(window->cxt, floating)
		ui_size_kind(window->cxt, UI_SizeKind_ChildrenSum)
		ui_col(window->cxt)
	{
		ed_titlebar(name, hash, window->cxt, tab->size, &tab->close, &tab->maximize, &tab->hide, &tab->grab);
		
		if(os_mouseHeld(window->win, SDL_BUTTON_LEFT) && (tab->grab))
		{
			f32 x, y;
			SDL_GetGlobalMouseState(&x, &y);
			
			tab->pos += v2f{{x, y}} - tab->old_pos;
			//os_setWindowPos(window->win, window->pos);
		}
		else
		{
			tab->grab = 0;
		}
		
		if(!tab->hide)
		{
			ui_padding_x(window->cxt, 5)
				ui_col(window->cxt)
			{
				tab->custom_draw(window, tab, delta, tab->custom_drawData);
			}
		}
		tab->old_pos = tab->pos;
		SDL_GetGlobalMouseState(&tab->old_pos.x, &tab->old_pos.y);
		
	}
}

function void ed_drawWindow(ED_Window *window)
{
	UI_Widget *parent = window->root;
	
	//parent->pos.x = parent->computed_rel_position[0];
	//parent->pos.y = parent->computed_rel_position[1];
	
	//parent->size.x = parent->computed_size[0];
	//parent->size.y = parent->computed_size[1];
	
	v2f size = {{parent->computed_size[0], parent->computed_size[1]}};
	d_rect(rect({}, size), window->color);
	if((window->flags & ED_WindowFlags_ChildrenSum) || window->hide)
	{
		os_setWindowSize(window->win, size);
	}
	else
	{
		os_setWindowSize(window->win, size);
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
		
		ui_set_next_size_kind_x(window->cxt, UI_SizeKind_ChildrenSum);
		ui_set_next_size_kind_y(window->cxt, UI_SizeKind_ChildrenSum);
		
		window->root = ui_makeWidget(window->cxt, str8_lit("root"));
		window->color = ED_THEME_BG;
		
		ui_parent(window->cxt, window->root)
			ui_text_color(window->cxt, ED_THEME_TEXT)
			ui_size_kind(window->cxt, UI_SizeKind_ChildrenSum)
			ui_col(window->cxt)
		{
			//printf("%d\n", window->grabbed);
			
			// NOTE(mizu):  title bar, dragging and hiding
			window->menu_bar = ed_titlebar(str8_lit("Mizu Mizu 1 Million"), i, window->cxt, window->size, &window->close, &window->maximize, &window->hide, &window->grab);
			
			if(os_mouseHeld(window->win, SDL_BUTTON_LEFT) && (window->grab))
			{
				f32 x, y;
				SDL_GetGlobalMouseState(&x, &y);
				window->pos += v2f{{x, y}} - window->old_pos;
				os_setWindowPos(window->win, window->pos);
			}
			else
			{
				window->grab = 0;
			}
			
			if(window->close)
			{
				window->win->close_requested = 1;
			}
			
			if(window->maximize)
			{
				os_toggleFullscreen(window->win);
			}
			
			if(!window->hide)
			{
				// tab list
				for(ED_Panel *panel = window->first_panel; panel; panel = panel->next)
				{
					ui_row(window->cxt)
						for(ED_Tab *tab = panel->first_tab; tab; tab = tab->next)
					{
						ui_size_kind(window->cxt, UI_SizeKind_TextContent)
						{
							if(ui_label(window->cxt, tab->name).active)
							{
								panel->active_tab = tab;
							}
						}
						
						ui_pref_width(window->cxt, 10)
							ui_size_kind(window->cxt, UI_SizeKind_Pixels)
							ui_spacer(window->cxt);
					}
					
					ED_Tab *tab = panel->active_tab;
					
					tab->custom_draw(window, tab, delta, tab->custom_drawData);
				}
			}
		}
		
		SDL_GetGlobalMouseState(&window->old_pos.x, &window->old_pos.y);
		
		ui_layout(window->root);
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