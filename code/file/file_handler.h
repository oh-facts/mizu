/* date = August 12th 2024 6:03 pm */

#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

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

function Rect rect(v2f pos, v2f scale)
{
	Rect out = {};
	out.tl.x = pos.x;
	out.tl.y = pos.y;
	
	out.br.x = out.tl.x + scale.x;
	out.br.y = out.tl.y + scale.y;
	
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
		printf("\nError loading%s :%s\n", path.c, stbi_failure_reason());
		INVALID_CODE_PATH();
	}
	
	return out;
}

function Glyph *glyph_from_codepoint(Atlas *atlas, char c)
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
									 (alpha <<16) |
									 (alpha << 8) |
									 (alpha ));
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

enum HotReloadState
{
	HotReloadState_Null,
	HotReloadState_Requested,
	HotReloadState_Completed,
};

struct HotReload
{
	HotReloadState state;
	void *entry;
	void * game_dll;
	Str8 path;
	Str8 cloned_path;
	time_t reloaded_time;
};

// NOTE(mizu): sys/stat works on windows because its a compatibility thing. I don't know how safe this is.
function time_t get_file_last_modified_time(char* pathname)
{
	struct stat stat_buf;
	if(stat(pathname,&stat_buf) == -1 )
	{
		perror("stat failed");
		return 0;
	}
	
	return stat_buf.st_mtime;
}

function void load_game_dll(HotReload *reload)
{
	while(!clone_file((char*)reload->path.c, (char*)reload->cloned_path.c))
	{
		_sleep(10);
	}
	
	reload->game_dll = SDL_LoadObject((char*)reload->cloned_path.c);
	if (!reload->game_dll)
	{
		printf("dll not found, reload failed\n\r");
		INVALID_CODE_PATH();
	}
	
	reload->entry = (void*)SDL_LoadFunction(reload->game_dll, "update_and_render");
	reload->reloaded_time = get_file_last_modified_time((char*)reload->path.c);
}

function void hot_reload(HotReload *reload)
{
	SDL_UnloadObject(reload->game_dll);
	load_game_dll(reload);
}

#endif //FILE_HANDLER_H
