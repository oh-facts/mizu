
#define R_DEBUG 1

enum R_OPENGL_INST_BUFFER
{
	R_OPENGL_INST_BUFFER_UI,
	R_OPENGL_INST_BUFFER_MESH,
	R_OPENGL_INST_BUFFER_SPRITE,
	
	R_OPENGL_INST_BUFFER_COUNT,
};

enum R_OPENGL_SHADER_PROG
{
	R_OPENGL_SHADER_PROG_UI,
	R_OPENGL_SHADER_PROG_MESH,
	R_OPENGL_SHADER_PROG_SPRITE,
	
	R_OPENGL_SHADER_PROG_COUNT,
};

struct R_Opengl_state
{
	Arena *arena;
	
	GLuint shader_prog[R_OPENGL_SHADER_PROG_COUNT];
	GLuint inst_buffer[R_OPENGL_INST_BUFFER_COUNT];
};

global R_Opengl_state *r_opengl_state;

global u32 quad_draw_indices[] = {
	0,1,3,
	1,2,3
};

global char *r_vs_fb_src = 
R"(
#version 450 core

struct Vertex
{
	vec2 pos;
	vec2 uv;
};

layout (std430, binding = 1) buffer ssbo {
	uvec2 tex_id;
	uvec2 screen_size;
};

flat out uvec2 a_texId;
out vec2 a_uv;
void main()
{
	
	Vertex vertices[] = {
		{{ 0, 0}, {0, 1}},
		{{ 2, 0}, {1, 1}},
		{{ 2, 2}, {1, 0}},
		{{ 0, 2}, {0, 0}},
	};

	a_texId = tex_id;
	Vertex vertex = vertices[gl_VertexID];
	a_uv = vertex.uv;
	
	vec2 norm_pos = (vertex.pos - 1);// / screen_size.xy * 2.0 - 1.0;
	norm_pos.y =  - norm_pos.y;

	gl_Position = vec4(norm_pos, 0.5, 1.0);
}

)";

global char* r_fs_fb_src = 
R"(
	#version 450 core
	#extension GL_ARB_bindless_texture: require

	flat in uvec2 a_texId;
	in vec2 a_uv;
	out vec4 FragColor;

void main()
{
	vec4 tex_col = texture(sampler2D(a_texId), a_uv);

	FragColor = tex_col;
}
)"
;

global char *r_vs_ui_src =
R"(
#version 450 core

#define Corner_00 0
#define Corner_01 1
#define Corner_10 2
#define Corner_11 3
#define Corner_COUNT 4

struct R_Handle
{
	uvec2 sprite_id;
	uint pad1;
	uint w;
	uint h;
	uint pad2;
	uvec2 pad3;
};

struct Rect
{
	vec2 tl;
	vec2 br;
};

struct Vertex 
{
	vec2 pos;
	vec2 uv;
	vec4 fade;
};

struct TextObject
{
	Rect src;
	Rect dst;
	vec4 border_color;
	vec4 fade[Corner_COUNT];
	R_Handle handle;
	float border_thickness;
	float radius;
	float pad[2];
};

layout (std430, binding = 0) buffer ssbo {
	vec4 screen_size;
	mat4 proj;
	TextObject objects[];
};

out vec4 border_color;
out vec4 fade;
out vec2 tex;
flat out uvec2 texId;
flat out vec2 tex_size;
flat out float border_thickness;
flat out vec2 half_size;
flat out float radius;
out vec2 norm_tex;

