#include <stdio.h>
#include <time.h>

// the problem with printing current time is that merging becomes a pain in the ass. And
// generated code, binaries and everything else related to the build environment should
// be comitted. Anyways, this is #if 0 until git merging becomes more sane.
static void print_now()
{
#if 0
	time_t t;
	struct tm tm_info;
	char buffer[128];
	
	time(&t);
	localtime_s(&tm_info, &t);
	
	strftime(buffer, 128, "%Y-%m-%d %H:%M:%S", &tm_info);
	printf("//%s\n\n", buffer);
#endif
}