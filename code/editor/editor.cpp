void ed_draw_spritesheet(ED_Panel *panel, f32 x, f32 y, Str8 path)
{
	R_Handle img = a_handle_from_path(path);
	
	f32 width = 1.f/x;
	f32 height = 1.f/y;
	
	f32 advance_y = 1 - height;
	
	ui_size_kind(panel->cxt, UI_SizeKind_ChildrenSum)
		ui_col(panel->cxt)
	{
		for(u32 i = 0; i < y; i ++)
		{
			f32 advance_x = 0;
			ui_row(panel->cxt)
			{
				ui_size_kind(panel->cxt, UI_SizeKind_Pixels)
					ui_pref_size(panel->cxt, 60)
				{
					for(u32 j = 0; j < x; j++)
					{
						Rect recty = rect(advance_x, advance_y, advance_x + width, advance_y + height);
						
						if(ui_imagef(panel->cxt, img, recty, D_COLOR_WHITE, "%.*s%d%d", str8_varg(path), i, j).active)
						{
							panel->selected_tile = path;
							panel->selected_tile_rect = recty;
						}
						advance_x += width;
					}
				}
				
			}
			advance_y -= height;
		}
	}
}

void ed_update(State *state, f32 delta)
{
	ED_State *ed_state = &state->ed_state;
	
	if(!ed_state->initialized)
	{
		ed_state->arena = arena_create();
		
		// panel construction
		{
			ED_Panel *p = ed_state->panels + ED_PanelKind_TileSetViewer;
			p->scale = v2f{{1.6,0.3}};
			p->name = push_str8f(ed_state->arena, "tile set viewer");
			p->cxt = ui_alloc_cxt();
			p->win = os_window_open(ed_state->arena, "tile set viewer", 960, 540, (OS_WindowKind)(OS_WindowKind_Opengl | OS_WindowKind_Undecorate));
			p->floating = 1;
			p->pos = os_get_window_pos(p->win);
		}
		
		{
			ED_Panel *p = ed_state->panels + ED_PanelKind_Debug;
			p->scale = v2f{{1.6,0.3}};
			p->hide = 1;
			p->name = push_str8f(ed_state->arena, "debug");
			p->cxt = ui_alloc_cxt();
			p->win = os_window_open(ed_state->arena, "debug", 960, 540, (OS_WindowKind)(OS_WindowKind_Opengl | OS_WindowKind_Undecorate));
			p->pos = os_get_window_pos(p->win);
			p->floating = 1;
			
		}
		
		{
			ED_Panel *p = ed_state->panels + ED_PanelKind_Inspector;
			p->scale = v2f{{1.6,0.3}};
			p->hide = 0;
			p->name = push_str8f(ed_state->arena, "Inspector");
			p->cxt = ui_alloc_cxt();
			
			p->hsva.x = 0;
			p->hsva.y = 0;
			p->hsva.z = 1;
			p->hsva.w = 1;
			p->win = os_window_open(ed_state->arena, "inspector", 960, 540, (OS_WindowKind)(OS_WindowKind_Opengl | OS_WindowKind_Undecorate));
			p->pos = os_get_window_pos(p->win);
			p->floating = 0;
			
		}
		
		ed_state->initialized = 1;
	}
	
	for(u32 i = 0; i < ED_PanelKind_COUNT; i++)
	{
		ED_Panel *panel = ed_state->panels + i;
		
		OS_Event_list *events = os_event_list_from_window(panel->win);
		
		panel->bucket = d_bucket();
		d_push_bucket(panel->bucket);
		
		ui_begin(panel->cxt, panel->win, &state->atlas, events);
		
		ui_set_next_child_layout_axis(panel->cxt, Axis2_X);
		UI_Widget *dad = ui_make_widget(panel->cxt, str8_lit(""));
		ui_parent(panel->cxt, dad)
		{
			ui_size_kind(panel->cxt, UI_SizeKind_ChildrenSum)
			{
				panel->root = ui_make_widget(panel->cxt, str8_lit(""));
				
				panel->color = ED_THEME_BG;
				
				if(panel->floating)
				{
					panel->root->flags = (UI_Flags)(UI_Flags_is_floating_x | UI_Flags_is_floating_y);
				}
				
				ui_parent(panel->cxt, panel->root)
					ui_text_color(panel->cxt, ED_THEME_TEXT)
					//ui_fixed_pos(panel->cxt, (panel->pos))
					ui_col(panel->cxt)
				{
					// NOTE(mizu):  title bar, dragging and hiding
					ui_row(panel->cxt)
					{
						UI_Signal hide = {};
						
						ui_size_kind(panel->cxt, UI_SizeKind_TextContent)
						{
							UI_Signal res = {};
							
							ui_pref_height(panel->cxt, 30)
								ui_size_kind_y(panel->cxt, UI_SizeKind_Pixels)
							{
								res = ui_label(panel->cxt, panel->name);
							}
							
							if(panel->floating)
							{
								ui_size_kind(panel->cxt, UI_SizeKind_Pixels)
									ui_pref_width(panel->cxt, 30)
								{
									ui_spacer(panel->cxt);
								}
								hide = ui_labelf(panel->cxt, "hide%d",i);
							}
							else
							{
								hide = res;
							}
							
							if(res.active)
							{
								panel->grabbed = 1;
							}
							
							if(hide.active)
							{
								panel->hide = !panel->hide;
							}
							
							os_mouse_released(events, panel->win, OS_MouseButton_Left);
							
							if(os_mouse_held(panel->win, OS_MouseButton_Left) && panel->grabbed)
							{
								f32 x, y;
								SDL_GetGlobalMouseState(&x, &y);
								panel->pos += v2f{{x, y}} - panel->old_pos;
								os_set_window_pos(panel->win, panel->pos);
							}
							else
							{
								panel->grabbed = 0;
							}
						}
					}
					
					//printf("%d\n", panel->grabbed);
					
					if(!panel->hide)
					{
						if((ED_PanelKind)i == ED_PanelKind_TileSetViewer)
						{
							ed_draw_spritesheet(panel, 3, 3, str8_lit("debug/numbers.png"));
							ed_draw_spritesheet(panel, 3, 2, str8_lit("fox/fox.png"));
							ed_draw_spritesheet(panel, 3, 3, str8_lit("impolo/impolo-east.png"));
							ed_draw_spritesheet(panel, 3, 1, str8_lit("tree/trees.png"));
							ed_draw_spritesheet(panel, 3, 3, str8_lit("grass/grass_tile.png"));
						}
						else if((ED_PanelKind)i == ED_PanelKind_Debug)
						{
							panel->update_timer += delta;
							if(panel->update_timer > 1.f)
							{
								panel->cc = tcxt->counters_last[DEBUG_CYCLE_COUNTER_UPDATE_AND_RENDER].cycle_count * 0.001f;
								panel->ft = delta;
								panel->update_timer = 0;
							}
							
							ui_size_kind(panel->cxt, UI_SizeKind_TextContent)
							{
								if(ui_labelf(panel->cxt, "cc : %.f K", panel->cc).active)
								{
									printf("pressed\n");
								}
								
								ui_labelf(panel->cxt, "ft : %.fms", panel->ft * 1000);
								ui_labelf(panel->cxt, "cmt: %.1f MB", state->cmt * 0.000001f);
								ui_labelf(panel->cxt, "res: %.1f GB", state->res * 0.000000001f);
								ui_labelf(panel->cxt, "textures: %.1f MB", a_asset_cache->tex_mem * 0.000001);
								
								for(u32 i = 0; i < ED_PanelKind_COUNT; i++)
								{
									if(ui_labelf(panel->cxt, "%d. [%.f, %.f]", i, ed_state->panels[i].pos.x, ed_state->panels[i].pos.y).active)
									{
										ed_state->panels[i].floating = !ed_state->panels[i].floating; 
									}
								}
							}
							
							R_Handle face = a_handle_from_path(str8_lit("debug/toppema.png"));
							
							ui_size_kind(panel->cxt, UI_SizeKind_Pixels)
								ui_pref_size(panel->cxt, 100)
							{
								ui_image(panel->cxt, face, rect(0,0,1,1), D_COLOR_WHITE, str8_lit("debug/toppema.png"));
							}
							
						}
						else if((ED_PanelKind)i == ED_PanelKind_Inspector)
						{
							if(panel->selected_tile.len > 0)
							{
								ui_size_kind(panel->cxt, UI_SizeKind_TextContent)
								{
									ui_labelf(panel->cxt, "%.*s", str8_varg(panel->selected_tile));
									
									
									ui_size_kind(panel->cxt, UI_SizeKind_Pixels)
										ui_pref_size(panel->cxt, 100)
									{
										R_Handle img = a_handle_from_path(panel->selected_tile);
										
										panel->selected_slot = ui_image(panel->cxt, img, panel->selected_tile_rect, hsva_to_rgba(panel->hsva), str8_lit("inspector panel image")).widget;
									}
									
									ui_labelf(panel->cxt, "[%.2f, %.2f]", rect_tl_varg(panel->selected_tile_rect));
									ui_labelf(panel->cxt, "[%.2f, %.2f]", rect_br_varg(panel->selected_tile_rect));
									
									ui_size_kind(panel->cxt, UI_SizeKind_Pixels)
										ui_pref_size(panel->cxt, 360)
									{
										ui_sat_picker(panel->cxt, panel->hsva.x, &panel->hsva.y, &panel->hsva.z, str8_lit("sat picker thing"));
										ui_pref_height(panel->cxt, 40)
										{
											ui_hue_picker(panel->cxt, &panel->hsva.x, str8_lit("hue picker thing"));
											
											ui_alpha_picker(panel->cxt, panel->hsva.xyz, &panel->hsva.w, str8_lit("alpha picker thing"));
										}
									}
									
									s32 hue = panel->hsva.x;
									s32 sat = panel->hsva.y * 100;
									s32 val = panel->hsva.z * 100;
									
									ui_labelf(panel->cxt, "hsv: %d, %d%, %d%", hue, sat, val);
									
									v4f rgba = hsva_to_rgba(panel->hsva);
									
									ui_labelf(panel->cxt, "rgb: %.2f, %.2f, %.2f, %.2f", v4f_varg(rgba));
								}
							}
						}
					}
				}
			}
		}
		
		SDL_GetGlobalMouseState(&panel->old_pos.x, &panel->old_pos.y);
		
		ui_layout(dad);
		ed_draw_panel(panel);
		ui_end(panel->cxt);
		d_pop_bucket();
		
		events->first = events->last = 0;
		events->count = 0; 
	}
	
	for(u32 i = 0; i < ED_PanelKind_COUNT; i++)
	{
		ED_Panel *panel = ed_state->panels + i;
		r_submit(panel->win, &panel->bucket->list);
	}
}

