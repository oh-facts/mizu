//Alloc nodes

//2024-10-04 15:02:28

function UI_Parent_node *ui_alloc_parent_node(UI_Context *cxt)
{
	UI_Parent_node *node = cxt->parent_stack.free;
	if(node)
	{
		cxt->parent_stack.free = cxt->parent_stack.free->next;
		*node = {};
	}
	else
	{
		node = push_struct(cxt->arena, UI_Parent_node);
	}
	return node;
}
function UI_Color_node *ui_alloc_text_color_node(UI_Context *cxt)
{
	UI_Color_node *node = cxt->text_color_stack.free;
	if(node)
	{
		cxt->text_color_stack.free = cxt->text_color_stack.free->next;
		*node = {};
	}
	else
	{
		node = push_struct(cxt->arena, UI_Color_node);
	}
	return node;
}
function UI_Color_node *ui_alloc_bg_color_node(UI_Context *cxt)
{
	UI_Color_node *node = cxt->bg_color_stack.free;
	if(node)
	{
		cxt->bg_color_stack.free = cxt->bg_color_stack.free->next;
		*node = {};
	}
	else
	{
		node = push_struct(cxt->arena, UI_Color_node);
	}
	return node;
}
function UI_Color_node *ui_alloc_hover_color_node(UI_Context *cxt)
{
	UI_Color_node *node = cxt->hover_color_stack.free;
	if(node)
	{
		cxt->hover_color_stack.free = cxt->hover_color_stack.free->next;
		*node = {};
	}
	else
	{
		node = push_struct(cxt->arena, UI_Color_node);
	}
	return node;
}
function UI_Color_node *ui_alloc_press_color_node(UI_Context *cxt)
{
	UI_Color_node *node = cxt->press_color_stack.free;
	if(node)
	{
		cxt->press_color_stack.free = cxt->press_color_stack.free->next;
		*node = {};
	}
	else
	{
		node = push_struct(cxt->arena, UI_Color_node);
	}
	return node;
}
function UI_Color_node *ui_alloc_border_color_node(UI_Context *cxt)
{
	UI_Color_node *node = cxt->border_color_stack.free;
	if(node)
	{
		cxt->border_color_stack.free = cxt->border_color_stack.free->next;
		*node = {};
	}
	else
	{
		node = push_struct(cxt->arena, UI_Color_node);
	}
	return node;
}
function UI_float_value_node *ui_alloc_border_thickness_node(UI_Context *cxt)
{
	UI_float_value_node *node = cxt->border_thickness_stack.free;
	if(node)
	{
		cxt->border_thickness_stack.free = cxt->border_thickness_stack.free->next;
		*node = {};
	}
	else
	{
		node = push_struct(cxt->arena, UI_float_value_node);
	}
	return node;
}
function UI_float_value_node *ui_alloc_radius_node(UI_Context *cxt)
{
	UI_float_value_node *node = cxt->radius_stack.free;
	if(node)
	{
		cxt->radius_stack.free = cxt->radius_stack.free->next;
		*node = {};
	}
	else
	{
		node = push_struct(cxt->arena, UI_float_value_node);
	}
	return node;
}
function UI_Pref_width_node *ui_alloc_pref_width_node(UI_Context *cxt)
{
	UI_Pref_width_node *node = cxt->pref_width_stack.free;
	if(node)
	{
		cxt->pref_width_stack.free = cxt->pref_width_stack.free->next;
		*node = {};
	}
	else
	{
		node = push_struct(cxt->arena, UI_Pref_width_node);
	}
	return node;
}
function UI_Pref_height_node *ui_alloc_pref_height_node(UI_Context *cxt)
{
	UI_Pref_height_node *node = cxt->pref_height_stack.free;
	if(node)
	{
		cxt->pref_height_stack.free = cxt->pref_height_stack.free->next;
		*node = {};
	}
	else
	{
		node = push_struct(cxt->arena, UI_Pref_height_node);
	}
	return node;
}
function UI_Fixed_pos_node *ui_alloc_fixed_pos_node(UI_Context *cxt)
{
	UI_Fixed_pos_node *node = cxt->fixed_pos_stack.free;
	if(node)
	{
		cxt->fixed_pos_stack.free = cxt->fixed_pos_stack.free->next;
		*node = {};
	}
	else
	{
		node = push_struct(cxt->arena, UI_Fixed_pos_node);
	}
	return node;
}
function UI_Axis2_node *ui_alloc_child_layout_axis_node(UI_Context *cxt)
{
	UI_Axis2_node *node = cxt->child_layout_axis_stack.free;
	if(node)
	{
		cxt->child_layout_axis_stack.free = cxt->child_layout_axis_stack.free->next;
		*node = {};
	}
	else
	{
		node = push_struct(cxt->arena, UI_Axis2_node);
	}
	return node;
}
function UI_SizeKind_x_node *ui_alloc_size_kind_x_node(UI_Context *cxt)
{
	UI_SizeKind_x_node *node = cxt->size_kind_x_stack.free;
	if(node)
	{
		cxt->size_kind_x_stack.free = cxt->size_kind_x_stack.free->next;
		*node = {};
	}
	else
	{
		node = push_struct(cxt->arena, UI_SizeKind_x_node);
	}
	return node;
}
function UI_SizeKind_y_node *ui_alloc_size_kind_y_node(UI_Context *cxt)
{
	UI_SizeKind_y_node *node = cxt->size_kind_y_stack.free;
	if(node)
	{
		cxt->size_kind_y_stack.free = cxt->size_kind_y_stack.free->next;
		*node = {};
	}
	else
	{
		node = push_struct(cxt->arena, UI_SizeKind_y_node);
	}
	return node;
}
function UI_AlignKind_x_node *ui_alloc_align_kind_x_node(UI_Context *cxt)
{
	UI_AlignKind_x_node *node = cxt->align_kind_x_stack.free;
	if(node)
	{
		cxt->align_kind_x_stack.free = cxt->align_kind_x_stack.free->next;
		*node = {};
	}
	else
	{
		node = push_struct(cxt->arena, UI_AlignKind_x_node);
	}
	return node;
}
//Free nodes

