/* date = September 27th 2024 4:22 pm */
typedef u32 EntityFlags;

enum
{
	EntityFlags_Control = 1 << 0,
	EntityFlags_Follow = 1 << 1,
	EntityFlags_Friendly = 1 << 2,
	EntityFlags_Enemy = 1 << 3,
	EntityFlags_Dead = 1 << 4
};

enum Art
{
	ArtKind_Null,
	ArtKind_Fox,
	ArtKind_Impolo,
	ArtKind_Trees,
	ArtKind_COUNT
};

global Str8 art_paths[ArtKind_COUNT] = 
{
	str8_lit(""),
	str8_lit("fox/fox.png"),
	str8_lit("impolo/impolo-east.png"),
	str8_lit("tree/trees.png"),
};

struct Entity;
struct EntityHandle
{
	u64 gen;
	Entity *v;
};

struct Entity
{
	Str8 name;
	u64 gen;
	
	EntityFlags flags;
	v2f pos;
	v2f old_pos;
	v2f size;
	s32 layer;
	
	f32 speed;
	f32 health;
	f32 max_health;
	
	v4f tint;
	
	Art art;
	s32 x;
	s32 y;
	s32 n;
	v2f basis;
	
	EntityHandle target;
	Entity *next;
};

function Entity *entityFromHandle(EntityHandle handle)
{
	Entity *out = 0;
	
	if(handle.gen == handle.v->gen)
	{
		out = handle.v;
	}
	
	return out;
}

function EntityHandle handleFromEntity(Entity *entity)
{
	EntityHandle out = {};
	out.v = entity;
	out.gen = entity->gen;
	return out;
}

#define MAX_ENTITIES 100

struct EntityStore
{
	Entity entities[MAX_ENTITIES];
	s32 num_entities;
	
	Entity *free;
};

function Entity *entity_alloc(EntityStore *store, EntityFlags flags)
{
	Entity *out = store->free;
	
	if(!out)
	{
		out = store->entities + store->num_entities++;
	}
	
	*out = {};
	
	out->flags = flags;
	out->gen ++;
	
	return out;
}

function void entity_free(EntityStore *store, EntityHandle handle)
{
	handle.v->gen++;
	if(!store->free)
	{
		store->free = handle.v;
	}
	else
	{
		handle.v->next = store->free;
		store->free = handle.v;
	}
}

struct Lister
{
	b32 initialized;
	EntityStore *e_store;
};

// TODO(mizu): Remove camera from base
// use Camera whole here
// GameCam.cam. is annoying
struct GameCam
{
	Camera cam;
	EntityHandle target;
};

struct Game
{
	Arena *arena;
	EntityStore e_store;
	GameCam cam;
	b32 initialized;
	b32 start;
};

function ED_CUSTOM_TAB(lister_panel)
{
	Lister *lister = (Lister*)(user_data); 
	EntityStore *store = lister->e_store;
	
	if(!lister->initialized)
	{
		lister->initialized = 1;
	}
	
	for(s32 i = 0; i < store->num_entities; i++)
	{
		Entity *entity = store->entities + i;
		
		ui_text_color(window->cxt, D_COLOR_WHITE)
			ui_size_kind(window->cxt, UI_SizeKind_TextContent)
		{
			ui_labelf(window->cxt, "%s", (char*)entity->name.c);
		}
		
		ui_size_kind(window->cxt, UI_SizeKind_ChildrenSum)
			ui_row(window->cxt)
		{
			ui_size_kind(window->cxt, UI_SizeKind_Pixels)
				ui_pref_width(window->cxt, 10)
			{
				ui_spacer(window->cxt);
			}
			
			ui_size_kind(window->cxt, UI_SizeKind_ChildrenSum)
				ui_col(window->cxt)
				ui_size_kind(window->cxt, UI_SizeKind_TextContent)
			{
				ui_labelf(window->cxt, "%dposition: [%.f %.f]", i, entity->pos.x, entity->pos.y);
				ui_labelf(window->cxt, "%dlayer: %d", i, entity->layer);
				ui_labelf(window->cxt, "%dspeed: %.f", i, entity->speed);
				ui_labelf(window->cxt, "%dhealth: %.f", i, entity->health);
			}
		}
		
	}
}

