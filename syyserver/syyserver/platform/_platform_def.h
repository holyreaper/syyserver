#ifndef _PLATFORM_DEF_FILE_2016__0626
#define _PLATFORM_DEF_FILE_2016__0626
#include <sys/timeb.h>

#if  defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define PLATFORM_WIN32
typedef __int64 int64;
typedef __int64 tick_t;


#define  TIMEB    _timeb
#define  ftime    _ftime


#else
typedef long long int64;
typedef long long tick_t;

#define TIMEB timeb



#endif

#ifndef NULL
#define NULL 0
#endif
/*
time_t ltime1, ltime2, tmp_time;
struct TIMEB tstruct1, tstruct2;

ftime (&tstruct1);            // start time ms
time (&ltime1);               // start time s
//work
time (&ltime2);               // end time sec
ftime (&tstruct2);            // end time ms


tmp_time = (ltime2 * 1000 + tstruct2.millitm) - (ltime1 * 1000 + tstruct1.millitm);
*/


#endif//_PLATFORM_DEF_FILE_2016__0626