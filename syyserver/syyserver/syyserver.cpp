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

int _tmain(int argc, _TCHAR* argv[])
{

//	IplayerInterface* player = GENERATE_CLASS(CPlayer);
	//_asm int 3;
// 	char buff[1==3];
	co_thread_t t =  co_thread_init(1024*64,10,100);

	cothread_ctx* ctx = (cothread_ctx*)t;
	int ticket = GetTickCount();

	while (g_count < 10000000)
	{
		//Sleep(10);
		coroutine* c=(coroutine*)bd_pop_back(&ctx->co_cache_list);
		if (c)
		{
			c->status = co_stat_wait;
			c->func = FiberFunc;
			c->data ="fiber";
			co_resume(t,c);
		}
		co_update(t);
	}
	int ticket_e = GetTickCount();
	printf("all time :%d\n",ticket_e - ticket);



	CDynamicStreamBuf buf;
	COStream os(buf);
	testsc dd;
	dd.a =10;
	os<<dd;
// 	CIStream is(buf);
	closure_test test;
// 	IClosure * closure = Closure_Helper(1,&test,&closure_test::print,1);
// 	closure->LoadFromStream(buf);
// 	closure->run();
// 	testsc dd;
// 	IClosure * closure2 = Closure_Helper(2,&test,&closure_test::test2,dd);
// 	os<<dd;
// 	closure2->LoadFromStream(buf);
// 	closure2->run();
	RMI_CPlayer<CPlayer> player;
	player.RegisterRmi();
	player.call_func(buf);
	return 0;
}
