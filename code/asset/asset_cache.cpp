void a_init()
{
	Arena *arena = arena_create(100, Megabytes(1));
	a_state = push_struct(arena, A_State);
	a_state->arena = arena;
	
	{
		A_AssetStore *cache = a_state->stores + A_AssetKind_Font;
    cache->num_slots = 10;
		cache->slots = push_array(arena, A_AssetCacheSlot, cache->num_slots);
	}
	
	{
		A_AssetStore *cache = a_state->stores + A_AssetKind_Texture;
		cache->num_slots = 1024;
		cache->slots = push_array(arena, A_AssetCacheSlot, cache->num_slots);
	}
	
	{
		A_AssetStore *cache = a_state->stores + A_AssetKind_Glyph;
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

void a_addToHash(A_AssetStore *store, A_AssetCache *cache)
{
  u64 hash = cache->hash;
	u64 slot = hash % store->num_slots;
	
	if(store->slots[slot].last)
	{
		store->slots[slot].last = store->slots[slot].last->next = cache;
	}
	else
	{
		store->slots[slot].last = store->slots[slot].first = cache;
	}
}

A_AssetCache *a_assetCacheFromKey(A_AssetKind kind, Str8 key)
{
  A_AssetStore *store = a_state->stores + kind;
	u64 hash = a_hash(key);
	u64 slot = hash % store->num_slots;
	
	A_AssetCache *cache = store->slots[slot].first;
  
	while(cache)
	{
		if(hash == cache->hash)
		{
			break;
		}
		cache = cache->next;
	}
	
	if(!cache)
	{
		printf("Added %.*s\n", str8_varg(key));
		
    cache = a_allocAssetCache(kind);
		cache->hash = hash;
    cache->key = key;
    
    switch(kind)
    {
      default: INVALID_CODE_PATH();
      case A_AssetKind_Texture:
      {
        if(a_state->tex_mem > A_MAX_TEXTURE_MEM)
        {
          a_evict(A_AssetKind_Texture);
        }
        
        Arena_temp temp = scratch_begin(0, 0);
        Str8 abs_path = str8_join(temp.arena, a_state->asset_dir, key);
        
        Bitmap bmp = bitmap(abs_path);
        
        // if bmp not found, use checkerboard art
        if(bmp.data)
        {
          cache->textureCache.v = r_alloc_texture(bmp.data, bmp.w, bmp.h, bmp.n, &pixel_params);
        }
        else
        {
          cache->textureCache.v = a_get_checker_tex();
        }
        
        a_state->tex_mem += bmp.w * bmp.h * 4; 
        ++a_state->num_tex;
        scratch_end(&temp);
        
      }break;
		}
		
    a_addToHash(store, cache);
	}
	
	a_state->frame_count++;
	cache->last_touched = a_state->frame_count;
	
  return cache;
	//R_Handle out = tex_cache->textureCache.v;
	//return out;
}

R_Handle a_handleFromPath(Str8 path)
{
  R_Handle out = {};
  out = a_assetCacheFromKey(A_AssetKind_Texture, path)->textureCache.v;
  
  return out;
}

void a_evict(A_AssetKind kind)
{
  A_AssetStore *store = a_state->stores + kind;
  for(u32 i = 0; i < store->num_slots; i++)
	{
		A_AssetCache *first_hash = (store->slots + i)->first;
		if(!first_hash)
		{
			continue;
		}
		if(first_hash)
		{
      A_AssetCache *cur = first_hash;
      A_AssetCache *prev = 0;
			while(cur)
			{
				if(cur->last_touched != a_state->frame_count)
				{
					printf("pruned %.*s\n", str8_varg(cur->key));
					
          switch(kind)
          {
            default: INVALID_CODE_PATH();
            case A_AssetKind_Texture:
            {
              --a_state->num_tex;
              v2s tex_size = r_tex_size_from_handle(cur->textureCache.v);
              a_state->tex_mem -= tex_size.x * tex_size.y * 4; 
              r_free_texture(cur->textureCache.v);
              
              if(prev)
              {
                prev->next = cur->next;
                if (!cur->next)
                {
                  (store->slots + i)->last = prev;
                }
              }
              else
              {
                (store->slots + i)->first = cur->next;
                if (!cur->next)
                {
                  (store->slots + i)->last = 0;
                }
              }
              
            }break;
          }
          
          // free to_free
          A_AssetCache *to_free = cur;
          cur = cur->next;
          a_freeAssetCache(to_free);
          
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