void main()
{
	TextObject obj = objects[gl_InstanceID];

	vec2 base_uv[] = 
	{
		{0, 1},
		{1, 1},
		{1, 0},
		{0, 0},
	};

	norm_tex = base_uv[gl_VertexID];

	Vertex vertices[] = 
	{
		{{ obj.dst.tl.x, obj.dst.tl.y}, {obj.src.tl.x, obj.src.br.y}, obj.fade[Corner_00]},
		{{ obj.dst.br.x, obj.dst.tl.y}, {obj.src.br.x, obj.src.br.y}, obj.fade[Corner_10]},
		{{ obj.dst.br.x, obj.dst.br.y}, {obj.src.br.x, obj.src.tl.y}, obj.fade[Corner_11]},
		{{ obj.dst.tl.x, obj.dst.br.y}, {obj.src.tl.x, obj.src.tl.y}, obj.fade[Corner_01]},
	};

	half_size = vec2((obj.dst.br.x - obj.dst.tl.x) * 0.5, (obj.dst.br.y - obj.dst.tl.y) * 0.5);

	Vertex vertex = vertices[gl_VertexID];

	texId = obj.handle.sprite_id;
	tex_size.x = obj.handle.w;
	tex_size.y = obj.handle.h;
	border_color = obj.border_color;
	fade = vertex.fade;
	border_thickness = obj.border_thickness;
	radius = obj.radius;
	tex = vertex.uv;
	vec2 norm_pos = vertex.pos / screen_size.xy * 2.0 - 1.0;
	norm_pos.y =  - norm_pos.y;
	gl_Position = vec4(norm_pos, 0.5, 1.0) * proj;
}

)"
;

global char* r_fs_ui_src = 
R"(
	#version 450 core
	#extension GL_ARB_bindless_texture: require

	in vec4 border_color;
	in vec4 fade;
	in vec2 tex;
	flat in uvec2 texId;
	flat in vec2 tex_size;
	flat in float border_thickness;
	flat in vec2 half_size;
	flat in float radius;
	in vec2 norm_tex;

	out vec4 FragColor;

float RectSDF(vec2 p, vec2 b, float r)
{
	vec2 d = abs(p) - b + vec2(r);
	return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - r;   
}

void main() 
{
	vec4 tex_col = texture(sampler2D(texId), tex);

	vec2 pos = half_size * 2 * norm_tex;
		
	float fDist = RectSDF(pos - half_size, half_size - border_thickness/2.0, radius);
	float fBlendAmount = smoothstep(-1.0, 0.0, abs(fDist) - border_thickness / 2.0);
  
	vec4 v4FromColor = border_color;
	vec4 v4ToColor = (fDist < 0.0) ? fade * tex_col : vec4(0);
	FragColor = mix(v4FromColor, v4ToColor, fBlendAmount);
}


)"
;

// sprite shader

global char *r_vs_sprite_src =
R"(
#version 450 core

#define Corner_00 0
#define Corner_01 1
#define Corner_10 2
#define Corner_11 3
#define Corner_COUNT 4

struct R_Handle
{
	uvec2 sprite_id;
	uint pad1;
	uint w;
	uint h;
	uint pad2;
	uvec2 pad3;
};

struct Rect
{
	vec2 tl;
	vec2 br;
};

struct Vertex 
{
	vec2 pos;
	vec2 uv;
	vec4 fade;
};

struct Sprite
{
	Rect src;
	Rect dst;
	vec4 border_color;
	vec4 fade[Corner_COUNT];
	R_Handle handle;
	float border_thickness;
	float radius;
	vec2 basis;
	float pad[3];
	float depth; // epic crash
};

layout (std430, binding = 0) buffer ssbo {
	vec4 screen_size;
	mat4 proj;
	Sprite objects[];
};

out vec4 border_color;
out vec4 fade;
out vec2 tex;
flat out uvec2 texId;
flat out vec2 tex_size;
flat out float border_thickness;
flat out vec2 half_size;
flat out float radius;
out vec2 norm_tex;

