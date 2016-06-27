/***********************************************
* @file		task.h
* @brief	Task库实现
* @author	仲子路(zhongzilu@h3d.com.cn)
*
* 功能说明：1.Task实现
*			2.TaskManager实现
*			3.START_TASK帮助函数实现
* 
* 更新日志:
*			[what] by [who] at [when]
***********************************************/
#ifndef PLATFORM_TASK_IMPLEMENTION_H
#define PLATFORM_TASK_IMPLEMENTION_H

#include <map>
#include <list>

#ifdef _HASH_MAP_
#include <unordered_map>
#endif
#include "process.h"

// #include <platform/platform_shared/BiboFrame/BiboPlatform.h>
// #include <platform/platform_shared/BiboFrame/BiboInterfaces.h>
// #include <platform/network/net.h>

typedef int TaskID;

const size_t DEFAULT_CO_POOL_MINNUM = 50;
const size_t DEFAULT_CO_POOL_MAXNUM = 100;
const size_t DEFAULT_STACK_SIZE		= 1024*64;
const size_t DEFAULT_YIELD_TIME		= 500;

//------------------------------------------
#include "taurus.h"
#include "../platform/_platform_def.h"
class CTaskManager;
class CTask : public ITask
{
	TaskID			m_id;
	IFuncClosure*	m_closure;

	CTaskManager*	m_tsk_mng;

	coroutine_t		m_co;
	co_thread_t		m_co_thread;

	ITaskResultNotifier*	m_result_notifier;
	ITaskFinishNotifier*	m_finish_notifier;
	ITaskStartNotifier*		m_start_notifier;

	params_t		m_user_params;

	int64			m_timer_id;
	IEventSelector* m_event_selector;

	TaskToken		m_task_token;

public:
	CTask( CTaskManager* mng, TaskID id, co_thread_t cothread, IEventSelector* slec, IFuncClosure* closure );
	~CTask();

	void Init( size_t stack_size );

	TaskID GetID( void ) const 
	{ 
		return m_id; 
	}

	void _Yield( TaskToken& TT, size_t milliseconds );

	void _Suspend( TaskToken& TT );
	void _Resume( void );

	void _Join( TaskToken& TT, TaskID id );

	virtual void _SetResultNotifier( ITaskResultNotifier* notifier );
	virtual void _SetFinishNotifier( ITaskFinishNotifier* notifier );
	virtual void _SetStartNotifier( ITaskStartNotifier* notifier );
	virtual ITaskFinishNotifier* _GetFinishNotifier();
	virtual ITaskResultNotifier* _GetResultNotifier();
	virtual ITaskStartNotifier* _GetStartNotifier();
	virtual TaskToken& _GetTaskToken();

	params_t& GetContext( void );

	void FetchResult( void* presult );
	void FetchOutParam( COStream& os );

	void OnTimer(void);

private:
	void Execute( void );
	static void FiberProc(void* lpParameter);

	void __ExecuteImpl( void );
};

class CTaskManager : public ITaskManager
				   , public ITimerCallback
				   , public INoticeCallback
{
	struct ReadyInfo
	{
		bool is_new;
		union
		{
			CTask* task;
			TaskID task_id;
		};

		ReadyInfo(CTask* t) : is_new(true), task(t)
		{
		}

		ReadyInfo(TaskID id) : is_new(false), task_id(id)
		{
		}

	};

	co_thread_t		m_co_thread;
	int				m_id_generator;

	size_t			m_default_stack_sz;

#ifdef _HASH_MAP_
	typedef std::unordered_map<TaskID,CTask*> SearchTable;
	typedef std::unordered_map<TaskID,TaskID> JoinTable;
#else
	typedef std::map<TaskID,CTask*> SearchTable;
	typedef std::map<TaskID,TaskID> JoinTable;
#endif
	typedef std::list<ReadyInfo> ReadyList;

	SearchTable		m_search_table;
	JoinTable		m_join_table;
	ReadyList		m_ready_list;

	IEventSelector* m_event_selector;

	bool _already_add_notice;

public:
	explicit CTaskManager( co_thread_t co_thread, int stack_size );
	~CTaskManager();

	TaskID StartTask( IFuncClosure* closure );
	void ResumeTask( TaskID id );

	ITask* CurrentTask( void );
	ITask* FindTask( TaskID id );

	virtual int TotalTask();

	virtual void Release();

	void _OnTaskYield( ITask* tsk, size_t cycle );
	void _OnTaskSuspend( ITask* tsk );
	void _OnTaskJoin( TaskID parent, TaskID child );
	void _OnTaskFinished( ITask* tsk );

	IEventSelector* GetEventSelector( void );

	virtual void OnTimer(void* user_ptr);
	virtual void ReleaseTimerCallback();
	virtual void OnNotice(void* user_ptr);
	virtual void ReleaseNoticeCallback();

	void AddNotice( void );

private:
	TaskID AllocID( void );
	void ResumeParentTask( TaskID id );
	CTask* _FindTask( TaskID id );

	void _CHECK_RESUME_SELF_ERROR( TaskID id );
};

