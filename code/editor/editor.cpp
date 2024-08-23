void ed_update(State *state, OS_Event_list *events, f32 delta)
{
	ED_State *ed_state = &state->ed_state;
	
	if(!ed_state->initialized)
	{
		ed_state->cxt = ui_alloc_cxt();
		ed_state->panels[0].pos = v2f{{-16/9.f, 1}};
		ed_state->panels[0].scale = v2f{{1.6,0.3}};
		
		ed_state->initialized = 1;
	}
	
	ui_begin(ed_state->cxt, os_get_window_size(state->win), &state->atlas, events);
	
	ui_push_size_kind(ed_state->cxt, UI_SizeKind_ChildrenSum);
	
	ui_bg_color(ed_state->cxt, D_COLOR_BLACK)
		ui_text_color(ed_state->cxt, D_COLOR_WHITE)
		ui_fixed_pos(ed_state->cxt, (ed_state->panels[0].pos))
		ui_col(ed_state->cxt)
	{
		local_persist f32 update_timer = 0;
		local_persist f32 cc = 0;
		update_timer += delta;
		if(update_timer > 0.3f)
		{
			cc = tcxt->counters_last[DEBUG_CYCLE_COUNTER_UPDATE_AND_RENDER].cycle_count * 0.001f;
			update_timer = 0;
		}
		
		ui_push_size_kind(ed_state->cxt, UI_SizeKind_TextContent);
		
		UI_Signal res = ui_labelf(ed_state->cxt, "Tilemap Editor");
		
		if(res.active)
		{
			ed_state->panels[0].grabbed = 1;
		}
		
		os_mouse_released(&state->events, OS_MouseButton_Left);
		
		if(os_mouse_held(OS_MouseButton_Left) && ed_state->panels[0].grabbed)
		{
			ed_state->panels[0].pos += (ed_state->cxt->mpos - ed_state->old_pos);
		}
		else
		{
			ed_state->panels[0].grabbed = 0;
		}
		
		if(ui_labelf(ed_state->cxt, "cc : %.f K", cc).active)
		{
			printf("pressed\n");
		}
		
		R_Handle img = a_handle_from_path(str8_lit("debug/numbers.png"));
		
		ui_push_size_kind(ed_state->cxt, UI_SizeKind_ChildrenSum);
		ui_col(ed_state->cxt)
		{
			for(u32 i = 0; i < 3; i ++)
			{
				ui_row(ed_state->cxt)
				{
					ui_push_size_kind(ed_state->cxt, UI_SizeKind_Pixels);
					ui_pref_width(ed_state->cxt, 0.3)
						ui_pref_height(ed_state->cxt, 0.3)
					{
						for(u32 j = 0; j < 3; j++)
						{
							f32 advance = 1/9.f;
							f32 index = (i*3 + j) * advance;
							
							Rect recty = rect(index, 0, advance + index, 1);
							
							ui_imagef(ed_state->cxt, img, recty, D_COLOR_WHITE, "facial%d%d", i, j);
						}
					}
					ui_pop_size_kind(ed_state->cxt);
					
				}
				
			}
		}
		
		ui_pop_size_kind(ed_state->cxt);
		
		ui_labelf(ed_state->cxt, "this work?");
		ui_labelf(ed_state->cxt, "cmt: %.1f MB", state->cmt * 0.000001f);
		ui_labelf(ed_state->cxt, "res: %.1f GB", state->res * 0.000000001f);
		
		ui_pop_size_kind(ed_state->cxt);
	}
	
	ui_pop_size_kind(ed_state->cxt);
	ui_end(ed_state->cxt);
	
	ui_layout(ed_state->cxt->root);
	
	ed_draw_panel(ed_state->cxt->root);
	
	ed_state->old_pos = ed_state->cxt->mpos;
}

void ed_draw_panel(UI_Widget *root)
{
	root = root->first;
	
	root->pos.x = root->computed_rel_position[0] + root->fixed_position.x;
	root->pos.y = root->computed_rel_position[1] + root->fixed_position.y;
	root->size.x = root->computed_size[0];
	root->size.y = root->computed_size[1];
	
	d_draw_rect(rect(root->pos, root->size), root->bg_color);
	
	ed_draw_children(root);
}

void ed_draw_children(UI_Widget *root)
{
	root->pos.x = root->computed_rel_position[0] + root->fixed_position.x;
	root->pos.y = root->computed_rel_position[1] + root->fixed_position.y;
	root->size.x = root->computed_size[0];
	root->size.y = root->computed_size[1];
	
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
		ed_draw_children(child);
	}
}