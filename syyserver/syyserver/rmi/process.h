#ifndef __PROCESS_H__90091kd__
#define __PROCESS_H__90091kd__

enum TASK_ERROR
{
	TE_NOERROR,
	TE_TIMEOUT,
	TE_NETERROR,
	TE_P2P_MISS,
};

enum TASK_STATE
{
	TS_INVALID,
	TS_RUNNING,
	TS_BLOCKED,
	TS_SLEEPING,
	TS_FINISHED,
};
typedef int TaskID;

#define TASK_SAFE_CHECK(TT) TT
#define NO_TASK_SAFE_CHECK
#include "../platform/ByteStream.h"
#include "rmi.h"
class CTask;
//struct TaskTupleHelper;
//struct InvokeHelper;
struct TaskToken//确保只在task内部生成
{
	friend class CTask;
// 	friend struct TaskTupleHelper;
// 	friend struct InvokeHelper;
// 	template<typename T0, typename T1, typename T2, typename T3, typename T4,
// 	typename T5, typename T6, typename T7, typename T8, typename T9> friend struct tuple;
// 	template<typename T0, typename T1, typename T2, typename T3, typename T4,
// 	typename T5, typename T6, typename T7, typename T8, typename T9> friend struct rmi_tuple;
private:
	TaskToken(){}
};
template<typename ST>
inline COStreamGeneric<ST> &operator <<(COStreamGeneric<ST> &os, const TaskToken& TT)
{
	return os;
}

template<typename ST>
inline CIStreamGeneric<ST> &operator >>(CIStreamGeneric<ST> &is, TaskToken& TT)
{
	return is;
}

struct fun_frame;
struct ITaskletManager;
struct ITaskletNotifier;
struct ITasklet
{
	struct params_t
	{
		int   p0;
		int   p1;
		int64 p2;

		params_t() : p0(0), p1(0), p2(0)
		{
		}
	};
	virtual ITaskletManager* GetTaskManager() = 0;
	virtual void UpdateTask() = 0;
	virtual void Sleep(int heart_beat) = 0;
	virtual int DecSleepBeat(int beat) = 0;

	virtual TaskID GetTaskID() = 0;
	virtual TASK_STATE GetTaskState() = 0;
	virtual void SetTaskState(TASK_STATE task_state) = 0;

	virtual params_t& GetContext() = 0;
	virtual void __SetResultPointer(ITaskletNotifier* notifier) = 0;
	virtual void __Release() = 0;
};
struct ITaskletNotifier
{
	virtual void OnTaskFinish(ITasklet* tasklet) = 0;
	virtual void* GetResultPointer() = 0;
};

struct ITaskletManager
{
	virtual void UpdateTaskletManager(unsigned int current_tick) = 0;
	virtual TaskID StartTask(fun_frame* fun) = 0;
	virtual ITasklet* FindTask(TaskID id) = 0;
	virtual int CountTask() = 0;
	virtual TaskID CurrentTask() = 0;
	//virtual TASK_ERROR LastError() = 0;
	//virtual void ClearError() = 0;
	//virtual void __SetError(TASK_ERROR err) = 0;

	virtual void __PushFrame(fun_frame* frame) = 0;	//压栈
	virtual void __PopFrame() = 0;					//弹出栈
	virtual fun_frame* __PeekFrame() = 0;		//得到最上面的frame，只能得到一次，第二次就没了
};

//--------------------新的Task系统--------------------
// #include <platform/platform_shared/BiboFrame/ByteStream.h>
/*****************************************//**
 * 函数闭包接口定义
 ********************************************/
// struct IFuncClosure
// {
// 	virtual void Execute( void ) = 0;
// 	virtual void Release( void ) = 0;
// 	virtual void FetchResult( void* presult ) = 0;
// 	virtual void FetchOutParam( COStream& os ) = 0;
// };

/*****************************************//**
 * Task接口定义
 ********************************************/
struct ITaskStartNotifier;
struct ITaskResultNotifier;
struct ITaskFinishNotifier;
struct ITask
{
	/*!
	 * \brief 用于保存用户信息的结构
	 */
	struct params_t
	{
		int   p0;
		int   p1;
		int64 p2;

		params_t() : p0(0), p1(0), p2(0)
		{
		}
	};

