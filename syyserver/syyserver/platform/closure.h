
#ifndef CLOSURE_H_2016_06_27_
#define CLOSURE_H_2016_06_27_

#include "../event/event.pb.h"
class NULL_TYPE{};

struct IClosure
	{	

	virtual void run()=0;
	virtual void Release( void ) = 0;
	virtual void LoadFromStream(Event * event) =0;

};
// template<typename Class , typename Func,typename Argc=NULL_TYPE, typename Argc2=NULL_TYPE,typename Argc3=NULL_TYPE,typename Argc4=NULL_TYPE >
// class Closure:public IClosure
// {
// public:
// 
// 	Closure(Class* obj,Func fun)
// 	{
// 		m_obj= obj;
// 		m_func = fun;
// 	}
// 	~Closure()
// 	{
// 		Release();
// 	}
// 	virtual void run()
// 	{
// 		(m_obj->*m_func)(m_argc,m_argc2,m_argc3);
// 	}
// 	virtual void Release( void )
// 	{
// 		delete this;
// 	}
// 	virtual void LoadFromStream(Event * event)
// 	{
// 		CIStream is(buf);
// 		is >> m_argc >>m_argc2>>m_argc3>>m_argc4 ;
// 	}
// 
// private: 
// 	Class* m_obj;
// 	Func  m_func;
// 	Argc m_argc;
// 	Argc2 m_argc2;
// 	Argc3 m_argc3;
// 	Argc4 m_argc4;
// };
// template<typename Class , typename Func,typename Argc, typename Argc2,typename Argc3 >
// class Closure<Class,Func,Argc,Argc2,Argc3,NULL_TYPE>:public IClosure
// {
// public:
// 
// 	Closure(Class* obj,Func fun)
// 	{
// 		m_obj= obj;
// 		m_func = fun;
// 
// 	}
// 	~Closure()
// 	{
// 		Release();
// 	}
// 	virtual void run()
// 	{
// 		(m_obj->*m_func)(m_argc,m_argc2,m_argc3);
// 	}
// 	virtual void Release( void )
// 	{
// 		delete this;
// 	}
// 	virtual void LoadFromStream(Event * event)
// 	{
// 		CIStream is(buf);
// 		is >> m_argc >>m_argc2>>m_argc3 ;
// 	}
// 
// private: 
// 	Class* m_obj;
// 	Func  m_func;
// 	Argc m_argc;
// 	Argc2 m_argc2;
// 	Argc3 m_argc3;
// };
// template<typename Class , typename Func,typename Argc, typename Argc2 >
// class Closure<Class,Func,Argc,Argc2,NULL_TYPE,NULL_TYPE>:public IClosure
// {
// public:
// 
// 	Closure(Class* obj,Func fun)
// 	{
// 		m_obj= obj;
// 		m_func = fun;
// 	}
// 	~Closure()
// 	{
// 		Release();
// 	}
// 	virtual void run()
// 	{
// 		(m_obj->*m_func)(m_argc,m_argc2);
// 	}
// 	virtual void Release( void )
// 	{
// 		delete this;
// 	}
// 	virtual void LoadFromStream(Event * event)
// 	{
// 		CIStream is(buf);
// 		is >> m_argc >>m_argc2;
// 	}
// 
// private: 
// 	Class* m_obj;
// 	Func  m_func;
// 	Argc m_argc;
// 	Argc2 m_argc2;
// 	
// };
template<typename Class,typename Func,typename Argc>
class Closure:public IClosure
{
public:
	Closure(Class* obj,Func fun)
	{
		m_obj= obj;
		m_func = fun;
	}
	~Closure()
	{
		Release();
	}
	virtual void run()
	{
		(m_obj->*m_func)(m_argc);
	}
	virtual void Release( void )
	{
		delete this;
	}
	virtual void LoadFromStream(Event * event)
	{
		m_argc.CopyFrom(*event);
	}

private: 
	Class* m_obj;
	Func  m_func;
	Argc m_argc;
};
// template<typename Class , typename Func>
// 
// class Closure<Class,Func,NULL_TYPE,NULL_TYPE,NULL_TYPE,NULL_TYPE>:public IClosure
// {
// public:
// 	Closure(Class* obj,Func fun)
// 	{
// 		m_obj= obj;
// 		m_func = fun;
// 	}
// 	~Closure()
// 	{
// 		Release();
// 	}
// 	virtual void run()
// 	{
// 		(m_obj->*m_func)();
// 	}
// 	virtual void Release( void )
// 	{
// 		delete this;
// 	}
// 	virtual void LoadFromStream(Event * event)
// 	{
// 		
// 	}
// 
// private: 
// 	Class* m_obj;
// 	Func  m_func;
// };
#endif