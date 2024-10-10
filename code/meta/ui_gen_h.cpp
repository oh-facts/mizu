#include "ui_gen.h"
#include "meta.h"

int main()
{
	printf("// Style Structs\n\n");
	print_now();
	
	for (int i = 0; i < num_styles; i++)
	{
		printf("struct UI_%s_node\n", styles[i].name);
		printf("{\n");
		printf("\tUI_%s_node *next;\n", styles[i].name);
		printf("\t%s v;\n", styles[i].type);
		printf("};\n\n");
	}
	
	printf("// Style Stack Structs\n\n");
	
	printf("#define UI_STYLE_STACKS \\\n");
	for (int i = 0; i < num_stacks; i++)
	{
		printf("struct\\\n");
		printf("{\\\n");
		printf("\tUI_%s_node *top;\\\n", stacks[i].Name);
		printf("\tUI_%s_node *free;\\\n", stacks[i].Name);
		printf("\tb32 auto_pop;\\\n");
		printf("}%s_stack;\\\n", stacks[i].name);
		printf(";");
	}
}