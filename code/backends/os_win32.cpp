global u64 total_cmt;
global u64 total_res;

function void *os_win32_reserve(u64 size)
{
	void *out = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
	if (out != NULL)
	{
		total_res += size;
	}
	return out;
}

function b32 os_win32_commit(void *ptr, u64 size)
{
	if (VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) == NULL)
	{
		printf("VirtualAlloc commit failed: %lu\r\n", GetLastError());
		return 0;
	}
	total_cmt += size;
	
	return 1;
}

function void os_win32_decommit(void *ptr, u64 size)
{
	VirtualFree(ptr, size, MEM_DECOMMIT);
}

function void os_win32_release(void *ptr, u64 size)
{
	VirtualFree(ptr, 0, MEM_RELEASE);
}

function u64 os_win32_getPageSize()
{
	SYSTEM_INFO sysinfo = {};
	GetSystemInfo(&sysinfo);
	return sysinfo.dwPageSize;
}

function Str8 os_win32_getAppDir(Arena *arena)
{
	char buffer[256];
	DWORD len = GetModuleFileName(0, buffer, 256);
	
	char *c = &buffer[len];
	while(*(--c) != '\\')
	{
		*c = 0;
		--len;
	}
	
	u8 *str = push_array(arena, u8, len);
	memcpy(str, buffer, len);
	
	Str8 out = str8(str, len);
	
	return out;
}