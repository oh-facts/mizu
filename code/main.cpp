#define _CRT_SECURE_NO_WARNINGS
// TODO(mizu): arbitrary tile sizes
// TODO(mizu): better visualization
// TODO(mizu): astar anti fuckup
// TODO(mizu): flexible astar
// TODO(mizu): neglect ui like u neglects i. This is deeper than the economic
// and financial state of the world
// TODO(mizu): combat

#define ED_THEME_BG v4f{{0.643, 0, 0.357, 1}}
#define ED_THEME_BG_FROSTED v4f{{0.643, 0, 0.357, 0.83}}
#define ED_THEME_BG_DARKER v4f{{0, 0, 0, 0.2}}
#define ED_THEME_TITLEBAR v4f{{0.902, 0.902, 0.902, 1}}
#define ED_THEME_TEXT v4f{{1, 0.576, 0.141, 1}}
#define ED_THEME_IMG D_COLOR_WHITE

#define STB_SPRINTF_IMPLEMENTATION
#include "third_party/stb/stb_sprintf.h"

#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "third_party/stb/stb_truetype.h"

#include <stdint.h>

#include <stdio.h>

#include <third_party/SDL3/SDL.h>

#include <third_party/KHR/khrplatform.h>
#include <third_party/glad/glad.h>
#include <third_party/glad/glad.c>
#include <third_party/box2d/box2d.h>

#include <context.cpp>
#if defined(OS_WIN32)
#define WIN32_LEAN_AND_MEAN
#undef function
#include <windows.h>

#if 1
extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001; //NVIDIA
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1; // AMD
}
#endif

#define function static
#else
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/mman.h>
#endif



#include <base.cpp>

#if defined(OS_WIN32)
#include <backends/os_win32.cpp>
#else
#include <backends/os_linux.cpp>
#endif

#include <os.cpp>
#include <render.cpp>
#include <texture.cpp>

// must be a way to map these to px sizes so I don't have to do this
// good for arial
//#define FONT_SIZE 0.013986

// good for genesis
//#define FONT_SIZE 0.025

// good for delius
#define FONT_SIZE 0.029

struct Font
{
	Atlas atlas;
	R_Handle atlas_tex[256];
};

global Font *font;

function Rect rectFromString(Str8 text, f32 scale)
{
	Rect out = {};
	
	for (u32 i = 0; i < text.len; i++)
	{
		char c = text.c[i];
		
		Glyph ch = *(font->atlas.glyphs + (u32)c);
		
		if (out.tl.y < ch.y1 && c != ' ')
		{
			out.tl.y = ch.y1;
		}
		if (out.br.y > ch.y0 && c != ' ')
		{
			out.br.y = ch.y0;
		}
		
		out.br.x += ch.advance;
	}
	
	out.tl *= scale;
	out.br *= scale;
	
	return out;
}

#include <draw.cpp>
#include <ui.cpp>

#include <backends/render_opengl.cpp>

#include <editor.cpp>

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
	ArtKind_One,
	ArtKind_COUNT
};

read_only Str8 art_paths[ArtKind_COUNT] = 
{
	str8_lit(""),
	str8_lit("fox/fox.png"),
	str8_lit("impolo/impolo-east.png"),
	str8_lit("tree/trees.png"),
	str8_lit("demons/ONE.png"),
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
	AS_Node *parent;
	
	AS_Node *next;
	u64 key;
};

function s32 as_fcost(AS_Node *node)
{
	return node->g + node->h;
}

struct AS_NodeArray
{
	AS_Node *v;
	u64 count;
	u64 size;
};

struct AS_NodeSlots
{
	AS_Node *first;
	AS_Node *last;
};

struct AS_HashMap
{
	AS_NodeSlots *slots;
	u64 count;
};

function AS_HashMap as_createHashMap(Arena *arena, u64 count)
{
	AS_HashMap out = {};
	out.count = count;
	out.slots = push_array(arena, AS_NodeSlots, out.count);
	return out;
}

// stupid fucking lousy hash function. Use something better later.
// 2 signed 32 bits -> unsigned 64 bit key
function u64 as_keyFromIndex(v2s index)
{
	u64 out = {};
	out = index.x * index.y;
	return out;
}