void main()
{
	Sprite obj = objects[gl_InstanceID];

	vec2 base_uv[] = 
	{
		{0, 1},
		{1, 1},
		{1, 0},
		{0, 0},
	};

	norm_tex = base_uv[gl_VertexID];

	Vertex vertices[] = 
	{
		{{ obj.dst.tl.x, obj.dst.tl.y}, {obj.src.tl.x, obj.src.br.y}, obj.fade[Corner_00]},
		{{ obj.dst.br.x, obj.dst.tl.y}, {obj.src.br.x, obj.src.br.y}, obj.fade[Corner_10]},
		{{ obj.dst.br.x, obj.dst.br.y}, {obj.src.br.x, obj.src.tl.y}, obj.fade[Corner_11]},
		{{ obj.dst.tl.x, obj.dst.br.y}, {obj.src.tl.x, obj.src.tl.y}, obj.fade[Corner_01]},
	};

	half_size = vec2((obj.dst.br.x - obj.dst.tl.x) * 0.5, (obj.dst.br.y - obj.dst.tl.y) * 0.5);

	Vertex vertex = vertices[gl_VertexID];

	texId = obj.handle.sprite_id;
	tex_size.x = obj.handle.w;
	tex_size.y = obj.handle.h;
	border_color = obj.border_color;
	fade = vertex.fade;
	border_thickness = obj.border_thickness;
	radius = obj.radius;
	tex = vertex.uv;
	
	// if we use screen size to normalize, we are basically saying fuck you to the
	// projection / view matrix. camera is doing this math specifically for us
	// for ui its ok since everything is wrt the screen, but for the game world, its not
	vec2 norm_pos = vertex.pos / screen_size.xy * 2.0 - 1.0;
	norm_pos.y =  - norm_pos.y;
	norm_pos = vertex.pos;
	gl_Position = vec4(norm_pos, 0, 1) * proj;
}

)"
;

global char* r_fs_sprite_src = 
R"(
#version 450 core
#extension GL_ARB_bindless_texture: require

in vec4 border_color;
in vec4 fade;
in vec2 tex;
flat in uvec2 texId;
flat in vec2 tex_size;
flat in float border_thickness;
flat in vec2 half_size;
flat in float radius;
in vec2 norm_tex;

out vec4 FragColor;

float RectSDF(vec2 p, vec2 b, float r)
{
	vec2 d = abs(p) - b + vec2(r);
	return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - r;   
}

void main() 
{
	vec4 tex_col = texture(sampler2D(texId), tex);

	vec2 pos = half_size * 2 * norm_tex;
		
	float fDist = RectSDF(pos - half_size, half_size - border_thickness/2.0, radius);
  
	if(tex_col.a < 0.1 || fDist > 1)
	{
		discard;
	}

	float fBlendAmount = smoothstep(-1.0, 0.0, abs(fDist) - border_thickness / 2.0);
  
	vec4 v4FromColor = border_color;
	vec4 v4ToColor = (fDist < 0.0) ? fade * tex_col : vec4(0);

	FragColor = mix(v4FromColor, v4ToColor, fBlendAmount);
}


)"
;

// mesh shader

global char *r_vs_mesh_src =
R"(
#version 450 core

struct Vertex 
{
	vec3 pos;
	float uv_x;
	vec3 norm;
	float uv_y;
	vec4 color;
};

layout (std430, binding = 0) buffer ssbo 
{
	Vertex vertices[];
};

layout (std430, binding = 1) buffer ssbo1 
{
	mat4 proj_view;
	mat4 model;
	uvec2 sprite_id;
	uvec2 pad;
};

out vec3 a_norm;
out vec2 a_tex;
flat out uvec2 a_sprite_id;
out vec4 a_color;
void main()
{
	Vertex vtx = vertices[gl_VertexID];
	a_norm = vtx.norm;
	a_tex.x = vtx.uv_x;
	a_tex.y = vtx.uv_y;
	a_sprite_id = sprite_id;
	a_color = vtx.color;

	gl_Position = vec4(vtx.pos, 1) *  model * proj_view;
}

)"
;

global char* r_fs_mesh_src = 
R"(
#version 450 core
#extension GL_ARB_bindless_texture: require

