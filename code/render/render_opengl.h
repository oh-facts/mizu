/* date = July 18th 2024 10:07 am */

#ifndef RENDER_OPENGL_H
#define RENDER_OPENGL_H

#define R_DEBUG 1

enum R_OPENGL_INST_BUFFER
{
	R_OPENGL_INST_BUFFER_UI,
	R_OPENGL_INST_BUFFER_COUNT,
};

enum R_OPENGL_SHADER_PROG
{
	R_OPENGL_SHADER_PROG_UI,
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

global u32 sprite_draw_indices[] = {
  0,1,3,
  1,2,3
};

global char* r_vs_ui_src =
R"(
#version 450 core
	
	struct Rect
	{
vec2 tl;
vec2 br;
};

struct Vertex 
{
	vec2 pos;
	vec2 uv;
};

struct TextObject
{
	Rect src;
Rect dst;
vec4 color;
uvec2 sprite_id;
uvec2 padd;
};

layout (std430, binding = 0) buffer ssbo {
	 mat4 proj;
TextObject objects[];
};

out vec4 col;
out vec2 tex;
flat out uvec2 texId;

void main()
{
	
	TextObject obj = objects[gl_InstanceID];

	Vertex vertices[] = {
		{{ obj.dst.tl.x, obj.dst.tl.y}, {obj.src.tl.x, obj.src.br.y}},
		{{ obj.dst.br.x, obj.dst.tl.y}, {obj.src.br.x, obj.src.br.y}},
		{{ obj.dst.br.x, obj.dst.br.y}, {obj.src.br.x, obj.src.tl.y}},
		{{ obj.dst.tl.x, obj.dst.br.y}, {obj.src.tl.x, obj.src.tl.y}},
};

	Vertex vertex = vertices[gl_VertexID];
	
texId = obj.sprite_id;
col = obj.color;
tex = vertex.uv;
	gl_Position =  vec4(vertex.pos, 0.5, 1.0) * proj;// * obj.model;
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
	in vec4 col;
	in vec2 tex;
	flat in uvec2 texId;
 
	out vec4 FragColor;
	void main()
	{
		vec4 tex_col = texture(sampler2D(texId), tex);
#if 1
		if(tex_col.a < 0.01f)
		{
			discard;
		}
#endif

		FragColor = col * tex_col;
	}
)"
;

#endif //RENDER_OPENGL_H
