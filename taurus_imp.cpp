/*
 * taurus_imp.cpp
 *
 *  Created on: 2011-2-18
 *      Author: neil
 */

#include "StdAfx.h"
#include "taurus_imp.h"
#include "taurus.h"
#include <iostream>

inline coroutine* malloc_coroutine(void)
{
	void* p =malloc(sizeof(coroutine));

	if(p == NULL)
		return 0;

	memset(p, 0, sizeof(coroutine));

	return (coroutine*)p;
}

inline void free_coroutine(coroutine* c)
{
	free(c);
}

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)


VOID __stdcall FiberFunc(LPVOID lpParameter);


inline int init_main_coroutine(coroutine* c)
{
	c->ctx = ConvertThreadToFiber(NULL);
	return (c->ctx==NULL) ? -1 : 0;
}

inline int setup_coroutine(cothread_ctx* tctx, coroutine* c, size_t ssize)
{
	c->ctx = CreateFiber(ssize, &FiberFunc, tctx);
	if (c->ctx == NULL)
	{
	}
	c->stk_size = ssize;
	return (c->ctx==NULL) ? -1 : 0;
}

inline int cleanup_coroutine(coroutine* c)
{
	DeleteFiber(c->ctx);
	c->ctx = 0;
	c->stk_size = 0;
	return 0;
}

inline int swap_context(cothread_ctx* tctx, coroutine* from, coroutine* to)
{
	//__CHECK_LAST_ERROR();
	tctx->co_curr = to;
	SwitchToFiber(to->ctx);
	return 0;
}


void __stdcall FiberFunc(LPVOID lpParameter)
{	
	cothread_ctx* tctx = (cothread_ctx*)lpParameter;
	coroutine* c = tctx->co_curr;

	while(1)
	{
		c->func(c->data);
		c->status = co_stat_dead;
		bd_push_front(&tctx->co_dead_list, &c->node);
		//std::cout<<"helo"<<std::endl;
		swap_context(tctx, c, tctx->co_main);
	}
}

#else

#include <sys/mman.h>

void FiberFunc(cothread_ctx* tctx, coroutine* c);

inline int init_main_coroutine(coroutine* c)
{
	return getcontext(&c->ctx);
}

inline int setup_coroutine(cothread_ctx* tctx, coroutine* c, size_t ssize)
{
	if (getcontext(&c->ctx) == -1)
		return -1;

	//void* sp = malloc(ssize);

	//Ìí¼Óstack overflow guard page
	size_t guard_page_size = getpagesize();
	size_t buff_size = ssize + guard_page_size;

	void* sp = valloc( buff_size ); 
	if ( !sp )
		return -2;

	int res = mprotect(sp, guard_page_size, PROT_READ);
	if ( res != 0 )
		;
	
	c->stk_size = ssize;
	c->ctx.uc_link = NULL;
	c->ctx.uc_stack.ss_sp = sp;
	c->ctx.uc_stack.ss_size = buff_size;
	c->ctx.uc_stack.ss_flags = 0;

	makecontext(&c->ctx, (void (*)(void))&FiberFunc, 2, tctx, c);

	return 0;
}

inline int cleanup_coroutine(coroutine* c)
{
	if(c->ctx.uc_stack.ss_sp)
	{
		c->stk_size = 0;
		
		size_t guard_page_size = getpagesize();
		
		void* sp = c->ctx.uc_stack.ss_sp;
		mprotect(sp, guard_page_size, PROT_READ|PROT_WRITE);
		free( sp );
		
		c->ctx.uc_stack.ss_sp = 0;
		c->ctx.uc_stack.ss_size = 0;
	}
	return 0;
}

inline int swap_context(cothread_ctx* tctx, coroutine* from, coroutine* to)
{
	__CHECK_LAST_ERROR();
	tctx->co_curr = to;
	return swapcontext(&from->ctx, &to->ctx);
}

void FiberFunc(cothread_ctx* tctx, coroutine* c)
{
	while(1)
	{
		__CHECK_LAST_ERROR();
		c->func(c->data);
		c->status = co_stat_dead;
		bd_push_front(&tctx->co_dead_list, &c->node);
		swap_context(tctx, c, tctx->co_main);
	}
}

#endif
/////////////////////////////////

coroutine* create_coroutine(cothread_ctx* tctx, size_t ssize)
{
	coroutine* c = malloc_coroutine();
	if(c == NULL)
	{
		return NULL;
	}

	if(setup_coroutine(tctx, c, ssize) == -1)
	{
		free_coroutine(c);
		return NULL;
	}

	return c;
}



co_thread_t co_thread_init(size_t stack_size, size_t lwm, size_t hwm)
{
	cothread_ctx* tctx = (cothread_ctx*)malloc(sizeof(cothread_ctx));
	if(tctx == NULL)
		return 0;

	memset(tctx, 0, sizeof(cothread_ctx));

	tctx->co_stack_size = stack_size;
	tctx->low_watermark = lwm;
	tctx->high_watermark = hwm;

	coroutine* c = malloc_coroutine();
	tctx->co_main = c;
	tctx->co_curr = c;

	if(init_main_coroutine(c) == 0)
	{
		for(size_t i=0; i<lwm; ++i)
		{
			c = create_coroutine(tctx, tctx->co_stack_size);
			if(c == NULL)
				break;
			c->status = co_stat_null;
			bd_push_front(&(tctx->co_cache_list), &c->node);
		}

		return tctx;
	}

	co_thread_cleanup(tctx);
	return 0;
}

