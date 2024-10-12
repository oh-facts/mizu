#define A_MAX_TEXTURE_MEM Megabytes(64)

read_only Str8 A_ASSET_DIRECTORY = str8_lit("../data/assets/");

// djb2
function u64 a_hash(Str8 str)
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

struct A_Key
{
	u64 v;
	Str8 path;
	R_Texture_params params;
};

function A_Key a_keyFromPath(Str8 path, R_Texture_params params)
{
	A_Key out = {};
	out.v = a_hash(path);
	out.path = path;
	out.params = params;
	return out;
}

function b32 a_keysAreSame(A_Key a, A_Key b)
{
	b32 out = 0;
	
	if((memcmp(&a.params, &b.params, sizeof(R_Texture_params)) == 0) && a.v == b.v)
	{
		out = 1;
	}
	
	return out;
}

struct A_TextureCache
{
	// used as hash link when in map, used as free list link when freed
	A_TextureCache *next;
	A_Key key;
	R_Handle v;
	b32 loaded;
	u64 last_touched;
};

struct A_TextureCacheSlot
{
	A_TextureCache *first;
	A_TextureCache *last;
};

struct A_State
{
	Arena *arena;
	A_TextureCacheSlot *slots;
	
	// I am thinking I use this to store bitmap hashes so i can asset reload?
	A_TextureCacheSlot *bitmaps;
	
	A_TextureCache *free;
	u32 num_slots;
	Str8 asset_dir;
	u32 num_tex;
	u64 tex_mem;
	u64 frame_count;
	
	R_Handle checker_tex;
	R_Handle alpha_bg_tex;
};

global A_State *a_state;

// you need to solve two more problems.
// 1) freeing must happen next frame
// 2) If I have multiple expensive textures would it be hiccupy to evict them at the same time?
// Should I only free as many as required, or as many as can be? Batch alloc / dealloc is better, no? I need to test this with massive textures to understand.
// 3) Lastly, all alloc and eviction must happen on separate thread

function void a_init()
{
	Arena *arena = arenaAlloc(100, Megabytes(1));
	a_state = push_struct(arena, A_State);
	a_state->arena = arena;
	a_state->num_slots = 100;
	a_state->slots = push_array(arena, A_TextureCacheSlot, a_state->num_slots); 
	
	ArenaTemp temp = scratch_begin(0, 0);
	Str8 app_dir = os_getAppDir(temp.arena);
	
	a_state->asset_dir = str8_join(arena, app_dir, A_ASSET_DIRECTORY);
	
	scratch_end(&temp);
	
	u32 checker[16] = {
		0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFFFF00FF,
		0xFFFF00FF, 0xFF000000, 0xFFFF00FF, 0xFF000000,
		0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFFFF00FF,
		0xFFFF00FF, 0xFF000000, 0xFFFF00FF, 0xFF000000,
	}; 
	
	a_state->checker_tex = r_allocTexture(checker, 4, 4, 4, &tiled_params);
	
	u32 alpha_bg[] = {
		0xFF808080, 0xFFc0c0c0,
		0xFFc0c0c0, 0xFF808080,
	};
	
	a_state->alpha_bg_tex = r_allocTexture(alpha_bg, 2, 2, 4, &pixel_tiled_params);
}

function A_TextureCache *a_allocTextureCache()
{
	A_TextureCache *out = a_state->free;
	
	if(!out)
	{
		out = push_struct(a_state->arena, A_TextureCache);
	}
	else
	{
		a_state->free = a_state->free->next;
	}
	*out = {};
	return out;
}

function void a_freeTextureCache(A_TextureCache *tex)
{
	tex->next = a_state->free;
	a_state->free = tex;
}

function void a_addToHash(A_TextureCache *tex)
{
	u64 slot = tex->key.v % a_state->num_slots;
	
	if(a_state->slots[slot].last)
	{
		a_state->slots[slot].last = a_state->slots[slot].last->next = tex;
	}
	else
	{
		a_state->slots[slot].last = a_state->slots[slot].first = tex;
	}
}

function R_Handle a_getCheckerTex()
{
	return a_state->checker_tex;
}

function R_Handle a_getAlphaBGTex()
{
	return a_state->alpha_bg_tex;
}

// NOTE(mizu): it is important i wait until the end of the frame
void a_evict()
{
	for(u32 i = 0; i < a_state->num_slots; i++)
	{
		A_TextureCache *first_hash = (a_state->slots + i)->first;
		if(!first_hash)
		{
			continue;
		}
		if(first_hash)
		{
			A_TextureCache *cur = first_hash;
			A_TextureCache *prev = 0;
			while(cur)
			{
				if(cur->last_touched != a_state->frame_count)
				{
					printf("pruned %.*s\n", str8_varg(cur->key.path));
					--a_state->num_tex;
					v2s tex_size = r_texSizeFromHandle(cur->v);
					a_state->tex_mem -= tex_size.x * tex_size.y * 4; 
					r_freeTexture(cur->v);
					cur->loaded = 0;
					
					if(prev)
					{
						prev->next = cur->next;
						if (!cur->next)
						{
							(a_state->slots + i)->last = prev;
						}
					}
					else
					{
						(a_state->slots + i)->first = cur->next;
						if (!cur->next)
						{
							(a_state->slots + i)->last = 0;
						}
					}
					
					A_TextureCache *to_free = cur;
					cur = cur->next;
					a_freeTextureCache(to_free);
				}
				else
				{
					prev = cur;
					cur = cur->next;
				}
			}
		}
	}
}

function R_Handle a_handleFromKey(A_Key key)
{
	u64 slot = key.v % a_state->num_slots;
	
	A_TextureCache *tex_cache = a_state->slots[slot].first;
	
	while(tex_cache)
	{
		if(a_keysAreSame(key, tex_cache->key))
		{
			break;
		}
		
		tex_cache = tex_cache->next;
	}
	
	if(!tex_cache)
	{
		printf("Added %.*s\n", str8_varg(key.path));
		
		if(a_state->tex_mem > A_MAX_TEXTURE_MEM)
		{
			a_evict();
		}
		
		tex_cache = a_allocTextureCache();
		ArenaTemp temp = scratch_begin(0, 0);
		Str8 abs_path = str8_join(temp.arena, a_state->asset_dir, key.path);
		Bitmap bmp = bitmap(abs_path);
		
		// if bmp not found, use checkerboard art
		if(bmp.data)
		{
			tex_cache->v = r_allocTexture(bmp.data, bmp.w, bmp.h, bmp.n, &key.params);
		}
		else
		{
			tex_cache->v = a_getCheckerTex();
		}
		
		tex_cache->loaded = 1;
		tex_cache->key = key;
		a_addToHash(tex_cache);
		
		scratch_end(&temp);
		a_state->tex_mem += bmp.w * bmp.h * 4; 
		++a_state->num_tex;
		
		// TODO(mizu): Must make it use my own memory. Modify this library or parse png yourself
		// or load with stbi and then copy it to your own buffer and then free it.
		stbi_image_free(bmp.data);
	}
	
	a_state->frame_count++;
	tex_cache->last_touched = a_state->frame_count;
	
	R_Handle out = tex_cache->v;
	return out;
}