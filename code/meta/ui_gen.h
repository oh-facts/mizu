/* date = October 4th 2024 2:13 pm */

#ifndef UI_GEN_H
#define UI_GEN_H

struct Style
{
	const char *name;
	const char *type;
};

#define ui_make_style_struct(Name, Type) \
struct UI_##Name##_node \
{ \
UI_##Name##_node *next; \
Type v;\
};

#define num_styles 10
static Style styles[num_styles] =
{ 
	{"Parent", "UI_Widget *"},
	{"Color", "v4f"},
	{"Pref_width", "f32"},
	{"Pref_height", "f32"},
	{"Fixed_pos", "v2f"},
	{"float_value", "f32"},
	
	{"AlignKind_x", "UI_AlignKind"},
	
	{"Axis2", "Axis2"},
	
	{"SizeKind_x", "UI_SizeKind"},
	{"SizeKind_y", "UI_SizeKind"},
};

struct Stacks
{
	const char *Name;
	const char *name;
	const char *type;
};

#define ui_make_style_struct_stack(Name, name) \
struct \
{\
UI_##Name##_node *top;\
UI_##Name##_node *free;\
b32 auto_pop;\
}name##_stack;

#define num_stacks 16
static Stacks stacks[num_stacks] =
{
	{"Parent", "parent", "UI_Widget*"},
	{"Color", "text_color", "v4f"},
	{"Color", "bg_color", "v4f"},
	{"Color", "hover_color", "v4f"},
	{"Color", "press_color", "v4f"},
	{"Color", "border_color", "v4f"},
	{"float_value", "border_thickness", "f32"},
	{"float_value", "radius", "f32"},
 {"float_value", "scale", "f32"},
	
	{"Pref_width", "pref_width", "f32"},
	{"Pref_height", "pref_height", "f32"},
	{"Fixed_pos", "fixed_pos", "v2f"},
	{"Axis2", "child_layout_axis", "Axis2"},
	{"SizeKind_x", "size_kind_x", "UI_SizeKind"},
	{"SizeKind_y", "size_kind_y", "UI_SizeKind"},
	{"AlignKind_x", "align_kind_x", "UI_AlignKind"},
};

#endif //UI_GEN_H