//--------------------
inline void OnCoroutineStackOverflow( void* addr )
{
// 	LogFatal( "Callstack overflow from a coroutine! at 0x", addr );
// 	alwaysAssert( false );
}

#if (PLATFORM_WIN32)
	inline int SigStackHandler(struct _EXCEPTION_POINTERS *ep)
	{
		if ( ep->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW )
		{
			void* addr = ep->ExceptionRecord->ExceptionAddress;
			OnCoroutineStackOverflow( addr );
			return EXCEPTION_EXECUTE_HANDLER;
		}
		return EXCEPTION_CONTINUE_SEARCH;
	}
	inline void Set_SIGSEGV_Handler( void )
	{
	}
#else

#include <signal.h>
#include <execinfo.h>
#include <ucontext.h>
#include <cxxabi.h>

	inline void PrintStack( ucontext_t* uc )
	{
		Dl_info dlinfo;
		void **bp = (void**)uc->uc_mcontext.gregs[REG_RBP];
		void *ip = (void*)uc->uc_mcontext.gregs[REG_RIP];

		unsigned int frame_idx = 0;
		while( bp && ip )
		{
			if(!dladdr(ip, &dlinfo))
				break;

			int status;
			char * symname = abi::__cxa_demangle( dlinfo.dli_sname, NULL, 0, &status);
			if ( !symname || status != 0 )
				break;


			if(!strcmp(symname, "CTask::Execute()"))
			{
				free( symname );
				break;
			}

			unsigned int len = (unsigned long long)(*(void**)bp[0]) - (unsigned long long)bp;
			LogFatal( "#",frame_idx, " (",len,"bytes) ",symname );

			free( symname );

			ip = bp[1];
			bp = (void**)bp[0];
			++frame_idx;
		}
	}

	inline void SigStackHandler(int signum, siginfo_t *info, void *secret)
	{
		LogFatal( "==================SIGSEGV===================" );

		PrintStack( (ucontext_t *)secret );
		OnCoroutineStackOverflow( info->si_addr );
	}
	inline void Set_SIGSEGV_Handler( void )
	{
		LogError( "Set_SIGSEGV_Handler" );
		//create signal stack
		stack_t sigstack;
		sigstack.ss_sp = malloc(SIGSTKSZ);
		sigstack.ss_size = SIGSTKSZ;
		sigstack.ss_flags = 0;
		if (sigaltstack(&sigstack, NULL) == -1)
			LogError( "Sigaltstack fail." );
		//bind signal SIGSEGV
		struct sigaction action;
		memset( &action, 0, sizeof(action) );
		action.sa_flags = SA_SIGINFO|SA_STACK;
		sigemptyset(&action.sa_mask);
		action.sa_sigaction = &SigStackHandler;
		if ( sigaction(SIGSEGV, &action, NULL) == -1 )
			LogError( "Set_SIGSEGV_Handler fail." );
	}
#endif


#endif //PLATFORM_TASK_IMPLEMENTION_H