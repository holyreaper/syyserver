#ifndef RMI_DEFINED_H_2016_0719
#define RMI_DEFINED_H_2016_0719
/*
	date: 2016-07-26
	subject:服务器rmi基类
	auth:lhm
*/
#include "../platform/ByteStream.h"
#include "../platform/closure.h"
#include "../event/event.pb.h"

// static std::map<int,IClosure*> _closure_map;


#define REGISTER_RMI(id,cls,func_name) \
	Closure_Helper<cls,func_name>(id,this, func_name); 


static int g_count =0;

struct testsc
{
	testsc()
	{
		a =1; b=2;
	}
	int a ;
	int b;
};

template<typename TT, typename FF,typename AA>
IClosure* Closure_Helper(int id,TT *a ,FF b,AA c)
{
	a->_closure_map[id] = new Closure<TT,FF,AA>(a,b);
	return  a->_closure_map[id];
};
template<typename TT, typename FF>
IClosure* Closure_Helper(int id,TT *a ,FF b)
{
	 a->_closure_map[id] = new Closure<TT,FF,Event>(a,b);
	return  a->_closure_map[id];
};
// template<typename FF>

// IClosure* Closure_Helper(FF b)
// {
// };

// inline void FiberFunc(void * param)
// {
// 	g_count++;
// 	//printf("%s count:%d\n",param,g_count++);
// };

template<typename ST>
inline CIStreamGeneric<ST> &operator >>(CIStreamGeneric<ST> &from, testsc& info)
{
	from >>info.a
		>>info.b;
	return from;
}
template<typename ST>
inline COStreamGeneric<ST> &operator <<(COStreamGeneric<ST> &from, const testsc& info)
{
	from <<info.a
		<<info.b;
	return from;
}
struct FuncInfo
{
	FuncInfo():id(0)
	{

	}
	int64 id;
};
// inline bool ParseData(CDynamicStreamBuf& buf,IClosure* closure)
// {
// 	closure->LoadFromStream(buf);
// }
struct RMIServerBase
{
	virtual void call_func(Event* ev)=0;
	virtual void  RegisterRmi()	=0;
};
#define RMI_SERVER_CLASS_DECLARE(class_name,inherit_class)			\
template<typename base_class>										\
class class_name:public RMIServerBase,public inherit_class{			\
public:																\
class_name(ITaskManager* taskmng):_task_mng(taskmng)				\
{																	\
																	\
}																	\
public:																\
virtual void call_func(Event* ev)									\
	{																\
		if(ev->cmd()<= PFPE_START || ev->cmd() >=PFPE_MAX)return;	\
		IClosure* clo = _closure_map[ev->cmd()];					\
		clo->LoadFromStream(ev);									\
		_task_mng->StartTask(clo);				\
	}																\
public:																\
IClosure * _closure_map[PFPE_MAX] ;								\
ITaskManager* _task_mng;												\

#define RMI_WRAPPER_CONSTRUCTOR										\
public:																\
	void RegisterRmi()											
															
#define RMI_SERVERFUNCTION_END() };

enum PLAYER_FUNC_PARAMER_ENUM
{	
	PFPE_START=0,
	PFPE_LOGIN,
	PFPE_REPORT,
	PFPE_MAX,
};
#endif//RMI_DEFINED_H_2016_0719