function void ui_free_parent_node(UI_Context *cxt, UI_Parent_node *node)
{
	node->next = cxt->parent_stack.free;
	cxt->parent_stack.free = node;
}
function void ui_free_text_color_node(UI_Context *cxt, UI_Color_node *node)
{
	node->next = cxt->text_color_stack.free;
	cxt->text_color_stack.free = node;
}
function void ui_free_bg_color_node(UI_Context *cxt, UI_Color_node *node)
{
	node->next = cxt->bg_color_stack.free;
	cxt->bg_color_stack.free = node;
}
function void ui_free_hover_color_node(UI_Context *cxt, UI_Color_node *node)
{
	node->next = cxt->hover_color_stack.free;
	cxt->hover_color_stack.free = node;
}
function void ui_free_press_color_node(UI_Context *cxt, UI_Color_node *node)
{
	node->next = cxt->press_color_stack.free;
	cxt->press_color_stack.free = node;
}
function void ui_free_border_color_node(UI_Context *cxt, UI_Color_node *node)
{
	node->next = cxt->border_color_stack.free;
	cxt->border_color_stack.free = node;
}
function void ui_free_border_thickness_node(UI_Context *cxt, UI_float_value_node *node)
{
	node->next = cxt->border_thickness_stack.free;
	cxt->border_thickness_stack.free = node;
}
function void ui_free_radius_node(UI_Context *cxt, UI_float_value_node *node)
{
	node->next = cxt->radius_stack.free;
	cxt->radius_stack.free = node;
}
function void ui_free_pref_width_node(UI_Context *cxt, UI_Pref_width_node *node)
{
	node->next = cxt->pref_width_stack.free;
	cxt->pref_width_stack.free = node;
}
function void ui_free_pref_height_node(UI_Context *cxt, UI_Pref_height_node *node)
{
	node->next = cxt->pref_height_stack.free;
	cxt->pref_height_stack.free = node;
}
function void ui_free_fixed_pos_node(UI_Context *cxt, UI_Fixed_pos_node *node)
{
	node->next = cxt->fixed_pos_stack.free;
	cxt->fixed_pos_stack.free = node;
}
function void ui_free_child_layout_axis_node(UI_Context *cxt, UI_Axis2_node *node)
{
	node->next = cxt->child_layout_axis_stack.free;
	cxt->child_layout_axis_stack.free = node;
}
function void ui_free_size_kind_x_node(UI_Context *cxt, UI_SizeKind_x_node *node)
{
	node->next = cxt->size_kind_x_stack.free;
	cxt->size_kind_x_stack.free = node;
}
function void ui_free_size_kind_y_node(UI_Context *cxt, UI_SizeKind_y_node *node)
{
	node->next = cxt->size_kind_y_stack.free;
	cxt->size_kind_y_stack.free = node;
}
function void ui_free_align_kind_x_node(UI_Context *cxt, UI_AlignKind_x_node *node)
{
	node->next = cxt->align_kind_x_stack.free;
	cxt->align_kind_x_stack.free = node;
}
// Push style functions

