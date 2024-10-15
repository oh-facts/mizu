#define ADD_FLAG(flags, flagToAdd)   ((flags) |= (flagToAdd))
#define REMOVE_FLAG(flags, flagToRemove) ((flags) &= ~(flagToRemove))
#define IS_FLAG_SET(flags, flagToCheck) ((flags) & (flagToCheck))
#define TOGGLE_FLAG(flags, flagToToggle) ((flags) ^= (flagToToggle))

function b32 is_pow_of_2(size_t addr)
{
	return (addr & (addr-1)) == 0;
}

typedef struct Arena Arena;
struct Arena
{
	Arena *next;
	u64 used;
	u64 align;
	u64 cmt;
	u64 res;
};

#define ARENA_COMMIT_SIZE Kilobytes(64)
#define ARENA_RESERVE_SIZE Megabytes(64)
#define ARENA_HEADER_SIZE 128
#define ARENA_ARR_LEN(arena, type) (arena->used / sizeof(type))

#define AlignPow2(x,b) (((x) + (b) - 1)&(~((b) - 1)))
#define Min(A,B) (((A)<(B))?(A):(B))
#define Max(A,B) (((A)>(B))?(A):(B))
#define ClampTop(A,X) Min(A,X)
#define ClampBot(X,B) Max(X,B)

typedef struct ArenaTemp ArenaTemp;
struct ArenaTemp
{
	Arena *arena;
	size_t pos;
};

#define push_struct(arena, type) (type*)_arenaPushImpl(arena, sizeof(type))
#define push_array(arena,type,count) (type*)_arenaPushImpl(arena, sizeof(type) * count)

function void* _arenaPushImpl(Arena* arena, size_t size)
{
	u64 pos_mem = AlignPow2(arena->used, arena->align);
	u64 pos_new = pos_mem + size;
	
	if(arena->res < pos_new)
	{
		// TODO(mizu): deal with reserving more (chain arenas)
		INVALID_CODE_PATH();
	}
	
	if(arena->cmt < pos_new)
	{
		u64 cmt_new_aligned, cmt_new_clamped, cmt_new_size;
		
		cmt_new_aligned = AlignPow2(pos_new, ARENA_COMMIT_SIZE);
		cmt_new_clamped = ClampTop(cmt_new_aligned, arena->res);
		cmt_new_size    = cmt_new_clamped - arena->cmt;
		os_commit((u8*)arena + arena->cmt, cmt_new_size);
		arena->cmt = cmt_new_clamped;
	}
	
	void *memory = 0;
	
	if (arena->cmt >= pos_new) 
	{
		memory = (u8*)arena + pos_mem;
		arena->used = pos_new;
	}
	
	return memory;
}

function ArenaTemp arenaTempBegin(Arena *arena)
{
	ArenaTemp out = {
		.arena = arena,
		.pos = arena->used,
	};
	return out;
}

function void arenaTempEnd(ArenaTemp *temp)
{
	for(u32 i = temp->pos; i < temp->arena->used; i ++)
	{
		u8 *base = (u8*)temp->arena;
		base[i] = 0;
	}
	temp->arena->used = temp->pos;
}

function Arena *arenaAlloc(u64 cmt, u64 res)
{
	Arena *arena = 0;
	
	void *memory = os_reserve(res);
	os_commit(memory, cmt);
	
	arena = (Arena*)memory;
	arena->used = ARENA_HEADER_SIZE;
	arena->align = DEFAULT_ALIGN;
	
	arena->cmt = AlignPow2(cmt, os_getPageSize());
	arena->res = res;
	
	return arena;
}

function Arena *arenaAlloc()
{
	return arenaAlloc(ARENA_COMMIT_SIZE, ARENA_RESERVE_SIZE);
}

#include <math.h>

// TODO(mizu): If sse 4.1 isn't available, don't include these and make rewrite functions
// that use intrinsics.

#include <xmmintrin.h>
#include <smmintrin.h>

#define PI (3.1415926535897f)
#define DEG_TO_RAD(deg) (deg * PI / 180.f)

