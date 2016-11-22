#ifndef SingleTone_H_2016_11_21
#define SingleTone_H_2016_11_21
#include<stdlib.h>
#include "_platform_def.h"

#ifndef PLATFORM_WIN32
	#include <pthread.h>
#else
	#include <winbase.h>
typedef bool pthread_once_t;
#define PTHREAD_ONCE_INIT false;
#endif

template<typename T>
class Singleton
{
public:

	static T& instance()
	{
#ifndef PLATFORM_WIN32
		pthread_once(&ponce_, &Singleton::init);
#else
		if(!ponce_) 
			init(),ponce_ = !ponce_;
#endif
		return *value_;
	}

private:
	Singleton();
	~Singleton();

	static void init()
	{
		value_ = new T();
		::atexit(destroy);
	}

	static void destroy()
	{
		typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
		delete value_;
	}

private:
	static pthread_once_t ponce_;
	static T*             value_;
};

template<typename T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;
template<typename T>
T* Singleton<T>::value_ = NULL;


#endif//SingleTone_H_2016_11_21