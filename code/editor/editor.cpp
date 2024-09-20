void ed_draw_spritesheet(ED_Window *window, f32 x, f32 y, Str8 path)
{
	R_Handle img = a_handleFromPath(path);
	
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
							window->inspector->selected_tile = path;
							window->inspector->selected_tile_rect = recty;
						}
						advance_x += width;
					}
				}
				
			}
			advance_y -= height;
		}
	}
}

ED_Window *ed_open_window(ED_State *ed_state, ED_WindowKind kind, ED_SizeKind sizeKind, v2f pos, v2f size)
{
  Assert(ed_state->num_windows < ED_MAX_WINDOWS);
  
  ED_Window *out = ed_state->windows + ed_state->num_windows++;
  out->kind = kind;
  out->sizeKind = sizeKind;
  out->size = size;
  out->name = push_str8f(ed_state->arena, ed_window_str[kind]);
  out->cxt = ui_alloc_cxt();
  out->win = os_window_open(ed_state->arena, ed_window_str[kind], out->size.x, out->size.y, (OS_WindowKind)(OS_WindowKind_Opengl | OS_WindowKind_Undecorate));
  out->floating = 1;
  
  out->pos = pos;
  os_set_window_pos(out->win, out->pos);
  
  return out;
}

void ed_init(State *state)
{
  ED_State *ed_state = &state->ed_state;
	
  ed_state->arena = arena_create();
  
  // default windows construction
  {
    ED_Window *ts_viewer = ed_open_window(ed_state, ED_WindowKind_TileSetViewer, ED_SizeKind_ChildrenSum, v2f{{1550, 16}}, v2f{{300, 0}});
    
    ED_Window *insp = ed_open_window(ed_state, ED_WindowKind_Inspector, ED_SizeKind_ChildrenSum, v2f{{39, 16}}, v2f{{300, 0}});
    insp->hsva = {{1,0,1,1}};
    ts_viewer->inspector = insp;
    
    /*
      ed_open_window(ed_state, ED_WindowKind_Debug, v2f{{431, 675}}, v2f{{300, 0}});
  */
  }
}