typedef union v2s v2s;
union v2s
{
	s32 e[2];
	struct
	{
		s32 x;
		s32 y;
	};
};

typedef union v2f v2f;
union v2f
{
	f32 e[2];
	struct
	{
		f32 x;
		f32 y;
	};
};

typedef union v3f v3f;
union v3f
{
	f32 e[3];
	struct
	{
		f32 x;
		f32 y;
		f32 z;
	};
	struct
	{
		v2f xy;
		f32 _z;
	};
};

typedef union v4f v4f;
union v4f
{
	f32 e[4];
	
	struct
	{
		v3f xyz;
		f32 aw;
	};
	
	struct
	{
		f32 x;
		f32 y;
		f32 z;
		f32 w;
	};
	
};

function void operator*=(v4f &a, f32 b)
{
	a.x *= b;
	a.y *= b;
	a.z *= b;
	a.w *= b;
}

function v4f operator*(v4f a, f32 b)
{
	v4f out = {};
	out.x = a.x * b;
	out.y = a.y * b;
	out.z = a.z * b;
	out.w = a.w * b;
	return out;
}

function v4f operator-(v4f a, v4f b)
{
	v4f out = {};
	out.x = a.x - b.x;
	out.y = a.y - b.y;
	out.z = a.z - b.z;
	out.w = a.w - b.w;
	return out;
}

typedef union m4f m4f;
union m4f
{
	f32 e[4][4];
};

typedef struct m4f_ortho_proj m4f_ortho_proj;
struct m4f_ortho_proj
{
	m4f fwd;
	m4f inv;
};

function s32 floor_f32_to_s32(f32 a)
{
	s32 res = _mm_cvtss_si32(_mm_floor_ss(_mm_setzero_ps(), _mm_set_ss(a)));
	return res;
}