function void ui_push_parent(UI_Context *cxt, UI_Widget* val)
{
	UI_Parent_node *node = ui_alloc_parent_node(cxt);
	node->v = val;
	if (!cxt->parent_stack.top)
	{
		cxt->parent_stack.top = node;
	}
	else
	{
		node->next = cxt->parent_stack.top;
		cxt->parent_stack.top = node;
	}
}

function void ui_push_text_color(UI_Context *cxt, v4f val)
{
	UI_Color_node *node = ui_alloc_text_color_node(cxt);
	node->v = val;
	if (!cxt->text_color_stack.top)
	{
		cxt->text_color_stack.top = node;
	}
	else
	{
		node->next = cxt->text_color_stack.top;
		cxt->text_color_stack.top = node;
	}
}

function void ui_push_bg_color(UI_Context *cxt, v4f val)
{
	UI_Color_node *node = ui_alloc_bg_color_node(cxt);
	node->v = val;
	if (!cxt->bg_color_stack.top)
	{
		cxt->bg_color_stack.top = node;
	}
	else
	{
		node->next = cxt->bg_color_stack.top;
		cxt->bg_color_stack.top = node;
	}
}

function void ui_push_hover_color(UI_Context *cxt, v4f val)
{
	UI_Color_node *node = ui_alloc_hover_color_node(cxt);
	node->v = val;
	if (!cxt->hover_color_stack.top)
	{
		cxt->hover_color_stack.top = node;
	}
	else
	{
		node->next = cxt->hover_color_stack.top;
		cxt->hover_color_stack.top = node;
	}
}

function void ui_push_press_color(UI_Context *cxt, v4f val)
{
	UI_Color_node *node = ui_alloc_press_color_node(cxt);
	node->v = val;
	if (!cxt->press_color_stack.top)
	{
		cxt->press_color_stack.top = node;
	}
	else
	{
		node->next = cxt->press_color_stack.top;
		cxt->press_color_stack.top = node;
	}
}

function void ui_push_border_color(UI_Context *cxt, v4f val)
{
	UI_Color_node *node = ui_alloc_border_color_node(cxt);
	node->v = val;
	if (!cxt->border_color_stack.top)
	{
		cxt->border_color_stack.top = node;
	}
	else
	{
		node->next = cxt->border_color_stack.top;
		cxt->border_color_stack.top = node;
	}
}

function void ui_push_border_thickness(UI_Context *cxt, f32 val)
{
	UI_float_value_node *node = ui_alloc_border_thickness_node(cxt);
	node->v = val;
	if (!cxt->border_thickness_stack.top)
	{
		cxt->border_thickness_stack.top = node;
	}
	else
	{
		node->next = cxt->border_thickness_stack.top;
		cxt->border_thickness_stack.top = node;
	}
}

function void ui_push_radius(UI_Context *cxt, f32 val)
{
	UI_float_value_node *node = ui_alloc_radius_node(cxt);
	node->v = val;
	if (!cxt->radius_stack.top)
	{
		cxt->radius_stack.top = node;
	}
	else
	{
		node->next = cxt->radius_stack.top;
		cxt->radius_stack.top = node;
	}
}

function void ui_push_pref_width(UI_Context *cxt, f32 val)
{
	UI_Pref_width_node *node = ui_alloc_pref_width_node(cxt);
	node->v = val;
	if (!cxt->pref_width_stack.top)
	{
		cxt->pref_width_stack.top = node;
	}
	else
	{
		node->next = cxt->pref_width_stack.top;
		cxt->pref_width_stack.top = node;
	}
}

function void ui_push_pref_height(UI_Context *cxt, f32 val)
{
	UI_Pref_height_node *node = ui_alloc_pref_height_node(cxt);
	node->v = val;
	if (!cxt->pref_height_stack.top)
	{
		cxt->pref_height_stack.top = node;
	}
	else
	{
		node->next = cxt->pref_height_stack.top;
		cxt->pref_height_stack.top = node;
	}
}

function void ui_push_fixed_pos(UI_Context *cxt, v2f val)
{
	UI_Fixed_pos_node *node = ui_alloc_fixed_pos_node(cxt);
	node->v = val;
	if (!cxt->fixed_pos_stack.top)
	{
		cxt->fixed_pos_stack.top = node;
	}
	else
	{
		node->next = cxt->fixed_pos_stack.top;
		cxt->fixed_pos_stack.top = node;
	}
}

