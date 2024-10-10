#include "ui_gen.h"
#include "meta.h"

#define ui_make_alloc_node(Name, name) \
function UI_##Name##_node *ui_alloc_##name##_node(UI_Context *cxt) \
{ \
UI_##Name##_node *node = cxt->stacks.name##_stack.free;\
if(node)\
{\
cxt->stacks.name##_stack.free = cxt->stacks.name##_stack.free->next;\
*node = {};\
}\
else\
{\
node = push_struct(cxt->arena, UI_##Name##_node);\
}\
return node;\
}

#define ui_make_free_node(Name, name) \
function void ui_free_##name##_node(UI_Context *cxt, UI_##Name##_node *node)\
{\
node->next = cxt->stacks.name##_stack.free;\
cxt->stacks.name##_stack.free = node;\
}

#define ui_make_push_style(Name, name, Type) \
function void ui_push_##name(UI_Context *cxt, Type val) { \
UI_##Name##_node *node = ui_alloc_##name##_node(cxt);\
node->v = val; \
if (!cxt->name##_stack.top) { \
cxt->name##_stack.top = node; \
} else { \
node->next = cxt->name##_stack.top; \
cxt->name##_stack.top = node; \
} \
}

#define ui_make_set_next_style(Name, name, Type) \
function void ui_set_next_##name(UI_Context *cxt, Type val) { \
UI_##Name##_node *node = ui_alloc_##name##_node(cxt); \
node->v = val; \
if (!cxt->name##_stack.top) { \
cxt->name##_stack.top = node; \
} else { \
node->next = cxt->name##_stack.top; \
cxt->name##_stack.top = node; \
} \
cxt->name##_stack.auto_pop = 1;\
}

#define ui_make_pop_style(Name, name) \
function void ui_pop_##name(UI_Context *cxt) { \
UI_##Name##_node *pop = cxt->name##_stack.top;\
cxt->name##_stack.top = cxt->name##_stack.top->next;\
ui_free_##name##_node(cxt, pop);\
}

int main()
{
	printf("//Alloc nodes\n\n");
	print_now();
	
	for(int i = 0; i < num_stacks; i++)
	{
		printf("function UI_%s_node *ui_alloc_%s_node(UI_Context *cxt)\n", stacks[i].Name, stacks[i].name);
		printf("{\n");
		printf("\tUI_%s_node *node = cxt->%s_stack.free;\n", stacks[i].Name, stacks[i].name);
		printf("\tif(node)\n");
		printf("\t{\n");
		printf("\t\tcxt->%s_stack.free = cxt->%s_stack.free->next;\n", stacks[i].name, stacks[i].name);
		printf("\t\t*node = {};\n");
		printf("\t}\n");
		
		printf("\telse\n");
		printf("\t{\n");
		printf("\t\tnode = push_struct(cxt->arena, UI_%s_node);\n", stacks[i].Name);
		printf("\t}\n");
		printf("\treturn node;\n");
		printf("}\n");
	}
	
	printf("//Free nodes\n\n");
	
	for(int i = 0; i < num_stacks; i++)
	{
		printf("function void ui_free_%s_node(UI_Context *cxt, UI_%s_node *node)\n", stacks[i].name, stacks[i].Name);
		printf("{\n");
		
		printf("\tnode->next = cxt->%s_stack.free;\n", stacks[i].name);
		printf("\tcxt->%s_stack.free = node;\n", stacks[i].name);
		
		printf("}\n");
	}
	
	printf("// Push style functions\n\n");
	
	for(int i = 0; i < num_stacks; i++)
	{
		printf("function void ui_push_%s(UI_Context *cxt, %s val)\n", stacks[i].name, stacks[i].type);
		printf("{\n");
		printf("\tUI_%s_node *node = ui_alloc_%s_node(cxt);\n", stacks[i].Name, stacks[i].name);
		printf("\tnode->v = val;\n");
		printf("\tif (!cxt->%s_stack.top)\n", stacks[i].name);
		printf("\t{\n");
		printf("\t\tcxt->%s_stack.top = node;\n", stacks[i].name);
		printf("\t}\n");
		printf("\telse\n");
		printf("\t{\n");
		printf("\t\tnode->next = cxt->%s_stack.top;\n", stacks[i].name);
		printf("\t\tcxt->%s_stack.top = node;\n", stacks[i].name);
		printf("\t}\n");
		printf("}\n\n");
	}
	
	printf("// Set next style functions\n\n");
	
	for(int i = 0; i < num_stacks; i++)
	{
		printf("function void ui_set_next_%s(UI_Context *cxt, %s val)\n", stacks[i].name, stacks[i].type);
		printf("{\n");
		printf("\tUI_%s_node *node = ui_alloc_%s_node(cxt);\n", stacks[i].Name, stacks[i].name);
		printf("\tnode->v = val;\n");
		printf("\tif (!cxt->%s_stack.top)\n", stacks[i].name);
		printf("\t{\n");
		printf("\t\tcxt->%s_stack.top = node;\n", stacks[i].name);
		printf("\t}\n");
		printf("\telse\n");
		printf("\t{\n");
		printf("\t\tnode->next = cxt->%s_stack.top;\n", stacks[i].name);
		printf("\t\tcxt->%s_stack.top = node;\n", stacks[i].name);
		printf("\t}\n");
		printf("\tcxt->%s_stack.auto_pop = 1;\n", stacks[i].name);
		printf("}\n\n");
	}
	
	printf("// Pop style functions\n\n");
	
	for(int i = 0; i < num_stacks; i++)
	{
		printf("function void ui_pop_%s(UI_Context *cxt)\n", stacks[i].name);
		printf("{\n");
		printf("\tUI_%s_node *pop = cxt->%s_stack.top;\n", stacks[i].Name, stacks[i].name);
		printf("\tcxt->%s_stack.top = cxt->%s_stack.top->next;\n", stacks[i].name, stacks[i].name);
		printf("\tui_free_%s_node(cxt, pop);\n", stacks[i].name);
		printf("}\n\n");
	}
	
	printf("// =================\n\n");
}