function AS_Node *as_nodeFromKey(Arena *arena, AS_HashMap *map, AS_Node other)
{
	u64 key = as_keyFromIndex(other.index);
	u64 slot = key % map->count;
	AS_Node *node = map->slots[slot].first;
	
	while(node)
	{
		if(key == node->key && other.index == node->index)
		{
			break;
		}
		
		node = node->next;
	}
	
	if(!node)
	{
		node = push_struct(arena, AS_Node);
		*node = {};
		
		if(map->slots[slot].last)
		{
			map->slots[slot].last = map->slots[slot].last->next = node;
		}
		else
		{
			map->slots[slot].last = map->slots[slot].first = node;
		}
	}
	
	*node = other;
	node->key = key;
	
	return node;
}

function b32 as_containsNodeinHash(AS_HashMap *map, v2s index)
{
	u64 key = as_keyFromIndex(index);
	u64 slot = key % map->count;
	AS_Node *node = map->slots[slot].first;
	
	while(node)
	{
		if((key == node->key) && (index == node->index))
		{
			break;
		}
		
		node = node->next;
	}
	
	return !!node;
}

struct AS_Grid
{
	b32 *cells;
	s32 col;
	s32 row;
	v2f size;
};

function AS_Node as_nodeFromGridIndex(AS_Grid *grid, v2s index)
{
	AS_Node out = {};
	out.index.x = index.x;
	out.index.y = index.y;
	
	out.unwalkable = grid->cells[out.index.y * grid->col + out.index.x];
	
	return out;
}

function AS_Node as_nodeFromPos(AS_Grid *grid, v2f pos)
{
	AS_Node out = {};
	out.index.x = pos.x / grid->size.x;
	out.index.y = pos.y / grid->size.y;
	
	out.unwalkable = grid->cells[out.index.y * grid->col + out.index.x];
	
	return out;
}

function v2f as_worldPosFromIndex(AS_Grid *grid, v2s index)
{
	v2f out = {};
	out.x = index.x * grid->size.x;
	out.y = index.y * grid->size.y;
	
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

function AS_NodeArray as_nodeReserve(Arena *arena, u64 size)
{
	AS_NodeArray out = {};
	out.count = 0;
	out.size = size;
	out.v = push_array(arena, AS_Node, size);
	return out;
}

function v2f normalize(v2f v)
{
	float length = sqrt(v.x * v.x + v.y * v.y);
	return (length != 0) ? v / length : v2f{{0, 0}};
}

function b32 as_containsNode(AS_NodeArray *list, AS_Node node)
{
	BEGIN_TIMED_BLOCK(PF_CONTAINS_NODE);
	b32 out = 0;
	
	for(s32 i = 0; i < list->count; i++)
	{
		AS_Node *iter = list->v + i;
		if(iter->index == node.index)
		{
			out = 1;
			break;
		}
	}
	
	END_TIMED_BLOCK(PF_CONTAINS_NODE);
	return out;
}

function AS_NodeArray as_findPath(Arena *arena, AS_Grid *grid, v2f start_pos, v2f end_pos)
{
	BEGIN_TIMED_BLOCK(PATHFINDING);
	AS_NodeArray out = {};
	
	AS_Node start = as_nodeFromPos(grid, start_pos);
	AS_Node end = as_nodeFromPos(grid, end_pos);
	
	AS_NodeArray open = as_nodeReserve(arena, 1024);
	AS_HashMap closed = as_createHashMap(arena, 1024);
	
	open.v[open.count++] = start;
	
	// lowest f cost
	
	while(open.count > 0)
	{
		BEGIN_TIMED_BLOCK(PF_LOWEST_FCOST);
		// current is our persistent pointer boy. pass him around. no need to move values
		AS_Node *cur = open.v;
		
		for(s32 i = 1; i < open.count; i++)
		{
			AS_Node *iter = open.v + i;
			b32 check = as_fcost(iter) < as_fcost(cur);
			check = check || (as_fcost(iter) == as_fcost(cur) && (iter->h < cur->h));
			
			if(check)
			{
				cur = iter;
			}
		}
		END_TIMED_BLOCK(PF_LOWEST_FCOST);
		
		{
			AS_Node newnode = *cur;
			
			// remove from open
			for(s32 i = 0; i < open.count; i++)
			{
				AS_Node *iter = open.v + i;
				if(cur->index == iter->index)
				{
					open.v[i] = open.v[open.count - 1];
					open.count--;
					break;
				}
			}
			
			// add to closed
			{
				cur = as_nodeFromKey(arena, &closed, newnode);
			}
			
		}
		
		if(cur->index == end.index)
		{
			//
#if 0
			printf("==\n");
			for(AS_Node *node = cur; node; node = node->parent)
			{
				printf("%d %d\n", node->index.x, node->index.y);
			}
			printf("==\n");
#endif
			
			BEGIN_TIMED_BLOCK(PF_PREPARE_PATH);
			out = as_nodeReserve(arena, 1024);
			
			for (AS_Node *node = cur; node; node = node->parent)
			{
				out.v[out.count++] = *node;
			}
			
			// remove self
			out.count -= 1;
			END_TIMED_BLOCK(PF_PREPARE_PATH);
			
			BEGIN_TIMED_BLOCK(PF_REVERSE_PATH);
			for (u64 i = 0; i < out.count / 2; ++i)
			{
				AS_Node temp = out.v[i];
				out.v[i] = out.v[out.count - i - 1];
				out.v[out.count - i - 1] = temp;
			}
			END_TIMED_BLOCK(PF_REVERSE_PATH);
			
#if 0
			printf("==\n");
			for(s32 i = 0; i < out.count; i++)
			{
				printf("%d %d\n", out.v[i].index.x, out.v[i].index.y);
			}
			printf("==\n");
#endif
			END_TIMED_BLOCK(PATHFINDING);
			return out;
		}
		
		BEGIN_TIMED_BLOCK(PF_GET_NEIGHBORS);
		AS_NodeArray neighbours = as_nodeReserve(arena, 8);
		
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
					AS_Node node = as_nodeFromGridIndex(grid, neighbourIndex);
					neighbours.v[neighbours.count++] = node;
				}
			}
		}
		END_TIMED_BLOCK(PF_GET_NEIGHBORS);
		
		for (s32 i = 0; i < neighbours.count; i++)
		{
			AS_Node *iter = neighbours.v + i;
			
			BEGIN_TIMED_BLOCK(PF_CLOSED_CONTAINS_NODE);
			
			b32 contains_close_node = as_containsNodeinHash(&closed, iter->index);
			END_TIMED_BLOCK(PF_CLOSED_CONTAINS_NODE);
			
			if (iter->unwalkable || contains_close_node)
			{
				continue;
			}
			
			s32 new_cost = cur->g + as_nodeDistance(*cur, *iter);
			
			BEGIN_TIMED_BLOCK(PF_OPEN_CONTAINS_NODE);
			b32 contains_node = as_containsNode(&open, *iter);
			END_TIMED_BLOCK(PF_OPEN_CONTAINS_NODE);
			
			if (new_cost < iter->g || !contains_node)
			{
				iter->g = new_cost;
				iter->h = as_nodeDistance(*iter, end);
				iter->parent = cur;
				
				if (!contains_node)
				{
					Assert(open.count < open.size);
					open.v[open.count++] = *iter;
				}
				else
				{
					volatile int ee = 0;
				}
			}
		}
	}
	
	END_TIMED_BLOCK(PATHFINDING);
	return out;
}

