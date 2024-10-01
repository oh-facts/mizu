// TODO(mizu): If a window is focussed, other windows come to the top too. Also, add a minimize and maximize and close button. And work on making the panels better to interact with.

// Also fix the focus stealing thing where if a window is being interacted with
// but it hovers over another window, other window and current window fight for focus.

// TODO(mizu): Then work on panels and then on tabs

// Also make it so that if a window is ontop of main window, it moves with the main window
// 
// oh yeah, and make parent sibling relationships for the windows
// Main window meaning parent window. child windows always display on top of main window
// and if they are inside the parent window's rect, then they should move with it too (?).

// And give the debug panel lots of functionality to control all other windows's features.

// padding in ui
// make text look proper
// UI_SizeKind_PercentOfParent
// make a transition_time so when you press on ui, you can see it as a certain color for
// some time

// DOTO(mizu): And ofc, make the main engine window also one of these ui windows. 
// Call panels windows, because thats what they are. 
// rects with borders.
// rects with round edges
// render game to an image, then render that as a ui element
// make a render parameter - output frame buffer. used by submit maybe? If 0, renders to default
// fb, otherwise, renders to the passed framebuffer

#define ED_MAX_WINDOWS 10

typedef int ED_WindowFlags;

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

enum ED_TabKind
{
	ED_TabKind_Game,
  ED_TabKind_ModelViewer,
  
  ED_TabKind_TileSetViewer,
	ED_TabKind_Inspector,
	ED_TabKind_Debug,
	ED_TabKind_COUNT,
};

global char *tab_names[ED_TabKind_COUNT] =
{
  "Game",
  "Model viewer",
  
  "tileset",
  "inspector",
  "debug"
};

struct ED_Window;
struct ED_Panel;

#define ED_CUSTOM_TAB(name) void name(R_Handle target, void *user_data)
typedef ED_CUSTOM_TAB(ED_CustomTab);

struct ED_Tab
{
  ED_Tab *next;
  ED_Tab *prev;
  ED_Panel *parent;
  
  ED_TabKind kind;
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

function void ed_draw_spritesheet(ED_Tab *tab, f32 x, f32 y, Str8 path)
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

function ED_State *ed_alloc()
{
  Arena *arena = arena_create();
  ED_State *state = push_struct(arena, ED_State);
  state->arena = arena;
  return state;
}

function ED_Window *ed_open_window(ED_WindowFlags flags, v2f pos, v2f size)
{
  ED_Window *out = ed_state->windows + ed_state->num_windows++;
  out->flags = flags;
  out->size = size;
  out->pos = pos;
  out->cxt = ui_alloc_cxt();
  out->win = os_window_open("alfia", out->size.x, out->size.y);
  
  os_set_window_pos(out->win, out->pos);
  
  if(ed_state->num_windows == 1)
  {
    ed_state->main_window = out;
  }
  
  return out;
}

function v2f ed_size_of_panel(ED_Panel *panel)
{
  v2f out = {};
  out.x = 960;
  out.y = 540;
  return out;
}

function ED_Panel *ed_open_panel(ED_Window *window, Axis2 axis, f32 pct_of_parent)
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

function ED_Tab *ed_open_tab(ED_Panel *panel, ED_TabKind kind)
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
  out->kind = kind;
  
  if(kind == ED_TabKind_Game)
  {
    v2f size = ed_size_of_panel(panel);
    out->target = r_alloc_frame_buffer(size.x, size.y);
  }
  
  if(kind == ED_TabKind_ModelViewer)
  {
    v2f size = ed_size_of_panel(panel);
    out->target = r_alloc_frame_buffer(size.x, size.y);
  }
  
  out->name = push_str8f(ed_state->arena, tab_names[kind]);
  
  panel->active_tab = out;
  
  return out;
}

function void ed_draw_children(ED_Panel *panel, UI_Widget *root)
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

function void ed_draw_panel(ED_Window *window, UI_Widget *root)
{
  for(ED_Panel *panel = window->first_panel; panel; panel = panel->next)
  {
    ed_draw_children(panel, root);
  }
}

function void ed_draw_window(ED_Window *window)
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