function ED_CUSTOM_TAB(game_update_and_render)
{
	Game *game = (Game*)(user_data); 
	
	GameCam *cam = &game->cam;
	EntityStore *store = &game->e_store;
	
	if(!game->initialized)
	{
		game->initialized = 1;
		game->arena = arena_create();
		ED_Window *test = ed_open_window(ED_WindowFlags_HasSurface, v2f{{1230, 50}}, v2f{{400,800}});
		
		ED_Panel *panel2 = ed_open_panel(test, Axis2_X, 1);
		ED_Tab *tab = ed_open_tab(panel2, ED_TabKind_Custom, {{400, 800}});
		tab->custom_draw = lister_panel;
		
		Lister *lister = push_struct(game->arena, Lister);
		lister->e_store = store;
		tab->custom_drawData = lister;
	}
	
	b32 restart = 0;
	
	ui_hover_color(window->cxt, D_COLOR_BLUE)
		ui_text_color(window->cxt, D_COLOR_BLUE)
		ui_size_kind(window->cxt, UI_SizeKind_ChildrenSum)
		ui_row(window->cxt)
	{
		
		ui_pref_size(window->cxt, 60)
			ui_size_kind(window->cxt, UI_SizeKind_Pixels)
		{
			R_Handle pause_play = a_handleFromPath(str8_lit("editor/pause_play.png"));
			
			ui_imagef(window->cxt, pause_play, rect(0.5, 0, 1, 1), D_COLOR_WHITE, "restart");
			ui_pref_width(window->cxt, 10)
			{
				ui_spacer(window->cxt);
			}
			
			restart = ui_imagef(window->cxt, pause_play, rect(0, 0, 0.5, 1), D_COLOR_WHITE, "pause").active;
			
		}
	}
	
	if(restart)
	{
		game->start = 0;
	}
	
	if(!game->start)
	{
		game->e_store.num_entities = 0;
		
		game->start = 1;
		Entity *py = entity_alloc(store, EntityFlags_Control);
		py->pos = {{70, 70}};
		py->size = {{64, 64}};
		py->tint = D_COLOR_WHITE;
		py->art = ArtKind_Impolo;
		py->basis.y = 48;
		py->layer = 1;
		py->n = 2;
		py->x = 3;
		py->y = 3;
		py->speed = 250;
		py->health = 300;
		py->name = str8_lit("impolo");
		
		Entity *fox = entity_alloc(store, EntityFlags_Follow);
		fox->pos = {{100, 100}};
		fox->size = {{32, 32}};
		fox->tint = D_COLOR_WHITE;
		fox->art = ArtKind_Fox;
		fox->basis.y = 65;
		fox->layer = 1;
		fox->n = 3;
		fox->x = 3;
		fox->y = 2;
		fox->speed = 150;
		fox->target = handleFromEntity(py);
		fox->health = 300;
		fox->name = str8_lit("fox");
		
		cam->target = handleFromEntity(py);
		cam->cam.target = WORLD_FRONT;
		cam->cam.up = WORLD_UP;
		cam->cam.zoom = 135.f;
		cam->cam.proj = CAMERA_PROJ_ORTHO;
		cam->cam.speed = 400;
		cam->cam.input_rot.x = 0;
		cam->cam.input_rot.y = 0;
		cam->cam.aspect = tab->target.u32_m[3] * 1.f / tab->target.u32_m[4];
	}
	
	// update camera
	{
		Entity *target = entityFromHandle(cam->target);
		if(target)
		{
			cam->cam.pos = v3f{{target->pos.x, -target->pos.y, -1}};
		}
	}
	
	m4f_ortho_proj world_proj_inv = cam_get_proj_inv(&cam->cam);
	
	m4f world_proj = world_proj_inv.fwd;
	
	m4f world_view = cam_get_view(&cam->cam);
	
	m4f world_proj_view = world_proj * world_view * m4f_make_scale({{1, -1, 1}});
	d_push_proj_view(world_proj_view);
	
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
			R_Handle tex = {};
			Rect src = rect(0, 0, 1, 1);
			s32 layer = 0;
			f32 basis = 0;
			f32 offset_y = 0;
			
			if(tile_id == 0)
			{
				read_only v4f colors[] = 
				{
					D_COLOR_WHITE,
					D_COLOR_RED,
					D_COLOR_GREEN,
					D_COLOR_BLUE,
					D_COLOR_CYAN,
					D_COLOR_MAGENTA,
					D_COLOR_YELLOW
				};
				
				color = colors[(row + col) % 7];
				tex = a_get_alpha_bg_tex();
			}
			else
			{
				color = D_COLOR_WHITE;
				tex = a_handleFromPath(str8_lit("tree/trees.png"));
				src = {{{0.333, 0}} , {{0.666, 1}}};
				layer = 1;
				basis = -25;
				offset_y = -64;
			}
			
			// 82, 140 for the tree
			f32 padding = 75;
			
			Rect dst = {};
			dst.tl.x = col * padding;
			dst.tl.y = row * padding + offset_y;
			dst.br.x = dst.tl.x + 128;
			dst.br.y = dst.tl.y + 128;
			
			R_Sprite *sprite = d_sprite(dst, color);
			sprite->basis.y = basis;
			sprite->layer = layer;
			sprite->radius = 64;
			sprite->tex = tex;
			sprite->src = src;
		}
	}
	
	for(s32 i = 0; i < store->num_entities; i++)
	{
		Entity *e = store->entities + i;
		
		if(e->health < 0)
		{
			e->flags |= EntityFlags_Dead;
		}
		
		if((e->flags & EntityFlags_Control) && !(e->flags & EntityFlags_Dead))
		{
			if(os_key_press(window->win, SDLK_A))
			{
				e->pos.x -= delta * e->speed;
			}
			
			if(os_key_press(window->win, SDLK_D))
			{
				e->pos.x += delta * e->speed;
			}
			
			if(os_key_press(window->win, SDLK_S))
			{
				e->pos.y += delta * e->speed;
			}
			
			if(os_key_press(window->win, SDLK_W))
			{
				e->pos.y -= delta * e->speed;
			}
		}
		
		if(e->flags & EntityFlags_Follow)
		{
			Entity *target = entityFromHandle(e->target);
			if(target)
			{
				v2f dir = target->pos - e->pos;
				f32 dist = sqrt(dir.x * dir.x + dir.y * dir.y);
				
				if(dist > 10)
				{
					dir = dir / dist;
					e->pos += dir * e->speed * delta;
				}
				else
				{
					target->health--;
				}
			}
		}
		
		// art
		{
			R_Sprite *sprite = d_sprite(rect(e->old_pos - e->size/2, e->size), e->tint);
			
			if(e->art == ArtKind_Null)
			{
				sprite->tex = a_get_checker_tex();
			}
			else
			{
				sprite->tex = a_handleFromPath(art_paths[e->art]);
			}
			
			sprite->basis.y = e->basis.y;
			sprite->layer = e->layer;
			
			s32 frame_col = e->n % e->x;
			s32 frame_row = e->n / e->x;
			
			Rect src = {};
			
			src.tl.x = (f32)frame_col / (f32)e->x;
			src.tl.y = (f32)frame_row / (f32)e->y;
			src.br.x = (f32)(frame_col + 1) / (f32)e->x;
			src.br.y = (f32)(frame_row + 1) / (f32)e->y;
			
			sprite->src = src;
		}
		
		// health 
		{
			v2f pos = e->old_pos;
			pos.y += e->size.y / 2;
			pos.x -= 15;
			s32 count = (e->health + 99) / 100;
			for(s32 j = 0; j < count; j++)
			{
				R_Sprite *health = d_sprite(rect(pos + v2f{.x = j * 10.f}, {{10, 5}}), D_COLOR_RED);
				health->layer = 2;
			}
		}
		e->old_pos = e->pos;
	}
	
	d_pop_target();
	d_pop_proj_view();
	
	v2s size = r_tex_size_from_handle(tab->target);
	
	ui_pref_width(window->cxt, size.x)
		ui_pref_height(window->cxt, size.y)
		ui_size_kind(window->cxt, UI_SizeKind_Pixels)
	{
		ui_imagef(window->cxt, tab->target, rect(0,0,1,1), D_COLOR_WHITE, "game image");
	}
	
	ui_size_kind(window->cxt, UI_SizeKind_TextContent)
	{
		ui_labelf(window->cxt, "Do not enter is written on the doorway, why can't everyone just go away.");
		ui_labelf(window->cxt, "Except for you, you can stay");
		//ui_labelf(window->cxt, "[%.f %.f]", pos.x, pos.y);
	}
	
}