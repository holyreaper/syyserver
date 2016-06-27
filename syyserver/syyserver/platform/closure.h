
#ifndef CLOSURE_H_2016_06_27_
#define CLOSURE_H_2016_06_27_



struct IClosure
{	

	virtual void run()=0;
	virtual void Release( void ) = 0;
};

template<typename Class , typename Func,typename Argc>
class Closure:public IClosure
{
public:


	Closure(Class* obj,Func fun, Argc argc)
	{
		m_obj= obj;
		m_func = fun;
		m_argc = argc;
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

private: 
	Class* m_obj;
	Func  m_func;
	Argc m_argc;
};


#endif