function void ui_push_child_layout_axis(UI_Context *cxt, Axis2 val)
{
	UI_Axis2_node *node = ui_alloc_child_layout_axis_node(cxt);
	node->v = val;
	if (!cxt->child_layout_axis_stack.top)
	{
		cxt->child_layout_axis_stack.top = node;
	}
	else
	{
		node->next = cxt->child_layout_axis_stack.top;
		cxt->child_layout_axis_stack.top = node;
	}
}

function void ui_push_size_kind_x(UI_Context *cxt, UI_SizeKind val)
{
	UI_SizeKind_x_node *node = ui_alloc_size_kind_x_node(cxt);
	node->v = val;
	if (!cxt->size_kind_x_stack.top)
	{
		cxt->size_kind_x_stack.top = node;
	}
	else
	{
		node->next = cxt->size_kind_x_stack.top;
		cxt->size_kind_x_stack.top = node;
	}
}

function void ui_push_size_kind_y(UI_Context *cxt, UI_SizeKind val)
{
	UI_SizeKind_y_node *node = ui_alloc_size_kind_y_node(cxt);
	node->v = val;
	if (!cxt->size_kind_y_stack.top)
	{
		cxt->size_kind_y_stack.top = node;
	}
	else
	{
		node->next = cxt->size_kind_y_stack.top;
		cxt->size_kind_y_stack.top = node;
	}
}

function void ui_push_align_kind_x(UI_Context *cxt, UI_AlignKind val)
{
	UI_AlignKind_x_node *node = ui_alloc_align_kind_x_node(cxt);
	node->v = val;
	if (!cxt->align_kind_x_stack.top)
	{
		cxt->align_kind_x_stack.top = node;
	}
	else
	{
		node->next = cxt->align_kind_x_stack.top;
		cxt->align_kind_x_stack.top = node;
	}
}

// Set next style functions

function void ui_set_next_parent(UI_Context *cxt, UI_Widget* val)
{
	UI_Parent_node *node = ui_alloc_parent_node(cxt);
	node->v = val;
	if (!cxt->parent_stack.top)
	{
		cxt->parent_stack.top = node;
	}
	else
	{
		node->next = cxt->parent_stack.top;
		cxt->parent_stack.top = node;
	}
	cxt->parent_stack.auto_pop = 1;
}

function void ui_set_next_text_color(UI_Context *cxt, v4f val)
{
	UI_Color_node *node = ui_alloc_text_color_node(cxt);
	node->v = val;
	if (!cxt->text_color_stack.top)
	{
		cxt->text_color_stack.top = node;
	}
	else
	{
		node->next = cxt->text_color_stack.top;
		cxt->text_color_stack.top = node;
	}
	cxt->text_color_stack.auto_pop = 1;
}

function void ui_set_next_bg_color(UI_Context *cxt, v4f val)
{
	UI_Color_node *node = ui_alloc_bg_color_node(cxt);
	node->v = val;
	if (!cxt->bg_color_stack.top)
	{
		cxt->bg_color_stack.top = node;
	}
	else
	{
		node->next = cxt->bg_color_stack.top;
		cxt->bg_color_stack.top = node;
	}
	cxt->bg_color_stack.auto_pop = 1;
}

function void ui_set_next_hover_color(UI_Context *cxt, v4f val)
{
	UI_Color_node *node = ui_alloc_hover_color_node(cxt);
	node->v = val;
	if (!cxt->hover_color_stack.top)
	{
		cxt->hover_color_stack.top = node;
	}
	else
	{
		node->next = cxt->hover_color_stack.top;
		cxt->hover_color_stack.top = node;
	}
	cxt->hover_color_stack.auto_pop = 1;
}

function void ui_set_next_press_color(UI_Context *cxt, v4f val)
{
	UI_Color_node *node = ui_alloc_press_color_node(cxt);
	node->v = val;
	if (!cxt->press_color_stack.top)
	{
		cxt->press_color_stack.top = node;
	}
	else
	{
		node->next = cxt->press_color_stack.top;
		cxt->press_color_stack.top = node;
	}
	cxt->press_color_stack.auto_pop = 1;
}

