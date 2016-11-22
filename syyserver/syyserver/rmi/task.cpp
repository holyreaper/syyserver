
#include "task.h"

#include "taurus_imp.h"
#include "rmi.h"
#include "../platform/_platform_def.h"
CTask::CTask( CTaskManager* mng, TaskID id, co_thread_t cothread, IClosure* closure )
	:m_tsk_mng( mng )
	,m_id( id )
	,m_co_thread( cothread )
	,m_timer_id( 0 )
	/*,m_event_selector( slec )*/
	,m_result_notifier(NULL)
	,m_start_notifier(NULL)
	,m_finish_notifier(NULL)
{
	m_closure = closure;
	m_co = 0;
}
CTask::~CTask()
{
	//m_closure->Release();

	if ( m_timer_id )
	{
// 		m_event_selector->CancelTimer( m_timer_id );
	}
}

void CTask::_Yield( TaskToken& TT, size_t milliseconds )
{
// 	sysAssert( m_timer_id == 0 );

// 	m_timer_id = m_event_selector->AddTimer( milliseconds, IEventSelector::TIMER_WAIT, m_tsk_mng, this ); 
	m_tsk_mng->_OnTaskYield( this, milliseconds );
	co_yield( m_co_thread, m_co );
}

void CTask::_Suspend( TaskToken& TT )
{
	m_tsk_mng->_OnTaskSuspend( this );
	co_yield( m_co_thread, m_co );
}

void CTask::_Resume( void )
{
	if ( m_timer_id != 0 ) //并非time回调，而是使用者主动Resume
	{
//		m_event_selector->CancelTimer( m_timer_id );
		m_timer_id = 0;
	}

	co_resume( m_co_thread, m_co );
}

void CTask::_Join( TaskToken& TT, TaskID id )
{
	ITask* task = m_tsk_mng->FindTask( id );
	if ( task )
	{
		m_tsk_mng->_OnTaskJoin( m_id, id );
		_Suspend(TT);
	}
}

void CTask::_SetResultNotifier( ITaskResultNotifier* notifier )
{
	m_result_notifier = notifier;
}

void CTask::_SetFinishNotifier( ITaskFinishNotifier* notifier )
{
	m_finish_notifier = notifier;
}

void CTask::_SetStartNotifier( ITaskStartNotifier* notifier )
{
	m_start_notifier = notifier;
}

ITaskFinishNotifier* CTask::_GetFinishNotifier()
{
	return m_finish_notifier;
}

ITaskResultNotifier* CTask::_GetResultNotifier()
{
	return m_result_notifier;
}

ITaskStartNotifier* CTask::_GetStartNotifier()
{
	return m_start_notifier;
}


CTask::params_t& CTask::GetContext( void )
{
	return m_user_params;
}

void CTask::FetchResult( void* presult )
{
	//m_closure->FetchResult( presult );
}

void CTask::FetchOutParam( COStream& os )
{
	//m_closure->FetchOutParam( os );
}

void CTask::Init( size_t stack_size )
{
 	m_co = co_create( m_co_thread, &CTask::FiberProc, this, stack_size );
}
 
void CTask::FiberProc(void* lpParameter)
{
	CTask* _this = reinterpret_cast<CTask*>(lpParameter);
	_this->Execute();
}

void CTask::Execute( void )
{
	bool execute = true;

	if(m_start_notifier)
	{
		execute = m_start_notifier->OnTaskStart(m_task_token, this);
	}

	if(execute)
	{
		__ExecuteImpl();
	}

	m_tsk_mng->_OnTaskFinished( this );

	if ( m_result_notifier )
	{
		m_result_notifier->OnTaskResult( m_task_token, this, execute );
	}
	if( m_finish_notifier )
	{
		m_finish_notifier->OnTaskFinish( m_task_token, this, execute );
	}

	delete this;
}

void CTask::__ExecuteImpl( void )
{
#if _PLATFORM_WIN32_
	__try
	{
		m_closure->run();
	}
	__except(SigStackHandler(GetExceptionInformation()))
	{
	}
#else
	m_closure->run();
#endif
}

void CTask::OnTimer(void)
{

	m_timer_id = 0;
	//_Resume();
}

TaskToken& CTask::_GetTaskToken()
{
	return m_task_token;
}

//----------------------------------------------
CTaskManager::CTaskManager( co_thread_t co_thread, int stack_size )
:_already_add_notice( false )
{
// 	m_event_selector = ::GetBiboRegistry()->CreateEventSelector();

	m_default_stack_sz = stack_size;

	m_co_thread = co_thread;

	m_id_generator = 0;
}