struct Game
{
	Arena *arena;
	Arena *frame;
	
	EntityStore e_store;
	Camera cam;
	b32 initialized;
	
	ED_Tab *lister_tab;
	ED_Tab *profiler_tab;
	ED_Tab *console_tab;
	
	s32 *tilemap;
	s32 col;
	s32 row;
	
	AS_Grid as_grid;
	
	b2WorldId world;
	
	b32 start;
	b32 paused;
	b32 fullscreen;
	
	b32 draw_spiral;
	b32 draw_health;
	b32 draw_collision;
	b32 draw_pathfinding;
	f32 debug_zoom;
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
	
	b32 hide_entity_panel = 0;
	
	ui_size_kind(window->cxt, UI_SizeKind_Pixels)
		ui_pref_height(window->cxt, 16)
	{
		ui_spacer(window->cxt);
	}
	
	ui_text_color(window->cxt, D_COLOR_BLACK)
		ui_size_kind(window->cxt, UI_SizeKind_TextContent)
	{
		hide_entity_panel = ui_labelf(window->cxt, "Entity Panel").toggle;
	}
	
	if(hide_entity_panel)
	{
		ui_size_kind(window->cxt, UI_SizeKind_ChildrenSum)
			ui_col(window->cxt)
		{
			UI_Widget *col = window->cxt->parent_stack.top->v;
			col->flags |= UI_Flags_draw_border;
			col->border_color = D_COLOR_BLACK;
			col->border_thickness = 4;
			ui_size_kind(window->cxt, UI_SizeKind_Pixels)
				ui_pref_height(window->cxt, 16)
			{
				ui_spacer(window->cxt);
			}
			
			for(s32 i = 0; i < store->num_entities; i++)
			{
				Entity *entity = store->entities + i;
				
				b32 hide_entity = 0;
				
				ui_text_color(window->cxt, D_COLOR_WHITE)
					ui_size_kind(window->cxt, UI_SizeKind_TextContent)
				{
					hide_entity = ui_labelf(window->cxt, "%s", (char*)entity->name.c).toggle;
				}
				
				if(hide_entity)
				{
					ui_press_color(window->cxt, D_COLOR_WHITE)
						ui_size_kind(window->cxt, UI_SizeKind_ChildrenSum)
						ui_col(window->cxt)
						ui_size_kind(window->cxt, UI_SizeKind_TextContent)
					{
						UI_Widget *row = window->cxt->parent_stack.top->v;
						row->padding[0] = 15;
						
						ui_labelf(window->cxt, "position: [%.f, %.f] #%d", entity->pos.x, entity->pos.y, i);
						ui_labelf(window->cxt, "layer: %d #%d", entity->layer, i);
						ui_labelf(window->cxt, "speed: %.f #%d", entity->speed, i);
						ui_labelf(window->cxt, "health: %.f #%d", entity->health, i);
						ui_labelf(window->cxt, "art: %.*s #%d", str8_varg(art_paths[entity->art]), i);
						ui_size_kind(window->cxt, UI_SizeKind_Pixels)
							ui_pref_height(window->cxt, 16)
						{
							ui_spacer(window->cxt);
						}
						
					}
				}
			}
			ui_size_kind(window->cxt, UI_SizeKind_Pixels)
				ui_pref_height(window->cxt, 16)
			{
				ui_spacer(window->cxt);
			}
			
		}
	}
	
