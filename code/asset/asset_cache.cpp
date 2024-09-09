A_FontCache *a_getFontCache()
{
	return a_state->slots.store + A_AssetKind_Font;
}

A_GlyphCache *a_getGlyphCache()
{
	return a_state->slots.store + A_AssetKind_Glyph;
}

A_TextureCache *a_getTextureCache()
{
	return a_state->slots.store + A_AssetKind_Texture;
}

void a_init()
{
	Arena *arena = arena_create(100, Megabytes(1));
	a_state = push_struct(arena, A_State);
	a_state->arena = arena;
	
	{
		A_AssetStore *cache = a_getFontCache();
		cache->num_slots = 10;
		cache->slots = push_array(arena, A_AssetCacheSlot, cache->num_slots);
	}
	
	{
		A_AssetStore *cache = a_getGlyphCache();
		cache->num_slots = 1024;
		cache->slots = push_array(arena, A_AssetCacheSlot, cache->num_slots);
	}
	
	{
		A_AssetStore *cache = a_getTextureCache();
		cache->num_slots = 1024;
		cache->slots = push_array(arena, A_AssetCacheSlot, cache->num_slots);
	}
	
	Arena_temp temp = scratch_begin(0, 0);
	Str8 app_dir = os_get_app_dir(temp.arena);
	
	a_state->asset_dir = str8_join(arena, app_dir, A_ASSET_DIRECTORY);
	
	scratch_end(&temp);
	
	u32 checker[16] = {
		0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFFFF00FF,
		0xFFFF00FF, 0xFF000000, 0xFFFF00FF, 0xFF000000,
		0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFFFF00FF,
		0xFFFF00FF, 0xFF000000, 0xFFFF00FF, 0xFF000000,
	}; 
	
	a_state->checker_tex = r_alloc_texture(checker, 4, 4, 4, &tiled_params);
	
	u32 alpha_bg[] = {
		0xFF808080, 0xFFc0c0c0,
		0xFFc0c0c0, 0xFF808080,
	};
	
	a_state->alpha_bg_tex = r_alloc_texture(alpha_bg, 2, 2, 4, &pixel_tiled_params);
	
}

// djb2
u64 a_hash(Str8 str)
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

A_AssetCache *a_allocAssetCache(A_AssetKind kind)
{
	A_AssetCache *out = a_state->free;
	
	if(!out)
	{
		out = push_struct(a_state->arena, A_AssetCache);
	}
	else
	{
		a_state->free = a_state->free->next;
	}
	*out = {};
	out->kind = kind;
	
	return out;
}

void a_freeAssetCache(A_AssetCache *ass)
{
	ass->next = a_state->free;
	a_state->free = ass;
}

void a_add_to_hash(A_TextureCache *tex)
{
	u64 hash = tex->key;
	u64 slot = hash % a_state->num_slots;
	
	if(a_state->slots[slot].last)
	{
		a_state->slots[slot].last = a_state->slots[slot].last->next = tex;
	}
	else
	{
		a_state->slots[slot].last = a_state->slots[slot].first = tex;
	}
}

R_Handle a_handle_from_path(Str8 path)
{
	u64 hash = a_hash(path);
	u64 slot = hash % a_state->num_slots;
	
	A_TextureCache *tex_cache = a_state->slots[slot].first;
	
	while(tex_cache)
	{
		if(hash == tex_cache->key)
		{
			break;
		}
		
		tex_cache = tex_cache->next;
	}
	
	if(!tex_cache)
	{
		printf("Added %.*s\n", str8_varg(path));
		
		if(a_state->tex_mem > A_MAX_TEXTURE_MEM)
		{
			a_evict();
		}
		
		tex_cache = a_alloc_texture_cache();
		Arena_temp temp = scratch_begin(0, 0);
		Str8 abs_path = str8_join(temp.arena, a_state->asset_dir, path);
		Bitmap bmp = bitmap(abs_path);
		
		// if bmp not found, use checkerboard art
		if(bmp.data)
		{
			tex_cache->v = r_alloc_texture(bmp.data, bmp.w, bmp.h, bmp.n, &pixel_params);
		}
		else
		{
			tex_cache->v = a_get_checker_tex();
		}
		
		tex_cache->key = hash;
		
		a_add_to_hash(tex_cache);
		
		scratch_end(&temp);
		a_state->tex_mem += bmp.w * bmp.h * 4; 
		++a_state->num_tex;
	}
	
	a_state->frame_count++;
	tex_cache->last_touched = a_state->frame_count;
	tex_cache->path = path;
	R_Handle out = tex_cache->v;
	return out;
}

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
					printf("pruned %.*s\n", str8_varg(cur->path));
					--a_state->num_tex;
					v2s tex_size = r_tex_size_from_handle(cur->v);
					a_state->tex_mem -= tex_size.x * tex_size.y * 4; 
					r_free_texture(cur->v);
					
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
					a_free_texture_cache(to_free);
					// free to_free
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

R_Handle a_get_checker_tex()
{
	return a_state->checker_tex;
}

R_Handle a_get_alpha_bg_tex()
{
	return a_state->alpha_bg_tex;
}
