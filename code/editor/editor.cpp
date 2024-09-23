void ed_draw_spritesheet(ED_Tab *tab, f32 x, f32 y, Str8 path)
{
  ED_Panel *panel = tab->parent;
  ED_Window *window = panel->parent;
  
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

ED_Window *ed_open_window(State *state, ED_WindowFlags flags, v2f pos, v2f size)
{
  ED_Window *out = state->windows + state->num_windows++;
  out->flags = flags;
  out->size = size;
  out->pos = pos;
  out->cxt = ui_alloc_cxt();
  out->win = os_window_open(state->arena, "alfia", out->size.x, out->size.y, (OS_WindowKind)(OS_WindowKind_Opengl | OS_WindowKind_Undecorate));
  
  os_set_window_pos(out->win, out->pos);
  
  return out;
}

v2f ed_size_of_panel(ED_Panel *panel)
{
  v2f out = {};
  out.x = 960;
  out.y = 540;
  return out;
}

ED_Panel *ed_open_panel(Arena *arena, ED_Window *window, Axis2 axis, f32 pct_of_parent)
{
  ED_Panel *out = push_struct(arena, ED_Panel);
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

ED_Tab *ed_open_tab(Arena *arena, ED_Panel *panel, ED_TabKind kind)
{
  ED_Tab *out = push_struct(arena, ED_Tab);
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
  out->kind = kind;
  
  if(kind == ED_TabKind_Game)
  {
    v2f size = ed_size_of_panel(panel);
    out->target = r_alloc_frame_buffer(size.x, size.y);
  }
  
  out->name = push_str8f(arena, tab_names[kind]);
  
  panel->active_tab = out;
  
  return out;
}

void ed_update(State *state, f32 delta)
{
	for(u32 i = 0; i < state->num_windows; i++)
	{
		ED_Window *window = state->windows + i;
		
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
				
				if(window->flags & ED_WindowFlags_Floating)
				{
					window->root->flags = (UI_Flags)(UI_Flags_is_floating_x | UI_Flags_is_floating_y);
				}
				
        // NOTE(mizu):  title bar, dragging and hiding
        ui_parent(window->cxt, window->root)
					ui_text_color(window->cxt, ED_THEME_TEXT)
					//ui_fixed_pos(window->cxt, (window->pos))
					ui_col(window->cxt)
				{
          ui_named_rowf(window->cxt, "%d editor title bar")
					{
            window->menu_bar = window->cxt->parent_stack.top->v;
            
            // hide button
            ui_pref_size(window->cxt, 32)
              ui_size_kind(window->cxt, UI_SizeKind_Pixels)
            {
              R_Handle hide_img = a_handleFromPath(str8_lit("editor/titlebar.png"));
              
              if(ui_imagef(window->cxt, hide_img, rect(0, 0, 0.25, 1), ED_THEME_TEXT, "hide img %d", i).active)
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
              
              if(ui_signal(menu_bar, window->cxt->mpos).active)
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
              }
            }
            
            // minimize, maximize, close
            ui_pref_size(window->cxt, 32)
              ui_size_kind(window->cxt, UI_SizeKind_Pixels)
            {
              R_Handle menu_button_img = a_handleFromPath(str8_lit("editor/titlebar.png"));
              
              if(ui_imagef(window->cxt, menu_button_img, rect(0.25, 0, 0.5, 1), ED_THEME_TEXT, "minimize img %d", i).active)
              {
                window->flags ^= ED_WindowFlags_Minimized;
              }
              
              if(ui_imagef(window->cxt, menu_button_img, rect(0.5, 0, 0.75, 1), ED_THEME_TEXT, "maximize img %d", i).active)
              {
                window->flags ^= ED_WindowFlags_Maximized;
              }
              
              if(ui_imagef(window->cxt, menu_button_img, rect(0.75, 0, 1, 1), ED_THEME_TEXT, "close img %d", i).active)
              {
                os_window_close(window->win);
              }
            }
            
            os_mouse_released(events, window->win, OS_MouseButton_Left);
            
            if(os_mouse_held(window->win, OS_MouseButton_Left) && (window->flags & ED_WindowFlags_Grabbed))
            {
              f32 x, y;
              SDL_GetGlobalMouseState(&x, &y);
              window->pos += v2f{{x, y}} - window->old_pos;
              os_set_window_pos(window->win, window->pos);
            }
            else
            {
              window->flags &= ~ED_WindowFlags_Grabbed;
            }
            
          }
          
          //printf("%d\n", window->grabbed);
          
          if(!(window->flags & ED_WindowFlags_Hidden))
          {
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
              
              switch(tab->kind)
              {
                default: INVALID_CODE_PATH();
                case ED_TabKind_Game:
                {
                  
                  s32 tilemap[9][16] = 
                  {
                    {1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1},
                    {1, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 0,  0, 0, 0, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
                    {1, 1, 0, 1,  1, 0, 0, 0,  1, 1, 1, 0,  0, 0, 0, 0},
                    {1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 1, 0,  0, 0, 0, 1},
                    {1, 0, 0, 0,  0, 0, 0, 0,  1, 0, 1, 0,  0, 0, 0, 1},
                    {1, 1, 0, 0,  1, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 1},
                    {1, 0, 1, 0,  1, 0, 1, 0,  0, 0, 1, 0,  1, 0, 0, 1},
                  };
                  
                  d_push_target(tab->target);
                  
                  for(s32 row = 0; row < 9; row++)
                  {
                    for(s32 col = 0; col < 16; col++)
                    {
                      s32 tile_id = tilemap[row][col];
                      v4f color = {};
                      if(tile_id == 0)
                      {
                        color = D_COLOR_RED;
                      }
                      else
                      {
                        color = D_COLOR_GREEN;
                      }
                      
                      f32 size = 64;
                      
                      Rect dst = {};
                      dst.tl.x = col * size;
                      dst.tl.y = row * size;
                      dst.br.x = dst.tl.x + size;
                      dst.br.y = dst.tl.y + size;
                      
                      d_rect(dst, color);
                    }
                  }
                  
                  d_pop_target();
                  
                  ui_pref_width(window->cxt, 960)
                    ui_pref_height(window->cxt, 540)
                    ui_size_kind(window->cxt, UI_SizeKind_Pixels)
                  {
                    ui_imagef(window->cxt, tab->target, rect(0,0,1,1), D_COLOR_WHITE, "game image");
                  }
                  
                  
                }break;
                
                case ED_TabKind_TileSetViewer:
                {
                  struct spritesheet
                  {
                    Str8 path;
                    s32 x;
                    s32 y;
                  };
                  
                  local_persist spritesheet sheets[] = {
                    {str8_lit("fox/fox.png"), 3, 2},
                    {str8_lit("impolo/impolo-east.png"), 3, 3},
                    {str8_lit("tree/trees.png"), 3, 1},
                    {str8_lit("grass/grass_tile.png"), 3, 3}
                  };
                  
                  
                  for(s32 i = 0; i < 2; i++)
                  {
                    ui_row(window->cxt)
                    {
                      for(s32 j = 0; j < 2; j++)
                      {
                        s32 index = i*2 + j;
                        ed_draw_spritesheet(tab, sheets[index].x, sheets[index].y, sheets[index].path);
                        
                        ui_size_kind(window->cxt, UI_SizeKind_Pixels)
                          ui_pref_width(window->cxt, 45)
                        {
                          ui_spacer(window->cxt);
                        }
                      }
                      
                    }
                  }
                }break;
                
                case ED_TabKind_Inspector:
                {
                  if(tab->selected_tile.len > 0)
                  {
                    ui_size_kind(window->cxt, UI_SizeKind_TextContent)
                    {
                      ui_labelf(window->cxt, "%.*s", str8_varg(tab->selected_tile));
                      
                      ui_size_kind(window->cxt, UI_SizeKind_Pixels)
                        ui_pref_size(window->cxt, 100)
                      {
                        R_Handle img = a_handleFromPath(tab->selected_tile);
                        
                        tab->selected_slot = ui_image(window->cxt, img, tab->selected_tile_rect, hsva_to_rgba(tab->hsva), str8_lit("inspector window image")).widget;
                      }
                      
                      ui_labelf(window->cxt, "[%.2f, %.2f]", rect_tl_varg(tab->selected_tile_rect));
                      ui_labelf(window->cxt, "[%.2f, %.2f]", rect_br_varg(tab->selected_tile_rect));
                      
                      ui_size_kind(window->cxt, UI_SizeKind_Pixels)
                        ui_pref_size(window->cxt, 360)
                      {
                        ui_sat_picker(window->cxt, tab->hsva.x, &tab->hsva.y, &tab->hsva.z, str8_lit("sat picker thing"));
                        ui_pref_height(window->cxt, 40)
                        {
                          ui_pref_height(window->cxt, 10)
                          {
                            ui_spacer(window->cxt);
                          }
                          ui_hue_picker(window->cxt, &tab->hsva.x, str8_lit("hue picker thing"));
                          
                          ui_alpha_picker(window->cxt, tab->hsva.xyz, &tab->hsva.w, str8_lit("alpha picker thing"));
                        }
                      }
                      
                      s32 hue = tab->hsva.x;
                      s32 sat = tab->hsva.y * 100;
                      s32 val = tab->hsva.z * 100;
                      
                      ui_labelf(window->cxt, "hsv: %d, %d%, %d%", hue, sat, val);
                      
                      v4f rgba = hsva_to_rgba(tab->hsva);
                      
                      ui_labelf(window->cxt, "rgb: %.2f, %.2f, %.2f, %.2f", v4f_varg(rgba));
                    }
                  }
                }break;
              }
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

void ed_draw_children(ED_Panel *panel, UI_Widget *root)
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
    if(root->active)
    {
      color = root->press_color;
    }
    else if(root->hot)
    {
      color = root->hover_color;
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
  
  ED_Tab *tab = panel->active_tab;
  if(root->flags & UI_Flags_has_custom_draw)
  {
    if(tab->selected_slot == root)
    {
      Rect selected_slot_rect = rect(tab->selected_slot->pos, tab->selected_slot->size);
      R_Rect *slot = d_rect(selected_slot_rect, D_COLOR_WHITE);
      slot->src = rect(0, 0, 2, 2);
      slot->tex = a_get_alpha_bg_tex();
    }
    
    root->custom_draw(root, root->custom_draw_data);
  }
  
  for(UI_Widget *child = root->first; child; child = child->next)
  {
    ed_draw_children(panel, child);
  }
}

void ed_draw_panel(ED_Window *window, UI_Widget *root)
{
  for(ED_Panel *panel = window->first_panel; panel; panel = panel->next)
  {
    ed_draw_children(panel, root);
  }
}

void ed_draw_window(ED_Window *window)
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
  
  d_rect(rect(window->menu_bar->pos, window->menu_bar->size), D_COLOR_BLUE);
  
  if((window->flags & ED_WindowFlags_ChildrenSum) || (window->flags & ED_WindowFlags_Hidden))
  {
    os_set_window_size(window->win, parent->size);
  }
  else
  {
    os_set_window_size(window->win, window->size);
  }
  ed_draw_panel(window, parent);
}
