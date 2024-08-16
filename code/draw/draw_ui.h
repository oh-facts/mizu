/* date = August 4th 2024 6:17 pm */

#ifndef DRAW_UI_H
#define DRAW_UI_H

function void d_draw_ui(D_Bucket *draw, UI_Widget *root)
{
	root->pos.x = root->computed_rel_position[0] + root->fixed_position.x;
	root->pos.y = root->computed_rel_position[1] + root->fixed_position.y;
	root->size.x = root->computed_size[0];
	root->size.y = root->computed_size[1];
	
	if(!root->parent)
	{
		v2f size = {};
		size.x = root->first->computed_size[0];
		size.y = root->first->computed_size[1];
		
		v2f pos = {};
		pos.x = root->first->pos.x;
		pos.y = root->first->pos.y;
		d_draw_rect(draw, pos, size, D_COLOR_BLUE);
		//root->pos.x += root->parent->computed_rel_position[0];
		//root->pos.y += root->parent->computed_rel_position[1];
	}
	
	if(root->flags & UI_Flags_has_text)
	{
		v4f color = {};
		if(root->hot)
		{
			color = D_COLOR_BLUE;
		}
		else
		{
			color = root->color;
		}
		
		D_Text_params params = 
		{
			color,
			draw->default_text_params.scale,
			draw->default_text_params.atlas,
			draw->default_text_params.atlas_tex,
		};
		
		d_draw_text(draw, root->text, root->pos, &params);
	}
	
	for(UI_Widget *child = root->first; child; child = child->next)
	{
		d_draw_ui(draw, child);
	}
}

#endif //DRAW_UI_H