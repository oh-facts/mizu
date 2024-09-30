#include <stdio.h>

struct Style
{
	const char *Name;
	const char *Type;
};

#define num_styles 
static Style styles[num_styles] =
{ 
	{"Parent", "UI_Widget"},
	{"Color", "v4f"},
	{"Pref_width", "f32"},
	{"Pref_height", "f32"},
	{"Pref_height", "f32"},
	
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