	ui_size_kind(window->cxt, UI_SizeKind_Pixels)
		ui_pref_height(window->cxt, 24)
	{
		ui_spacer(window->cxt);
	}
	
	b32 show_settings = 0;
	ui_text_color(window->cxt, D_COLOR_BLACK)
		ui_size_kind(window->cxt, UI_SizeKind_TextContent)
	{
		show_settings = ui_labelf(window->cxt, "Settings").toggle;
	}
	
	if(show_settings)
	{
		ui_size_kind(window->cxt, UI_SizeKind_ChildrenSum)
			ui_col(window->cxt)
		{
			ui_size_kind(window->cxt, UI_SizeKind_Pixels)
				ui_pref_height(window->cxt, 16)
			{
				ui_spacer(window->cxt);
			}
			
			UI_Widget *col = window->cxt->parent_stack.top->v;
			col->flags |= UI_Flags_draw_border;
			col->border_color = D_COLOR_BLACK;
			col->border_thickness = 4;
			col->padding[0] = 15;
			ui_size_kind(window->cxt, UI_SizeKind_Pixels)
				ui_pref_height(window->cxt, 45)
				ui_pref_width(window->cxt, 256)
				ui_border_thickness(window->cxt, 4)
				ui_border_color(window->cxt, ED_THEME_TEXT)
				ui_flags(window->cxt, UI_Flags_text_centered)
			{
				game->draw_health = ui_buttonf(window->cxt, "draw health").toggle;
				game->draw_spiral = ui_buttonf(window->cxt, "draw spiral").toggle;
				game->draw_collision = ui_buttonf(window->cxt, "draw collison").toggle;
				game->draw_pathfinding = ui_buttonf(window->cxt, "draw pathfinding").toggle;
				//game->debug_zoom = ui_buttonf(window->cxt, "debug zoom").toggle;
				
				//ui_buttonf(window->cxt, "debug zoom");
				
				ui_size_kind(window->cxt, UI_SizeKind_Pixels)
					ui_pref_height(window->cxt, 16)
				{
					ui_spacer(window->cxt);
				}
				
				ui_labelf(window->cxt, "debug zoom");
				
				ui_sliderf(window->cxt, &game->debug_zoom, 1, 10, ED_THEME_TEXT, "debug zoom slider");
				
			}
			ui_size_kind(window->cxt, UI_SizeKind_Pixels)
				ui_pref_height(window->cxt, 16)
			{
				ui_spacer(window->cxt);
			}
			
		}
	}
}

struct DebugLogBuffer
{
	Arena *arena;
	
	char buf[1024];
	u64 used;
};

global DebugLogBuffer debug_log_buffer;