out vec4 FragColor;
in vec3 a_norm;
in vec2 a_tex;
flat in uvec2 a_sprite_id;
in vec4 a_color;
void main() 
{
	vec4 tex_col = texture(sampler2D(a_sprite_id), a_tex);

	//FragColor = a_color;
	FragColor = tex_col;
	//FragColor = vec4(a_tex.x, a_tex.y, 0, 1);
}

)"
;

/*
 
vec4 linear_to_srgb(vec4 linearCol) {
		vec4 srgbCol;
		for (int i = 0; i < 3; ++i) {
				if (linearCol[i] <= 0.0031308) {
						srgbCol[i] = 12.92 * linearCol[i];
				} else {
						srgbCol[i] = 1.055 * pow(linearCol[i], 1.0 / 2.4) - 0.055;
				}
		}
srgbCol[3] = linearCol[3];
		return srgbCol;
}
vec4 srgb_to_linear(vec4 srgbCol) {
		vec4 linearCol;
		for (int i = 0; i < 3; ++i) {
				if (srgbCol[i] <= 0.04045) {
						linearCol[i] = srgbCol[i] / 12.92;
				} else {
						linearCol[i] = pow((srgbCol[i] + 0.055) / 1.055, 2.4);
				}
		}
		linearCol[3] = srgbCol[3];
return linearCol;
}


if (tex.x < border_thickness || tex.x > (1 - border_thickness) || tex.y < border_thickness || tex.y > (1 - border_thickness)) 
{
	FragColor = fade * tex_col;
}
*/


// ============= //

function void APIENTRY glDebugOutput(GLenum source, 
                            		 GLenum type, 
								     unsigned int id, 
									 GLenum severity, 
									 GLsizei length, 
									 const char *message, 
									 const void *userParam)
{
	if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 
	
	printf("---------------\n");
	printf("Debug Message (%u): %s\n", id, message);
	
	printf("Source:");
	switch (source)
	{
		case GL_DEBUG_SOURCE_API:
		{
			printf("api");
		}break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		{
			printf("Window system");
		}break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
		{
			printf("Shader compiler");
		}break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:
		{
			printf("third party");
		}break;
		case GL_DEBUG_SOURCE_APPLICATION:
		{
			printf("application");
		}break;
		case GL_DEBUG_SOURCE_OTHER:
		{
			printf("other");
		}break;
		default:
		{
			INVALID_CODE_PATH();
		}
	}
	
	printf("\n");
	
	printf("Type:");
	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR:
		{
			printf("Error");
		}break;;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		{
			printf("Deprecated behaviour");
		}break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  
		{
			printf("undefined behaviour");
		}break;
		case GL_DEBUG_TYPE_PORTABILITY:
		{
			printf("portability");
		}break;
		case GL_DEBUG_TYPE_PERFORMANCE:
		{
			printf("performance");
		}break;
		case GL_DEBUG_TYPE_MARKER:
		{
			printf("marker");
		}break;
		case GL_DEBUG_TYPE_PUSH_GROUP:
		{
			printf("push group");
		}break;
		case GL_DEBUG_TYPE_POP_GROUP:
		{
			printf("pop group");
		}break;
		case GL_DEBUG_TYPE_OTHER:
		{
			printf("other");
		}break;
		default:
		{
			INVALID_CODE_PATH();
		}break;
	}
	printf("\n");
	
	printf("severity: ");
	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:
		{
			printf("high");
		}break;
		case GL_DEBUG_SEVERITY_MEDIUM:
		{
			printf("medium");
		}break;
		case GL_DEBUG_SEVERITY_LOW:
		{
			printf("low");
		}break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
		{
			printf("notification");
		}break;
	}
	printf("\n");
}

function void checkCompileErrors(GLuint shader, const char *type)
{
	int success;
	char infoLog[1024];
	
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 1024, 0, infoLog);
		printf("\n%s compilation error:\n%s\n", type, infoLog);
		INVALID_CODE_PATH();
	}
}

