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
  Rect src;
	Rect dst;
	
	v4f border_color;
	v4f fade[Corner_COUNT];
	R_Handle tex;
  f32 border_thickness;
  f32 radius;
  f32 pad[2];
};

struct R_MeshInst
{
  m4f model;
};

struct R_Primitive
{
  u32 start;
  u32 count;
  
  R_Handle tex;
};

struct R_Mesh
{
  R_Primitive *primitives;
  u64 num_primitives;
  
  GLuint index;
  GLuint vert;
  u32 num_indices;
};

struct R_Model
{
  R_Mesh *meshes;
  u64 num_meshes;
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
  R_Handle target;
};

struct R_Sprite_pass
{
  R_Batch_list sprites;
  m4f proj_view;
  R_Handle target;
};

struct R_Mesh_pass
{
  R_Batch_list mesh;
  R_Handle target;
  m4f proj_view;
  
  GLuint index;
  u32 num_indices;
  GLuint vert;
};

enum R_PASS_KIND
{
	R_PASS_KIND_UI,
	R_PASS_KIND_MESH,
  R_PASS_KIND_SPRITE,
  R_PASS_KIND_COUNT,
};

struct R_Pass
{
	R_PASS_KIND kind;
	
	union
	{
		R_Rect_pass rect_pass;
    R_Mesh_pass mesh_pass;
    R_Sprite_pass sprite_pass;
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

function R_Batch *r_push_batch_list(Arena *arena, R_Batch_list *list)
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

#define r_push_batch(arena,batch, type) (type*)r_push_batch_(arena, batch, sizeof(type))

function void *r_push_batch_(Arena *arena, R_Batch_list *list, u64 size)
{
	R_Batch *batch = list->last;
	
	if(!batch || (batch->used + size > batch->size))
	{
		batch = r_push_batch_list(arena, list);
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

function R_Pass *r_push_pass_list(Arena *arena, R_Pass_list *list, R_PASS_KIND kind)
{
	R_Pass_node *node = push_struct(arena, R_Pass_node);
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
function R_Handle r_alloc_texture(void *data, s32 w, s32 h, s32 n, R_Texture_params *p);
function void r_free_texture(R_Handle handle);
function R_Handle r_alloc_frame_buffer(s32 w, s32 h);
function void r_submit(OS_Window win, R_Pass_list *list);
function v2s r_tex_size_from_handle(R_Handle handle);