void pls_printfv(char *fmt, va_list args)
{
	va_list args_copy;
	va_copy(args_copy, args);
	
	int bytes_req = stbsp_vsnprintf(0, 0, fmt, args) + 1;
	
	char *buf = debug_log_buffer.buf + debug_log_buffer.used;
	
	debug_log_buffer.used += stbsp_vsnprintf(buf, bytes_req, fmt, args_copy);
	va_end(args_copy);
}

void pls_print(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	pls_printfv(fmt, args);
	va_end(args);
}

function ED_CUSTOM_TAB(console_panel)
{
	ui_size_kind(window->cxt, UI_SizeKind_TextContent)
	{
		ui_labelf(window->cxt, "%s", debug_log_buffer.buf);
	}
}

struct ProfilerCycle
{
	ProfilerCycle *first;
	ProfilerCycle *last;
	
	ProfilerCycle *next;
	ProfilerCycle *prev;
	
	char *name;
	u64 cc;
	u64 hc;
	DEBUG_CYCLE_COUNTER id;
};

struct Profiler
{
	Arena *arena;
	
	f32 delta;
	f32 last_delta;
	f32 update_timer;
	
	ProfilerCycle *root;
	b32 initialized;
	
	struct
	{
		ProfilerCycle *top;
		ProfilerCycle *next;
	}parent_stack;
};

function void pushProfilerCycleParent(Profiler *profiler, ProfilerCycle *parent)
{
	profiler->parent_stack.next = profiler->parent_stack.top;
	profiler->parent_stack.top = parent;
}

function void popProfilerCycleParent(Profiler *profiler)
{
	profiler->parent_stack.top = profiler->parent_stack.next;
}

#define DeferLoop(begin, end) for(int _i_ = ((begin), 0); !_i_; _i_ += 1, (end))

#define profilerParent(profiler, parent) DeferLoop(pushProfilerCycleParent(profiler, parent), popProfilerCycleParent(profiler))

function ProfilerCycle *allocProfilerCycle(Profiler *profiler, DEBUG_CYCLE_COUNTER id)
{
	ProfilerCycle *out = push_struct(profiler->arena, ProfilerCycle);
	
	out->name = debug_cycle_to_str[id];
	out->id = id;
	out->cc = tcxt->counters_last[id].cycle_count;
	
	ProfilerCycle *root = profiler->parent_stack.top;
	
	if(!root->last)
	{
		root->last = root->first = out;
	}
	else
	{
		out->prev = root->last;
		root->last = root->last->next = out;
	}
	
	return out;
}

// TODO(mizu): Sort the times and do nesting. It's very hard to tell otherwise

function void profilerCycleChildren(UI_Context *cxt, ProfilerCycle *root, u64 level)
{
	for(s32 i = 0; i < level; i++)
	{
		//printf(" ");
	}
	//printf("%s\n", root->name);
	
	if(root->name)
	{
		ui_size_kind(cxt, UI_SizeKind_ChildrenSum)
			ui_row(cxt)
		{
			ui_size_kind(cxt, UI_SizeKind_Pixels)
				ui_pref_width(cxt, 40 * (level) - 40)
			{
				ui_spacer(cxt);
			}
			ui_size_kind(cxt, UI_SizeKind_TextContent)
			{
				ui_labelf(cxt, "%s %llu", root->name, root->cc);
			}
		}
	}
	
	for(ProfilerCycle *cur = root->first; cur; cur = cur->next)
	{
		profilerCycleChildren(cxt, cur, level + 1);
	}
};

function void profilerCycleChildrenUpdate(ProfilerCycle *root, f32 update_timer)
{
	if(update_timer > 0.4)
	{
		root->cc = tcxt->counters_last[root->id].cycle_count;
	}
	
	for(ProfilerCycle *cur = root->first; cur; cur = cur->next)
	{
		profilerCycleChildrenUpdate(cur, update_timer);
	}
};

