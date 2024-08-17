void a_init()
{
	Arena *arena = arena_create();
	a_asset_cache = push_struct(arena, A_AssetCache);
	a_asset_cache->arena = arena;
	a_asset_cache->num_slots = 1024;
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

void a_add_to_hash(Str8 path, R_Handle handle)
{
	u64 hash = a_hash(path);
	u64 slot = hash % A_HASH_SLOTS;
	
	A_TextureCache *node = push_struct(a_asset_cache->arena, A_TextureCache);
	node->key = path;
	node->v = handle;
	
	if(a_asset_cache->slots[slot].last)
	{
		a_asset_cache->slots[slot].last = a_asset_cache->slots[slot].last->hash_next = node;
	}
	else
	{
		a_asset_cache->slots[slot].last = a_asset_cache->slots[slot].first = node;
	}
}

A_TextureCache *a_tex_from_hash(Str8 path)
{
	u64 hash = a_hash(path);
	u64 slot = hash % A_HASH_SLOTS;
	
	A_TextureCache *out = a_asset_cache->slots[slot].first;
	
	while(out)
	{
		if(str8_equals(out->key, path))
		{
			break;
		}
		
		out = out->hash_next;
	}
	
	if(!out)
	{
		printf("Added %.*s\n", str8_varg(path));
		if(a_asset_cache->num_tex == A_MAX_TEXTURES)
		{
			// TODO(mizu): asset eviction
			printf("Work on asset eviction! Quitting ...");
			INVALID_CODE_PATH();
		}
		
		out = push_struct(a_asset_cache->arena, A_TextureCache);
		Arena_temp temp = scratch_begin(0, 0);
		Str8 abs_path = str8_join(temp.arena, a_asset_cache->asset_dir, path);
		Bitmap bmp = bitmap(abs_path);
		out->v = r_alloc_texture(bmp.data, bmp.w, bmp.h, bmp.n, &pixel_params);
		a_add_to_hash(path, out->v);
		out->loaded = 1;
		scratch_end(&temp);
		
		++a_asset_cache->num_tex;
	}
	
	return out;
}