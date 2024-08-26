void ed_draw_spritesheet(ED_State *ed_state, f32 x, f32 y, Str8 path)
{
	R_Handle img = a_handle_from_path(path);
	
	f32 width = 1.f/x;
	f32 height = 1.f/y;
	
	f32 advance_y = 1 - height;
	
	ui_push_size_kind(ed_state->cxt, UI_SizeKind_ChildrenSum);
	ui_col(ed_state->cxt)
	{
		
		for(u32 i = 0; i < y; i ++)
		{
			f32 advance_x = 0;
			ui_row(ed_state->cxt)
			{
				ui_push_size_kind(ed_state->cxt, UI_SizeKind_Pixels);
				ui_pref_width(ed_state->cxt, 60)
					ui_pref_height(ed_state->cxt, 60)
				{
					for(u32 j = 0; j < x; j++)
					{
						Rect recty = rect(advance_x, advance_y, advance_x + width, advance_y + height);
						
						if(ui_imagef(ed_state->cxt, img, recty, D_COLOR_WHITE, "%.*s%d%d", str8_varg(path), i, j).active)
						{
							ed_state->selected_tile = path;
							ed_state->selected_tile_rect = recty;
						}
						advance_x += width;
					}
				}
				ui_pop_size_kind(ed_state->cxt);
				
			}
			advance_y -= height;
		}
	}
	ui_pop_size_kind(ed_state->cxt);
}