void co_thread_cleanup(co_thread_t t)
{
	if(t == NULL)
		return;

	cothread_ctx* tctx = (cothread_ctx*)t;
	free_coroutine(tctx->co_main);
	tctx->co_main = NULL;
	tctx->co_curr = NULL;

	while(coroutine* c=(coroutine*)bd_pop_back(&tctx->co_cache_list))
	{
		cleanup_coroutine(c);
		free_coroutine(c);
	}

	while(coroutine* c=(coroutine*)bd_pop_back(&tctx->co_dead_list))
	{
		cleanup_coroutine(c);
		free_coroutine(c);
	}

	//while(coroutine* c=(coroutine*)bd_pop_back(&tctx->co_wait_list))
	//{
	//	cleanup_coroutine(c);
	//	free_coroutine(c);
	//}

	while(coroutine* c=(coroutine*)bd_pop_back(&tctx->co_ready_list))
	{
		cleanup_coroutine(c);
		free_coroutine(c);
	}

	free(tctx);
}



coroutine_t co_create(co_thread_t t, void(*func)(void*), void* data, size_t stack_size)
{
	cothread_ctx* tctx = (cothread_ctx*)t;

	coroutine* c = NULL;
	if(stack_size==0 || stack_size==tctx->co_stack_size)
	{
		if(tctx->co_cache_list.count > 0)
		{
			c = (coroutine*)bd_pop_back(&tctx->co_cache_list);
			assert(c);
		}
		else
		{
			c = create_coroutine(tctx, tctx->co_stack_size);
		}
	}
	else
	{
		c = create_coroutine(tctx, stack_size);
	}

	if(c != NULL)
	{
		c->func = func;
		c->data = data;
		c->status = co_stat_ready;
		bd_push_front(&tctx->co_ready_list, &c->node);
	}
	return c;
}

int co_yield(co_thread_t t, coroutine_t co)
{
	coroutine* c = (coroutine*)co;
	if(c == NULL)
		return -1;

	cothread_ctx* tctx = (cothread_ctx*)t;

	if(c != tctx->co_curr)
		return 0;

	//bd_remove_node(&tctx->co_ready_list, &c->node);
	//bd_push_front(&tctx->co_wait_list, &c->node);
	if (c->status != co_stat_run)
	{
		return 0;
	}
	c->status = co_stat_wait;
	return swap_context((cothread_ctx*)t, c, tctx->co_main);
}

int co_resume(co_thread_t t, coroutine_t co)
{
	coroutine* c = (coroutine*)co;
	if(c == NULL)
		return -1;

	cothread_ctx* tctx = (cothread_ctx*)t;
	if(c == tctx->co_curr)
		return 0;

	//if ( !bd_find_node(&tctx->co_wait_list, &c->node) )
	//{
	//	LogError( "$HI_RED$co_resume:Can not find corotuine in co_wait_list" );
	//	LogError( "$HI_RED$-------------------------------" );
	//	LogError( "$HI_RED$In co_ready_list\t :", bd_find_node(&tctx->co_ready_list, &c->node) );
	//	LogError( "$HI_RED$In co_dead_list\t :" , bd_find_node(&tctx->co_dead_list , &c->node) );
	//	LogError( "$HI_RED$In co_cache_list\t :", bd_find_node(&tctx->co_cache_list, &c->node) );
	//	LogError( "-------------------------------" );
	//	return 0;
	//}

	//bd_remove_node(&tctx->co_wait_list, &c->node);
	if (c->status != co_stat_wait)
	{
// 		LogError( "$HI_RED$co_resume: coroutine invalid stat: ", c->status );
		return 0;
	}
	c->status = co_stat_ready;
	bd_push_front(&tctx->co_ready_list, &c->node);

	return 0;
}

coroutine_t co_current(co_thread_t t)
{
	cothread_ctx* tctx = (cothread_ctx*)t;
	return tctx->co_curr;
}

coroutine_t co_main(co_thread_t t)
{
	cothread_ctx* tctx = (cothread_ctx*)t;
	return tctx->co_main;
}


void co_update(co_thread_t t)
{
	cothread_ctx* tctx = (cothread_ctx*)t;

	while(coroutine* c=(coroutine*)bd_pop_back(&tctx->co_ready_list))
	{
		c->status = co_stat_run;
		swap_context(tctx, tctx->co_main, c);
	}

	while(coroutine* c=(coroutine*)bd_pop_back(&tctx->co_dead_list))
	{
		if(c->stk_size == tctx->co_stack_size)
		{
			if(tctx->co_cache_list.count < (tctx->high_watermark))
			{
				c->status = co_stat_null;
				bd_push_front(&tctx->co_cache_list, &c->node);
				continue;
			}
		}

		cleanup_coroutine(c);
		free_coroutine(c);
	}
}

void co_set_data(coroutine_t co, void* d)
{
	coroutine* c = (coroutine*)co;
	c->data = d;
}

void* co_get_data(coroutine_t co)
{
	coroutine* c = (coroutine*)co;
	return c->data;
}

int co_ready_count(co_thread_t t)
{
	cothread_ctx* tctx = (cothread_ctx*)t;
	return tctx->co_ready_list.count;
}
