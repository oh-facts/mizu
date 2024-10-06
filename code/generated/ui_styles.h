// Style Structs

//2024-10-06 13:37:02

struct UI_Parent_node
{
	UI_Parent_node *next;
	UI_Widget * v;
};

struct UI_Color_node
{
	UI_Color_node *next;
	v4f v;
};

struct UI_Pref_width_node
{
	UI_Pref_width_node *next;
	f32 v;
};

struct UI_Pref_height_node
{
	UI_Pref_height_node *next;
	f32 v;
};

struct UI_Fixed_pos_node
{
	UI_Fixed_pos_node *next;
	v2f v;
};

struct UI_float_value_node
{
	UI_float_value_node *next;
	f32 v;
};

struct UI_AlignKind_x_node
{
	UI_AlignKind_x_node *next;
	UI_AlignKind v;
};

struct UI_Axis2_node
{
	UI_Axis2_node *next;
	Axis2 v;
};

struct UI_SizeKind_x_node
{
	UI_SizeKind_x_node *next;
	UI_SizeKind v;
};

struct UI_SizeKind_y_node
{
	UI_SizeKind_y_node *next;
	UI_SizeKind v;
};

// Style Stack Structs

#define UI_STYLE_STACKS \
struct\
{\
	UI_Parent_node *top;\
	UI_Parent_node *free;\
	b32 auto_pop;\
}parent_stack;\
;struct\
{\
	UI_Color_node *top;\
	UI_Color_node *free;\
	b32 auto_pop;\
}text_color_stack;\
;struct\
{\
	UI_Color_node *top;\
	UI_Color_node *free;\
	b32 auto_pop;\
}bg_color_stack;\
;struct\
{\
	UI_Color_node *top;\
	UI_Color_node *free;\
	b32 auto_pop;\
}hover_color_stack;\
;struct\
{\
	UI_Color_node *top;\
	UI_Color_node *free;\
	b32 auto_pop;\
}press_color_stack;\
;struct\
{\
	UI_Color_node *top;\
	UI_Color_node *free;\
	b32 auto_pop;\
}border_color_stack;\
;struct\
{\
	UI_float_value_node *top;\
	UI_float_value_node *free;\
	b32 auto_pop;\
}border_thickness_stack;\
;struct\
{\
	UI_float_value_node *top;\
	UI_float_value_node *free;\
	b32 auto_pop;\
}radius_stack;\
;struct\
{\
	UI_Pref_width_node *top;\
	UI_Pref_width_node *free;\
	b32 auto_pop;\
}pref_width_stack;\
;struct\
{\
	UI_Pref_height_node *top;\
	UI_Pref_height_node *free;\
	b32 auto_pop;\
}pref_height_stack;\
;struct\
{\
	UI_Fixed_pos_node *top;\
	UI_Fixed_pos_node *free;\
	b32 auto_pop;\
}fixed_pos_stack;\
;struct\
{\
	UI_Axis2_node *top;\
	UI_Axis2_node *free;\
	b32 auto_pop;\
}child_layout_axis_stack;\
;struct\
{\
	UI_SizeKind_x_node *top;\
	UI_SizeKind_x_node *free;\
	b32 auto_pop;\
}size_kind_x_stack;\
;struct\
{\
	UI_SizeKind_y_node *top;\
	UI_SizeKind_y_node *free;\
	b32 auto_pop;\
}size_kind_y_stack;\
;struct\
{\
	UI_AlignKind_x_node *top;\
	UI_AlignKind_x_node *free;\
	b32 auto_pop;\
}align_kind_x_stack;\
;