function void checkLinkErrors(GLuint shader, const char *type)
{
	int success;
	char infoLog[1024];
	glGetProgramiv(shader, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shader, 1024, 0, infoLog);
		printf("\n%s linking error:\n%s\n", type, infoLog);
		INVALID_CODE_PATH();
	}
}

function GLuint r_opengl_makeShaderProgram(char *vertexShaderSource, char *fragmentShaderSource)
{
	GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert_shader, 1, &vertexShaderSource, 0);
	glCompileShader(vert_shader);
	checkCompileErrors(vert_shader, "vertex shader");
	
	GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag_shader, 1, &fragmentShaderSource, 0);
	glCompileShader(frag_shader);
	checkCompileErrors(frag_shader, "fragment shader");
	
	GLuint shader_prog = glCreateProgram();
	glAttachShader(shader_prog, vert_shader);
	glAttachShader(shader_prog, frag_shader);
	
	glLinkProgram(shader_prog);
	checkLinkErrors(shader_prog, "vert/frag shader");
	
	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);
	
	return shader_prog;
}

enum R_BufferKind
{
	R_BufferKind_Null,
	R_BufferKind_SSBO,
	R_BufferKind_Index,
	R_BufferKind_COUNT
};

function GLuint r_opengl_makeBuffer(size_t size)
{
	GLuint ssbo = 0;
	
	// use discard buffer?
	
	glCreateBuffers(1, &ssbo);
	glNamedBufferData(ssbo, size, 0, GL_STREAM_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
	
	return ssbo;
}

function GLuint r_opengl_makeBuffer(void *data, size_t size)
{
	GLuint buffer = 0;
	
	glCreateBuffers(1, &buffer);
	glNamedBufferData(buffer, size, data, GL_DYNAMIC_COPY);
	
	return buffer;
}

function void r_opengl_init()
{
	Arena *arena = arenaAlloc();
	r_opengl_state = push_struct(arena, R_Opengl_state);
	r_opengl_state->arena = arena;
	
#if R_DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
	glDebugMessageCallback(glDebugOutput, 0);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
#endif
	
	r_opengl_state->shader_prog[R_OPENGL_SHADER_PROG_UI] = r_opengl_makeShaderProgram(r_vs_ui_src, r_fs_ui_src);
	r_opengl_state->inst_buffer[R_OPENGL_INST_BUFFER_UI] = r_opengl_makeBuffer(Megabytes(8));
	
	r_opengl_state->shader_prog[R_OPENGL_SHADER_PROG_MESH] = r_opengl_makeShaderProgram(r_vs_mesh_src, r_fs_mesh_src);
	r_opengl_state->inst_buffer[R_OPENGL_INST_BUFFER_MESH] = r_opengl_makeBuffer(Megabytes(8));
	
	r_opengl_state->shader_prog[R_OPENGL_SHADER_PROG_SPRITE] = r_opengl_makeShaderProgram(r_vs_sprite_src, r_fs_sprite_src);
	r_opengl_state->inst_buffer[R_OPENGL_INST_BUFFER_SPRITE] = r_opengl_makeBuffer(Megabytes(8));
}

function R_Handle r_allocTexture(void *data, s32 w, s32 h, s32 n, R_Texture_params *p)
{
	R_Handle out = {};
	
	GLuint texture;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture);
	
	local_persist GLenum filter_table[R_TEXTURE_FILTER_COUNT] = 
	{
		GL_NEAREST,
		GL_LINEAR,
	};
	
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, filter_table[p->min]);
	glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, filter_table[p->max]);
	
	local_persist GLenum wrap_table[R_TEXTURE_WRAP_COUNT] = 
	{
		GL_CLAMP_TO_BORDER,
		GL_REPEAT,
	};
	
	glTextureParameteri(texture, GL_TEXTURE_WRAP_S, wrap_table[p->wrap]);
	glTextureParameteri(texture, GL_TEXTURE_WRAP_T, wrap_table[p->wrap]);
	
	glTextureStorage2D(texture, 1, GL_SRGB8_ALPHA8, w, h);
	
	if(data)
	{
		GLenum channels = GL_RGBA;
		
		if(n == 3)
		{
			channels = GL_RGB;
		}
		
		glTextureSubImage2D(texture, 0, 0, 0, w, h, channels, GL_UNSIGNED_BYTE, data);
	}
	
	GLuint64 gpu_handle = glGetTextureHandleARB(texture);
	glMakeTextureHandleResidentARB(gpu_handle);
	
	out.u64_m[0] = gpu_handle;
	out.u32_m[2] = texture;
	out.u32_m[3] = w;
	out.u32_m[4] = h;
	return out;
}

