/* date = August 17th 2024 9:48 pm */

#ifndef ASSET_CACHE_H
#define ASSET_CACHE_H

#define A_MAX_TEXTURES 10

const Str8 A_ASSET_DIRECTORY = str8_lit("../data/assets/");

struct A_TextureCache
{
	// used as hash link when in map, used as free list link when freed
	A_TextureCache *next;
	u64 key;
	Str8 path;
	R_Handle v;
	b32 loaded;
	u64 last_touched;
};

struct A_TextureCacheSlot
{
	A_TextureCache *first;
	A_TextureCache *last;
};

struct A_AssetCache
{
	Arena *arena;
	A_TextureCacheSlot *slots;
	A_TextureCache *free;
	u32 num_slots;
	Str8 asset_dir;
	u32 num_tex;
	u64 frame_count;
};

global A_AssetCache *a_asset_cache;

// djb2
function void a_init();
function u64 a_hash(Str8 str);
function A_TextureCache *a_alloc_texture_cache();
function void a_free_texture_cache(A_TextureCache *tex);
function void a_add_to_hash(A_TextureCache *tex);
function R_Handle a_handle_from_path(Str8 path);
function void a_evict();

#endif //ASSET_CACHE_H