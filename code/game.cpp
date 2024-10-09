/* date = September 27th 2024 4:22 pm */

// TODO(mizu): Clean up this file
// Have a temp arena to save start and end checkpoints.

typedef u32 EntityFlags;

enum
{
	EntityFlags_Control = 1 << 0,
	EntityFlags_Follow = 1 << 1,
	EntityFlags_Friendly = 1 << 2,
	EntityFlags_Enemy = 1 << 3,
	EntityFlags_Dead = 1 << 4,
	EntityFlags_Dynamic = 1 << 5,
	EntityFlags_Static = 1 << 6,
	
};

#define EntityFlags_Physics (EntityFlags_Static | EntityFlags_Dynamic)

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
	v2f mv;
	
	f32 speed;
	f32 health;
	f32 max_health;
	f32 damage;
	
	v4f tint;
	
	Art art;
	s32 x;
	s32 y;
	s32 n;
	v2f basis;
	
	b2BodyId body;
	b2Polygon box;
	b2Capsule caps;
	
	EntityHandle target;
	Entity *next;
};

function Entity *entityFromHandle(EntityHandle handle)
{
	Entity *out = 0;
	
	if(handle.v && handle.gen == handle.v->gen)
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

#define WORLD_UP (v3f){{0,1,0}}
#define WORLD_FRONT (v3f){{0,0,-1}}

struct Camera
{
	v3f pos;
	v3f target;
	v3f up;
	f32 pitch;
	f32 yaw;
	f32 zoom;
	v3f mv;
	v3f input_rot;
	f32 speed;
	f32 aspect;
	
	EntityHandle follow;
};

function m4f_ortho_proj cam_get_proj_inv(Camera *cam)
{
	f32 z = cam->zoom;
	f32 za = z * cam->aspect;
	
	m4f_ortho_proj out = m4f_ortho(-za, za, -z, z, 0.001, 1000);
	return out;
}

function m4f cam_get_proj(Camera *cam)
{
	f32 z = cam->zoom;
	f32 za = z * cam->aspect;
	
	m4f out = m4f_ortho(-za, za, -z, z, 0.001, 1000).fwd;
	return out;
}

function m4f cam_get_view(Camera *cam)
{
	return m4f_look_at(cam->pos, cam->pos + cam->target, cam->up);
}

function void cam_update(Camera *cam, f32 delta)
{
	cam->pos += cam->mv * cam->speed * delta;
}

struct AS_Node
{
	s32 g;
	s32 h;
	b32 unwalkable;
	
	v2s index;
	
	AS_Node *next;
	AS_Node *prev;
	AS_Node *parent;
};

struct AS_NodeList
{
	AS_Node *first;
	AS_Node *last;
	
	u64 count;
};

function AS_Node *as_pushNode(Arena *arena, AS_NodeList *list, AS_Node node)
{
	AS_Node *out = push_struct(arena, AS_Node);
	*out = node;
	out->next = 0;
	out->prev = 0;
	
	if (!list->first)
	{
		list->first = list->last = out;
	}
	else
	{
		out->prev = list->last;
		list->last->next = out;
		list->last = out;
	}
	
	list->count++;
	
	return out;
}

#if 0
// NOTE(mizu): Asserts are disabled when shipping. If the assert fails, it means there is
// a bug in the pathfinding code. There is no reason to pop a node from an empty linked list
function void as_popNode(AS_NodeList *list)
{
	Assert(list->count > 0);
	AS_Node *node = list->last;
	
	list->last = node->prev;
	
	if (list->last)
	{
		list->last->next = 0;
	}
	else
	{
		list->first = 0;
	}
	
	list->count--;
	
	//free(node);
}
#endif

function void as_removeNode(AS_NodeList *list, AS_Node *node)
{
	Assert(list->count > 0);
	
	if (node->prev)
	{
		node->prev->next = node->next;
	}
	else
	{
		list->first = node->next;
	}
	
	if (node->next)
	{
		node->next->prev = node->prev;
	}
	else
	{
		list->last = node->prev;
	}
	
	list->count--;
	
	node->next = node->prev = 0;
	
	//free(node);
}

function b32 as_containsNode(AS_NodeList *list, AS_Node *node)
{
	b32 out = 0;
	for (AS_Node *iter = list->first; iter; iter = iter->next)
	{
		if (iter->index == node->index)
		{
			out = 1;
			break;
		}
	}
	return out;
}

function s32 as_fcost(AS_Node node)
{
	return node.g + node.h;
}

struct AS_Grid
{
	AS_Node *nodes;
	s32 col;
	s32 row;
	v2f size;
};

// TODO(mizu): have a grid size instead of doing / 64 and * 64 like a psychopath.
// Same for the tilemap

function v2s as_nodePosFromWorldPos(AS_Grid *grid, v2f pos)
{
	v2s out = {};
	
	out.x = pos.x / grid->size.x;
	out.y = pos.y / grid->size.y;
	
	return out;
}

function v2f as_worldPosFromNodePos(AS_Grid *grid, v2s pos)
{
	v2f out = {{pos.x * grid->size.x, pos.y * grid->size.y}};
	
	return out;
}

function AS_Node as_nodeFromWorldPos(AS_Grid *grid, v2f pos)
{
	v2s out = {};
	
	out.x = pos.x / grid->size.x;
	out.y = pos.y / grid->size.y;
	
	AS_Node node = grid->nodes[out.y * grid->col + out.x];
	
	return node;
}

function v2f as_worldPosFromNode(AS_Grid *grid, AS_Node node)
{
	v2f out = {{node.index.x * grid->size.x, node.index.y * grid->size.y}};
	
	return out;
}

function s32 as_nodeDistance(AS_Node a, AS_Node b)
{
	s32 out = 0;
	
	s32 x = abs(a.index.x - b.index.x);
	s32 y = abs(a.index.y - b.index.y);
	
	if (x > y)
	{
		out = 14 * y + 10 * (x - y);
	}
	else
	{
		out = 14 * x + 10 * (y - x);
	}
	return out;
}

v2f normalize(v2f v)
{
	float length = sqrt(v.x * v.x + v.y * v.y);
	return (length != 0) ? v / length : v2f{{0, 0}};
}


function AS_NodeList as_findPath(Arena *arena, AS_Grid *grid, v2f start, v2f end)
{
	AS_NodeList open = {};
	AS_NodeList close = {};
	
	AS_Node *first_node = as_pushNode(arena, &open, as_nodeFromWorldPos(grid, start));
	
	AS_Node *end_node = push_struct(arena, AS_Node);
	*end_node = as_nodeFromWorldPos(grid, end);
	
	while (open.count > 0)
	{
		// cur is node in open w/ the lowest f cost
		AS_Node *cur = open.first;
		
		for (AS_Node *iter = cur->next; iter; iter = iter->next)
		{
			b32 check = as_fcost(*iter) < as_fcost(*cur);
			check = check || (as_fcost(*iter) == as_fcost(*cur) && (iter->h < cur->h));
			
			if (check)
			{
				cur = iter;
			}
		}
		
		as_removeNode(&open, cur);
		cur = as_pushNode(arena, &close, *cur);
		
		if (cur->index == end_node->index)
		{
			AS_NodeList rev = {};
			// yay
			//printf("e");
			
			while(!(cur->index == first_node->index))
			{
				//printf("[%d %d] \n", cur->index.x, cur->index.y);
				cur = as_pushNode(arena, &rev, *cur);
				cur = cur->parent;
			}
			
			AS_NodeList out = {};
			
			for (AS_Node *iter = rev.last; iter; iter = iter->prev) 
			{
				as_pushNode(arena, &out, *iter);
			}
			
			return out;
		}
		
		// get neighbours
		AS_NodeList neighbours = {};
		
		for (s32 i = -1; i <= 1; i++)
		{
			for (s32 j = -1; j <= 1; j++)
			{
				if (i == 0 && j == 0)
				{
					continue;
				}
				
				v2s neighbourIndex = cur->index + v2s{{j, i}};
				
				if (neighbourIndex.x >= 0 && neighbourIndex.x < grid->col && neighbourIndex.y >= 0 && neighbourIndex.y < grid->row)
				{
					AS_Node node = grid->nodes[neighbourIndex.y * grid->col + neighbourIndex.x];
					as_pushNode(arena, &neighbours, node);
				}
			}
		}
		
		for (AS_Node *iter = neighbours.first; iter; iter = iter->next)
		{
			// NOTE(mizu): profile if hashset is better for contains node. Memory is always contiguous
			// so i can't imagine its slow to be a problem, but something to consider.
			if (iter->unwalkable || as_containsNode(&close, iter))
			{
				continue;
			}
			
			s32 new_cost = cur->g + as_nodeDistance(*cur, *iter);
			
			if (new_cost < iter->g || !as_containsNode(&open, iter))
			{
				iter->g = new_cost;
				iter->h = as_nodeDistance(*iter, *end_node);
				iter->parent = cur;
				
				if (!as_containsNode(&open, iter))
				{
					as_pushNode(arena, &open, *iter);
				}
			}
		}
	}
	
	return {};
}

struct Game
{
	Arena *arena;
	EntityStore e_store;
	Camera cam;
	b32 initialized;
	
	s32 *tilemap;
	s32 col;
	s32 row;
	
	AS_Grid as_grid;
	
	b2WorldId world;
	
	b32 start;
	b32 paused;
	
	b32 draw_spiral;
	b32 draw_health;
	b32 draw_collision;
	b32 draw_pathfinding;
};

struct Lister
{
	b32 initialized;
	Game *game;
};

function ED_CUSTOM_TAB(lister_panel)
{
	Lister *lister = (Lister*)(user_data); 
	
	Game *game = lister->game;
	EntityStore *store = &game->e_store;
	
	if(!lister->initialized)
	{
		lister->initialized = 1;
	}
	
	for(s32 i = 0; i < store->num_entities; i++)
	{
		Entity *entity = store->entities + i;
		
		ui_text_color(window->cxt, D_COLOR_WHITE)
			//ui_size_kind(window->cxt, UI_SizeKind_TextContent)
			ui_size_kind(window->cxt, UI_SizeKind_Pixels)
			ui_pref_height(window->cxt, 45)
			ui_pref_width(window->cxt, 256)
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
			
			ui_press_color(window->cxt, D_COLOR_RED)
				ui_size_kind(window->cxt, UI_SizeKind_ChildrenSum)
				ui_col(window->cxt)
				ui_size_kind(window->cxt, UI_SizeKind_TextContent)
				//ui_size_kind_y(window->cxt, UI_SizeKind_Pixels)
				//ui_size_kind(window->cxt, UI_SizeKind_Pixels)
				//ui_pref_height(window->cxt, 45)
				//ui_pref_width(window->cxt, 256)
			{
				ui_labelf(window->cxt, "position: [%.f, %.f] #%d", entity->pos.x, entity->pos.y, i);
				ui_labelf(window->cxt, "layer: %d #%d", entity->layer, i);
				ui_labelf(window->cxt, "speed: %.f #%d", entity->speed, i);
				ui_labelf(window->cxt, "health: %.f #%d", entity->health, i);
				ui_labelf(window->cxt, "art: %.*s #%d", str8_varg(art_paths[entity->art]), i);
			}
		}
	}
	
	ui_size_kind(window->cxt, UI_SizeKind_TextContent)
	{
		game->draw_health = ui_labelf(window->cxt, "draw health").toggle;
		game->draw_spiral = ui_labelf(window->cxt, "draw spiral").toggle;
		game->draw_collision = ui_labelf(window->cxt, "draw collison").toggle;
		game->draw_pathfinding = ui_labelf(window->cxt, "draw pathfinding").toggle;
	}
}