function f32 v2f_dist_sq(v2f a, v2f b)
{
	f32 out = (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
	return out;
}

function b32 v2s_equals(v2s a, v2s b)
{
	return a.x == b.x && a.y == b.y;
}

function b32 operator==(v2s a, v2s b)
{
	return a.x == b.x && a.y == b.y;
}

function v2s operator-(v2s a, v2s b)
{
	v2s out = {};
	out.x = a.x - b.x,
	out.y = a.y - b.y;
	
	return out;
}

function v2s operator+(v2s a, v2s b)
{
	v2s out = {};
	out.x = a.x + b.x,
	out.y = a.y + b.y;
	
	return out;
}

function void operator+=(v2s &a, v2s b)
{
	a.x += b.x;
	a.y += b.y;
}

function void operator-=(v2s &a, v2s b)
{
	a.x -= b.x;
	a.y -= b.y;
}

function v2f operator+(v2f a, f32 b)
{
	return(v2f){
		{
			a.x + b,
			a.y + b
		}
	};
}

function v2f operator-(v2f a, f32 b)
{
	return(v2f){
		{
			a.x - b,
			a.y - b
		}
	};
}

function v2f operator*(v2f a, f32 b)
{
	return(v2f){
		{
			a.x * b,
			a.y * b
		}
	};
}

function v2f operator/(v2f a, f32 b)
{
	return(v2f){
		{
			a.x / b,
			a.y / b
		}
	};
}

function void operator+=(v2f& a, f32 b)
{
	a.x += b;
	a.y += b;
}

function void operator*=(v2f &a, f32 b)
{
	a.x *= b;
	a.y *= b;
}

function v2f operator+(v2f a, v2f b)
{
	return (v2f){
		{
			a.x + b.x,
			a.y + b.y
		}
		
	};
}

function v2f operator-(v2f a, v2f b)
{
	return (v2f){
		{
			a.x - b.x,
			a.y - b.y
		}
		
	};
}

function void operator+=(v2f &a, v2f b)
{
	a.x += b.x;
	a.y += b.y;
}

function void operator-=(v2f &a, v2f b)
{
	a.x -= b.x;
	a.y -= b.y;
}

function f32 v3f_len(v3f v)
{
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

function v3f v3f_normalize(v3f v)
{
	f32 l = v3f_len(v);
	
	return v3f{
		{
			v.x/l,
			v.y/l,
			v.z/l
		}};
}

function f32 v3f_dot(v3f a, v3f b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

function v3f v3f_cross(v3f a, v3f b)
{
	v3f result;
	result.x = a.y * b.z - a.z * b.y;
	result.y = a.z * b.x - a.x * b.z;
	result.z = a.x * b.y - a.y * b.x;
	return result;
}

function v3f operator*(v3f a, f32 b)
{
	return (v3f){
		{
			a.x * b,
			a.y * b,
			a.z * b
		}
		
	};
}

function v3f operator*(f32 a, v3f b)
{
	return b * a;
}

function void operator+=(v3f &a, v3f b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
}

function void operator*=(v3f &a, f32 b)
{
	a.x *= b;
	a.y *= b;
	a.z *= b;
}

function v3f operator+(v3f a, v3f b)
{
	return (v3f){
		{
			a.x + b.x,
			a.y + b.y,
			a.z + b.z
		}
		
	};
}

function v3f operator-(v3f a, v3f b)
{
	return v3f{{
			a.x - b.x,
			a.y - b.y,
			a.z - b.z
		}};
}

function b32 operator==(v3f a, v3f b)
{
	if(a.x == b.x && a.y == b.y && a.z == b.z)
	{
		return true;
	}
	else
	{
		return false;
	}
}

function v4f operator*(m4f a, v4f b)
{
	v4f out = {};
	
	for(u32 r = 0; r < 4; r ++)
	{
		out.e[r] =
			a.e[r][0] * b.e[0] +
			a.e[r][1] * b.e[1] +
			a.e[r][2] * b.e[2] +
			a.e[r][3] * b.e[3];
	}
	
	return out;
}

function m4f m4f_identity()
{
	return (m4f){
		{
			{1, 0, 0, 0},
			{0, 1, 0, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 1},
		}
	};
}

function m4f m4f_make_trans(v3f v)
{
	return (m4f){
		{
			{1, 0, 0, v.x},
			{0, 1, 0, v.y},
			{0, 0, 1, v.z},
			{0, 0, 0, 1},
		}
	};
}

function m4f m4f_make_trans(f32 x, f32 y, f32 z)
{
	return (m4f){
		{
			{1, 0, 0, x},
			{0, 1, 0, y},
			{0, 0, 1, z},
			{0, 0, 0, 1},
		}
	};
}

function m4f m4f_make_scale(v3f v)
{
	return (m4f){
		{
			{v.x, 0, 0, 0},
			{0, v.y, 0, 0},
			{0, 0, v.z, 0},
			{0, 0, 0, 1},
		}
	};
}

// ty billy madison
#undef near
#undef far

function m4f m4f_make_perspective(f32 fov, f32 aspect, f32 near, f32 far)
{
	f32 tan_half_fov = tan(fov / 2.0f);
	f32 z_range = near - far;
	
	return (m4f){
		{
			{1.0f / (aspect * tan_half_fov), 0, 0, 0},
			{0, 1.0f / tan_half_fov, 0, 0},
			{0, 0, (far + near) / z_range, 2 * far * near / z_range},
			{0, 0, -1, 0}
		}
	};
}

function m4f m4f_make_scale(f32 s)
{
	return (m4f){
		{
			{s, 0, 0, 0},
			{0, s, 0, 0},
			{0, 0, s, 0},
			{0, 0, 0, 1},
		}
	};
}

function m4f m4f_make_rot_x(f32 rad)
{
	float c = cos(rad);
	float s = sin(rad);
	
	return (m4f){
		{
			{1, 0, 0, 0},
			{0, c, -s, 0},
			{0, s, c, 0},
			{0, 0, 0, 1}
		}
	};
}

function m4f m4f_make_rot_y(f32 rad)
{
	float c = cos(rad);
	float s = sin(rad);
	
	return (m4f){
		{
			{c, 0, s, 0},
			{0, 1, 0, 0},
			{-s, 0, c, 0},
			{0, 0, 0, 1}
		}
	};
}

function m4f m4f_make_rot_z(f32 rad)
{
	float c = cos(rad);
	float s = sin(rad);
	
	return (m4f){
		{
			{c,-s, 0, 0},
			{s, c, 0, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 1}
		}
	};
}

function m4f_ortho_proj m4f_ortho(f32 left,f32 right,f32 bottom, f32 top, f32 _near, f32 _far)
{
	f32 rpl = right + left;
	f32 rml = right - left;
	f32 tpb = top + bottom;
	f32 tmb = top - bottom;
	f32 fpn = _far + _near;
	f32 fmn = _far - _near;
	
	m4f fwd = {{
			{2/rml,     0,          0,       -rpl/rml},
			{0,         2/tmb,      0,       -tpb/tmb},
			{0,         0,          -2/fmn,   - fpn/fmn},
			{0, 0,  0, 1}
		}};
	
	m4f inv = {{
			{rml/2,   0,       0,       rpl/2},
			{0,       tmb/2,   0,       tpb/2},
			{0,       0,       -fmn/2,  -fpn/2},
			{0,   0,   0,   1}
		}};
	
	return m4f_ortho_proj{fwd, inv};
}

function m4f m4f_look_at(v3f eye, v3f center, v3f up)
{
	v3f z = v3f_normalize(center - eye);
	v3f x = v3f_normalize(v3f_cross(z, up));
	v3f y = v3f_cross(x, z);
	
	m4f mat = {
		{
			{x.x, y.x, -z.x, -v3f_dot(x,eye)},
			{x.y, y.y, -z.y, -v3f_dot(y,eye)},
			{x.z, y.z, -z.z, -v3f_dot(z,eye)},
			{0,0,0, 1}
		}
	};
	
	return mat;
}

function m4f operator*(m4f a, m4f b)
{
	m4f out = {};
	
	for(u32 r = 0; r < 4; r ++)
	{
		for(u32 c = 0; c < 4; c ++)
		{
			out.e[r][c] =
				a.e[r][0] * b.e[0][c] +
				a.e[r][1] * b.e[1][c] +
				a.e[r][2] * b.e[2][c] +
				a.e[r][3] * b.e[3][c];
		}
	}
	
	return out;
}

#define str8_varg(S) (int)((S).len), ((S).c)

struct Str8
{
	u8 *c;
	u64 len;
};

#define str8_lit(c) Str8{(u8*)c, sizeof(c) - 1}

function u64 cstr8Len(char *c)
{
	u64 out = 0;
	while(*c++)
	{
		out++;
	}
	return out;
}

function Str8 str8(u8 *c, u64 len)
{
	Str8 out = 
	{
		c,len
	};
	return out;
}

function void str8_cpy(Str8 *dst, Str8 *src)
{
	for(u32 i = 0; i < src->len; i ++)
	{
		dst->c[i] = src->c[i];
	}
}

function Str8 push_str8fv(Arena *arena, char *fmt, va_list args)
{
	Str8 out = {};
	va_list args_copy;
	va_copy(args_copy, args);
	
	int bytes_req = stbsp_vsnprintf(0, 0, fmt, args) + 1;
	
	out.c = push_array(arena, u8, bytes_req);
	
	out.len = stbsp_vsnprintf((char *)out.c, bytes_req, fmt, args_copy);
	va_end(args_copy);
	
	return out;
}

function Str8 push_str8f(Arena *arena, char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	Str8 result = push_str8fv(arena, fmt, args);
	va_end(args);
	return(result);
}

function b32 str8_equals(Str8 a, Str8 b)
{
	b32 res = 1;
	
	if(a.len == b.len)
	{
		for(u32 i = 0; i < a.len; i++)
		{
			if(a.c[i] != b.c[i])
			{
				res = 0;
				break;
			}
		}
	}
	else
	{
		res = 0;
	}
	
	return res;
}

function Str8 str8_join(Arena *arena, Str8 a, Str8 b)
{
	Str8 out = {};
	out.c = push_array(arena, u8, a.len + b.len);
	
	memcpy(out.c, a.c, a.len);
	
	memcpy((u8*)out.c + a.len, b.c, b.len);
	//printf("%s %lu\r\n", b.c, b.len);
	
	out.len = a.len + b.len;
	return out;
}

enum DEBUG_CYCLE_COUNTER
{
	DEBUG_CYCLE_COUNTER_UPDATE_AND_RENDER,
	DEBUG_CYCLE_COUNTER_PATHFINDING,
	DEBUG_CYCLE_COUNTER_PF_GET_NEIGHBORS,
	DEBUG_CYCLE_COUNTER_PF_CONTAINS_NODE,
	DEBUG_CYCLE_COUNTER_PF_LOWEST_FCOST,
	DEBUG_CYCLE_COUNTER_PF_PREPARE_PATH,
	DEBUG_CYCLE_COUNTER_PF_REVERSE_PATH,
	DEBUG_CYCLE_COUNTER_COUNT
};

read_only char *debug_cycle_to_str[DEBUG_CYCLE_COUNTER_COUNT] = 
{
	"update & render",
	"pathfinding",
	"pf get neighbours",
	"pf contains node",
	"lowest cost",
	"prepare path",
	"reverse path"
};

struct debug_cycle_counter
{
	u64 cycle_count;
	u32 hit_count;
};

struct TCXT
{
	Arena *arena;
	Arena *arenas[2];
	debug_cycle_counter counters[DEBUG_CYCLE_COUNTER_COUNT];
	debug_cycle_counter counters_last[DEBUG_CYCLE_COUNTER_COUNT];
};

global TCXT *tcxt;

#if defined (OS_WIN32) || defined(OS_LINUX)
#define BEGIN_TIMED_BLOCK(ID) u64 start_cycle_count_##ID = __rdtsc(); ++tcxt->counters[DEBUG_CYCLE_COUNTER_##ID].hit_count
#define END_TIMED_BLOCK(ID)  tcxt->counters[DEBUG_CYCLE_COUNTER_##ID].cycle_count += __rdtsc() - start_cycle_count_##ID

#else
#define BEGIN_TIMED_BLOCK(ID)
#define END_TIMED_BLOCK(ID)
#endif

function void tcxt_init()
{
	Arena *arena = arenaAlloc();
	tcxt = push_struct(arena, TCXT);
	tcxt->arena = arena;
	for(u32 i = 0; i < ARRAY_LEN(tcxt->arenas); i ++)
	{
		tcxt->arenas[i] = arenaAlloc(Megabytes(10), Megabytes(64));
	}
}

function void tcxt_process_debug_counters()
{
	for(u32 i = 0; i < ARRAY_LEN(tcxt->counters); i ++)
	{
		debug_cycle_counter *counter = tcxt->counters + i;
		debug_cycle_counter *counter_last = tcxt->counters_last + i;
		
		counter_last->hit_count = counter->hit_count;
		counter_last->cycle_count = counter->cycle_count;
		
		//printf("%d: %lu\n", i, counter->cycle_count);
		counter->hit_count = 0;
		counter->cycle_count = 0;
	}
}

function void tcxt_print_debug_counters()
{
	for(u32 i = 0; i < ARRAY_LEN(tcxt->counters); i ++)
	{
		debug_cycle_counter *counter = tcxt->counters_last + i;
		
		printf("%s: %llu\n", debug_cycle_to_str[i], counter->cycle_count);
	}
}

function Arena *tcxt_get_scratch(Arena **conflicts, u64 count)
{
	Arena *out = 0;
	for(u32 i = 0; i < ARRAY_LEN(tcxt->arenas); i ++)
	{
		b32 has_conflict = 0;
		for(u32 j = 0; j < count; j ++)
		{
			if(tcxt->arenas[i] == conflicts[j])
			{
				has_conflict = 1;
				break;
			}
		}
		if(!has_conflict)
		{
			out = tcxt->arenas[i];
		}
	}
	
	return out;
}

#define scratch_begin(conflicts, count) arenaTempBegin(tcxt_get_scratch(conflicts, count))
#define scratch_end(scratch) arenaTempEnd(scratch);

struct Rect
{
	v2f tl;
	v2f br;
};

function Rect rect(f32 tl_x, f32 tl_y, f32 br_x, f32 br_y)
{
	Rect out = {};
	
	out.tl.x = tl_x;
	out.tl.y = tl_y;
	
	out.br.x = br_x;
	out.br.y = br_y;
	
	return out;
}

#define rect_tl_varg(v) (v).tl.x, (v).tl.y
#define rect_br_varg(v) (v).br.x, (v).br.y
#define rect_varg(v) rect_tl_varg(v), rect_br_varg(v)

#define v4f_varg(v) (v).x, (v).y, (v).z, (v).w

function Rect rect(v2f pos, v2f scale)
{
	Rect out = {};
	out.tl.x = pos.x;
	out.tl.y = pos.y;
	
	out.br.x = out.tl.x + scale.x;
	out.br.y = out.tl.y + scale.y;
	
	return out;
}

function v2f size_from_rect(Rect rect)
{
	v2f out = {};
	out.x = rect.br.x - rect.tl.x;
	out.y = rect.br.y - rect.tl.y;
	
	return out;
}

function v2f center_from_rect(Rect rect)
{
	v2f out = {};
	
	out.x = (rect.br.x + rect.tl.x) / 2.f;
	out.y = (rect.br.y + rect.tl.y) / 2.f;
	
	return out;
}

struct Bitmap
{
	void *data;
	s32 w;
	s32 h;
	s32 n;
};

struct Glyph
{
	u8 *bmp;
	s32 w;
	s32 h;
	v2s bearing;
	s32 x0, y0, x1, y1;
	s32 advance;
};

struct Atlas
{
	Glyph glyphs[256];
};

enum FILE_TYPE
{
	FILE_TYPE_TEXT,
	FILE_TYPE_BINARY,
	FILE_TYPE_COUNT
};

struct File_data
{
	u8 *bytes;
	u64 size;
};

#if defined(OS_WIN32)
#define _file_open(file, filepath, mode) fopen_s(file, filepath, mode)
#define _sleep(len) Sleep(len)
#elif defined (OS_LINUX) || defined (OS_APPLE)
#define _file_open(file, filepath, mode) *file = fopen(filepath, mode)
#define _sleep(len) sleep(len)
#endif

function File_data read_file(Arena *arena, const char *filepath, FILE_TYPE type)
{
	File_data out = {};
	FILE *file;
	
	local_persist char *file_type_table[FILE_TYPE_COUNT] = 
	{
		"r",
		"rb"
	};
	
	_file_open(&file, filepath, file_type_table[type]);
	
	fseek(file, 0, SEEK_END);
	
	out.size = ftell(file);
	//print("%d", len);
	
	fseek(file, 0, SEEK_SET);
	
	out.bytes = push_array(arena, u8, out.size);
	fread(out.bytes, sizeof(u8), out.size, file);
	
	fclose(file);
	
	return out;
}

function void write_file(const char *filepath, FILE_TYPE type, void *data, size_t size)
{
	FILE *file;
	
	local_persist char *file_type_table[FILE_TYPE_COUNT] = 
	{
		"w",
		"wb"
	};
	
	_file_open(&file, filepath, file_type_table[type]);
	
	fwrite(data, size, 1, file);
	
	fclose(file);
	
}

function b32 clone_file(const char* sourcePath, const char* destinationPath)
{
	b32 out = 0;
	
	FILE* sourceFile, * destinationFile;
	char buffer[4096];
	size_t bytesRead;
	
	_file_open(&sourceFile, sourcePath, "rb");
	
	if(sourceFile)
	{
		_file_open(&destinationFile, destinationPath, "wb");
		
		if(destinationFile)
		{
			while ((bytesRead = fread(buffer, 1, sizeof(buffer), sourceFile)) > 0) 
			{
				fwrite(buffer, 1, bytesRead, destinationFile);
			}
			
			fclose(sourceFile);
			fclose(destinationFile);
			
			out = 1;
		}
	}
	
	return out;
}

function Bitmap bitmap(Str8 path)
{
	Bitmap out = {};
	
	stbi_set_flip_vertically_on_load(true);
	
	out.data = stbi_load((char*)path.c, &out.w, &out.h, &out.n, 0);
	
	if(!out.data)
	{
		//printf("\nError loading%s :%s\n", path.c, stbi_failure_reason());
		//INVALID_CODE_PATH();
	}
	
	return out;
}

function Glyph *glyphFromCodepoint(Atlas *atlas, char c)
{
	Glyph *out = atlas->glyphs + (u32)c;
	return out;
}

function Glyph *make_bmp_font(u8* path, char *codepoints, u32 num_cp, Arena* arena)
{
	u8 *file_data = read_file(arena, (char*)path, FILE_TYPE_BINARY).bytes;
	
	stbtt_fontinfo font;
	stbtt_InitFont(&font, (u8*)file_data, stbtt_GetFontOffsetForIndex((u8*)file_data,0));
	
	Glyph *out = push_array(arena, Glyph, num_cp);
	for(u32 i = 0; i < num_cp; i++)
	{
		s32 w,h,xoff,yoff;
		f32 size = stbtt_ScaleForPixelHeight(&font, 64);
		
		u8* bmp = stbtt_GetCodepointBitmap(&font, 0, size, codepoints[i] ,&w,&h, &xoff, &yoff);
		
		stbtt_GetCodepointHMetrics(&font, codepoints[i], &out[i].advance, &out[i].bearing.x);
		out[i].w = w;
		out[i].h = h;
		
		s32 x0, y0, x1, y1;
		stbtt_GetCodepointBox(&font, codepoints[i], &x0, &y0, &x1, &y1);
		
		out[i].bearing.y = y0;
		
		out[i].x0 = x0;
		out[i].y0 = y0;
		out[i].x1 = x1;
		out[i].y1 = y1;
		
		out[i].bmp = push_array(arena,u8,w * h * 4);
		
		u8* src_row = bmp + w*(h-1);
		u8* dest_row = out[i].bmp;
		
		for(s32 y = 0; y < h; y ++)
		{
			u32 *dest = (u32*)dest_row;
			u8 *src = src_row;
			for(s32 x = 0; x < w; x ++)
			{
				u8 alpha = *src++;
				
				*dest++ = ((alpha <<24) |
															(0xFF <<16) |
															(0xFF << 8) |
															(0xFF));
			}
			dest_row += 4 * w;
			src_row -= w;
		}
		
		stbtt_FreeBitmap(bmp, 0);
	}
	
	return out;
}

function Str8 file_name_from_path(Arena *arena, Str8 path)
{
	char *cur = (char*)&path.c[path.len - 1];
	u32 count = 0;
	
	//NOTE(mizu): pig
	while(*cur != '/' && *cur != '\\' && *cur != '\0')
	{
		cur--;
		count++;
	}
	
	Str8 file_name_cstr = {};
	file_name_cstr.c = push_array(arena, u8, count + 1);
	file_name_cstr.len = count + 1;
	memcpy(file_name_cstr.c, cur + 1, count);
	file_name_cstr.c[count] = '\0';
	
	return file_name_cstr;
}

// NOTE(mizu): sys/stat works on windows because its a compatibility thing. I don't know how safe this is.
function time_t get_file_last_modified_time(char* pathname)
{
	struct stat stat_buf;
	if(stat(pathname,&stat_buf) == -1 )
	{
		printf("stat failed for %s", pathname);
		perror("error: ");
		return 0;
	}
	
	return stat_buf.st_mtime;
}