/* date = August 16th 2024 7:50 pm */

#ifndef RENDER_CACHE_H
#define RENDER_CACHE_H

#define R_CACHE_HASH_SLOTS 32

struct R_HandleCache
{
	R_HandleCache *hash_next;
	u64 hash;
	R_Handle v;
	b32 loaded;
};

struct R_CacheSlot
{
	R_HandleCache *first;
	R_HandleCache *last;
};

struct R_RenderCache
{
	Arena *arena;
	R_CacheSlot *slots;
};

global R_RenderCache *r_render_cache;

// djb2
function u64 r_hash(Str8 str)
{
	u64 hash = 5381;
	int c;
	
	for(u32 i = 0; i < str.len; i++)
	{
		c = str.c[i];
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}
	
	return hash;
}

function void r_add_to_hash(Str8 path, R_Handle handle)
{
	u64 hash = r_hash(path);
	u64 slot = hash % R_CACHE_HASH_SLOTS;
	
	R_HandleCache *node = push_struct(r_render_cache->arena, R_HandleCache);
	node->hash = hash;
	node->v = handle;
	
	if(r_render_cache->slots[slot].last)
	{
		r_render_cache->slots[slot].last = r_render_cache->slots[slot].last->hash_next = node;
	}
	else
	{
		r_render_cache->slots[slot].last = r_render_cache->slots[slot].first = node;
	}
}

function R_Handle r_handle_from_hash(Str8 path)
{
	R_Handle out = {};
	u64 hash = r_hash(path);
	u64 slot = hash % R_CACHE_HASH_SLOTS;
	
	R_HandleCache *cur = r_render_cache->slots[slot].first;
	
	while(cur)
	{
		if(cur->hash == hash)
		{
			out = cur->v;
			break;
		}
		
		cur = cur->hash_next;
	}
	
	if(!cur)
	{
		Bitmap bmp = bitmap(path);
		out = r_alloc_texture(bmp.data, bmp.w, bmp.h, bmp.n, &pixel_params);
		r_add_to_hash(path, out);
	}
	
	return out;
}

#endif //RENDER_CACHE_H