function void r_freeTexture(R_Handle handle)
{
	glMakeTextureHandleNonResidentARB(handle.u64_m[0]);
	
	GLuint tex = handle.u32_m[2];
	glDeleteTextures(1, &tex);
}

function R_Handle r_allocFramebuffer(s32 w, s32 h)
{
	GLuint fbo;
	glCreateFramebuffers(1, &fbo);
	
	// color attachment
	GLuint tex;
	glCreateTextures(GL_TEXTURE_2D, 1, &tex);
	glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureStorage2D(tex, 1, GL_RGB8, w, h);
	glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, tex, 0);
	
	// depth attachment
	GLuint depthTex;
	glCreateTextures(GL_TEXTURE_2D, 1, &depthTex);
	glTextureParameteri(depthTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(depthTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(depthTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(depthTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureStorage2D(depthTex, 1, GL_DEPTH_COMPONENT24, w, h);
	glNamedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, depthTex, 0);
	
	auto status = glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER);
	
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("frame buffer creation failed");
		INVALID_CODE_PATH();
	}
	
	GLuint64 gpu_handle = glGetTextureHandleARB(tex);
	glMakeTextureHandleResidentARB(gpu_handle);
	
	R_Handle out = {};
	out.u64_m[0] = gpu_handle;
	out.u32_m[2] = fbo;
	out.u32_m[3] = w;
	out.u32_m[4] = h;
	
	return out;
}

