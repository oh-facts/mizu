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

vec3 rgb2hsv(vec3 rgb) {
    float maxComponent = max(rgb.r, max(rgb.g, rgb.b));
    float minComponent = min(rgb.r, min(rgb.g, rgb.b));
    float diff = maxComponent - minComponent;
    float hue = 0.0;
    
    if (maxComponent == rgb.r) {
        hue = (rgb.g - rgb.b) / diff;
    } else if (maxComponent == rgb.g) {
        hue = 2.0 + (rgb.b - rgb.r) / diff;
    } else if (maxComponent == rgb.b) {
        hue = 4.0 + (rgb.r - rgb.g) / diff;
    }
    
    hue = fract(hue / 6.0);
    float saturation = (maxComponent == 0.0) ? 0.0 : (diff / maxComponent);
    float value = maxComponent;
    
    return vec3(hue, saturation, value);
}


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
        {{ obj.dst.br.x, obj.dst.br.y}, {obj.src.br.x, obj.src.tl.y}, obj.fade[Corner_01]},
        {{ obj.dst.tl.x, obj.dst.br.y}, {obj.src.tl.x, obj.src.tl.y}, obj.fade[Corner_11]},
    };

    Vertex vertex = vertices[gl_VertexID];

    texId = obj.handle.sprite_id;
tex_size.x = obj.handle.w;
    tex_size.y = obj.handle.h;
 tint = obj.tint;
    fade = vec4(rgb2hsv(vertex.fade.xyz), 1);
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

vec3 hue2rgb(float hue) {
    hue = fract(hue);
    float r = abs(hue * 6.0 - 3.0) - 1.0;
    float g = 2.0 - abs(hue * 6.0 - 2.0);
    float b = 2.0 - abs(hue * 6.0 - 4.0);
    vec3 rgb = vec3(r, g, b);
    rgb = clamp(rgb, 0.0, 1.0);
    return rgb;
}

vec3 hsv2rgb(vec3 hsv) {
    vec3 rgb = hue2rgb(hsv.x); 
    rgb = mix(vec3(1.0), rgb, hsv.y); 
    rgb = rgb * hsv.z; 
    return rgb;
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

#if 1
    if (tex_col.a < 0.01f)
    {
        discard;
    }
#endif

float diagonal = tex.x - tex.y;
	 vec3 col = hsv2rgb(vec3(diagonal, tex.x, tex.y));

FragColor =  srgb_to_linear(tint * tex_col);
//FragColor = vec4(hsv2rgb(fade.xyz), 1);
}
)"
;

#endif //RENDER_OPENGL_H
