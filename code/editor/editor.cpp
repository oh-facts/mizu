void ed_update(State *state, OS_Event_list *events, f32 delta)
{
	ED_State *ed_state = &state->ed_state;
	
	if(!ed_state->initialized)
	{
		ed_state->cxt = ui_alloc_cxt();
		ed_state->panels[0].pos = v2f{{-1.3, -0.8}};
		ed_state->panels[0].scale = v2f{{0.6,0.3}};
		
		ed_state->initialized = 1;
	}
	
	ui_begin(ed_state->cxt, os_get_window_size(state->win), &state->atlas, events);
	
	ui_push_size_kind(ed_state->cxt, UI_SizeKind_ChildrenSum);
	
	ui_bg_color(ed_state->cxt, D_COLOR_BLACK)
		ui_text_color(ed_state->cxt, D_COLOR_WHITE)
		ui_fixed_pos(ed_state->cxt, (ed_state->panels[0].pos))
		ui_colf(ed_state->cxt, "diagnostics")
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
		
		ui_push_size_kind_x(ed_state->cxt, UI_SizeKind_Pixels);
		ui_push_pref_width(ed_state->cxt, 0.3f);
		
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
		
		ui_pop_pref_width(ed_state->cxt);
		ui_pop_size_kind_x(ed_state->cxt);
		
		if(ui_labelf(ed_state->cxt, "cc : %.f K", cc).active)
		{
			printf("pressed\n");
		}
		
		ui_labelf(ed_state->cxt, "this work?");
		ui_labelf(ed_state->cxt, "cmt: %.1f MB", state->cmt * 0.000001f);
		ui_labelf(ed_state->cxt, "res: %.1f GB", state->res * 0.000000001f);
		
		ui_pop_size_kind(ed_state->cxt);
	}
	
	ui_pop_size_kind(ed_state->cxt);
	ui_end(ed_state->cxt);
	
	ui_layout(ed_state->cxt->root);
	
	d_draw_ui(&state->draw, ed_state->cxt->root);
	
	ed_state->old_pos = ed_state->cxt->mpos;
	
}