void ed_update(State *state, f32 delta)
{
	ED_State *ed_state = &state->ed_state;
	
	for(u32 i = 0; i < ed_state->num_windows; i++)
	{
		ED_Window *window = ed_state->windows + i;
		
		OS_Event_list *events = os_event_list_from_window(window->win);
		
		window->bucket = d_bucket();
		d_push_bucket(window->bucket);
		
		ui_begin(window->cxt, window->win, &state->atlas, events);
		
		ui_set_next_child_layout_axis(window->cxt, Axis2_X);
		UI_Widget *dad = ui_make_widget(window->cxt, str8_lit(""));
		ui_parent(window->cxt, dad)
		{
			ui_size_kind(window->cxt, UI_SizeKind_ChildrenSum)
			{
				window->root = ui_make_widget(window->cxt, str8_lit(""));
				
				window->color = ED_THEME_BG;
				
				if(window->floating)
				{
					window->root->flags = (UI_Flags)(UI_Flags_is_floating_x | UI_Flags_is_floating_y);
				}
				
        // NOTE(mizu):  title bar, dragging and hiding
        ui_parent(window->cxt, window->root)
					ui_text_color(window->cxt, ED_THEME_TEXT)
					//ui_fixed_pos(window->cxt, (window->pos))
					ui_col(window->cxt)
				{
          ui_row(window->cxt)
					{
            UI_Signal hide = {};
            UI_Signal drag = {};
            
            // hide button
            ui_pref_size(window->cxt, 32)
              ui_size_kind(window->cxt, UI_SizeKind_Pixels)
            {
              R_Handle hide_img = a_handleFromPath(str8_lit("editor/titlebar.png"));
              hide = ui_imagef(window->cxt, hide_img, rect(0, 0, 0.25, 1), ED_THEME_TEXT, "hide img %d", i);
						}
            
            // drag titlebar region (entire titlebar - labels/buttons + title)
            ui_size_kind_x(window->cxt, UI_SizeKind_Pixels)
              // NOTE(mizu): bandaid fix until I get aligning done
              ui_pref_width(window->cxt, window->size.x - 32 * 4)
              ui_named_rowf(window->cxt, "menu bar %d", i)
            {
              UI_Widget *menu_bar = window->cxt->parent_stack.top->v;
              
              drag = ui_signal(menu_bar, window->cxt->mpos);
              
              ui_size_kind(window->cxt, UI_SizeKind_TextContent)
                ui_label(window->cxt, window->name);
            }
            
            // minimize, maximize, close
            ui_pref_size(window->cxt, 32)
              ui_size_kind(window->cxt, UI_SizeKind_Pixels)
            {
              R_Handle hide_img = a_handleFromPath(str8_lit("editor/titlebar.png"));
              
              if(ui_imagef(window->cxt, hide_img, rect(0.25, 0, 0.5, 1), ED_THEME_TEXT, "minimize img %d", i).active)
              {
                window->minimize = !window->minimize;
              }
              
              if(ui_imagef(window->cxt, hide_img, rect(0.5, 0, 0.75, 1), ED_THEME_TEXT, "maximize img %d", i).active)
              {
                window->maximize = !window->maximize;
              }
              
              if(ui_imagef(window->cxt, hide_img, rect(0.75, 0, 1, 1), ED_THEME_TEXT, "close img %d", i).active)
              {
                os_window_close(window->win);
              }
            }
            
            if(drag.active)
            {
              window->grabbed = 1;
            }
            
            if(hide.active)
            {
              window->hide = !window->hide;
            }
            
            os_mouse_released(events, window->win, OS_MouseButton_Left);
            
            if(os_mouse_held(window->win, OS_MouseButton_Left) && window->grabbed)
            {
              f32 x, y;
              SDL_GetGlobalMouseState(&x, &y);
              window->pos += v2f{{x, y}} - window->old_pos;
              os_set_window_pos(window->win, window->pos);
            }
            else
            {
              window->grabbed = 0;
            }
            
          }
          
          //printf("%d\n", window->grabbed);
          
          if(!window->hide)
          {
            switch(window->kind)
            {
              default: INVALID_CODE_PATH();
              case ED_WindowKind_Game:
              {
                d_rect(rect(0,0,60,60), D_COLOR_WHITE);
                R_Rect *test_sq = d_rect(rect(100,100,600,300), D_COLOR_BLUE);
                test_sq->border_thickness = 3.f;
                test_sq->border_color = D_COLOR_GREEN;
                test_sq->radius = 30;
              }break;
              
              case ED_WindowKind_TileSetViewer:
              {
                ed_draw_spritesheet(window, 3, 3, str8_lit("debug/numbers.png"));
                ed_draw_spritesheet(window, 3, 2, str8_lit("fox/fox.png"));
                ed_draw_spritesheet(window, 3, 3, str8_lit("impolo/impolo-east.png"));
                ed_draw_spritesheet(window, 3, 1, str8_lit("tree/trees.png"));
                ed_draw_spritesheet(window, 3, 3, str8_lit("grass/grass_tile.png"));
              }break;
              
              case ED_WindowKind_Debug:
              {
                window->update_timer += delta;
                if(window->update_timer > 1.f)
                {
                  window->cc = tcxt->counters_last[DEBUG_CYCLE_COUNTER_UPDATE_AND_RENDER].cycle_count * 0.001f;
                  window->ft = delta;
                  window->update_timer = 0;
                }
                
                ui_size_kind(window->cxt, UI_SizeKind_TextContent)
                {
                  if(ui_labelf(window->cxt, "cc : %.f K", window->cc).active)
                  {
                    printf("pressed\n");
                  }
                  
                  ui_labelf(window->cxt, "ft : %.fms", window->ft * 1000);
                  ui_labelf(window->cxt, "cmt: %.1f MB", state->cmt * 0.000001f);
                  ui_labelf(window->cxt, "res: %.1f GB", state->res * 0.000000001f);
                  ui_labelf(window->cxt, "textures: %.1f MB", a_state->tex_mem * 0.000001);
                  
                  for(u32 i = 0; i < ed_state->num_windows; i++)
                  {
                    if(ui_labelf(window->cxt, "%d. [%.f, %.f]", i, ed_state->windows[i].pos.x, ed_state->windows[i].pos.y).active)
                    {
                      ed_state->windows[i].floating = !ed_state->windows[i].floating; 
                    }
                  }
                }
                
                R_Handle face = a_handleFromPath(str8_lit("debug/toppema.png"));
                
                ui_size_kind(window->cxt, UI_SizeKind_Pixels)
                  ui_pref_size(window->cxt, 100)
                {
                  ui_image(window->cxt, face, rect(0,0,1,1), D_COLOR_WHITE, str8_lit("debug/toppema.png"));
                }
                
              }break;
              
              case ED_WindowKind_Inspector:
              {
                if(window->selected_tile.len > 0)
                {
                  ui_size_kind(window->cxt, UI_SizeKind_TextContent)
                  {
                    ui_labelf(window->cxt, "%.*s", str8_varg(window->selected_tile));
                    
                    
                    ui_size_kind(window->cxt, UI_SizeKind_Pixels)
                      ui_pref_size(window->cxt, 100)
                    {
                      R_Handle img = a_handleFromPath(window->selected_tile);
                      
                      window->selected_slot = ui_image(window->cxt, img, window->selected_tile_rect, hsva_to_rgba(window->hsva), str8_lit("inspector window image")).widget;
                    }
                    
                    ui_labelf(window->cxt, "[%.2f, %.2f]", rect_tl_varg(window->selected_tile_rect));
                    ui_labelf(window->cxt, "[%.2f, %.2f]", rect_br_varg(window->selected_tile_rect));
                    
                    ui_size_kind(window->cxt, UI_SizeKind_Pixels)
                      ui_pref_size(window->cxt, 360)
                    {
                      ui_sat_picker(window->cxt, window->hsva.x, &window->hsva.y, &window->hsva.z, str8_lit("sat picker thing"));
                      ui_pref_height(window->cxt, 40)
                      {
                        ui_hue_picker(window->cxt, &window->hsva.x, str8_lit("hue picker thing"));
                        
                        ui_alpha_picker(window->cxt, window->hsva.xyz, &window->hsva.w, str8_lit("alpha picker thing"));
                      }
                    }
                    
                    s32 hue = window->hsva.x;
                    s32 sat = window->hsva.y * 100;
                    s32 val = window->hsva.z * 100;
                    
                    ui_labelf(window->cxt, "hsv: %d, %d%, %d%", hue, sat, val);
                    
                    v4f rgba = hsva_to_rgba(window->hsva);
                    
                    ui_labelf(window->cxt, "rgb: %.2f, %.2f, %.2f, %.2f", v4f_varg(rgba));
                  }
                }
              }break;
            }
          }
        }
      }
    }
    
    SDL_GetGlobalMouseState(&window->old_pos.x, &window->old_pos.y);
    
    ui_layout(dad);
    ed_draw_window(window);
    ui_end(window->cxt);
    d_pop_bucket();
    
    events->first = events->last = 0;
    events->count = 0; 
  }
}