CTaskManager::~CTaskManager()
{
// 	m_event_selector->Release();
// 	alwaysAssert(m_search_table.empty());
}

TaskID CTaskManager::StartTask( IClosure* closure )
{
	TaskID id = AllocID();
	CTask* task = new CTask( this, id, m_co_thread, closure );
	
	m_search_table[id] = task;

	
	//不再直接启动，而是放到start_list里面
	//task->Init( m_default_stack_sz, closure );
	m_ready_list.push_back(task);
	AddNotice();

	return id;
}

void CTaskManager::ResumeTask( TaskID id )
{
#if _DEBUG
	_CHECK_RESUME_SELF_ERROR(id);
#endif

	if ( ITask* task = FindTask( id ) )
	{
		//不直接Resume，先放到resume_list里面
		//task->_Resume();

		m_ready_list.push_back(id);
		AddNotice();
	}
}

ITask* CTaskManager::CurrentTask( void )
{
	coroutine_t curr = co_current( m_co_thread );
	return static_cast<CTask*>(co_get_data( curr ));
}

ITask* CTaskManager::FindTask( TaskID id )
{
	return _FindTask( id );
}

void CTaskManager::_OnTaskJoin( TaskID parent, TaskID child )
{
	m_join_table.insert( std::make_pair( child, parent ) );
}

void CTaskManager::_OnTaskFinished( ITask* tsk )
{
	TaskID id = tsk->GetID();

	m_search_table.erase( id );
	ResumeParentTask( id );
}

void CTaskManager::_OnTaskYield( ITask* tsk, size_t cycle )
{
}

void CTaskManager::_OnTaskSuspend( ITask* tsk )
{
}

// IEventSelector* CTaskManager::GetEventSelector( void )
// {
// 	return m_event_selector;
// }

TaskID CTaskManager::AllocID( void )
{
	return ( ++m_id_generator ) ? m_id_generator : ++m_id_generator; 
}

void CTaskManager::ResumeParentTask( TaskID id )
{
	JoinTable::iterator itr = m_join_table.find( id );
	if ( itr != m_join_table.end() )
	{
		ResumeTask( itr->second );
		m_join_table.erase( itr );
	}
}

void CTaskManager::_CHECK_RESUME_SELF_ERROR( TaskID id )
{
	ITask* curr = CurrentTask();
	if ( curr && curr->GetID() == id )
	{
// 		lightAssert( "$HI_RED$CTaskManager::ResumeTask : Task resume self." );
		return;
	}
}

CTask* CTaskManager::_FindTask( TaskID id )
{
	SearchTable::iterator it = m_search_table.find( id );
	if ( it != m_search_table.end() )
		return it->second;
	return 0;
}

void CTaskManager::OnTimer(void* user_ptr)
{
// 	sysAssert( user_ptr );
	CTask* tsk = (CTask*)user_ptr;
	tsk->OnTimer();
	ResumeTask(tsk->GetID());
}

void CTaskManager::ReleaseTimerCallback()
{
}

void CTaskManager::OnNotice(void* user_ptr)
{
	_already_add_notice = false;
	for ( ReadyList::iterator itr = m_ready_list.begin()
		; itr != m_ready_list.end(); ++itr )
	{
		ReadyInfo& info = *itr;
		if(info.is_new)
		{
			info.task->Init(m_default_stack_sz);
		}
		else
		{
			ITask* task = FindTask(info.task_id);
			if(task)
			{
				task->_Resume();
			}
		}
	}

	m_ready_list.clear();
	//3.处理
	co_update( m_co_thread );

#ifdef _PERFORMANCE_TEST_
	{
		static tick_t last_log_taskcnt = get_tick_count();
		tick_t now = get_tick_count();
		if ( now - last_log_taskcnt > 1000*30 )
		{
			LOG_PERFORMANCE( "TASK_CNT ", m_search_table.size() );
			last_log_taskcnt = now;
		}
	}
#endif //_PERFORMANCE_TEST_
}

void CTaskManager::ReleaseNoticeCallback()
{
}

void CTaskManager::AddNotice( void )
{
	if ( !_already_add_notice )
	{
// 		m_event_selector->NotifyEvent( this, 0 );
		_already_add_notice = true;
	}
}

void CTaskManager::Release()
{
	delete this;
}

int CTaskManager::TotalTask()
{
	return m_search_table.size();
}