function void ed_update(Atlas *atlas, f32 delta)
{
	for(u32 i = 0; i < ed_state->num_windows; i++)
	{
		ED_Window *window = ed_state->windows + i;
		
		window->bucket = d_bucket();
		d_push_bucket(window->bucket);
		d_push_proj_view(m4f_identity());
    
		ui_begin(window->cxt, window->win, atlas);
		
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
            R_Handle titlebar_img = a_handleFromPath(str8_lit("editor/xp_titlebar.png"));
						
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
            
            if(os_mouse_held(window->win, SDL_BUTTON_LEFT) && (window->flags & ED_WindowFlags_Grabbed))
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
              
              switch(tab->kind)
              {
                default: INVALID_CODE_PATH();
                case ED_TabKind_Game:
                {
                  static v2f pos = {{1402, 825}};
                  
                  f32 speed = 600;
                  
                  if(os_key_press(window->win, SDLK_A))
                  {
                    pos.x -= delta * speed;
                  }
                  
                  if(os_key_press(window->win, SDLK_D))
                  {
                    pos.x += delta * speed;
                  }
                  
                  if(os_key_press(window->win, SDLK_S))
                  {
                    pos.y += delta * speed;
                  }
                  
                  if(os_key_press(window->win, SDLK_W))
                  {
                    pos.y -= delta * speed;
                  }
                  
                  static Camera cam = {};
                  cam.target = WORLD_FRONT;
                  cam.up = WORLD_UP;
                  cam.zoom = 600.f;
                  cam.proj = CAMERA_PROJ_ORTHO;
                  cam.speed = 400;
                  cam.input_rot.x = 0;
                  cam.input_rot.y = 0;
                  cam.pos = v3f{{pos.x, -pos.y, -1}};
                  cam.aspect = tab->target.u32_m[3] * 1.f / tab->target.u32_m[4];
                  
                  m4f_ortho_proj world_proj_inv = cam_get_proj_inv(&cam);
                  
                  m4f world_proj = world_proj_inv.fwd;
                  
                  m4f world_view = cam_get_view(&cam);
                  
                  m4f world_proj_view = world_proj * world_view * m4f_make_scale({{1, -1, 1}});
                  
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
                  d_push_proj_view(world_proj_view);
                  
									for(s32 row = 0; row < 9; row++)
                  {
                    for(s32 col = 0; col < 16; col++)
                    {
                      s32 tile_id = tilemap[row][col];
                      v4f color = {};
                      R_Handle tex = {};
                      Rect src = rect(0, 0, 1, 1);
                      
                      if(tile_id == 0)
                      {
                        color = D_COLOR_RED;
                        tex = a_get_alpha_bg_tex();
                      }
                      else
                      {
                        color = D_COLOR_WHITE;
                        tex = a_handleFromPath(str8_lit("tree/trees.png"));
                        src = {{{0.333, 0}} , {{0.666, 1}}};
                      }
                      
                      f32 size = 256;
                      f32 padding = 0;
                      
                      Rect dst = {};
                      dst.tl.x = col * (size + padding);
                      dst.tl.y = row * (size + padding);
                      dst.br.x = dst.tl.x + size;
                      dst.br.y = dst.tl.y + size;
                      
                      R_Sprite *sprite = d_sprite(dst, color);
                      
                      sprite->radius = size / 2;
                      sprite->tex = tex;
                      sprite->src = src;
                    }
                  }
                  
									
                  R_Sprite *py = d_sprite(rect(pos, {{256, 256}}), D_COLOR_WHITE);
                  py->tex = a_handleFromPath(str8_lit("impolo/impolo-east.png"));
                  
                  py->src = {{{0.666, 0}}, {{0.999, 0.333}}};
                  
                  d_pop_proj_view();
                  d_pop_target();
                  
                  ui_pref_width(window->cxt, 960)
                    ui_pref_height(window->cxt, 540)
                    ui_size_kind(window->cxt, UI_SizeKind_Pixels)
                  {
                    ui_imagef(window->cxt, tab->target, rect(0,0,1,1), D_COLOR_WHITE, "game image");
                  }
                  
                  ui_size_kind(window->cxt, UI_SizeKind_TextContent)
                  {
                    ui_labelf(window->cxt, "Do not enter is written on the doorway, why can't everyone just go away.");
                    ui_labelf(window->cxt, "Except for you, you can stay");
                    ui_labelf(window->cxt, "[%.f %.f]", pos.x, pos.y);
                  }
                  
                }break;
                
                // sometimes I get bored so I try to learn 3d
                case ED_TabKind_ModelViewer:
                {
                  d_push_target(tab->target);
                  
                  local_persist b32 done = 0;
                  local_persist GLTF_Model model = {};
                  local_persist R_Model rodel = {};
                  local_persist Arena *arena = 0;
                  if(!done)
                  {
                    arena = arena_create();
                    model = gltf_load_mesh(arena, str8_lit("gltf_test/asuka/asuka.gltf"));
                    rodel = upload_model(arena, &model);
                    
                    done = 1;
                  }
                  
                  static v3f pos = {{0, 0, -1}};
                  f32 speed = 3;
                  
                  m4f proj_view = m4f_make_perspective(90, 16.f / 9, 0.001, 1000);
                  
                  m4f cam = m4f_identity();
                  //cam = m4f_translate(cam, {{0, 0, 1}});
                  //cam = m4f_rot(cam, {{0, 0, 1}});
                  
                  m4f trans = m4f_make_trans(pos);
                  m4f rot = m4f_make_rot_x(DEG_TO_RAD(0));
                  //m4f rot_z = m4f_make_rot_y(DEG_TO_RAD(90));
                  cam = rot * trans;
                  cam = proj_view * cam;
                  
                  if(os_key_press(window->win, SDLK_A))
                  {
                    pos.x += delta * speed;
                  }
                  
                  if(os_key_press(window->win, SDLK_D))
                  {
                    pos.x -= delta * speed;
                  }
                  
                  if(os_key_press(window->win, SDLK_S))
                  {
                    pos.z -= delta * speed;
                  }
                  
                  if(os_key_press(window->win, SDLK_W))
                  {
                    pos.z += delta * speed;
                  }
                  
                  if(os_key_press(window->win, SDLK_Q))
                  {
                    pos.y -= delta * speed;
                  }
                  
                  if(os_key_press(window->win, SDLK_E))
                  {
                    pos.y += delta * speed;
                  }
                  
                  glUseProgram(r_opengl_state->shader_prog[R_OPENGL_SHADER_PROG_MESH]);
                  glBindFramebuffer(GL_FRAMEBUFFER, tab->target.u32_m[2]);
                  
                  glEnable(GL_DEPTH_TEST);
                  glDepthFunc(GL_LESS);
                  
                  f32 color[3] = {1,0,1};
                  f32 clear_value = 1;
                  
                  glClearNamedFramebufferfv(tab->target.u32_m[2], GL_COLOR, 0, color);
                  glClearNamedFramebufferfv(tab->target.u32_m[2], GL_DEPTH, 0, &clear_value);
                  
                  static f32 rotty = 0;
                  rotty += delta * 5;
                  
                  m4f model_mat = m4f_identity();
                  model_mat = m4f_make_trans({{0, -0.8, 0}}) * m4f_make_rot_x(DEG_TO_RAD(90)) * m4f_make_rot_z(rotty);
                  
                  for(u32 j = 0; j < rodel.num_meshes; j++)
                  {
                    R_Mesh* mesh = &rodel.meshes[j];
                    
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mesh->vert);
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, r_opengl_state->inst_buffer[R_OPENGL_INST_BUFFER_MESH]);
                    
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index);         
                    
                    for (u64 k = 0; k < mesh->num_primitives; ++k) 
                    {
                      R_Primitive* primitive = &mesh->primitives[k];
                      
                      void *ssbo_data = glMapNamedBufferRange(r_opengl_state->inst_buffer[R_OPENGL_INST_BUFFER_MESH], 0, sizeof(m4f) * 2 + sizeof(u64) * 2, GL_MAP_WRITE_BIT | 
                                                              GL_MAP_INVALIDATE_BUFFER_BIT);
                      
                      u64 tex_id = primitive->tex.u64_m[0];
                      
                      memcpy(ssbo_data, &cam, sizeof(m4f));
                      memcpy((u8*)ssbo_data + sizeof(m4f), &model_mat, sizeof(m4f));
                      memcpy((u8*)ssbo_data + sizeof(m4f) + sizeof(m4f), &tex_id, sizeof(u64));
                      
                      glUnmapNamedBuffer(r_opengl_state->inst_buffer[R_OPENGL_INST_BUFFER_MESH]);
                      
                      glDrawElements(GL_TRIANGLES, primitive->count, GL_UNSIGNED_INT, (void*)(primitive->start * sizeof(u32)));
                    }
                  }
                  
                  glDisable(GL_DEPTH_TEST);
                  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                  
                  ui_pref_width(window->cxt, 960)
                    ui_pref_height(window->cxt, 540)
                    ui_size_kind(window->cxt, UI_SizeKind_Pixels)
                  {
                    ui_imagef(window->cxt, tab->target, rect(0,0,1,1), D_COLOR_WHITE, "model image");
                  }
                  
                  ui_size_kind(window->cxt, UI_SizeKind_TextContent)
                  {
                    ui_labelf(window->cxt, "crapfest 3000");
                  }
                  
                  d_pop_target();
                  
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
                        
                        ui_pref_height(window->cxt, 10)
                        {
                          ui_spacer(window->cxt);
                        }
                        
                        ui_pref_height(window->cxt, 40)
                        {
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
                
                case ED_TabKind_Debug:
                {
                  tab->update_timer += delta;
                  if(tab->update_timer > 1.f)
                  {
                    tab->cc = tcxt->counters_last[DEBUG_CYCLE_COUNTER_UPDATE_AND_RENDER].cycle_count * 0.001f;
                    tab->ft = delta;
                    tab->update_timer = 0;
                  }
                  
                  ui_size_kind(window->cxt, UI_SizeKind_TextContent)
                  {
                    if(ui_labelf(window->cxt, "cc : %.f K", tab->cc).active)
                    {
                      printf("pressed\n");
                    }
                    
                    ui_labelf(window->cxt, "ft : %.fms", tab->ft * 1000);
                    ui_labelf(window->cxt, "cmt: %.1f MB", total_cmt * 0.000001f);
                    ui_labelf(window->cxt, "res: %.1f GB", total_res * 0.000000001f);
                    ui_labelf(window->cxt, "textures: %.1f MB", a_state->tex_mem * 0.000001);
                  }
                  
                  R_Handle face = a_handleFromPath(str8_lit("debug/toppema.png"));
                  
                  ui_size_kind(window->cxt, UI_SizeKind_Pixels)
                    ui_pref_size(window->cxt, 100)
                  {
                    ui_image(window->cxt, face, rect(0,0,1,1), D_COLOR_WHITE, str8_lit("debug/toppema.png"));
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
    
    d_pop_proj_view();
    d_pop_bucket();
    
  }
}