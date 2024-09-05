/* date = July 18th 2024 10:07 am */

#ifndef RENDER_OPENGL_H
#define RENDER_OPENGL_H

#define R_DEBUG 1

enum R_OPENGL_INST_BUFFER
{
	R_OPENGL_INST_BUFFER_UI,
	R_OPENGL_INST_BUFFER_FB,
	R_OPENGL_INST_BUFFER_COUNT,
};

enum R_OPENGL_SHADER_PROG
{
	R_OPENGL_SHADER_PROG_UI,
	R_OPENGL_SHADER_PROG_FB,
	R_OPENGL_SHADER_PROG_COUNT,
};

struct R_Opengl_state
{
	Arena *arena;
	GLuint shader_prog[R_OPENGL_SHADER_PROG_COUNT];
	GLuint inst_buffer[R_OPENGL_INST_BUFFER_COUNT];
};

global R_Opengl_state *r_opengl_state;

function void APIENTRY glDebugOutput(GLenum source, 
																		 GLenum type, 
																		 unsigned int id, 
																		 GLenum severity, 
																		 GLsizei length, 
																		 const char *message, 
																		 const void *userParam);


function void check_compile_errors(GLuint shader, const char *type);
function void check_link_errors(GLuint shader, const char *type);
function GLuint r_opengl_make_shader_program(char *vertexShaderSource, char *fragmentShaderSource);
function GLuint r_opengl_make_buffer(size_t size);

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
    vec4 tint;
vec4 fade[Corner_COUNT];
    R_Handle handle;
};

layout (std430, binding = 0) buffer ssbo {
    vec4 screen_size;
    TextObject objects[];
};

out vec4 tint;
out vec4 fade;
out vec2 tex;
flat out uvec2 texId;
flat out vec2 tex_size;

void main()
{
    TextObject obj = objects[gl_InstanceID];

    Vertex vertices[] = {
        {{ obj.dst.tl.x, obj.dst.tl.y}, {obj.src.tl.x, obj.src.br.y}, obj.fade[Corner_00]},
        {{ obj.dst.br.x, obj.dst.tl.y}, {obj.src.br.x, obj.src.br.y}, obj.fade[Corner_10]},
        {{ obj.dst.br.x, obj.dst.br.y}, {obj.src.br.x, obj.src.tl.y}, obj.fade[Corner_11]},
        {{ obj.dst.tl.x, obj.dst.br.y}, {obj.src.tl.x, obj.src.tl.y}, obj.fade[Corner_01]},
    };

    Vertex vertex = vertices[gl_VertexID];

    texId = obj.handle.sprite_id;
tex_size.x = obj.handle.w;
    tex_size.y = obj.handle.h;
 tint = obj.tint;
    fade = vertex.fade;

tex = vertex.uv;
    vec2 norm_pos = vertex.pos / screen_size.xy * 2.0 - 1.0;
    norm_pos.y =  - norm_pos.y;

    gl_Position = vec4(norm_pos, 0.5, 1.0);
}

)"
;

global char* r_fs_ui_src = 
R"(
	#version 450 core
	#extension GL_ARB_bindless_texture: require

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

in vec4 tint;
 in vec4 fade;
in vec2 tex;
flat in uvec2 texId;
flat in vec2 tex_size;

out vec4 FragColor;

void main()
{
		vec4 tex_col = texture(sampler2D(texId), tex);

#if 0
		if (tex_col.a < 0.01f && fade.a < 0.01f)
		{
				discard;
		}
#endif

//FragColor =  tint * tex_col * fade;
//FragColor =  srgb_to_linear(tint * tex_col);
//FragColor = vec4(hsvToRgb(fade.xyz), 1);
FragColor = fade * tex_col;
}
)"
;

#endif //RENDER_OPENGL_H