function ED_CUSTOM_TAB(profiler_panel)
{
	d_push_target(tab->target);
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
		
		for(u32 i = 0; i < 2; i++)
		{
			ED_Window *w = ed_state->windows + i;
			ui_labelf(window->cxt, "[%.f %.f]", w->pos.x, w->pos.y);
		}
	}

	R_Handle face = a_handleFromPath(str8_lit("debug/toppema.png"));

	ui_size_kind(window->cxt, UI_SizeKind_Pixels)
		ui_pref_size(window->cxt, 100)
	{
		ui_image(window->cxt, face, rect(0,0,1,1), D_COLOR_WHITE, str8_lit("debug/toppema.png"));
	}
	d_pop_target();

	v2s size = r_texSizeFromHandle(tab->target);
	ui_pref_width(window->cxt, size.x)
	ui_pref_height(window->cxt, size.y)
	ui_size_kind(window->cxt, UI_SizeKind_Pixels)
	{
		ui_imagef(window->cxt, tab->target, rect(0,0,1,1), D_COLOR_WHITE, "game image");
	}
}

function ED_CUSTOM_TAB(game_update_and_render)
{
	Game *game = (Game*)(user_data); 
	
	Camera *cam = &game->cam;
	EntityStore *store = &game->e_store;
	
	if(!game->initialized)
	{
		game->initialized = 1;
		game->arena = arenaAlloc();
		ED_Window *test = ed_openWindow(ED_WindowFlags_HasSurface, v2f{{1230, 50}}, v2f{{400,800}});
		
		ED_Panel *panel2 = ed_openPanel(test, Axis2_X, 1);
		{
			ED_Tab *debug_tab = ed_openTab(panel2, "Profiler");
			debug_tab->custom_draw = profiler_panel;
		}
		// lister tab
		{
			ED_Tab *lister_tab = ed_openTab(panel2, "Lister", {{400, 800}});
			lister_tab->custom_draw = lister_panel;
			Lister *lister = push_struct(game->arena, Lister);
			lister->game = game;
			lister_tab->custom_drawData = lister;
		}
	}
	
	b32 restart = 0;
	
	ui_hover_color(window->cxt, (v4f{{0.8, 0.8, 0.8, 1}}))
		ui_size_kind_x(window->cxt, UI_SizeKind_PercentOfParent)
		ui_size_kind_y(window->cxt, UI_SizeKind_ChildrenSum)
		ui_named_rowf(window->cxt, "boba tea")
	{
		ui_align_kind_x(window->cxt, UI_AlignKind_Center)
			ui_pref_size(window->cxt, 60)
			ui_size_kind(window->cxt, UI_SizeKind_Pixels)
		{
			R_Handle pause_play = a_handleFromPath(str8_lit("editor/pause_play.png"));
			restart = ui_imagef(window->cxt, pause_play, rect(0, 0, 0.5, 1), ED_THEME_TEXT, "restart").active;
			
			game->paused = ui_imagef(window->cxt, pause_play, rect(0.5, 0, 1, 1), ED_THEME_TEXT, "paused").toggle;
		}
	}
	
	if(restart)
	{
		game->start = 0;
	}
	
	if(game->paused)
	{
		goto end_of_sim;
	}
	
	if(!game->start)
	{
		b2WorldDef worldDef = b2DefaultWorldDef();
		worldDef.gravity = {};
		
		game->world = b2CreateWorld(&worldDef);
		
		game->e_store.num_entities = 0;
		
		game->start = 1;
		
		Entity *py = entity_alloc(store, EntityFlags_Control | EntityFlags_Dynamic);
		py->pos = {{614, 408}};
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
		{
			b2BodyDef bodyDef = b2DefaultBodyDef();
			bodyDef.type = b2_dynamicBody;
			bodyDef.position = (b2Vec2){py->pos.x, py->pos.y};
			py->body = b2CreateBody(game->world, &bodyDef);
			
			// TODO(mizu): Temp! added small padding for now so i can slip through nooks. 
			py->box = b2MakeBox(py->size.x / 4, py->size.y / 4);
			
			b2ShapeDef shapeDef = b2DefaultShapeDef();
			shapeDef.density = 0.3f;
			//shapeDef.friction = 0.3f;
			
			b2CreatePolygonShape(py->body, &shapeDef, &py->box);
		}
		
		Entity *fox = entity_alloc(store, EntityFlags_Dynamic | EntityFlags_Follow);
		fox->pos = {{270, 150}};
		fox->size = {{32, 32}};
		fox->tint = D_COLOR_WHITE;
		fox->art = ArtKind_Fox;
		fox->basis.y = 65;
		fox->layer = 1;
		fox->n = 3;
		fox->x = 3;
		fox->y = 2;
		fox->speed = 180;
		fox->target = handleFromEntity(py);
		fox->health = 300;
		fox->name = str8_lit("fox");
		fox->damage = 0;
		
		{
			b2BodyDef bodyDef = b2DefaultBodyDef();
			bodyDef.type = b2_kinematicBody;
			bodyDef.position = (b2Vec2){fox->pos.x, fox->pos.y};
			fox->body = b2CreateBody(game->world, &bodyDef);
			b2MassData mass = {};
			mass.mass = 1000;
			b2Body_SetMassData(fox->body, mass);
			fox->caps = b2Capsule{{-10, 10}, {10}};
			b2ShapeDef shapeDef = b2DefaultShapeDef();
			shapeDef.density = 1.f;
			//shapeDef.friction = 0.3f;
			b2CreateCapsuleShape(fox->body,&shapeDef, &fox->caps);
			//b2CreatePolygonShape(fox->body, &shapeDef, &fox->box);
		}
		
		cam->follow = handleFromEntity(py);
		cam->target = WORLD_FRONT;
		cam->up = WORLD_UP;
		cam->zoom = 135.f * 2;
		cam->speed = 400;
		cam->input_rot.x = 0;
		cam->input_rot.y = 0;
		cam->aspect = tab->target.u32_m[3] * 1.f / tab->target.u32_m[4];
		cam->pos.x = py->pos.x;
		cam->pos.y = -py->pos.y;
		cam->pos.z = -1;
		
#if 0
		Entity *tree = entity_alloc(store, EntityFlags_Static);
		tree->pos = {{877, 414}};
		tree->size = {{128, 128}};
		tree->tint = D_COLOR_WHITE;
		tree->art = ArtKind_Trees;
		tree->n = 0;
		tree->x = 3;
		tree->y = 1;
		tree->layer = 1;
		tree->basis.y = -25;
		tree->health = 100;
		tree->name = str8_lit("Treehouse");
		
		b2BodyDef groundBodyDef = b2DefaultBodyDef();
		groundBodyDef.position = (b2Vec2){tree->pos.x, tree->pos.y};
		tree->body = b2CreateBody(game->world, &groundBodyDef);
		
		tree->box = b2MakeBox(tree->size.x / 2.f, tree->size.y / 2.f);
		
		b2ShapeDef groundShapeDef = b2DefaultShapeDef();
		b2CreatePolygonShape(tree->body, &groundShapeDef, &tree->box);
#endif
		
		game->row = 9;
		game->col = 16;
		game->tilemap = push_array(game->arena, s32, game->row * game->col);
		
		s32 tilemap_data[] = {
			1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
			1, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 0,  0, 0, 0, 1,
			1, 0, 0, 0,  0, 0, 0, 0,  1, 1, 1, 1,  1, 1, 0, 1,
			1, 0, 0, 1,  1, 0, 1, 1,  1, 0, 0, 0,  0, 0, 0, 1,
			1, 1, 0, 0,  1, 0, 1, 0,  0, 0, 1, 0,  1, 1, 0, 1,
			1, 0, 0, 1,  0, 1, 1, 0,  1, 1, 1, 0,  0, 0, 0, 1,
			1, 0, 0, 1,  0, 1, 1, 0,  1, 0, 1, 0,  0, 0, 0, 1,
			1, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,
			1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1
		};
		
		for(s32 i = 0; i < game->row * game->col; i++)
		{
			game->tilemap[i] = tilemap_data[i];
		}
		
		// generate collisions for tilemap
		{
			b2BodyDef groundBodyDef = b2DefaultBodyDef();
			b2ShapeDef groundShapeDef = b2DefaultShapeDef();
			
			for(s32 row = 0; row < game->row; row++)
			{
				for(s32 col = 0; col < game->col; col++)
				{
					s32 tile_id = game->tilemap[row * game->col + col];
					
					if(tile_id == 1)
					{
						v2f pos = {{col * 64.f, row * 64.f}};
						
						groundBodyDef.position = (b2Vec2){pos.x + 32, pos.y + 32};
						b2BodyId body = b2CreateBody(game->world, &groundBodyDef);
						
						b2Polygon box = b2MakeBox(32.f, 32.f);
						
						b2CreatePolygonShape(body, &groundShapeDef, &box);
					}
				}
			}
			
		}
		
		game->as_grid.row = 9 * 4;
		game->as_grid.col = 16 * 4;
		game->as_grid.nodes = push_array(game->arena, AS_Node, game->as_grid.row * game->as_grid.col);
		game->as_grid.size = {{16, 16}};
		
		AS_Grid *grid = &game->as_grid;
		
		for(s32 row = 0; row < grid->row; row++)
		{
			for(s32 col = 0; col < grid->col; col++)
			{
				if(game->tilemap[(row / 4) * (grid->col / 4) + (col / 4)] == 1)
				{
					grid->nodes[row * grid->col + col].unwalkable = 1;
				}
				grid->nodes[row * grid->col + col].index = {{col, row}};
			}
		}
	}
	
	// update camera
	{
		Entity *target = entityFromHandle(cam->follow);
		if(target)
		{
			cam->pos = v3f{{target->pos.x, -target->pos.y, -1}};// + v3f{{target->size.x / 2, - target->size.y / 2, 0}};
		}
	}
	
	d_push_target(tab->target);
	
	{
		m4f_ortho_proj world_proj_inv = cam_get_proj_inv(cam);
		
		m4f world_proj = world_proj_inv.fwd;
		
		m4f world_view = cam_get_view(cam);
		
		m4f world_proj_view = world_proj * world_view * m4f_make_scale({{1, -1, 1}});
		d_push_proj_view(world_proj_view);
		
		// render tilemap
		for(s32 row = 0; row < game->row; row++)
		{
			for(s32 col = 0; col < game->col; col++)
			{
				s32 tile_id = game->tilemap[row * game->col + col];
				
				if(tile_id == 0 )
				{
					read_only v4f colors[] = 
					{
						D_COLOR_WHITE,
						D_COLOR_GREEN,
						D_COLOR_BLUE,
						D_COLOR_CYAN,
						D_COLOR_MAGENTA,
						D_COLOR_YELLOW
					};
					
					v4f color = colors[(row + col) % 6];
					
					Rect dst = {};
					dst.tl.x = col * 64;
					dst.tl.y = row * 64;
					dst.br.x = dst.tl.x + 64;
					dst.br.y = dst.tl.y + 64;
					
					R_Sprite *sprite = d_sprite(dst, color);
					sprite->layer = 0;
					sprite->tex = a_getAlphaBGTex();
				}
			}
		}
		
		// render a* grid
		if(1)
		{
			AS_Grid *grid = &game->as_grid;
			for(s32 row = 0; row < grid->row; row++)
			{
				for(s32 col = 0; col < grid->col; col++)
				{
					v2s index = {{col, row}};
					AS_Node node = grid->nodes[row * grid->col + col];
					
					v4f color = {};
					
					if(node.unwalkable == 1)
					{
						color = D_COLOR_CYAN;
					}
					
					v2f pos = as_worldPosFromNodePos(grid, index);
					
					R_Sprite *grid = d_sprite(rect(pos, {{game->as_grid.size.x, game->as_grid.size.x}}), color);
					grid->border_color = D_COLOR_BLACK;
					grid->border_thickness = 2;
					grid->layer = 2;
				}
			}
		}
		
		// Kill entities with health < 0 
		for(s32 i = 0; i < store->num_entities; i++)
		{
			Entity *e = store->entities + i;
			if(e->health < 0)
			{
				e->flags |= EntityFlags_Dead;
			}
		}
		
		// player control
		for(s32 i = 0; i < store->num_entities; i++)
		{
			Entity *e = store->entities + i;
			
			if((e->flags & EntityFlags_Control) && !(e->flags & EntityFlags_Dead))
			{
				v2f dir = {};
				
				if(os_keyPress(window->win, SDLK_A))
				{
					dir.x = -1;
				}
				
				if(os_keyPress(window->win, SDLK_D))
				{
					dir.x = 1;
				}
				
				if(os_keyPress(window->win, SDLK_S))
				{
					dir.y = 1;
				}
				
				if(os_keyPress(window->win, SDLK_W))
				{
					dir.y = -1;
				}
				
				e->mv = dir;
			}
		}
		
		// pathfind
		for(s32 i = 0; i < store->num_entities; i++)
		{
			Entity *e = store->entities + i;
			
			if(e->flags & EntityFlags_Follow)
			{
				ArenaTemp temp = arenaTempBegin(game->arena);
				Entity *target = entityFromHandle(e->target);
				
				if(target)
				{
					v2f pos = e->pos;
					v2f dir2 = {{pos.x - target->pos.x, pos.y - target->pos.y}};
					float length2 = sqrt(dir2.x * dir2.x + dir2.y * dir2.y);
					
					if (length2 > 50)
					{
						AS_NodeList list = as_findPath(game->arena, &game->as_grid, e->pos, target->pos);
						
						if(list.first)
						{
							v2f next_pos = as_worldPosFromNode(&game->as_grid, *list.first) + v2f{{8, 8}};
							
							v2f dir = {{ next_pos.x - pos.x, next_pos.y - pos.y }};
							float length = sqrt(dir.x * dir.x + dir.y * dir.y);
							
							
							if (length > 0) 
							{
								dir.x /= length;
								dir.y /= length;
								e->mv = dir;
							}
							
						}
						
						if(game->draw_pathfinding)
						{
							v2s poss = {{(s32)(e->pos.x / game->as_grid.size.x * 1.f), (s32)(e->pos.y / game->as_grid.size.x * 1.f)}};
							v2f pos = {{poss.x * game->as_grid.size.x * 1.f, poss.y * game->as_grid.size.x * 1.f}};
							R_Sprite *sprite = d_sprite(rect(pos, {{game->as_grid.size.x, game->as_grid.size.x}}), D_COLOR_BLACK);
							sprite->layer = 1;
							for(AS_Node *iter = list.first; iter; iter = iter->next)
							{
								v2f pos = as_worldPosFromNode(&game->as_grid, *iter);
								
								v4f color = D_COLOR_RED;
								if(iter == list.last)
								{
									color = D_COLOR_YELLOW;
								}
								
								R_Sprite *sprite = d_sprite(rect(pos, {{game->as_grid.size.x, game->as_grid.size.x}}), color);
								sprite->layer = 1;
							}
						}
						
						arenaTempEnd(&temp);
					}
					else
					{
						e->mv = {};
					}
				}
			}
		}
		
		// update positions
		for(s32 i = 0; i < store->num_entities; i++)
		{
			Entity *e = store->entities + i;
			
			e->old_pos = e->pos;
			//e->pos += e->mv * delta * e->speed;
		}
		
		// physics
		float timeStep = 1.0f / 60.0f;
		int subStepCount = 4;
		b2World_Step(game->world, timeStep, subStepCount);
		
		for(s32 i = 0; i < store->num_entities; i++)
		{
			Entity *e = store->entities + i;
			
			if(e->flags & EntityFlags_Dynamic)
			{
				v2f vel = e->mv * delta * e->speed * 100;
				
				b2Body_SetLinearVelocity(e->body, {vel.x, vel.y});
				
				b2Vec2 position = b2Body_GetPosition(e->body);
				//b2Rot rotation = b2Body_GetRotation(e->body);
				e->pos.x = position.x;
				e->pos.y = position.y;
			}
			
		}
		
		// draw visualizers
		for(s32 i = 0; i < store->num_entities; i++)
		{
			Entity *e = store->entities + i;
			
			// collider
			
			if(game->draw_collision)
			{
				if(e->flags & EntityFlags_Physics)
				{
					b2Vec2 pos = b2Body_GetPosition(e->body);
					v2f posf = {{pos.x, pos.y}};
					
					f32 rad = 10 + 10 + 10;
					R_Sprite *sprite = d_spriteCenter(posf, {{rad, 10}}, {{1, 0, 0, 1}});
					sprite->layer = 1;
					
				}
			}
			
			// art
			{
				R_Sprite *sprite = d_spriteCenter(e->old_pos, e->size, e->tint);
				
				if(e->art == ArtKind_Null)
				{
					sprite->tex = a_getCheckerTex();
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
			
			// health bar
			if(game->draw_health)
			{
				v2f pos = e->old_pos;
				pos.y += e->size.y / 2;
				
				R_Sprite *health = d_spriteCenter(pos, {{e->health  * 0.1f, 5}}, D_COLOR_RED);
				health->layer = 2;
			}
			
			if(game->draw_spiral)
			{
				v2f pos = e->old_pos;
				v2f size = {{0.5, 0.5f}};
				f32 thickness = 10;
				f32 rot_speed = 0.01f;
				
				static f32 timer = 0;
				timer += delta;
				
				for(s32 j = 0; j < 100; j++)
				{
					f32 angle = rot_speed * timer * j;
					v2f rel_pos = pos - e->old_pos;
					f32 cos_t = cos(angle);
					f32 sin_t = sin(angle);
					
					v2f rot_pos = { };
					rot_pos.x = rel_pos.x * cos_t - rel_pos.y * sin_t;
					rot_pos.y = rel_pos.x * sin_t + rel_pos.y * cos_t;
					
					v2f final_pos = rot_pos + e->old_pos;
					R_Sprite *line = d_sprite(rect(final_pos, size * thickness), D_COLOR_WHITE);
					line->layer = 2;
					pos = pos + size * 2;
				}
			}
			
			
		}
		
		d_pop_target();
		d_pop_proj_view();
	}
	
	end_of_sim:
	
	v2s size = r_texSizeFromHandle(tab->target);
	
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
	}
}

/*

enum Cmd
{
	CmdKind_DoFoo,
	CmdKind_DoBar,
	CmdKind_COUNT
};

struct CmdFoo
{
	Cmd kind;
	s32 foo;
}

struct CmdBar
{
	Cmd kind;
	f32 bar;
}

struct CmdBuffer
{
	u8 *base;
	u64 used;
	u64 size;
	u64 num;
};

void cmdDoFoo(CmdBuffer *buffer, s32 foo)
{
	CmdFoo *data = buffer->base + buffer->used;
	data->kind = CmdKind_DoFoo;
	data->foo = foo;
	
	buffer->used += sizeof(CmdFoo);
	buffer->num += 1;
}

void cmdDoBar(CmdBuffer *buffer, f32 bar)
{
	CmdBar *data = buffer->base + buffer->used;
	data->kind = CmdKind_DoBar;
	data->bar = bar;
	
	buffer->used += sizeof(CmdBar);
	buffer->num += 1;
}

void processCmds(CmdBuffer *buffer)
{
	u8 *pos = buffer->base;
	for(u64 i = 0; i < buffer->num; i+=1)
	{
		Cmd type = *(Cmd*)pos;
		if(type == CmdKind_DoFoo)
		{
			CmdFoo *foo = (CmdFoo*)pos;
			// do foo stuff
			pos += sizeof(CmdFoo);
		}
		else if(type == CmdKind_DoBar)
		{
			CmdBar *bar = (CmdBar*)pos;
			// do bar stuff
			pos += sizeof(CmdBar);
		}
	}
}*/