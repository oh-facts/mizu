struct OS_State
{
	u64 temp;
};

global OS_State os_state;

void os_api_init(OS_Api *api)
{
	os_reserve = api->os_reserve;
	os_commit = api->os_commit;
	os_decommit = api->os_decommit;
	os_release = api->os_release;
}

OS_Event *os_push_event(Arena *arena, OS_Event_list *list)
{
	OS_Event_node *node = push_struct(arena, OS_Event_node);
	*node = {};
	
	if(list->last)
	{
		node->prev = list->last;
		list->last = list->last->next = node;
	}
	else
	{
		list->last = list->first = node;
	}
	list->count ++;
	
	OS_Event *event = &node->v;
	return event;
}

b32 os_key_press(OS_Event_list *list, OS_Window win, OS_Key key)
{
	b32 out = 0;
	
	for(OS_Event_node *event = list->first; event; event = event->next)
	{
		if(event->v.kind == OS_EventKind_KeyPressed && event->v.key == key)
		{
			os_consume_event(list, event);
			out = 1;
			break;
		}
	}
	
	return out;
}

v2s os_mouse_pos(OS_Event_list *list, OS_Window win)
{
	v2s out = os_window_get_mouse_pos(win);
	
	for(OS_Event_node *event = list->first; event; event = event->next)
	{
		if(event->v.kind == OS_EventKind_MouseMove)
		{
			os_consume_event(list, event);
			out = event->v.mpos;
			os_window_set_mouse_pos(win, out);
			break;
		}
	}
	return out;
}

b32 os_mouse_held(OS_Window win, OS_MouseButton button)
{
	return os_window_get_mouse_state(win, button);
}

b32 os_mouse_pressed(OS_Event_list *list, OS_Window win, OS_MouseButton button)
{
	b32 out = 0;
	
	for(OS_Event_node *event = list->first; event; event = event->next)
	{
		if(event->v.kind == OS_EventKind_MousePressed && event->v.button == button)
		{
			
			os_consume_event(list, event);
			os_window_set_mouse_state(win, button, 1);
			out = 1;
			break;
		}
	}
	return out;
}

b32 os_mouse_released(OS_Event_list *list, OS_Window win, OS_MouseButton button)
{
	b32 out = 0;
	
	for(OS_Event_node *event = list->first; event; event = event->next)
	{
		if(event->v.kind == OS_EventKind_MouseReleased && event->v.button == button)
		{
			
			os_consume_event(list, event);
			os_window_set_mouse_state(win, button, 0);
			out = 1;
			break;
		}
	}
	
	return out;
}

void os_consume_event(OS_Event_list *list, OS_Event_node *node)
{
	
	if(node->prev)
	{
		node->prev->next = node->next;
	}
	else
	{
		list->first = node->next;
	}
	
	if(node->next)
	{
		node->next->prev = node->prev;
	}
	else
	{
		list->last = node->prev;
	}
	
	
	list->count --;
	// uses trans memory so not freeing node
}