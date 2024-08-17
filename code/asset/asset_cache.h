/* date = August 17th 2024 9:48 pm */

#ifndef ASSET_CACHE_H
#define ASSET_CACHE_H

#define A_HASH_SLOTS 32
#define A_MAX_TEXTURES 256

const Str8 A_ASSET_DIRECTORY = str8_lit("../data/assets/");

struct A_TextureCache
{
	A_TextureCache *hash_next;
	Str8 key;
	R_Handle v;
	b32 loaded;
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
	u32 num_slots;
	Str8 asset_dir;
	u32 num_tex;
};

global A_AssetCache *a_asset_cache;

// djb2
function void a_init();
function u64 a_hash(Str8 str);
function void a_add_to_hash(Str8 path, R_Handle handle);
function A_TextureCache *a_tex_from_hash(Str8 path);

#endif //ASSET_CACHE_H