void ed_draw_window(ED_Window *window)
{
	UI_Widget *parent = window->root;
	
	parent->pos.x = parent->computed_rel_position[0];
	parent->pos.y = parent->computed_rel_position[1];
	
	if(window->floating)
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
	
  if(window->sizeKind == ED_SizeKind_ChildrenSum || window->hide)
  {
    os_set_window_size(window->win, parent->size);
	}
  else
  {
    os_set_window_size(window->win, window->size);
  }
  ed_draw_children(window, parent);
}

void ed_draw_children(ED_Window *window, UI_Widget *root)
{
	root->pos.x = root->computed_rel_position[0];
	root->pos.y = root->computed_rel_position[1];
	root->size.x = root->computed_size[0];
	root->size.y = root->computed_size[1];
	
	if(window->floating)
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
		
		if(window->selected_slot == root)
		{
			Rect selected_slot_rect = rect(window->selected_slot->pos, window->selected_slot->size);
			R_Rect *slot = d_rect(selected_slot_rect, D_COLOR_WHITE);
      slot->src = rect(0, 0, 2, 2);
      slot->tex = a_get_alpha_bg_tex();
    }
		
		root->custom_draw(root, root->custom_draw_data);
	}
	
	for(UI_Widget *child = root->first; child; child = child->next)
	{
		ed_draw_children(window, child);
	}
}