/*
 * taurus_imp.h
 *
 *  Created on: 2011-2-18
 *      Author: neil
 */

#ifndef TAURUS_IMP_H_
#define TAURUS_IMP_H_


#include "bd_list.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
	#include <SDKDDKVer.h>
	#include <Windows.h>
	#define co_platform_ctx LPVOID
#else
	#include <ucontext.h>
	#include <stdlib.h>
	#include <string.h>
	#define co_platform_ctx ucontext_t
#endif

enum co_stat_t
{
	co_stat_null,
	co_stat_ready,
	co_stat_run,
	co_stat_wait,
	co_stat_dead,
};

struct coroutine
{
	bd_node node;
	co_platform_ctx ctx;
	size_t stk_size;
	void (*func)(void*);
	void* data;
	co_stat_t status;
};

struct cothread_ctx
{
	coroutine* co_main;
	coroutine* co_curr;
	bd_list co_ready_list;
	//bd_list co_wait_list;
	bd_list co_dead_list;
	bd_list co_cache_list;
	size_t co_stack_size;
	size_t low_watermark;
	size_t high_watermark;
};


#endif /* TAURUS_IMP_H_ */