function void r_submit(OS_Window *win, R_PassList *list)
{
	SDL_GL_MakeCurrent(win->raw, win->gl_cxt);
	
	f32 color[3] = {1,0,1};
	glClearNamedFramebufferfv(0, GL_COLOR, 0, color);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	
	for(R_PassNode *node = list->first; node; node = node->next)
	{
		R_Pass *pass = &node->pass;
		
		switch(pass->kind)
		{
			default:{}break;
			case R_PASS_KIND_UI:
			{
				R_BatchList *batches = &pass->rect_pass.rects;
				
				v4f win_size_float = {};
				
				if(pass->rect_pass.target.u64_m[0] == 0)
				{
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					win_size_float.x = win->w;
					win_size_float.y = win->h;
				}
				else
				{
					glBindFramebuffer(GL_FRAMEBUFFER, pass->rect_pass.target.u32_m[2]);
					glClearNamedFramebufferfv(pass->rect_pass.target.u32_m[2], GL_COLOR, 0, color);
					win_size_float.x = pass->rect_pass.target.u32_m[3];
					win_size_float.y = pass->rect_pass.target.u32_m[4];
				}
				
				glViewport(0, 0, win_size_float.x, win_size_float.y);
				
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, r_opengl_state->inst_buffer[R_OPENGL_INST_BUFFER_UI]);
				
				R_Batch *batch = batches->first;
				
				glUseProgram(r_opengl_state->shader_prog[R_OPENGL_SHADER_PROG_UI]);
				
				for(u32 j = 0; j < batches->num; j++)
				{
					void *ssbo_data = glMapNamedBufferRange(r_opengl_state->inst_buffer[R_OPENGL_INST_BUFFER_UI], 0, sizeof(v4f) + sizeof(m4f) + batch->count * sizeof(R_Rect), GL_MAP_WRITE_BIT | 
																																													GL_MAP_INVALIDATE_BUFFER_BIT);
					
					m4f mat = pass->rect_pass.proj_view;
					//m4f mat = m4f_make_scale({{0.5, 0.5, 0.5}});
					
					memcpy(ssbo_data, &win_size_float, sizeof(win_size_float));
					memcpy((u8*)ssbo_data + sizeof(win_size_float), &mat, sizeof(m4f));
					memcpy((u8*)ssbo_data + sizeof(win_size_float) + sizeof(m4f), batch->base, batch->count * sizeof(R_Rect));
					
					glUnmapNamedBuffer(r_opengl_state->inst_buffer[R_OPENGL_INST_BUFFER_UI]);
					
					glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, quad_draw_indices, batch->count);
					batch = batch->next;
				}
				
			}break;
			
			case R_PASS_KIND_SPRITE:
			{
				R_BatchList *batches = &pass->sprite_pass.sprites;
				
				v4f win_size_float = {};
				
				if(pass->sprite_pass.target.u64_m[0] == 0)
				{
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					win_size_float.x = win->w;
					win_size_float.y = win->h;
				}
				else
				{
					glBindFramebuffer(GL_FRAMEBUFFER, pass->rect_pass.target.u32_m[2]);
					glClearNamedFramebufferfv(pass->sprite_pass.target.u32_m[2], GL_COLOR, 0, color);
					win_size_float.x = pass->sprite_pass.target.u32_m[3];
					win_size_float.y = pass->sprite_pass.target.u32_m[4];
				}
				
				glViewport(0, 0, win_size_float.x, win_size_float.y);
				
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, r_opengl_state->inst_buffer[R_OPENGL_INST_BUFFER_SPRITE]);
				
				glUseProgram(r_opengl_state->shader_prog[R_OPENGL_SHADER_PROG_SPRITE]);
				
				f32 clear_value = 0;
				glClearNamedFramebufferfv(pass->sprite_pass.target.u32_m[2], GL_DEPTH, 0, &clear_value);
				
				m4f mat = pass->sprite_pass.proj_view;
				//m4f mat = m4f_identity();
				//m4f mat = m4f_make_scale({{0.05, 0.05, 0.05}});
				
				R_Batch *batch = batches->first;
				
				for(u32 j = 0; j < batches->num; j++)
				{
					R_Sprite *first = (R_Sprite*)batch->base;
					
					qsort(first, batch->count, sizeof(R_Sprite), r_spriteSort);
					
					void *ssbo_data = glMapNamedBufferRange(r_opengl_state->inst_buffer[R_OPENGL_INST_BUFFER_SPRITE], 0, sizeof(v4f) + sizeof(m4f) + batch->count * sizeof(R_Sprite), GL_MAP_WRITE_BIT | 
																																													GL_MAP_INVALIDATE_BUFFER_BIT);
					
					memcpy(ssbo_data, &win_size_float, sizeof(win_size_float));
					memcpy((u8*)ssbo_data + sizeof(win_size_float), &mat, sizeof(m4f));
					memcpy((u8*)ssbo_data + sizeof(win_size_float) + sizeof(m4f), batch->base, batch->count * sizeof(R_Sprite));
					
					glUnmapNamedBuffer(r_opengl_state->inst_buffer[R_OPENGL_INST_BUFFER_SPRITE]);
					
					glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, quad_draw_indices, batch->count);
					batch = batch->next;
				}
				
			}break;
		}
	}
	
	SDL_GL_SwapWindow(win->raw);
}

function v2s r_texSizeFromHandle(R_Handle handle)
{
	v2s out = {};
	out.x = handle.u32_m[3];
	out.y = handle.u32_m[4];
	return out;
}