function void ui_set_next_border_color(UI_Context *cxt, v4f val)
{
	UI_Color_node *node = ui_alloc_border_color_node(cxt);
	node->v = val;
	if (!cxt->border_color_stack.top)
	{
		cxt->border_color_stack.top = node;
	}
	else
	{
		node->next = cxt->border_color_stack.top;
		cxt->border_color_stack.top = node;
	}
	cxt->border_color_stack.auto_pop = 1;
}

function void ui_set_next_border_thickness(UI_Context *cxt, f32 val)
{
	UI_float_value_node *node = ui_alloc_border_thickness_node(cxt);
	node->v = val;
	if (!cxt->border_thickness_stack.top)
	{
		cxt->border_thickness_stack.top = node;
	}
	else
	{
		node->next = cxt->border_thickness_stack.top;
		cxt->border_thickness_stack.top = node;
	}
	cxt->border_thickness_stack.auto_pop = 1;
}

function void ui_set_next_radius(UI_Context *cxt, f32 val)
{
	UI_float_value_node *node = ui_alloc_radius_node(cxt);
	node->v = val;
	if (!cxt->radius_stack.top)
	{
		cxt->radius_stack.top = node;
	}
	else
	{
		node->next = cxt->radius_stack.top;
		cxt->radius_stack.top = node;
	}
	cxt->radius_stack.auto_pop = 1;
}

function void ui_set_next_pref_width(UI_Context *cxt, f32 val)
{
	UI_Pref_width_node *node = ui_alloc_pref_width_node(cxt);
	node->v = val;
	if (!cxt->pref_width_stack.top)
	{
		cxt->pref_width_stack.top = node;
	}
	else
	{
		node->next = cxt->pref_width_stack.top;
		cxt->pref_width_stack.top = node;
	}
	cxt->pref_width_stack.auto_pop = 1;
}

function void ui_set_next_pref_height(UI_Context *cxt, f32 val)
{
	UI_Pref_height_node *node = ui_alloc_pref_height_node(cxt);
	node->v = val;
	if (!cxt->pref_height_stack.top)
	{
		cxt->pref_height_stack.top = node;
	}
	else
	{
		node->next = cxt->pref_height_stack.top;
		cxt->pref_height_stack.top = node;
	}
	cxt->pref_height_stack.auto_pop = 1;
}

function void ui_set_next_fixed_pos(UI_Context *cxt, v2f val)
{
	UI_Fixed_pos_node *node = ui_alloc_fixed_pos_node(cxt);
	node->v = val;
	if (!cxt->fixed_pos_stack.top)
	{
		cxt->fixed_pos_stack.top = node;
	}
	else
	{
		node->next = cxt->fixed_pos_stack.top;
		cxt->fixed_pos_stack.top = node;
	}
	cxt->fixed_pos_stack.auto_pop = 1;
}

function void ui_set_next_child_layout_axis(UI_Context *cxt, Axis2 val)
{
	UI_Axis2_node *node = ui_alloc_child_layout_axis_node(cxt);
	node->v = val;
	if (!cxt->child_layout_axis_stack.top)
	{
		cxt->child_layout_axis_stack.top = node;
	}
	else
	{
		node->next = cxt->child_layout_axis_stack.top;
		cxt->child_layout_axis_stack.top = node;
	}
	cxt->child_layout_axis_stack.auto_pop = 1;
}

function void ui_set_next_size_kind_x(UI_Context *cxt, UI_SizeKind val)
{
	UI_SizeKind_x_node *node = ui_alloc_size_kind_x_node(cxt);
	node->v = val;
	if (!cxt->size_kind_x_stack.top)
	{
		cxt->size_kind_x_stack.top = node;
	}
	else
	{
		node->next = cxt->size_kind_x_stack.top;
		cxt->size_kind_x_stack.top = node;
	}
	cxt->size_kind_x_stack.auto_pop = 1;
}

function void ui_set_next_size_kind_y(UI_Context *cxt, UI_SizeKind val)
{
	UI_SizeKind_y_node *node = ui_alloc_size_kind_y_node(cxt);
	node->v = val;
	if (!cxt->size_kind_y_stack.top)
	{
		cxt->size_kind_y_stack.top = node;
	}
	else
	{
		node->next = cxt->size_kind_y_stack.top;
		cxt->size_kind_y_stack.top = node;
	}
	cxt->size_kind_y_stack.auto_pop = 1;
}

function void ui_set_next_align_kind_x(UI_Context *cxt, UI_AlignKind val)
{
	UI_AlignKind_x_node *node = ui_alloc_align_kind_x_node(cxt);
	node->v = val;
	if (!cxt->align_kind_x_stack.top)
	{
		cxt->align_kind_x_stack.top = node;
	}
	else
	{
		node->next = cxt->align_kind_x_stack.top;
		cxt->align_kind_x_stack.top = node;
	}
	cxt->align_kind_x_stack.auto_pop = 1;
}