	virtual TaskID GetID( void ) const = 0;
	/*!
	 * \brief 放弃当前操作，处于可继续执行状态(不占用CPU时间)，经过指定
	 *		  时间后自动恢复。
	 *	\param milliseconds 指定时间间隔(单位:毫秒)
	 */
	virtual void _Yield( TaskToken& TT, size_t milliseconds ) = 0;
	/*!
	 * \brief 挂起当前操作，但处于可继续执行的状态。不占用CPU时间，不阻塞。
	 *	\warning 如果不调用 _Resume 操作，则一直处于挂起状态.
	 */
	virtual void _Suspend( TaskToken& TT ) = 0;
	/*!
	 * \brief 恢复挂起的操作
	 */
	virtual void _Resume( void ) = 0;
	/*!
	 * \brief 结合一个SubTask， SubTask结束前当前操作处于挂起状态
	 *	\param id SubTask的ID
	 */
	virtual void _Join( TaskToken& TT, TaskID id ) = 0;
	/*!
	 * \brief 设置Task通知者，以获取Task执行结果，掌握Task结束信号。
	 *	\param notifier Task通知者
	 */
	virtual void _SetResultNotifier( ITaskResultNotifier* notifier ) = 0;
	/*!
	 * \brief 设置Task通知者，以获取Task执行结果，掌握Task结束信号。
	 *	\param notifier Task通知者
	 */
	virtual void _SetFinishNotifier( ITaskFinishNotifier* notifier ) = 0;
	/*!
	 * \brief 设置Task通知者，以获取Task执行结果，掌握Task结束信号。
	 *	\param notifier Task通知者
	 */
	virtual void _SetStartNotifier( ITaskStartNotifier* notifier ) = 0;
	/*!
	 * \brief 返回Task通知者
	 */
	virtual ITaskFinishNotifier* _GetFinishNotifier() = 0;
	virtual ITaskResultNotifier* _GetResultNotifier() = 0;
	virtual ITaskStartNotifier* _GetStartNotifier() = 0;
	/*!
	 * \brief 获取/设置用户数据，数据具体含义由用户自行解释。
	 */
	virtual params_t& GetContext( void ) = 0;

	virtual void FetchResult( void* presult ) = 0;
	virtual void FetchOutParam( COStream& os  ) = 0;

	virtual TaskToken& _GetTaskToken() = 0;
};

/*****************************************//**
 * Task管理器接口
 ********************************************/
// interface IEventSelector;
struct ITaskManager//:public ITimerCallback
{
	virtual TaskID StartTask( IClosure* closure ) = 0;
	virtual void ResumeTask( TaskID id ) = 0;
	/*!
	 * \brief 获取当前运行的Task
	 *	\warning 如果当前不在Task内，则返回NULL
	 */
	virtual ITask* CurrentTask( void ) = 0;
	virtual ITask* FindTask( TaskID id ) = 0;

	virtual int TotalTask() = 0;

// 	virtual IEventSelector* GetEventSelector( void ) = 0;

	virtual void Release() = 0;
};

/*****************************************//**
 * Task通知接口
 *		1.Task结束通知
 ********************************************/
struct ITaskFinishNotifier
{
	virtual void OnTaskFinish( TaskToken& TT, ITask* tsk, bool executed ) = 0;
};

struct ITaskResultNotifier
{
	virtual void OnTaskResult( TaskToken& TT, ITask* tsk, bool executed ) = 0;
};

