/* date = September 27th 2024 4:22 pm */
typedef u32 EntityFlags;

enum
{
	EntityFlags_PlayerControl = 1 << 0,
	EntityFlags_Follow = 1 << 1,
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

struct Entity
{
	EntityFlags flags;
	
	v2f pos;
	v2f old_pos;
	v2f size;
	s32 layer;
	
	f32 speed;
	
	v4f tint;
	
	Art art;
	s32 x;
	s32 y;
	s32 n;
	v2f basis;
	
	Entity *follow;
};

#define MAX_ENTITIES 100

struct EntityStore
{
	Entity entities[MAX_ENTITIES];
	s32 num_entities;
};

function Entity *entity_alloc(EntityStore *store, EntityFlags flags)
{
	Entity *out = store->entities + store->num_entities++;
	out->flags = flags;
	return out;
}

struct GameCam
{
	Camera cam;
	Entity *target;
};

struct Game
{
	EntityStore e_store;
	GameCam cam;
	b32 initialized;
};

function ED_CUSTOM_TAB(game_update_and_render)
{
	Game *game = (Game*)(user_data); 
	GameCam *cam = &game->cam;
	EntityStore *store = &game->e_store;
	if(!game->initialized)
	{
		game->initialized = 1;
		
		Entity *py = entity_alloc(store, EntityFlags_PlayerControl);
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
		
		Entity *fox = entity_alloc(store, EntityFlags_Follow);
		fox->pos = {{70, 70}};
		fox->size = {{32, 32}};
		fox->tint = D_COLOR_WHITE;
		fox->art = ArtKind_Fox;
		fox->basis.y = 75;
		fox->layer = 1;
		fox->n = 3;
		fox->x = 3;
		fox->y = 2;
		fox->speed = 100;
		
		cam->target = py;
		cam->cam.target = WORLD_FRONT;
		cam->cam.up = WORLD_UP;
		cam->cam.zoom = 135.f;
		cam->cam.proj = CAMERA_PROJ_ORTHO;
		cam->cam.speed = 400;
		cam->cam.input_rot.x = 0;
		cam->cam.input_rot.y = 0;
		cam->cam.aspect = tab->target.u32_m[3] * 1.f / tab->target.u32_m[4];
	}
	
	cam->cam.pos = v3f{{cam->target->pos.x, -cam->target->pos.y, -1}};
	
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
	
	for(s32 row = 0; row < 9; row++)
	{
		for(s32 col = 0; col < 16; col++)
		{
			s32 tile_id = tilemap[row][col];
			v4f color = {};
			R_Handle tex = {};
			Rect src = rect(0, 0, 1, 1);
			s32 layer = 0;
			
			if(tile_id == 0)
			{
				color = D_COLOR_GREEN;
				tex = a_get_alpha_bg_tex();
			}
			else
			{
				color = D_COLOR_WHITE;
				tex = a_handleFromPath(str8_lit("tree/trees.png"));
				src = {{{0.333, 0}} , {{0.666, 1}}};
				layer = 1;
			}
			
			// 82, 140 for the tree
			f32 size = 75;
			f32 padding = 0;
			
			Rect dst = {};
			dst.tl.x = col * (size + padding);
			dst.tl.y = row * (size + padding);
			dst.br.x = dst.tl.x + size;
			dst.br.y = dst.tl.y + size;
			
			R_Sprite *sprite = d_sprite(dst, color);
			sprite->layer = layer;
			sprite->basis.y = 25;
			sprite->radius = size / 2;
			sprite->tex = tex;
			sprite->src = src;
		}
	}
	
	for(s32 i = 0; i < store->num_entities; i++)
	{
		Entity *e = store->entities + i;
		
		if(e->flags & EntityFlags_PlayerControl)
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
		
		R_Sprite *sprite = d_sprite(rect(e->old_pos, e->size), e->tint);
		
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
		
		e->old_pos = e->pos;
	}
	
	d_pop_proj_view();
}