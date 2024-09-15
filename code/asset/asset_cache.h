/* date = August 17th 2024 9:48 pm */

#ifndef ASSET_CACHE_H
#define ASSET_CACHE_H

#define A_MAX_TEXTURE_MEM Megabytes(64)

const Str8 A_ASSET_DIRECTORY = str8_lit("../data/assets/");

// two things need to happen here. Caching font data, and caching glyphs

// For font; it has hashed font binary data;
struct A_Handle
{
	u64 m_u64;
};

struct A_FontCache
{
	A_Handle atlas;
};

struct A_GlyphCache
{
	A_Handle font;
	R_Handle tex;
	Glyph metrics;
};

struct A_TextureCache
{
	R_Handle v;
};

enum A_AssetKind
{
	A_AssetKind_Null,
	A_AssetKind_Font,
	A_AssetKind_Glyph,
	A_AssetKind_Texture,
	A_AssetKind_COUNT
};

struct A_AssetCache
{
	A_AssetKind kind;
	A_AssetCache *next;
	Str8 key;
	u64 hash;
	u64 last_touched;
  
	union
	{
		A_FontCache fontCache;
		A_GlyphCache glyphCache;
		A_TextureCache textureCache;
	};
};

struct A_AssetCacheSlot
{
	A_AssetCache *first;
	A_AssetCache *last;
};

struct A_AssetStore
{
	A_AssetCacheSlot *slots;
	u32 num_slots;
};

struct A_State
{
	Arena *arena;
	A_AssetStore stores[A_AssetKind_COUNT];
	A_AssetCache *free;
	
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

// public
function R_Handle a_handleFromPath(Str8 path);
function R_Handle a_get_checker_tex();
function R_Handle a_get_alpha_bg_tex();

function void a_init();
// djb2
function u64 a_hash(Str8 str);
function A_AssetCache *a_allocAssetCache(A_AssetKind kind);
function void a_freeAssetCache(A_AssetCache *ass);

function void a_addToHash(A_AssetStore *store, A_AssetCache *cache);
function A_AssetCache *a_assetCacheFromKey(A_AssetKind kind, Str8 key);

// Texture Store
function void a_evict(A_AssetKind kind);

#endif //ASSET_CACHE_H