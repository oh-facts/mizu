#include <stdio.h>

enum Type
{
	Type_UI_Widget,
	Type_v4f,
	Type_f32,
	Type_v2f,
	Type_Axis2,
	Type_SizeKind,
	Type_COUNT
};

enum Name
{
	Name_Parent,
	Name_Color,
	Name_PrefWidth,
	Name_PrefHeight,
};

const char *Type_strings[Type_COUNT] = 
{
	"UI_Widget",
	"v4f",
	"f32",
	"v2f",
	"Axis2",
	"SizeKind",
};

struct Style
{
	const char *name;
	Type type;
};

#define Num_Styles 8

Style styles[Num_Styles] = 
{
	{"Parent", Type_UI_Widget},
	{"Color", Type_v4f},
	{"Pref_width", Type_f32},
	{"Pref_height", Type_f32},
	{"Fixed_pos", Type_v2f},
	{"Axis2", Type_Axis2},
	{"SizeKind_x", Type_SizeKind},
	{"SizeKind_y", Type_SizeKind},
};

int main()
{
	printf("// UI Style Structs\n\n");
	for(int i = 0; i < Num_Styles; i++)
	{
		printf("struct UI_%s_node\n{\n", styles[i].name);
		printf("\tUI_%s_node *next;\n", styles[i].name);
		printf("\t%s v;\n", Type_strings[styles[i].type]);
		printf("};\n\n");
	}
	printf("// =================\n\n");
	
	printf("// UI Style Struct stack\n\n1");
	for(int i = 0; i < Num_Styles; i++)
	{
		printf("struct\n{\n");
		printf("\tUI_%s_node *top;\n", styles[i].name);
		printf("\tUI_%s_node *free;\n", styles[i].name);
		printf("\tb32 auto_pop;\n");
		printf("};\n\n");
	}
	printf("// =================\n\n");
}

#define ui_make_style_struct(Name, Type) \
struct UI_##Name##_node \
{ \
UI_##Name##_node *next; \
Type v;\
};

#define ui_make_style_struct_stack(Name, name) \
struct \
{\
UI_##Name##_node *top;\
UI_##Name##_node *free;\
b32 auto_pop;\
}name##_stack;