function ED_CUSTOM_TAB(profiler_panel)
{
	Profiler *profiler = (Profiler*)user_data;
	
	if(!profiler->initialized)
	{
		profiler->initialized = 1;
		profiler->arena = arenaAlloc();
		
		profiler->root = push_struct(profiler->arena, ProfilerCycle);
		
		profilerParent(profiler, profiler->root)
		{
			allocProfilerCycle(profiler, DEBUG_CYCLE_COUNTER_UPDATE_AND_RENDER);
			ProfilerCycle *pf = allocProfilerCycle(profiler, DEBUG_CYCLE_COUNTER_PATHFINDING);
			
			profilerParent(profiler, pf)
			{
				allocProfilerCycle(profiler, DEBUG_CYCLE_COUNTER_PF_GET_NEIGHBORS);
				ProfilerCycle *cn = allocProfilerCycle(profiler, DEBUG_CYCLE_COUNTER_PF_CONTAINS_NODE);
				
				profilerParent(profiler, cn)
				{
					allocProfilerCycle(profiler, DEBUG_CYCLE_COUNTER_PF_OPEN_CONTAINS_NODE);
					allocProfilerCycle(profiler, DEBUG_CYCLE_COUNTER_PF_CLOSED_CONTAINS_NODE);
				}
				
				allocProfilerCycle(profiler, DEBUG_CYCLE_COUNTER_PF_LOWEST_FCOST);
				allocProfilerCycle(profiler, DEBUG_CYCLE_COUNTER_PF_PREPARE_PATH);
				allocProfilerCycle(profiler, DEBUG_CYCLE_COUNTER_PF_REVERSE_PATH);
			}
		}
	}
	
	profiler->update_timer += delta;
	
	if(profiler->update_timer > 0.4)
	{
		profiler->delta = delta;
	}
	
	profilerCycleChildrenUpdate(profiler->root, profiler->update_timer);
	
	if(profiler->update_timer > 0.4)
	{
		profiler->update_timer = 0;
	}
	
	ui_size_kind(window->cxt, UI_SizeKind_TextContent)
	{
		profilerCycleChildren(window->cxt, profiler->root, 0);
		
		ui_labelf(window->cxt, "ft : %.fms", profiler->delta * 1000);
		ui_labelf(window->cxt, "cmt: %.1f MB", total_cmt * 0.000001f);
		ui_labelf(window->cxt, "res: %.1f GB", total_res * 0.000000001f);
		ui_labelf(window->cxt, "textures: %.1f MB", a_state->tex_mem * 0.000001);
	}
	printf("\n\n");
	TEX_Handle key = a_keyFromPath(str8_lit("debug/toppema.png"), font_params);
	R_Handle face = a_handleFromKey(key);
	
	ui_size_kind(window->cxt, UI_SizeKind_Pixels)
		ui_pref_size(window->cxt, 100)
	{
		ui_image(window->cxt, face, rect(0,0,1,1), D_COLOR_WHITE, str8_lit("debug/toppema.png"));
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
		game->frame = arenaAlloc();
		
		pls_print("hello");
		pls_print("\n");
		pls_print("hi");
		pls_print("hello my friend");
		pls_print("\n\n\n");
		pls_print("hi");
		
		// profiler tab
		{
			game->profiler_tab = ed_openFloatingTab(window->first_panel, "Profiler", {{1513, 0}}, {{400, 600}});
			game->profiler_tab->custom_draw = profiler_panel;
			
			Profiler *profiler = push_struct(game->arena, Profiler);
			profiler->arena = game->frame;
			game->profiler_tab->custom_drawData = profiler;
		}
		
		// lister tab
		{
			game->lister_tab = ed_openFloatingTab(window->first_panel, "Lister", {{1076, 0}}, {{400, 600}});
			game->lister_tab->custom_draw = lister_panel;
			Lister *lister = push_struct(game->arena, Lister);
			lister->game = game;
			game->lister_tab->custom_drawData = lister;
		}
		
		// lister tab
		{
			game->console_tab = ed_openFloatingTab(window->first_panel, "Console", {{1513, 800}}, {{400, 600}});
			game->console_tab->custom_draw = console_panel;
		}
		
		game->debug_zoom = 1;
	}
	
	// pause play ui
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
			TEX_Handle key = a_keyFromPath(str8_lit("editor/pause_play.png"), font_params);
			R_Handle pause_play = a_handleFromKey(key);
			restart = ui_imagef(window->cxt, pause_play, rect(0, 0, 0.33, 1), ED_THEME_TEXT, "restart").active;
			
			game->paused = ui_imagef(window->cxt, pause_play, rect(0.33, 0, 0.66, 1), ED_THEME_TEXT, "paused").toggle;
			
			game->fullscreen = ui_imagef(window->cxt, pause_play, rect(0.66, 0, 1, 1), ED_THEME_TEXT, "fullscreen").toggle;
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
		Entity *fox = 0;
		{
			fox = entity_alloc(store, EntityFlags_Dynamic | EntityFlags_Follow);
			fox->pos = {{270, 150}};
			fox->size = {{12, 26}};
			fox->tint = D_COLOR_WHITE;
			fox->art = ArtKind_Fox;
			fox->basis.y = 65;
			fox->layer = 1;
			fox->n = 0;
			fox->x = 1;
			fox->y = 1;
			fox->speed = 180;
			fox->target = handleFromEntity(py);
			fox->health = 300;
			fox->name = str8_lit("fox");
			fox->damage = 0;
			
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
		
		// enemy
#if 1
		{
			Entity *enemy = entity_alloc(store, EntityFlags_Dynamic | EntityFlags_Follow);
			enemy->pos = {{500, 150}};
			enemy->size = {{64, 64}};
			enemy->tint = D_COLOR_WHITE;
			enemy->art = ArtKind_One;
			enemy->basis.y = 65;
			enemy->layer = 1;
			enemy->n = 0;
			enemy->x = 1;
			enemy->y = 1;
			enemy->speed = 100;
			enemy->target = handleFromEntity(fox);
			enemy->health = 300;
			enemy->name = str8_lit("enemy");
			enemy->damage = 0;
			
			b2BodyDef bodyDef = b2DefaultBodyDef();
			bodyDef.type = b2_kinematicBody;
			bodyDef.position = (b2Vec2){enemy->pos.x, enemy->pos.y};
			enemy->body = b2CreateBody(game->world, &bodyDef);
			b2MassData mass = {};
			mass.mass = 1000;
			b2Body_SetMassData(enemy->body, mass);
			enemy->caps = b2Capsule{{-10, 10}, {10}};
			b2ShapeDef shapeDef = b2DefaultShapeDef();
			shapeDef.density = 1.f;
			//shapeDef.friction = 0.3f;
			b2CreateCapsuleShape(enemy->body,&shapeDef, &enemy->caps);
			//b2CreatePolygonShape(enemy->body, &shapeDef, &enemy->box);
			
		}
#endif
		
		cam->follow = handleFromEntity(py);
		cam->target = WORLD_FRONT;
		cam->up = WORLD_UP;
		cam->zoom = 135.f;// * 2;
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
		game->as_grid.cells = push_array(game->arena, b32, game->as_grid.row * game->as_grid.col);
		game->as_grid.size = {{16, 16}};
		
		AS_Grid *grid = &game->as_grid;
		
		for(s32 row = 0; row < grid->row; row++)
		{
			for(s32 col = 0; col < grid->col; col++)
			{
				if(game->tilemap[(row / 4) * (grid->col / 4) + (col / 4)] == 1)
				{
					grid->cells[row * grid->col + col] = 1;
				}
				
			}
		}
	}
	
	ArenaTemp tempFrame = arenaTempBegin(game->frame);
	
	// update camera
	{
		cam->zoom = 135 * game->debug_zoom;
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
					b32 cell = grid->cells[row * grid->col + col];
					
					v4f color = {};
					
					if(cell == 1)
					{
						color = D_COLOR_CYAN;
					}
					
					v2f pos = as_worldPosFromIndex(grid, index);
					
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
						AS_NodeArray list = as_findPath(game->arena, &game->as_grid, e->pos, target->pos);
						
						if(list.count > 0)
						{
							v2f next_pos = as_worldPosFromIndex(&game->as_grid, list.v[0].index) + v2f{{8, 8}};
							
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
							for(s32 i = 0; i < list.count; i++)
							{
								AS_Node *iter = list.v + i;
								v2f pos = as_worldPosFromIndex(&game->as_grid, iter->index);
								
								v4f color = D_COLOR_RED;
								if(iter->index == list.v[list.count - 1].index)
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
					TEX_Handle key = a_keyFromPath(art_paths[e->art], pixel_params);
					sprite->tex = a_handleFromKey(key);
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
	
	if(game->fullscreen && window->win->fullscreen)
	{
		UI_Widget *floating = ui_makeWidget(window->cxt, str8_lit("floating window 2"));
		floating->flags = UI_Flags_is_floating | UI_Flags_has_bg | UI_Flags_draw_border | UI_Flags_rounded_corners;
		floating->computed_rel_position[0] = 0;
		floating->computed_rel_position[1] = 0;
		floating->bg_color = ED_THEME_BG_FROSTED;
		floating->border_color = ED_THEME_TITLEBAR;
		floating->radius = 15 / 1.8f;
		
		ui_scale(window->cxt, FONT_SIZE * 0.8)
			ui_parent(window->cxt, floating)
			ui_col(window->cxt)
		{
			size.x *= 2;
			size.y *= 2;
			
			ui_pref_width(window->cxt, size.x)
				ui_pref_height(window->cxt, size.y)
				ui_size_kind(window->cxt, UI_SizeKind_Pixels)
			{
				ui_imagef(window->cxt, tab->target, rect(0,0,1,1), D_COLOR_WHITE, "game image");
			}
		}
	}
	else
	{
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
	
	// render tabs
	{
		ed_floatingTab(delta, str8_lit("lister"), game->lister_tab);
		ed_floatingTab(delta, str8_lit("profiler"), game->profiler_tab);
		ed_floatingTab(delta, str8_lit("Console"), game->console_tab);
	}
	
	arenaTempEnd(&tempFrame);
}

int main(int argc, char **argv)
{
	Arena *arena = arenaAlloc();
	Arena *trans = arenaAlloc();
	
	Str8 app_dir = os_getAppDir(arena);
	u64 start = os_getPerfCounter();
	u64 freq = os_getPerfFreq();
	
	f64 time_elapsed = 0;
	f64 delta = 0;
	
	gladLoadGL();
	tcxt_init();
	ed_init();
	os_init();
	
	SDL_GL_SetSwapInterval(0);
	ED_Window *game_win = ed_openWindow(ED_WindowFlags_HasSurface | ED_WindowFlags_ChildrenSum, v2f{{251,50}}, v2f{{960, 540}});
	
	ED_Panel *main_panel = ed_openPanel(game_win, Axis2_X, 1);
	
	ED_Tab *game = ed_openTab(main_panel, "Game");
	game->custom_draw = game_update_and_render;
	game->custom_drawData = push_struct(arena, Game);
	
	r_opengl_init();
	
	d_init();
	a_init();
	
	char codepoints[] =
	{
		'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 
		's', 't', 'u', 'v', 'w', 'x', 'y','z',
		
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
		'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
		
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		
		'&', '.', '?', ',', '-', ':', '!', '%', '#',
		
		'(', ')', '[', ']', '{', '}',
		
		' ', '\n', '\\', '/'
	};
	
	ArenaTemp temp = arenaTempBegin(trans);
	
	font = push_struct(arena, Font);
	
	Str8 font_path = str8_join(trans,app_dir, str8_lit("../data/assets/fonts/delius.ttf"));
	Glyph *temp_font = make_bmp_font(font_path.c, codepoints, ARRAY_LEN(codepoints), trans);
	
	for(u32 i = 0; i < ARRAY_LEN(codepoints); i ++)
	{
		u32 c = codepoints[i];
		
		if(c != '\n' && c != ' ')
		{
			font->atlas_tex[c] = r_allocTexture(temp_font[i].bmp, temp_font[i].w, temp_font[i].h, 1, &font_params);
		}
		
		font->atlas.glyphs[c].bearing = temp_font[i].bearing;
		font->atlas.glyphs[c].advance = temp_font[i].advance;
		font->atlas.glyphs[c].x0 = temp_font[i].x0;
		font->atlas.glyphs[c].x1 = temp_font[i].x1;
		font->atlas.glyphs[c].y0 = temp_font[i].y0;
		font->atlas.glyphs[c].y1 = temp_font[i].y1;
	}
	
	arenaTempEnd(&temp);
	
	for (;!game_win->win->close_requested;)
	{		
		BEGIN_TIMED_BLOCK(UPDATE_AND_RENDER);
		f64 time_since_last = time_elapsed;
		
		os_pollEvents();
		d_begin();
		
		ed_update(delta);
		ed_submit();
		
		d_end();
		
		u64 end = os_getPerfCounter();
		time_elapsed = (double)(end - start) / freq;
		
		delta = time_elapsed - time_since_last;
		
		END_TIMED_BLOCK(UPDATE_AND_RENDER);
		tcxt_process_debug_counters();
	}
	
	return 0;
}

/*
loop()
{

}

a : 3 : int;
a : 10 : int;

a : 3;

	a :: 3;
a = 3;

: constant decl
:: mut decl

= assignment

b8
b16
b32
b64

f32
f64

str8
str16
str32

x : 10 -> b8;
y : 5 -> b8;

z : x - y;
z : y - x; 1000 0101 ; 1 + 4 + 128

print("%s", z);
print("%u", z);

update :: (a : int) -> int, float, double
	{

}

		a : (b b32) : int
		{
		return b;
}

		import math/fm as fast_math
		import radio/fm as fm_radio

		use game.players[10].pos as pos;

same as #define pos game.players[10]

different from
v2 pos = game.players[10].pos

or

v2 *pos = &game.players[10].pos

@compile
@inline
@export

iterate through any structure, union, enum
Full introspection
*/

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