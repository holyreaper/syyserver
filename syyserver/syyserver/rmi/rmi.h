#ifndef RMI_DEFINED_H_2016_0719
#define RMI_DEFINED_H_2016_0719
/*
	date: 2016-07-26
	subject:服务器rmi基类
	auth:lhm
*/
#include "../platform/ByteStream.h"
#include "../platform/closure.h"


static std::map<int,IClosure*> g_closure_map;


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
	void test2(testsc c)
	{
		std::cout<<c.a<<std::endl;
		std::cout<<c.b<<std::endl;

	}
protected:
private:
};

template<typename TT, typename FF,typename AA>
IClosure* Closure_Helper(int id,TT *a ,FF b,AA c)
{

	 g_closure_map[id] = new Closure<TT,FF,AA>(a,b);
	return NULL;
};
template<typename TT, typename FF>
IClosure* Closure_Helper(int id,TT *a ,FF b)
{
	 g_closure_map[id] = new Closure<TT,FF>(a,b);
	return NULL;
};
// template<typename FF>

// IClosure* Closure_Helper(FF b)
// {
// };

inline void FiberFunc(void * param)
{
	g_count++;
	//printf("%s count:%d\n",param,g_count++);
};

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
inline bool ParseData(CDynamicStreamBuf& buf,IClosure* closure)
{
	closure->LoadFromStream(buf);
}
struct RMIServerBase
{
	virtual void call_func(CDynamicStreamBuf& buf)=0;
};
#define RMI_SERVER_CLASS_DECLARE(class_name,inherit_class)			\
template<typename base_class>										\
class class_name:public RMIServerBase,public inherit_class{			\
public:																\
class_name()														\
{																	\
																	\
}																	\
																	\
virtual void call_func(CDynamicStreamBuf& buf)						\
	{	FuncInfo info;												\
		CIStream is(buf);											\
		auto it = g_closure_map.find(2);							\
		it->second->LoadFromStream(buf);							\
		it->second->run();											\
		std::cout<<"helo"<<std::endl;								\
	}																
#define RMI_WRAPPER_CONSTRUCTOR										\
	void RegisterRmi()											
															
#define RMI_SERVERFUNCTION_END() };


#endif//RMI_DEFINED_H_2016_0719