void ed_update(State *state, OS_Event_list *events, f32 delta)
{
	ED_State *ed_state = &state->ed_state;
	
	if(!ed_state->initialized)
	{
		ed_state->arena = arena_create();
		
		ed_state->cxt = ui_alloc_cxt();
		
		// panel construction
		{
			ED_Panel *p = ed_state->panels + ED_PanelKind_TileSetViewer;
			p->scale = v2f{{1.6,0.3}};
			p->name = push_str8f(ed_state->arena, "tile set viewer");
		}
		
		{
			ED_Panel *p = ed_state->panels + ED_PanelKind_TileMap;
			p->pos = {{312, 353}};
			p->scale = v2f{{1.6,0.3}};
			p->name = push_str8f(ed_state->arena, "tile map");
			p->floating = 1;
		}
		
		{
			ED_Panel *p = ed_state->panels + ED_PanelKind_Debug;
			p->scale = v2f{{1.6,0.3}};
			p->hide = 1;
			p->name = push_str8f(ed_state->arena, "debug");
		}
		
		{
			ED_Panel *p = ed_state->panels + ED_PanelKind_Inspector;
			p->scale = v2f{{1.6,0.3}};
			p->hide = 0;
			p->name = push_str8f(ed_state->arena, "Inspector");
		}
		ed_state->initialized = 1;
	}
	
	v2s win_size = os_get_window_size(state->win);
	ui_begin(ed_state->cxt, win_size, &state->atlas, events);
	
	ui_set_next_child_layout_axis(ed_state->cxt, Axis2_X);
	UI_Widget *dad = ui_make_widget(ed_state->cxt, str8_lit(""));
	ui_push_parent(ed_state->cxt, dad);
	
	for(u32 i = 0; i < ED_PanelKind_COUNT; i++)
	{
		ED_Panel *panel = ed_state->panels + i;
		
		ui_push_size_kind(ed_state->cxt, UI_SizeKind_ChildrenSum);
		panel->root = ui_make_widget(ed_state->cxt, str8_lit(""));
		panel->color = ED_THEME_BG;
		
		if(panel->floating)
		{
			panel->root->flags = (UI_Flags)(UI_Flags_is_floating_x | UI_Flags_is_floating_y);
		}
		
		ui_push_parent(ed_state->cxt, panel->root);
		
		ui_text_color(ed_state->cxt, ED_THEME_TEXT)
			ui_fixed_pos(ed_state->cxt, (panel->pos))
			ui_col(ed_state->cxt)
		{
			// NOTE(mizu):  title bar, dragging and hiding
			ui_row(ed_state->cxt)
			{
				UI_Signal hide = {};
				
				ui_push_size_kind(ed_state->cxt, UI_SizeKind_TextContent);
				
				UI_Signal res = {};
				
				ui_pref_height(ed_state->cxt, 30)
					ui_size_kind_y(ed_state->cxt, UI_SizeKind_Pixels)
				{
					res = ui_label(ed_state->cxt, panel->name);
				}
				
				if(panel->floating)
				{
					ui_push_size_kind(ed_state->cxt, UI_SizeKind_Pixels);
					ui_pref_width(ed_state->cxt, 30)
					{
						ui_spacer(ed_state->cxt);
					}
					ui_pop_size_kind(ed_state->cxt);
					hide = ui_labelf(ed_state->cxt, "hide%d",i);
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
				
				os_mouse_released(&state->events, OS_MouseButton_Left);
				
				if(os_mouse_held(OS_MouseButton_Left) && panel->grabbed)
				{
					panel->pos += (ed_state->cxt->mpos - ed_state->old_pos);
				}
				else
				{
					panel->grabbed = 0;
				}
				ui_pop_size_kind(ed_state->cxt);
			}
			
			if(!panel->hide)
			{
				if((ED_PanelKind)i == ED_PanelKind_TileSetViewer)
				{
					ed_draw_spritesheet(ed_state, 3, 3, str8_lit("debug/numbers.png"));
					ed_draw_spritesheet(ed_state, 3, 2, str8_lit("fox/fox.png"));
					ed_draw_spritesheet(ed_state, 3, 3, str8_lit("impolo/impolo-east.png"));
					ed_draw_spritesheet(ed_state, 3, 1, str8_lit("tree/trees.png"));
					ed_draw_spritesheet(ed_state, 3, 3, str8_lit("grass/grass_tile.png"));
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
					
					ui_push_size_kind(ed_state->cxt, UI_SizeKind_TextContent);
					if(ui_labelf(ed_state->cxt, "cc : %.f K", panel->cc).active)
					{
						printf("pressed\n");
					}
					
					ui_labelf(ed_state->cxt, "ft : %.fms", panel->ft * 1000);
					ui_labelf(ed_state->cxt, "cmt: %.1f MB", state->cmt * 0.000001f);
					ui_labelf(ed_state->cxt, "res: %.1f GB", state->res * 0.000000001f);
					ui_labelf(ed_state->cxt, "textures: %.1f MB", a_asset_cache->tex_mem * 0.000001);
					
					for(u32 i = 0; i < ED_PanelKind_COUNT; i++)
					{
						if(ui_labelf(ed_state->cxt, "%d. [%.f, %.f]", i, ed_state->panels[i].pos.x, ed_state->panels[i].pos.y).active)
						{
							ed_state->panels[i].floating = !ed_state->panels[i].floating; 
						}
					}
					ui_pop_size_kind(ed_state->cxt);
					
					R_Handle face = a_handle_from_path(str8_lit("debug/toppema.png"));
					
					ui_push_size_kind(ed_state->cxt, UI_SizeKind_Pixels);
					ui_pref_width(ed_state->cxt, 100)
						ui_pref_height(ed_state->cxt, 100)
					{
						ui_image(ed_state->cxt, face, rect(0,0,1,1), D_COLOR_WHITE, str8_lit("debug/toppema.png"));
					}
					ui_pop_size_kind(ed_state->cxt);
				}
				else if((ED_PanelKind)i == ED_PanelKind_Inspector)
				{
					if(ed_state->selected_tile.len > 0)
					{
						ui_push_size_kind(ed_state->cxt, UI_SizeKind_TextContent);
						ui_labelf(ed_state->cxt, "%.*s", str8_varg(ed_state->selected_tile));
						
						ui_push_size_kind(ed_state->cxt, UI_SizeKind_Pixels);
						ui_pref_width(ed_state->cxt, 100)
							ui_pref_height(ed_state->cxt, 100)
						{
							R_Handle img = a_handle_from_path(ed_state->selected_tile);
							
							ui_image(ed_state->cxt, img, ed_state->selected_tile_rect, D_COLOR_WHITE, str8_lit("inspector panel image"));
						}
						ui_pop_size_kind(ed_state->cxt);
						
						ui_labelf(ed_state->cxt, "[%.2f, %.2f]", rect_tl_varg(ed_state->selected_tile_rect));
						ui_labelf(ed_state->cxt, "[%.2f, %.2f]", rect_br_varg(ed_state->selected_tile_rect));
						ui_pop_size_kind(ed_state->cxt);
						
					}
				}
			}
		}
		ui_pop_size_kind(ed_state->cxt);
		ui_pop_parent(ed_state->cxt);
	}
	ui_layout(dad);
	
	for(u32 i = 0; i < 4; i ++)
	{
		ED_Panel *p = ed_state->panels + i;
		ed_draw_panel(p);
	}
	ui_pop_parent(ed_state->cxt);
	ui_end(ed_state->cxt);
	
	ed_state->old_pos = ed_state->cxt->mpos;
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
		panel->pos = panel->root->pos;
	}
	
	parent->size.x = parent->computed_size[0];
	parent->size.y = parent->computed_size[1];
	
	d_draw_rect(rect(parent->pos, parent->size), panel->color);
	
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
	
	if(root->flags & UI_Flags_has_img)
	{
		root->custom_draw(root, root->custom_draw_data);
	}
	
	for(UI_Widget *child = root->first; child; child = child->next)
	{
		ed_draw_children(panel, child);
	}
}