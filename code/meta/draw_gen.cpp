#include <stdio.h>

#define Num_Styles 3

struct Style
{
  const char *Name;
  const char *name;
  const char *type;
};

Style styles[Num_Styles] = 
{
	{"Proj_view", "proj_view", "m4f"},
	{"Target", "target", "R_Handle"},
  {"Viewport", "viewport", "Rect"},
};

int main()
{
	printf("// Draw Cmd Style Structs\n\n");
	
  for(int i = 0; i < Num_Styles; i++)
	{
		printf("struct D_%s_node\n{\n", styles[i].Name);
		printf("\tD_%s_node *next;\n", styles[i].Name);
		printf("\t%s v;\n", styles[i].type);
		printf("};\n\n");
	}
	printf("// =================\n\n");
	
	printf("// Draw Cmd push pop\n\n");
	for(int i = 0; i < Num_Styles; i++)
	{
		printf("function void d_push_%s(%s v)\n", styles[i].name, styles[i].type);
    printf("{\n");
    
    printf("\tD_Bucket *bucket = d_state->top;\n");
    printf("\tD_%s_node *node = push_struct(d_state->arena, D_%s_node);\n", styles[i].Name, styles[i].Name);
    printf("\tnode->v = v;\n\n");
    
    printf("\tif(bucket->%s_top)\n\t{\n", styles[i].name);
    printf("\t\tnode->next = bucket->%s_top;\n", styles[i].name);
    printf("\t}\n");
    printf("\telse\n\t{\n");
    printf("\t\tbucket->%s_top = node;\n", styles[i].name);
    printf("\t}\n");
    printf("\tbucket->stack_gen++;\n");
    
    printf("}\n\n");
    
    printf("function void d_pop_%s()\n", styles[i].name);
    printf("{\n");
    printf("\tD_Bucket *bucket = d_state->top;\n");
    printf("\tbucket->%s_top = bucket->%s_top->next;\n", styles[i].name, styles[i].name);
    printf("\tbucket->stack_gen++;\n");
    printf("}\n\n");
    
	}
	printf("// =================\n\n");
}