void ed_draw_panel(ED_Panel *panel)
{
	UI_Widget *parent = panel->root;
	
	parent->pos.x = parent->computed_rel_position[0];
	parent->pos.y = parent->computed_rel_position[1];
	
	if(panel->floating)
	{
		parent->pos.x +=  panel->pos.x;
		parent->pos.y +=  panel->pos.y;
	}
	else
	{
		//panel->pos = panel->root->pos;
	}
	
	parent->size.x = parent->computed_size[0];
	parent->size.y = parent->computed_size[1];
	
	d_draw_rect(rect({}, parent->size), panel->color);
	
	os_set_window_size(panel->win, parent->size);
	
	ed_draw_children(panel, parent);
}

void ed_draw_children(ED_Panel *panel, UI_Widget *root)
{
	root->pos.x = root->computed_rel_position[0];
	root->pos.y = root->computed_rel_position[1];
	root->size.x = root->computed_size[0];
	root->size.y = root->computed_size[1];
	
	if(panel->floating)
	{
		root->pos.x += root->fixed_position.x;
		root->pos.y += root->fixed_position.y;
	}
	
	if(root->flags & UI_Flags_has_text)
	{
		v4f color = {};
		if(root->hot)
		{
			color = D_COLOR_BLUE;
		}
		else
		{
			color = root->color;
		}
		
		D_Text_params params = 
		{
			color,
			d_state->default_text_params.scale,
			d_state->default_text_params.atlas,
			d_state->default_text_params.atlas_tex,
		};
		
		d_draw_text(root->text, root->pos, &params);
	}
	
	if(root->flags & UI_Flags_has_custom_draw)
	{
		
		if(panel->selected_slot == root)
		{
			Rect selected_slot_rect = rect(panel->selected_slot->pos, panel->selected_slot->size);
			d_draw_img(selected_slot_rect, rect(0, 0, 2, 2), D_COLOR_WHITE, a_get_alpha_bg_tex());
		}
		
		root->custom_draw(root, root->custom_draw_data);
	}
	
	for(UI_Widget *child = root->first; child; child = child->next)
	{
		ed_draw_children(panel, child);
	}
}