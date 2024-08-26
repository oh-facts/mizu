void a_init()
{
	Arena *arena = arena_create(100, Megabytes(1));
	a_asset_cache = push_struct(arena, A_AssetCache);
	a_asset_cache->arena = arena;
	a_asset_cache->num_slots = 1;
	a_asset_cache->slots = push_array(arena, A_TextureCacheSlot, a_asset_cache->num_slots); 
	
	Arena_temp temp = scratch_begin(0, 0);
	Str8 app_dir = os_get_app_dir(temp.arena);
	
	a_asset_cache->asset_dir = str8_join(arena, app_dir, A_ASSET_DIRECTORY);
	
	scratch_end(&temp);
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

A_TextureCache *a_alloc_texture_cache()
{
	A_TextureCache *out = a_asset_cache->free;
	
	if(!out)
	{
		out = push_struct(a_asset_cache->arena, A_TextureCache);
	}
	else
	{
		a_asset_cache->free = a_asset_cache->free->next;
	}
	*out = {};
	return out;
}

void a_free_texture_cache(A_TextureCache *tex)
{
	tex->next = a_asset_cache->free;
	a_asset_cache->free = tex;
}

void a_add_to_hash(A_TextureCache *tex)
{
	u64 hash = tex->key;
	u64 slot = hash % a_asset_cache->num_slots;
	
	if(a_asset_cache->slots[slot].last)
	{
		a_asset_cache->slots[slot].last = a_asset_cache->slots[slot].last->next = tex;
	}
	else
	{
		a_asset_cache->slots[slot].last = a_asset_cache->slots[slot].first = tex;
	}
}

R_Handle a_handle_from_path(Str8 path)
{
	u64 hash = a_hash(path);
	u64 slot = hash % a_asset_cache->num_slots;
	
	A_TextureCache *tex_cache = a_asset_cache->slots[slot].first;
	
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
		
		if(a_asset_cache->tex_mem > A_MAX_TEXTURE_MEM)
		{
			a_evict();
		}
		
		tex_cache = a_alloc_texture_cache();
		Arena_temp temp = scratch_begin(0, 0);
		Str8 abs_path = str8_join(temp.arena, a_asset_cache->asset_dir, path);
		Bitmap bmp = bitmap(abs_path);
		
		// if bmp not found, use checkerboard art
		
		if(!bmp.data)
		{
			local_persist u32 checker[16] = {
				0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFFFF00FF,
				0xFFFF00FF, 0xFF000000, 0xFFFF00FF, 0xFF000000,
				0xFF000000, 0xFFFF00FF, 0xFF000000, 0xFFFF00FF,
				0xFFFF00FF, 0xFF000000, 0xFFFF00FF, 0xFF000000,
			}; 
			
			bmp.data = checker;
			bmp.w = 4;
			bmp.h = 4;
			bmp.n = 4;
		}
		
		tex_cache->v = r_alloc_texture(bmp.data, bmp.w, bmp.h, bmp.n, &pixel_params);
		tex_cache->key = hash;
		tex_cache->loaded = 1;
		
		a_add_to_hash(tex_cache);
		
		scratch_end(&temp);
		a_asset_cache->tex_mem += bmp.w * bmp.h * 4; 
		++a_asset_cache->num_tex;
	}
	
	a_asset_cache->frame_count++;
	tex_cache->last_touched = a_asset_cache->frame_count;
	tex_cache->path = path;
	R_Handle out = tex_cache->v;
	return out;
}

void a_evict()
{
	for(u32 i = 0; i < a_asset_cache->num_slots; i++)
	{
		A_TextureCache *first_hash = (a_asset_cache->slots + i)->first;
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
				if(cur->last_touched != a_asset_cache->frame_count)
				{
					printf("pruned %.*s\n", str8_varg(cur->path));
					--a_asset_cache->num_tex;
					v2s tex_size = r_tex_size_from_handle(cur->v);
					a_asset_cache->tex_mem -= tex_size.x * tex_size.y * 4; 
					r_free_texture(cur->v);
					cur->loaded = 0;
					
					if(prev)
					{
						prev->next = cur->next;
						if (!cur->next)
						{
							(a_asset_cache->slots + i)->last = prev;
						}
					}
					else
					{
						(a_asset_cache->slots + i)->first = cur->next;
						if (!cur->next)
						{
							(a_asset_cache->slots + i)->last = 0;
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