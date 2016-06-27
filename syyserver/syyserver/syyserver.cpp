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
static int g_count =0;
// #include <sys/time.h>   
#include "./platform/closure.h"
inline void FiberFunc(void * param)
{
	g_count++;
	//printf("%s count:%d\n",param,g_count++);
}


class closure_test
{
public:

	closure_test()
	{

	}
	~closure_test()
	{

	}

	void print(int c)
	{
		std::cout<<c<<std::endl;
	}
protected:
private:
};


template<typename TT, typename FF,typename AA>
IClosure* Closure_Helper(TT *a ,FF b, AA c)
{
	return new Closure<TT,FF,AA>(a,b,c);
}
int _tmain(int argc, _TCHAR* argv[])
{


	//_asm int 3;
	CDynamicStreamBuf buf;
	COStream os(buf);
	os<<2;
	CIStream is(buf);
	int a =0;
	is>>a;
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




	closure_test test;
	IClosure * closure = Closure_Helper(&test,&closure_test::print,1);
	closure->run();
	return 0;
}

