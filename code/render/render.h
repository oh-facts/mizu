/* date = July 18th 2024 10:18 am */

#ifndef RENDER_H
#define RENDER_H

struct R_Handle
{
  u64 u64_m[1];
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

global R_Texture_params pixel_params = {
	R_TEXTURE_FILTER_NEAREST,
	R_TEXTURE_FILTER_NEAREST,
	R_TEXTURE_WRAP_CLAMP_TO_BORDER
};

struct R_Rect
{
	v2f tl;
	v2f br;
	
	v4f color;
	R_Handle tex;
	R_Handle pad;
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
	m4f proj_view;
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

function R_Pass *r_push_pass_list(Arena *arena, R_Pass_list *list, R_PASS_KIND kind);
function R_Batch *r_push_batch_list(Arena *arena, R_Batch_list *list);

#define r_push_batch(arena,batch, type) (type*)r_push_batch_(arena, batch, sizeof(type))

function void *r_push_batch_(Arena *arena, R_Batch_list *list, u64 size);

typedef R_Handle (*r_alloc_texture_fn)(void *data, s32 w, s32 h, s32 n, R_Texture_params *p);

typedef void (*r_submit_fn)(R_Pass_list *list, v2s win_size);

global r_alloc_texture_fn r_alloc_texture;
global r_submit_fn r_submit;

#endif //RENDER_H