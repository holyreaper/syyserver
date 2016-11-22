// cothread_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include "./platform/ByteStream.h"
// #include <WinBase.h>
#include <windows.h>
#include "./rmi/taurus.h"
#include "./rmi/bd_list.h"
#include "./rmi/taurus_imp.h"
// #include <sys/time.h>   
#include "./platform/closure.h"
#include "./rmi/rmi.h"
#include "./logic/player.h"
#include "./platform/random_util.h"
#include "./rmi/task.h"
#include "./platform/time_util.h"
#include "./event/event.pb.h"
#include "./queue/kfifol.h"
#include "./platform/singleton.h"
//  #include "./platform/timer_manager.h"
ITaskManager* _InitTask(co_thread_t* t);
int _tmain(int argc, _TCHAR* argv[])
{
	Event ev ;
	ev.set_cmd(1);
	std::string sev; 
	ev.SerializeToString(&sev);
	bool exit = false;
	co_thread_t t =  co_thread_init(DEFAULT_STACK_SIZE,10,10);
	ITaskManager * task = _InitTask(&t);
	RMIServerBase* player = new RMI_CPlayer<CPlayer>(task);
	player->RegisterRmi();
	tick_t t1  = get_tick_count();
	
	int g_count = 0;
	assert(task);
	do 
	{
		player->call_func(&ev);
		((CTaskManager * )task)->OnNotice(NULL);
		g_count++;
	} while (1000000 > g_count);
	
// 	co_thread_t t =  co_thread_init(1024*64,10,100);

// 	cothread_ctx* ctx = (cothread_ctx*)t;
// 	int ticket = GetTickCount();
// // 
// 	while (g_count < 10000000)
// 	{
// 		//Sleep(10);
// 		coroutine* c=(coroutine*)bd_pop_back(&ctx->co_cache_list);
// 		if (c)
// 		{
// 			c->status = co_stat_wait;
// 			c->func = FiberFunc;
// 			c->data ="fiber";
// 			co_resume(t,c);
// 		}
// 		co_update(t);
// 	}
// 	int ticket_e = GetTickCount();
// 	printf("all time :%d\n",ticket_e - ticket);
// 


// 	CDynamicStreamBuf buf;
// 	COStream os(buf);
// 	testsc dd;
// 	dd.a =10;
// 	os<<dd;
// 	CIStream is(buf);
// 	testsc dd;
// 	IClosure * closure2 = Closure_Helper(2,&test,&closure_test::test2,dd);
// 	os<<dd;
// 	closure2->LoadFromStream(buf);
// 	closure2->run();
	tick_t t2  = get_tick_count();

	std::cout<<t2- t1<<std::endl;
//	Singleton<kfifol>::instance().init();
	return 0;
}
ITaskManager* _InitTask(co_thread_t* t)
{
	return new CTaskManager(*t,DEFAULT_STACK_SIZE);
}
