void *r_push_batch_(Arena *arena, R_Batch_list *list, u64 size)
{
	R_Batch *batch = list->last;
	
	if(!batch || (batch->used + size > batch->size))
	{
		batch = r_push_batch_list(arena, list);
	}
	
	if(!batch->base)
	{
		batch->base = push_array(arena, u8, size * 1000);
		batch->size = size * 1000;
	}
	
	void *out = batch->base + batch->used;
	batch->used += size;
	batch->count++;
	
	return out;
}

R_Batch *r_push_batch_list(Arena *arena, R_Batch_list *list)
{
	R_Batch *node = push_struct(arena, R_Batch);
	list->num ++;
	if(list->last)
	{
		list->last = list->last->next = node;
	}
	else
	{
		list->last = list->first = node;
	}
	
	return node;
}

R_Pass *r_push_pass_list(Arena *arena, R_Pass_list *list, R_PASS_KIND kind)
{
	R_Pass_node *node = push_struct(arena, R_Pass_node);
	list->num ++;
	if(list->last)
	{
		list->last = list->last->next = node;
	}
	else
	{
		list->last = list->first = node;
	}
	node->pass.kind = kind;
	R_Pass *pass = &node->pass;
	return pass;
}