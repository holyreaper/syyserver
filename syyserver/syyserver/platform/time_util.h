#ifndef __TIMEUTIL_H__8923lkjsk__
#define __TIMEUTIL_H__8923lkjsk__
#include <string>
#include "_platform_def.h"
#include <stdio.h>
//获取当前时间
static tick_t get_tick_count()
{
#ifdef PLATFORM_WIN32
#include <windows.h>
#pragma warning(disable:4996)
	static struct TIMEB tb;
	ftime (&tb);            // end time ms
	return tb.time *1000 +tb.millitm ;
#else
#include <sys/time.h>
	static struct timeval tv;
	gettimeofday( &tv, NULL );
	return tv.tv_sec* 1000 + tv.tv_usec /1000;
#endif
}
//

#endif //__TIMEUTIL_H__8923lkjsk__