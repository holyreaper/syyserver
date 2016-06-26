#ifndef AUTO_PTR_HPP_2016_06_26
#define AUTO_PTR_HPP_2016_06_26
#include "_platform_def.h"

typedef unsigned int auto_ptr_ref;

struct auto_ptr_err
{
	enum
	{
		P_NULL=-1,
		P_COPY_SELF=-2,
	};
};
template<typename T>

class auto_ptr
{
public:

// 	auto_ptr():p(new T),ref(new sizeof(auto_ptr_ref))
// 	{
// 
// 	}
	explicit auto_ptr(T*):p(new T),ref(new sizeof(auto_ptr_ref))
	{

	}
	T* operator ->()
	{
		return p;
	}
	T& operator* ()
	{
		if (p)
		{
			return *p;
		}
		throw  auto_ptr_err(auto_ptr_err::P_NULL);
	//	assert(0);
	}
	auto_ptr(auto_ptr& other )
	{
		if (this != &other)
		{
			releasecount();
			p = other.p;
			ref=other.ref;
			*ref++;
		}
		else
		{
			throw  auto_ptr_err::P_COPY_SELF;
		}
	}

	auto_ptr& operator = (const auto_ptr& other)
	{
		if (this != &other)
		{
			releasecount();
			p = other.p;
			ref=other.ref;
			*ref++;
		}
		else
		{
			throw  auto_ptr_err::P_COPY_SELF;
		}
	}
	~auto_ptr()
	{
		releasecount();
	}
private:
	T* p;
	auto_ptr_ref* ref;

	void releasecount()
	{
		if (ref)
		{
			*ref--;
			if (*ref <=0)
			{
				if (p)
				{
					delete p;
					p=NULL;
				}
				delete ref;
				ref =NULL;
			}
		}
	}

};
#endif//AUTO_PTR_HPP_2016_06_26