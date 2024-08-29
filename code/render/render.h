/* date = July 18th 2024 10:18 am */

#ifndef RENDER_H
#define RENDER_H

// for opengl4.5, textures, 
// R: Resident handle
// G: gpu handle
// W: Width
// H: height
// 0: empty
// RRRRRRRR GGGGWWWW HHHH0000 00000000

union R_Handle
{
  u64 u64_m[4];
	u32 u32_m[8];
	u16 u16_m[16];
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
	
	v4f color;
	v4f fade[Corner_COUNT];
	R_Handle tex;
};

struct R_Batch
{
	R_Batch *next;
	size_t size;
	u8 *base;
	size_t used;
	u32 count;
};

struct R_Batch_list
{
	R_Batch *first;
	R_Batch *last;
	
	u32 num;
};

struct R_Rect_pass
{
	R_Batch_list rects;
};

enum R_PASS_KIND
{
	R_PASS_KIND_UI,
	R_PASS_KIND_COUNT,
};

struct R_Pass
{
	R_PASS_KIND kind;
	
	union
	{
		R_Rect_pass rect_pass;
	};
	
};

struct R_Pass_node
{
	R_Pass_node *next;
	R_Pass pass;
};

struct R_Pass_list
{
	R_Pass_node *first;
	R_Pass_node *last;
	
	u32 num;
};

#define r_push_batch(arena,batch, type) (type*)r_push_batch_(arena, batch, sizeof(type))

function void *r_push_batch_(Arena *arena, R_Batch_list *list, u64 size);

function R_Batch *r_push_batch_list(Arena *arena, R_Batch_list *list);

function R_Pass *r_push_pass(Arena *arena, R_Pass_list *list, R_PASS_KIND kind);

function R_Pass *r_push_pass_list(Arena *arena, R_Pass_list *list, R_PASS_KIND kind);


// backend hooks

function R_Handle r_alloc_texture(void *data, s32 w, s32 h, s32 n, R_Texture_params *p);
function void r_free_texture(R_Handle handle);
function void r_submit(R_Pass_list *list, v2s win_size);
function v2s r_tex_size_from_handle(R_Handle handle);

#endif //RENDER_H