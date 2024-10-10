// Draw Cmd Style Structs

struct D_Proj_view_node
{
	D_Proj_view_node *next;
	m4f v;
};

struct D_Target_node
{
	D_Target_node *next;
	R_Handle v;
};

struct D_Viewport_node
{
	D_Viewport_node *next;
	Rect v;
};

// =================

// Draw Cmd push pop

function void d_push_proj_view(m4f v)
{
	D_Bucket *bucket = d_state->top;
	D_Proj_view_node *node = push_struct(d_state->arena, D_Proj_view_node);
	node->v = v;

	if(bucket->proj_view_top)
	{
		node->next = bucket->proj_view_top;
		bucket->proj_view_top = node;
	}
	else
	{
		bucket->proj_view_top = node;
	}
	bucket->stack_gen++;
}

function void d_pop_proj_view()
{
	D_Bucket *bucket = d_state->top;
	bucket->proj_view_top = bucket->proj_view_top->next;
	bucket->stack_gen++;
}

function void d_push_target(R_Handle v)
{
	D_Bucket *bucket = d_state->top;
	D_Target_node *node = push_struct(d_state->arena, D_Target_node);
	node->v = v;

	if(bucket->target_top)
	{
		node->next = bucket->target_top;
		bucket->target_top = node;
	}
	else
	{
		bucket->target_top = node;
	}
	bucket->stack_gen++;
}

function void d_pop_target()
{
	D_Bucket *bucket = d_state->top;
	bucket->target_top = bucket->target_top->next;
	bucket->stack_gen++;
}

function void d_push_viewport(Rect v)
{
	D_Bucket *bucket = d_state->top;
	D_Viewport_node *node = push_struct(d_state->arena, D_Viewport_node);
	node->v = v;

	if(bucket->viewport_top)
	{
		node->next = bucket->viewport_top;
		bucket->viewport_top = node;
	}
	else
	{
		bucket->viewport_top = node;
	}
	bucket->stack_gen++;
}

function void d_pop_viewport()
{
	D_Bucket *bucket = d_state->top;
	bucket->viewport_top = bucket->viewport_top->next;
	bucket->stack_gen++;
}

// =================