struct ITaskStartNotifier
{
	virtual bool OnTaskStart( TaskToken& TT, ITask* tsk ) = 0;
};
// /*****************************************//**
//  * 函数闭包对象
//  ********************************************/
// // #include <common_lib/cpp_templates/closure.h>
// 
// template < typename FunctionT, int IS_RMI = 0 >
// struct CFuncClosure : public IFuncClosure
// {
// public:
// 	typedef memfun_trait<FunctionT> FunTraitT;
// 
// 	typedef typename FunTraitT::class_type		ClassT;
// 	typedef typename FunTraitT::function_type	FuncT;
// 	typedef typename FunTraitT::result_type		ReT;
// 
// 	typedef typename __arg_type_selector<FunctionT, IS_RMI>::args_type ArgsT;
// 
// 	typedef typename __type_selector< h3d_is_void<ReT>::value
// 									, closure_impl_normal<FuncT, IS_RMI>
// 									, closure_impl_void<FuncT, IS_RMI> >::type ClosureT;
// private:
// 	ClosureT m_closure;
// public:
// 	CFuncClosure( ClassT* cls, FuncT func, const ArgsT& args )
// 		:m_closure( cls, func, args )
// 	{
// 	}
// 	CFuncClosure( ClassT* cls, FuncT func, CIStream& is )
// 		:m_closure( cls, func, is )
// 	{
// 	}
// 	virtual void Execute( void )
// 	{
// 		m_closure();
// 	}
// 	virtual void Release( void )
// 	{
// 		delete this;
// 	}
// 	virtual void FetchResult( void* presult )
// 	{
// 		m_closure.fetch_result( presult );
// 	}
// 	virtual void FetchOutParam( COStream& os )
// 	{
// 		m_closure.m_args.LoadWriteable( os );
// 	}
// };
// 
// /*****************************************//**
//  * Task帮助函数
//  ********************************************/
// inline void YIELD( TaskToken& TT, size_t milliseconds )
// {
// 	ITaskManager* tskmng = GetBiboRegistry()->GetTaskManager();
// 	ITask* curr_task = tskmng->CurrentTask();
// 
// // 	strongAssert( curr_task );
// 	curr_task->_Yield( TT, milliseconds );
// }
// 
// inline void SUSPEND( TaskToken& TT )
// {
// 	ITaskManager* tskmng = GetBiboRegistry()->GetTaskManager();
// 	ITask* curr_task = tskmng->CurrentTask();
// 
// // 	strongAssert( curr_task );
// 	curr_task->_Suspend(TT);
// }
// 
// inline void JOIN( TaskToken& TT, TaskID child )
// {
// 	ITaskManager* tskmng = GetBiboRegistry()->GetTaskManager();
// 	ITask* curr_task = tskmng->CurrentTask();
// 
// // 	strongAssert( curr_task && curr_task->GetID()!=child );
// 	curr_task->_Join( TT, child );
// }
// 
// inline void RESUME( TaskID tskid )
// {
// 	ITaskManager* tskmng = GetBiboRegistry()->GetTaskManager();
// 	tskmng->ResumeTask( tskid );
// }
// 
// inline TaskID CurrentTaskID( void )
// {
// 	ITaskManager* tskmng = GetBiboRegistry()->GetTaskManager();
// 	ITask* curr_task = tskmng->CurrentTask();
// 	return curr_task ? curr_task->GetID() : 0;
// }
// 
// inline ITask* CurrentTask( void )
// {
// 	ITaskManager* tskmng = GetBiboRegistry()->GetTaskManager();
// 	return tskmng->CurrentTask();
// }
// 
// inline ITask* FindTask( TaskID id )
// {
// 	ITaskManager* tskmng = GetBiboRegistry()->GetTaskManager();
// 	return tskmng->FindTask( id );
// }
// 
// /*! 启动Task */
// template< typename ClassT, typename FuncT >
// TaskID START_TASK_IMP( ClassT* cls, FuncT func );
// 
// /*! 启动远程调用Task */
// template< typename ClassT, typename FuncT, typename ArgsT >
// inline TaskID START_RMI_TASK( ClassT* cls, FuncT func, const ArgsT& args )
// {
// 	ITaskManager* tskmng = GetBiboRegistry()->GetTaskManager();
// 	IFuncClosure* closure = new CFuncClosure<FuncT,1>(cls,func,args);
// 	return tskmng->StartTask( closure );
// }
// 
// /*! 使用Stream作为参数启动Task */
// template< typename ClassT, typename FuncT>
// inline TaskID START_TASK_FROMSTREAM( ClassT* cls, FuncT func, CIStream& is)
// {
// 	ITaskManager* tskmng = GetBiboRegistry()->GetTaskManager();
// 	IFuncClosure* closure = new CFuncClosure<FuncT,0>(cls,func,is);
// 	return tskmng->StartTask( closure );
// }
// 
// /*! 使用Stream作为参数启动远程调用Task */
// template< typename ClassT, typename FuncT>
// inline TaskID START_RMI_TASK_FROMSTREAM( ClassT* cls, FuncT func, CIStream& is)
// {
// 	ITaskManager* tskmng = GetBiboRegistry()->GetTaskManager();
// 	IFuncClosure* closure = new CFuncClosure<FuncT,1>(cls,func,is);
// 	return tskmng->StartTask( closure );
// }
// 
// inline void MUST_BE_RUN_AS_TASK( void )
// {
// 	ITask* curr_task = GetBiboRegistry()->GetTaskManager()->CurrentTask();
// 	if ( !curr_task )
// 	{
// #if _DEBUG
// // 		strongAssert(0 && "Current Task not found, you cannot call a task directly!!!");
// #else
// // 		LogError( "Current Task not found!!!" );
// #endif
// 	}
// }

//----------------------------------------------
// #include "process.inl"
// 
// #include "../process/function_object.h"
// 
// 
// inline void __CHECK_LAST_ERROR( void )
// {
// 	if ( int err = GetBiboRegistry()->LastError() )
// 	{
// 		LogError( "WARNING!!! Unhandled LastError : ", err);
// 		GetBiboRegistry()->ClearError();
// 	}
// }

#endif
