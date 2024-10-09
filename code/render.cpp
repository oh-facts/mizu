// for opengl4.5, 
// R: Resident handle
// G: gpu handle
// T: Texture
// W: Width
// H: height
// 0: empty

// textures, 
// RRRRRRRR GGGGWWWW HHHH0000 00000000

// frame buffers,
// RRRRRRRR GGGGWWWW HHHH0000 00000000

union R_Handle
{
	u64 u64_m[4];
	u32 u32_m[8];
	u16 u16_m[16];
};

struct R_Vertex
{
	v3f pos;
	f32 uv_x;
	v3f normal;
	f32 uv_y;
	v4f color;
};

enum R_TEXTURE_FILTER
{
	R_TEXTURE_FILTER_NEAREST,
	R_TEXTURE_FILTER_LINEAR,
	R_TEXTURE_FILTER_COUNT
};

enum R_TEXTURE_WRAP
{
	R_TEXTURE_WRAP_CLAMP_TO_BORDER,
	R_TEXTURE_WRAP_REPEAT,
	R_TEXTURE_WRAP_COUNT
};

enum R_POLYGON_MODE
{
	R_POLYGON_MODE_UNINITIALIZED,
	R_POLYGON_MODE_POINT,
	R_POLYGON_MODE_LINE,
	R_POLYGON_MODE_FILL,
	R_POLYGON_MODE_COUNT
};

struct R_Texture_params
{
	R_TEXTURE_FILTER min;
	R_TEXTURE_FILTER max;
	R_TEXTURE_WRAP wrap;
};

global R_Texture_params font_params = {
	R_TEXTURE_FILTER_LINEAR,
	R_TEXTURE_FILTER_LINEAR,
	R_TEXTURE_WRAP_CLAMP_TO_BORDER
};

global R_Texture_params tiled_params = {
	R_TEXTURE_FILTER_LINEAR,
	R_TEXTURE_FILTER_LINEAR,
	R_TEXTURE_WRAP_REPEAT
};

global R_Texture_params pixel_tiled_params = {
	R_TEXTURE_FILTER_NEAREST,
	R_TEXTURE_FILTER_NEAREST,
	R_TEXTURE_WRAP_REPEAT
};

global R_Texture_params pixel_params = {
	R_TEXTURE_FILTER_NEAREST,
	R_TEXTURE_FILTER_NEAREST,
	R_TEXTURE_WRAP_CLAMP_TO_BORDER
};

enum Corner
{
	Corner_00,
	Corner_01,
	Corner_10,
	Corner_11,
	Corner_COUNT
};

struct R_Rect
{
	Rect src;
	Rect dst;
	
	v4f border_color;
	v4f fade[Corner_COUNT];
	
	R_Handle tex;
	
	f32 border_thickness;
	f32 radius;
	f32 pad[2];
};

struct R_Sprite
{
	Rect src; // 2 * v2f
	Rect dst;
	
	v4f border_color;
	v4f fade[Corner_COUNT]; // 4 corners
	R_Handle tex; // 8 * u32
	
	f32 border_thickness;
	f32 radius;
	v2f basis;
	
	f32 pad[3];
	s32 layer; //causes epic crash
};

int r_spriteSort(const void* a, const void* b) 
{
	R_Sprite *first = (R_Sprite*)a;
	R_Sprite *sec = (R_Sprite*)b;
	
	if (first->layer != sec->layer) 
	{
		return (first->layer > sec->layer) - (first->layer < sec->layer);
	}
	
	f32 pos_1 = first->dst.tl.y - first->basis.y;
	f32 pos_2 = sec->dst.tl.y - sec->basis.y;
	
	return (pos_1 > pos_2) - (pos_1 < pos_2);
}

struct R_Batch
{
	R_Batch *next;
	size_t size;
	u8 *base;
	size_t used;
	u32 count;
};

struct R_BatchList
{
	R_Batch *first;
	R_Batch *last;
	
	u32 num;
};

struct R_RectPass
{
	R_BatchList rects;
	m4f proj_view;
	R_Handle target;
};

struct R_SpritePass
{
	R_BatchList sprites;
	m4f proj_view;
	R_Handle target;
};

enum R_PASS_KIND
{
	R_PASS_KIND_UI,
	R_PASS_KIND_SPRITE,
	R_PASS_KIND_COUNT,
};

struct R_Pass
{
	R_PASS_KIND kind;
	
	union
	{
		R_RectPass rect_pass;
		R_SpritePass sprite_pass;
	};
};

struct R_PassNode
{
	R_PassNode *next;
	R_Pass pass;
};

struct R_PassList
{
	R_PassNode *first;
	R_PassNode *last;
	
	u32 num;
};

function R_Batch *r_pushBatchList(Arena *arena, R_BatchList *list)
{
	R_Batch *node = push_struct(arena, R_Batch);
	list->num ++;
	if(list->last)
	{
		list->last = list->last->next = node;
	}
	else
	{
		list->last = list->first = node;
	}
	
	return node;
}

#define r_pushBatch(arena,batch, type) (type*)r_pushBatch_(arena, batch, sizeof(type))

function void *r_pushBatch_(Arena *arena, R_BatchList *list, u64 size)
{
	R_Batch *batch = list->last;
	
	if(!batch || (batch->used + size > batch->size))
	{
		batch = r_pushBatchList(arena, list);
	}
	
	if(!batch->base)
	{
		batch->base = push_array(arena, u8, size * 1000);
		batch->size = size * 1000;
	}
	
	void *out = batch->base + batch->used;
	batch->used += size;
	batch->count++;
	
	return out;
}

function R_Pass *r_pushPassList(Arena *arena, R_PassList *list, R_PASS_KIND kind)
{
	R_PassNode *node = push_struct(arena, R_PassNode);
	list->num ++;
	if(list->last)
	{
		list->last = list->last->next = node;
	}
	else
	{
		list->last = list->first = node;
	}
	node->pass.kind = kind;
	R_Pass *pass = &node->pass;
	return pass;
}

// backend hooks
function R_Handle r_allocTexture(void *data, s32 w, s32 h, s32 n, R_Texture_params *p);
function void r_freeTexture(R_Handle handle);
function R_Handle r_allocFramebuffer(s32 w, s32 h);
function void r_submit(OS_Window win, R_PassList *list);
function v2s r_texSizeFromHandle(R_Handle handle);