// Pop style functions

function void ui_pop_parent(UI_Context *cxt)
{
	UI_Parent_node *pop = cxt->parent_stack.top;
	cxt->parent_stack.top = cxt->parent_stack.top->next;
	ui_free_parent_node(cxt, pop);
}

function void ui_pop_text_color(UI_Context *cxt)
{
	UI_Color_node *pop = cxt->text_color_stack.top;
	cxt->text_color_stack.top = cxt->text_color_stack.top->next;
	ui_free_text_color_node(cxt, pop);
}

function void ui_pop_bg_color(UI_Context *cxt)
{
	UI_Color_node *pop = cxt->bg_color_stack.top;
	cxt->bg_color_stack.top = cxt->bg_color_stack.top->next;
	ui_free_bg_color_node(cxt, pop);
}

function void ui_pop_hover_color(UI_Context *cxt)
{
	UI_Color_node *pop = cxt->hover_color_stack.top;
	cxt->hover_color_stack.top = cxt->hover_color_stack.top->next;
	ui_free_hover_color_node(cxt, pop);
}

function void ui_pop_press_color(UI_Context *cxt)
{
	UI_Color_node *pop = cxt->press_color_stack.top;
	cxt->press_color_stack.top = cxt->press_color_stack.top->next;
	ui_free_press_color_node(cxt, pop);
}

function void ui_pop_border_color(UI_Context *cxt)
{
	UI_Color_node *pop = cxt->border_color_stack.top;
	cxt->border_color_stack.top = cxt->border_color_stack.top->next;
	ui_free_border_color_node(cxt, pop);
}

function void ui_pop_border_thickness(UI_Context *cxt)
{
	UI_float_value_node *pop = cxt->border_thickness_stack.top;
	cxt->border_thickness_stack.top = cxt->border_thickness_stack.top->next;
	ui_free_border_thickness_node(cxt, pop);
}

function void ui_pop_radius(UI_Context *cxt)
{
	UI_float_value_node *pop = cxt->radius_stack.top;
	cxt->radius_stack.top = cxt->radius_stack.top->next;
	ui_free_radius_node(cxt, pop);
}

function void ui_pop_pref_width(UI_Context *cxt)
{
	UI_Pref_width_node *pop = cxt->pref_width_stack.top;
	cxt->pref_width_stack.top = cxt->pref_width_stack.top->next;
	ui_free_pref_width_node(cxt, pop);
}

function void ui_pop_pref_height(UI_Context *cxt)
{
	UI_Pref_height_node *pop = cxt->pref_height_stack.top;
	cxt->pref_height_stack.top = cxt->pref_height_stack.top->next;
	ui_free_pref_height_node(cxt, pop);
}

function void ui_pop_fixed_pos(UI_Context *cxt)
{
	UI_Fixed_pos_node *pop = cxt->fixed_pos_stack.top;
	cxt->fixed_pos_stack.top = cxt->fixed_pos_stack.top->next;
	ui_free_fixed_pos_node(cxt, pop);
}

function void ui_pop_child_layout_axis(UI_Context *cxt)
{
	UI_Axis2_node *pop = cxt->child_layout_axis_stack.top;
	cxt->child_layout_axis_stack.top = cxt->child_layout_axis_stack.top->next;
	ui_free_child_layout_axis_node(cxt, pop);
}

function void ui_pop_size_kind_x(UI_Context *cxt)
{
	UI_SizeKind_x_node *pop = cxt->size_kind_x_stack.top;
	cxt->size_kind_x_stack.top = cxt->size_kind_x_stack.top->next;
	ui_free_size_kind_x_node(cxt, pop);
}

function void ui_pop_size_kind_y(UI_Context *cxt)
{
	UI_SizeKind_y_node *pop = cxt->size_kind_y_stack.top;
	cxt->size_kind_y_stack.top = cxt->size_kind_y_stack.top->next;
	ui_free_size_kind_y_node(cxt, pop);
}

function void ui_pop_align_kind_x(UI_Context *cxt)
{
	UI_AlignKind_x_node *pop = cxt->align_kind_x_stack.top;
	cxt->align_kind_x_stack.top = cxt->align_kind_x_stack.top->next;
	ui_free_align_kind_x_node(cxt, pop);
}

// =================

