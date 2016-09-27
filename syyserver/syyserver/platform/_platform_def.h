#ifndef _PLATFORM_DEF_FILE_2016__0626
#define _PLATFORM_DEF_FILE_2016__0626
#if  defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
	#define PLATFORM_WIN32
typedef __int64 int64;
typedef __int64 tick_t;

#else
typedef long long int64;
typedef long long tick_t;
#endif




#endif//_PLATFORM_DEF_FILE_2016__0626