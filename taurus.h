/*
 * taurus.h
 *
 *  Created on: 2011-2-18
 *      Author: neil
 */

#ifndef TAURUS_H_
#define TAURUS_H_

#include <cstddef>

typedef void* coroutine_t;
typedef void* co_thread_t;

co_thread_t co_thread_init(size_t stack_size, size_t lwm, size_t hwm);

void co_thread_cleanup(co_thread_t t);

coroutine_t co_create(co_thread_t t, void(*func)(void*), void* data, size_t stack_size=0);

int co_yield(co_thread_t t, coroutine_t co);

int co_resume(co_thread_t t, coroutine_t co);

coroutine_t co_current(co_thread_t t);

coroutine_t co_main(co_thread_t t);

void co_update(co_thread_t t);

void co_set_data(coroutine_t co, void* d);

void* co_get_data(coroutine_t co);

int co_ready_count(co_thread_t t);


#endif /* TAURUS_H_ */
