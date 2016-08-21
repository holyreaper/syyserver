// ByteStream.h: interface for the CByteStream class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BYTESTREAM_H__B009F0DA_3478_4652_B154_260EA0EE14C1__INCLUDED_)
#define AFX_BYTESTREAM_H__B009F0DA_3478_4652_B154_260EA0EE14C1__INCLUDED_

// #include "BiboPlatform.h"
typedef long long int64;
typedef unsigned long long uint64;
typedef unsigned int size32_t;
#include <stdio.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <Windows.h> 
#include <TCHAR.h>
#include <direct.h>

#include <iostream>

enum STREAM_ERROR_CODE
{
	SEC_NO_ERROR = 0,
	SEC_OUT_OF_RANGE,
	SEC_INVALID_STRING,
	SEC_FIXED_STREAM_OVERFLOW,
	SEC_INVALID_CHAR,
	SEC_CHECKSUM_ERROR,
};

struct stream_error
{
	int code;

	stream_error(int c) : code(c)
	{
	}
};



class CBasicStreamBuf
{
protected:
	char*	m_bytes;
	size32_t	m_maxlen;
	size32_t	m_deflen;
public:
	CBasicStreamBuf()
		:m_bytes(0), m_maxlen(1), m_deflen(0) //maxlen不为0，是为了移位需要
	{
	}
	virtual ~CBasicStreamBuf()
	{
	}
	char* GetBytes()const
	{
		return m_bytes;
	}
	size32_t Size()const
	{
		return m_deflen;
	}
	size32_t Capacity()const
	{
		return m_maxlen;
	}
	size32_t SpaceLeft()const
	{
		return m_maxlen - m_deflen;
	}

	void Clear()
	{
		m_deflen=0;
	}
	char* GetBufRange(size32_t pos, size32_t n)
	{
		size32_t np = pos + n;

		if(pos>m_deflen || n>m_deflen || np > m_deflen)
		{
			return 0;
		}
		else
		{
			return m_bytes+pos;
		}
	}

	virtual void ReAlloc(size32_t size) = 0;
	virtual void Free() = 0;

	size32_t ExpandTo(size32_t size)
	{
		if(size > m_maxlen)
		{
			ReAlloc(size);
		}

		if(m_deflen < size)
			m_deflen = size;

		return size;
	}

	size32_t Get(void* p, size32_t pos, size32_t n)
	{
		size32_t np = pos + n;
		if(pos>m_deflen || n>m_deflen || np > m_deflen)
		{
			throw stream_error(SEC_OUT_OF_RANGE);
		}
		memcpy(p, m_bytes+pos, n);
		return n;
	}
	size32_t GetNoThrow(void* p, size32_t pos, size32_t n)
	{
		size32_t np = pos + n;
		if(pos>m_deflen || n>m_deflen || np > m_deflen)
		{
			n = m_deflen-pos;

			size32_t np2 = pos + n;
			if(n>m_deflen || np2 > m_deflen) //如果还是烂的，应该不会到这里，保险起见。
			{
				n = 0;
			}
		}
		memcpy(p, m_bytes+pos, n);
		return n;
	}
	size32_t Put(const void* p, size32_t pos, size32_t n)
	{
		size32_t total = pos + n;
		ExpandTo(total);
		memcpy(m_bytes+pos, p, n);
#if _DEBUG
		int intc = n/4; //不满4的被忽略
		const unsigned int* ptr = (const unsigned int*)p;
		for(int i=0; i<intc; ++i)
		{
			unsigned int val = ptr[i];
			if(val == 0xcccccccc
			|| val == 0xcdcdcdcd)
			{
				//strongAssert(0&&"putting uninitilized memory");
			}
		}
#endif
		return n;
	}

	void CopyFrom(const CBasicStreamBuf& src)
	{
		if(this == &src)
			return;

		Clear();
		Put(src.GetBytes(), 0, src.Size());
	}

	CBasicStreamBuf& operator=(const CBasicStreamBuf& buf)
	{
		CopyFrom(buf);
		return *this;
	}
};

template<DWORD BUF_SIZE>
class CFixedStreamBuf : public CBasicStreamBuf
{
protected:
	char buf[BUF_SIZE];
public:
	CFixedStreamBuf()
	{
		m_bytes = buf;
		m_maxlen = BUF_SIZE;
	}
	CFixedStreamBuf(const CFixedStreamBuf& input):CBasicStreamBuf()
	{
		m_bytes = buf;
		m_maxlen = BUF_SIZE;

		Put(input.GetBytes(), 0, input.Size());
	}
	CFixedStreamBuf& operator=(const CFixedStreamBuf& buf)
	{
		CopyFrom(buf);
		return *this;
	}
	CFixedStreamBuf& operator=(const CBasicStreamBuf& buf)
	{
		CopyFrom(buf);
		return *this;
	}

	virtual void ReAlloc(size32_t size) override
	{
		throw stream_error(SEC_FIXED_STREAM_OVERFLOW);
	}
	virtual void Free() override
	{
		strongAssert(0 && "no alloc, no free");
	}
};

class CDynamicStreamBuf : public CBasicStreamBuf
{
public:
	CDynamicStreamBuf(size32_t n=128)
	{
		ReAlloc(n);
	}
	CDynamicStreamBuf(const CDynamicStreamBuf& input)
		:CBasicStreamBuf()
	{
		ReAlloc(input.Capacity());
		Put(input.GetBytes(), 0, input.Size());
	}
	CDynamicStreamBuf& operator=(const CDynamicStreamBuf& buf)
	{
		CopyFrom(buf);
		return *this;
	}
	CDynamicStreamBuf& operator=(const CBasicStreamBuf& buf)
	{
		CopyFrom(buf);
		return *this;
	}
	virtual ~CDynamicStreamBuf()
	{
		Free();
	}
	virtual void ReAlloc(size32_t size) override
	{
		if(size <= m_maxlen)
			return;

		do
		{
			m_maxlen <<= 1;
		}while(size > m_maxlen);
		char* nbuf = new char[m_maxlen];
		if(m_bytes)
		{
			if(Size())
			{
				memcpy(nbuf, m_bytes, Size());
			}
			delete []m_bytes;
		}
		m_bytes = nbuf;
	}
	virtual void Free() override
	{
		delete []m_bytes;
	}
};

class CBorrowedStreamBuf : public CBasicStreamBuf
{
public:
	CBorrowedStreamBuf(char* bytes, size32_t len)
	{
		m_bytes = bytes;
		m_maxlen = len;
		m_deflen = len;
	}
protected:
	virtual void ReAlloc(size32_t size) override
	{
//		LogCritical("borrowed buffer overflow");
		throw stream_error(SEC_FIXED_STREAM_OVERFLOW);
	}
	virtual void Free() override
	{
//		strongAssert(0 && "borrowed, no free");
	}
};


// ------------- stream classes -----------------

class CStreamBase
{
protected:
	CBasicStreamBuf* m_streambuf;
	size32_t m_pos;
public:
	CStreamBase(CBasicStreamBuf *buf)
	{
		m_streambuf = buf;
		m_pos = 0;
	}
	virtual ~CStreamBase()
	{
	}
	size32_t Pos()
	{
		return m_pos;
	}
	CBasicStreamBuf* GetBuf()const
	{
		return m_streambuf;
	}
	size32_t Seek(size32_t n)
	{
		m_pos = n;
		return m_pos;
	}
};

struct network_endian_filter
{
	static short N2H(const short& v){return ntohs(v);}
	static short H2N(const short& v){return htons(v);}
	//------------------------------------
	static unsigned short N2H(const unsigned short& v){return ntohs(v);}
	static unsigned short H2N(const unsigned short& v){return htons(v);}
	//------------------------------------
	static int N2H(const int& v){return ntohl(v);}
	static int H2N(const int& v){return htonl(v);}
	//------------------------------------
	static unsigned int N2H(const unsigned int& v){return ntohl(v);}
	static unsigned int H2N(const unsigned int& v){return htonl(v);}
	//------------------------------------
	static long N2H(const long& v){return ntohl(v);}
	static long H2N(const long& v){return htonl(v);}
	//------------------------------------
	static unsigned long N2H(const unsigned long& v){return ntohl(v);}
	static unsigned long H2N(const unsigned long& v){return htonl(v);}
	//------------------------------------
	static int64 N2H(const int64& v)
	{
		int64 hi=N2H((DWORD)((v>>32)&0xffffffffL));
		int64 lo=N2H((DWORD)(v&0xffffffffL));

		return (hi | (lo<<32));
	}
	static int64 H2N(const int64& v)
	{
		int64 hi=H2N((DWORD)((v>>32)&0xffffffffL));
		int64 lo=H2N((DWORD)(v&0xffffffffL));

		return (hi | (lo<<32));
	}
	//------------------------------------
	static uint64 N2H(const uint64& v)
	{
		uint64 hi=N2H((DWORD)((v>>32)&0xffffffffL));
		uint64 lo=N2H((DWORD)(v&0xffffffffL));

		return (hi | (lo<<32));
	}
	static uint64 H2N(const uint64& v)
	{
		uint64 hi=H2N((DWORD)((v>>32)&0xffffffffL));
		uint64 lo=H2N((DWORD)(v&0xffffffffL));

		return (hi | (lo<<32));
	}
};

struct default_endian_filter
{
	static const short& N2H(const short& v){return v;}
	static const short& H2N(const short& v){return v;}
	//------------------------------------
	static const unsigned short& N2H(const unsigned short& v){return v;}
	static const unsigned short& H2N(const unsigned short& v){return v;}
	//------------------------------------
	static const int& N2H(const int& v){return v;}
	static const int& H2N(const int& v){return v;}
	//------------------------------------
	static const unsigned int& N2H(const unsigned int& v){return v;}
	static const unsigned int& H2N(const unsigned int& v){return v;}
	//------------------------------------
	static const long& N2H(const long& v){return v;}
	static const long& H2N(const long& v){return v;}
	//------------------------------------
	static const unsigned long& N2H(const unsigned long& v){return v;}
	static const unsigned long& H2N(const unsigned long& v){return v;}
	//------------------------------------
	static const int64& N2H(const int64& v){return v;}
	static const int64& H2N(const int64& v){return v;}
	//------------------------------------
	static const uint64& N2H(const uint64& v){return v;}
	static const uint64& H2N(const uint64& v){return v;}
};

class CStreamDefaultEndian : public CStreamBase
{
public:
	typedef default_endian_filter TF;
	CStreamDefaultEndian(CBasicStreamBuf* buf) : CStreamBase(buf){}
	CStreamDefaultEndian(CBasicStreamBuf& buf) : CStreamBase(&buf){}
};
class CStreamNetworkEndian : public CStreamBase
{
public:
	typedef network_endian_filter TF;
	CStreamNetworkEndian(CBasicStreamBuf* buf) : CStreamBase(buf){}
	CStreamNetworkEndian(CBasicStreamBuf& buf) : CStreamBase(&buf){}
};

template<class Base>
class CIStreamGeneric : public Base
{
public:
	CIStreamGeneric(const CBasicStreamBuf* buf) : Base(const_cast<CBasicStreamBuf*>(buf)){}
	CIStreamGeneric(const CBasicStreamBuf& buf) : Base(const_cast<CBasicStreamBuf*>(&buf)){}
	~CIStreamGeneric(){}

	int BytesLeft()
	{
		//int temp = Base::m_streambuf->size();
		//int temp2 = Base::m_pos;
		//return temp-temp2;
		return (int)Base::m_streambuf->Size() - (int)Base::m_pos;
	}
	operator bool()
	{
		return BytesLeft() > 0;
	}
	char* Skip(size32_t n)
	{
		char* ret = Base::m_streambuf->GetBufRange(Base::m_pos, n);

		if(ret)
		{
			Base::m_pos += n;
		}
		else
		{
			LogNotice("stream out of range in Skip()");
			throw stream_error(SEC_OUT_OF_RANGE);
		}
		return ret;
	}
	size32_t Read(void* p, size32_t n)
	{
		size32_t res = Base::m_streambuf->Get(p, Base::m_pos, n);
		Base::m_pos += res;
		return res;
	}
	size32_t ReadNoThrow(void* p, size32_t n)
	{
		size32_t res = Base::m_streambuf->GetNoThrow(p, Base::m_pos, n);
		Base::m_pos += res;
		return res;
	}
	//////////////////////////////////////////////////
	CIStreamGeneric& operator >>(CBasicStreamBuf &buf)
	{
		unsigned int size;
		(*this)>>size;
		buf.Put(Skip(size), 0, size);
		return *this;
	}
	///////////////////////////////////////////////////////////
	CIStreamGeneric& operator >>(bool &v)
	{
		Base::m_pos += Base::m_streambuf->Get(&v,Base::m_pos,sizeof(v));
		return *this;
	}
	///////////////////////////////////////////////////////////
	CIStreamGeneric& operator >>(char &v)
	{
		Base::m_pos += Base::m_streambuf->Get(&v,Base::m_pos,sizeof(v));
		return *this;
	}
	CIStreamGeneric& operator >>(unsigned char &v)
	{
		Base::m_pos += Base::m_streambuf->Get(&v,Base::m_pos,sizeof(v));
		return *this;
	}
	///////////////////////////////////////////////////////////
	CIStreamGeneric& operator >>(int &v)
	{
		Base::m_pos += Base::m_streambuf->Get(&v,Base::m_pos,sizeof(v));
		v = Base::TF::N2H(v);
		return *this;
	}
	CIStreamGeneric& operator >>(unsigned int &v)
	{
		Base::m_pos += Base::m_streambuf->Get(&v,Base::m_pos,sizeof(v));
		v = Base::TF::N2H(v);
		return *this;
	}
	///////////////////////////////////////////////////////////
	CIStreamGeneric& operator >>(short &v)
	{
		Base::m_pos += Base::m_streambuf->Get(&v,Base::m_pos,sizeof(v));
		v = Base::TF::N2H(v);
		return *this;
	}
	CIStreamGeneric& operator >>(unsigned short &v)
	{
		Base::m_pos += Base::m_streambuf->Get(&v,Base::m_pos,sizeof(v));
		v = Base::TF::N2H(v);
		return *this;
	}
	///////////////////////////////////////////////////////////
	CIStreamGeneric& operator >>(long &v)
	{
		int64 v64;
		operator >>(v64);
		v = (long)v64;
		return *this;
	}
	CIStreamGeneric& operator >>(unsigned long &v)
	{
		uint64 v64;
		operator >>(v64);
		v = (unsigned long)v64;
		return *this;
	}
	///////////////////////////////////////////////////////////
	CIStreamGeneric& operator >>(float &v)
	{
		Base::m_pos += Base::m_streambuf->Get(&v,Base::m_pos,sizeof(v));
		return *this;
	}
	///////////////////////////////////////////////////////////
	CIStreamGeneric& operator >>(double &v)
	{
		Base::m_pos += Base::m_streambuf->Get(&v,Base::m_pos,sizeof(v));
		return *this;
	}
	///////////////////////////////////////////////////////////
	CIStreamGeneric &operator >>(int64 &v)
	{
		Base::m_pos += Base::m_streambuf->Get(&v,Base::m_pos,sizeof(v));
		v = Base::TF::N2H(v);
		return *this;
	}
	///////////////////////////////////////////////////////////
	CIStreamGeneric &operator >>(uint64 &v)
	{
		Base::m_pos += Base::m_streambuf->Get(&v,Base::m_pos,sizeof(v));
		v = Base::TF::N2H(v);
		return *this;
	}
	///////////////////////////////////////////////////////////
	//template<typename T>
	//CIStreamGeneric &operator >>(T &v)
	//{
	//	lightAssert(0);
	//	return *this;
	//}
	///////////////////////////////////////////////////////////

	CIStreamGeneric& ReadString(char* &str, size32_t maxlen)
	{
		unsigned int len;
		(*this)>>len;
		if(maxlen == 0 || len>maxlen-1)
		{
			throw stream_error(SEC_INVALID_STRING);
		}
		str = Base::GetBuf()->GetBytes()+Base::Pos();
		Skip(len+sizeof(char));

		if(str[len]!=0)
		{
			LogNotice("invalid string");
			throw stream_error(SEC_INVALID_STRING);
		}
		return *this;
	}
};

template<class Base>
class COStreamGeneric : public Base
{
public:
	COStreamGeneric(CBasicStreamBuf *buf) : Base(buf){}
	COStreamGeneric(CBasicStreamBuf &buf) : Base(&buf){}
	~COStreamGeneric(){}
	size32_t Write(const void* p, size32_t n)
	{
		size32_t res = Base::m_streambuf->Put(p, Base::m_pos, n);
		Base::m_pos += res;
		return res;
	}
	size32_t Write(CIStreamGeneric<Base> &is)
	{
		size32_t size = is.GetBuf()->Size() - is.Pos();
		return Write(is.Skip(size), size);
	}
	char* Skip(size32_t n)
	{
		size32_t oldp = Base::m_pos;
		Base::m_pos += n;
		Base::m_streambuf->ExpandTo(Base::m_pos);
		return Base::m_streambuf->GetBytes() + oldp;
	}
	//////////////////////////////////////////////////
	COStreamGeneric& operator <<(const CBasicStreamBuf &buf)
	{
		unsigned int size = (unsigned int)buf.Size();
		(*this)<<size;
		Write(buf.GetBytes(), size);
		return *this;
	}
	//////////////////////////////////////////////////
	COStreamGeneric& operator <<(const bool &v)
	{
		Base::m_pos += Base::m_streambuf->Put(&v,Base::m_pos,sizeof(v));
		return *this;
	}
	//////////////////////////////////////////////////
	COStreamGeneric& operator <<(const char &v)
	{
		Base::m_pos += Base::m_streambuf->Put(&v,Base::m_pos,sizeof(v));
		return *this;
	}
	COStreamGeneric& operator <<(const unsigned char &v)
	{
		Base::m_pos += Base::m_streambuf->Put(&v,Base::m_pos,sizeof(v));
		return *this;
	}
	//////////////////////////////////////////////////
	COStreamGeneric& operator <<(const int &v)
	{
		int var = Base::TF::H2N(v);
		Base::m_pos += Base::m_streambuf->Put(&var,Base::m_pos,sizeof(v));
		return *this;
	}
	COStreamGeneric& operator <<(const unsigned int &v)
	{
		unsigned int var = Base::TF::H2N(v);
		Base::m_pos += Base::m_streambuf->Put(&var,Base::m_pos,sizeof(v));
		return *this;
	}
	//////////////////////////////////////////////////
	COStreamGeneric& operator <<(const short &v)
	{
		short var = Base::TF::H2N(v);
		Base::m_pos += Base::m_streambuf->Put(&var,Base::m_pos,sizeof(v));
		return *this;
	}
	COStreamGeneric& operator <<(const unsigned short &v)
	{
		unsigned short var = Base::TF::H2N(v);
		Base::m_pos += Base::m_streambuf->Put(&var,Base::m_pos,sizeof(v));
		return *this;
	}
	//////////////////////////////////////////////////
	COStreamGeneric& operator <<(const long &v)
	{
		return operator <<((const int64)v);
	}
	COStreamGeneric& operator <<(const unsigned long &v)
	{
		return operator <<((const uint64)v);
	}
	//////////////////////////////////////////////////
	COStreamGeneric& operator <<(const float &v)
	{
		Base::m_pos += Base::m_streambuf->Put(&v,Base::m_pos,sizeof(v));
		return *this;
	}
	//////////////////////////////////////////////////
	COStreamGeneric& operator <<(const double &v)
	{
		Base::m_pos += Base::m_streambuf->Put(&v,Base::m_pos,sizeof(v));
		return *this;
	}
	//////////////////////////////////////////////////
	COStreamGeneric &operator <<(const int64 &v)
	{
		int64 var = Base::TF::H2N(v);
		Base::m_pos += Base::m_streambuf->Put(&var,Base::m_pos,sizeof(v));
		return *this;
	}
	//////////////////////////////////////////////////
	COStreamGeneric &operator <<(const uint64 &v)
	{
		uint64 var = Base::TF::H2N(v);
		Base::m_pos += Base::m_streambuf->Put(&var,Base::m_pos,sizeof(v));
		return *this;
	}
	//////////////////////////////////////////////////
	//template <typename T>
	//COStreamGeneric &operator <<(const T&)
	//{
	//	lightAssert(0);
	//	return *this;
	//}
};


template<class ST>
inline COStreamGeneric<ST> &operator <<(COStreamGeneric<ST> &os, const char* str)
{
	unsigned int len = (unsigned int)strlen(str);
	os<<len;
	os.Write(str, len);
	char eos=0;
	os<<eos;
	return os;
}
template<class ST>
inline COStreamGeneric<ST> &operator <<(COStreamGeneric<ST> &os, char* str)
{
	unsigned int len = (unsigned int)strlen(str);
	os<<len;
	os.Write(str, len);
	char eos=0;
	os<<eos;
	return os;
}

template<class ST>
inline CIStreamGeneric<ST> &operator >>(CIStreamGeneric<ST> &is, const char* &str)
{
	unsigned int len;
	is>>len;
	str = is.GetBuf()->GetBytes()+is.Pos();
	is.Skip(len);
	is.Skip(sizeof(char));

	if(str[len]!=0)
	{
		LogNotice("invalid string");
		throw stream_error(SEC_INVALID_STRING);
	}
	return is;
}
template<class ST>
inline CIStreamGeneric<ST> &operator >>(CIStreamGeneric<ST> &is, char* &str)
{
	unsigned int len;
	is>>len;
	str = is.GetBuf()->GetBytes()+is.Pos();
	is.Skip(len);
	is.Skip(sizeof(char));

	if(str[len]!=0)
	{
		LogNotice("invalid string");
		throw stream_error(SEC_INVALID_STRING);
	}
	return is;
}



template<class ST, int L>
inline CIStreamGeneric<ST> &operator >>(CIStreamGeneric<ST> &is, char (&str)[L])
{
	char* src;
	is.ReadString(src, L);
	safe_strcpy(str, src);//不应该用memcpy，因为存储的时候用的是strcpy
	return is;
}

template<class ST>
inline COStreamGeneric<ST> &operator <<(COStreamGeneric<ST> &os, const std::string &str)
{
	return os<<str.c_str();
}
template<class ST>
inline CIStreamGeneric<ST> &operator >>(CIStreamGeneric<ST> &is, std::string &str)
{
	const char* s;
	is>>s;
	str = s;
	return is;
}

//////////////////////////////////////////////////////////////////////////
template<typename ST, typename T>
inline COStreamGeneric<ST> &operator <<(COStreamGeneric<ST> &os, T* p)
{
//	i_am_cursed();
	os << (int64)p;
	return os;
}

template<typename ST, typename T>
inline CIStreamGeneric<ST> &operator >>(CIStreamGeneric<ST> &is, T* &p)
{
//	i_am_cursed();
	int64 tmp;
	is >> tmp;
	p = (T*)tmp;
	return is;
}
template<typename ST, typename T, int L>
inline CIStreamGeneric<ST> &operator >>(CIStreamGeneric<ST> &is, T (&p)[L])
{
	//i_am_cursed();
	lightAssert(0);
	return is;
}


/*
template<class ST>
inline COStreamGeneric<ST> &operator <<(COStreamGeneric<ST> &os, const uchar_t* str)
{
	size_t len = ustrlen(str);
	for(size_t i=0;i<len;++i)
		os<<str[i];
	uchar_t eos=0;
	os<<eos;
	return os;
}
template<class ST>
inline CIStreamGeneric<ST> &operator >>(CIStreamGeneric<ST> &is, uchar_t* &str)
{
	str = reinterpret_cast<uchar_t*>(is.GetBuf()->GetBytes()+is.Pos());
	uchar_t c;
	is>>c;
	int i=0;
	while(c)//NOTE!!! we should check length to avoid bad string
	{
		str[i++] = c;//correct byte order
		is>>c;
	}
	return is;
}
template<class ST>
inline COStreamGeneric<ST> &operator <<(COStreamGeneric<ST> &os, const ustring &str)
{
	return os<<str.c_str();
}
template<class ST>
inline CIStreamGeneric<ST> &operator >>(CIStreamGeneric<ST> &is, ustring &str)
{
	str.reserve(128);
	str.clear();
	uchar_t c;
	is>>c;
	while(c)//NOTE!!! we should check length to avoid bad string
	{
		str+=c;
		is>>c;
	}
	return is;
}
*/

#include <vector>

template<typename ST, typename T1, typename T2>
inline COStreamGeneric<ST> &operator <<(COStreamGeneric<ST> &os, const std::pair<T1, T2> & pair)
{
	os << pair.first << pair.second;
	return os;
}
template<typename ST, typename T1, typename T2>
inline CIStreamGeneric<ST> &operator >>(CIStreamGeneric<ST> &is, std::pair<T1, T2> & pair)
{
	is >> pair.first >> pair.second;
	return is;
}

template<typename ST, typename T>
inline COStreamGeneric<ST> &operator <<(COStreamGeneric<ST> &os, const std::vector<T> & vec)
{
	os << static_cast<int> ( vec.size() );
	for( size_t i=0; i<vec.size(); ++i)
	{
		os << vec[i];
	}
	return os;
}
template<typename ST, typename T>
inline CIStreamGeneric<ST> &operator >>(CIStreamGeneric<ST> &is, std::vector<T> & vec)
{
	int num;
	is >> num;
	for(int i = 0; i < num; ++ i )
	{
		T val;
		is >> val;
		vec.push_back( val );
	}
	return is;
}

#include <list>
template<typename ST, typename T>
inline COStreamGeneric<ST> &operator <<(COStreamGeneric<ST> &os, const std::list<T> & ilist)
{
	os << static_cast<int> ( ilist.size() );
	for(typename std::list<T>::const_iterator it = ilist.begin(); it != ilist.end(); ++ it)
	{
		os << * it;
	}
	return os;
}
template<typename ST, typename T>
inline CIStreamGeneric<ST> &operator >>(CIStreamGeneric<ST> &is, std::list<T> & ilist)
{
	int num;
	is >> num;
	for(int i = 0; i < num; ++ i )
	{
		T val;
		is >> val;
		ilist.push_back( val );
	}
	return is;
}

#include <map>
template<typename ST, typename K, typename V>
inline COStreamGeneric<ST> &operator <<(COStreamGeneric<ST> &os, const std::map<K, V> & amap)
{
	os << static_cast<int> ( amap.size() );
	for(typename std::map<K, V>::const_iterator it = amap.begin(); it != amap.end(); ++ it)
	{
		os << it->first << it->second;
	}
	return os;
}

template<typename ST, typename K, typename V>
inline CIStreamGeneric<ST> &operator >>(CIStreamGeneric<ST> &is, std::map<K, V> & amap)
{
	amap.clear();
	int num;
	is >> num;
	for(int i = 0; i < num; ++ i )
	{
		K key;
		V val;
		is >> key >> val;
		amap.insert(std::pair<K,V>(key, val));
	}
	return is;
}

#include <set>
template<typename ST, typename V, typename LESS>
inline COStreamGeneric<ST> &operator <<(COStreamGeneric<ST> &os, const std::set<V, LESS> & aset)
{
	os << static_cast<int> ( aset.size() );
	for(typename std::set<V, LESS>::const_iterator it = aset.begin(); it != aset.end(); ++ it)
	{
		os << *it;
	}
	return os;
}

template<typename ST, typename V, typename LESS>
inline CIStreamGeneric<ST> &operator >>(CIStreamGeneric<ST> &is, std::set<V, LESS> & aset)
{
	aset.clear();
	int num;
	is >> num;
	for(int i = 0; i < num; ++ i )
	{
		V val;
		is >> val;
		aset.insert(val);
	}
	return is;
}

inline void CheckCharValid(const char* str)//checks only the validness of chars
{
	while(*str)
	{
		if((unsigned char)*str < 32)
		{
			throw stream_error(SEC_INVALID_CHAR);
		}
		++str;
	}
}
inline void CheckCharValid(const std::string& str)//checks only the validness of chars
{
	CheckCharValid(str.c_str());
}

typedef CIStreamGeneric<CStreamDefaultEndian> CIStream;
typedef COStreamGeneric<CStreamDefaultEndian> COStream;
typedef CIStreamGeneric<CStreamNetworkEndian> CIStreamNet;
typedef COStreamGeneric<CStreamNetworkEndian> COStreamNet;

#endif // !defined(AFX_BYTESTREAM_H__B009F0DA_3478_4652_B154_260EA0EE14C1__INCLUDED_)
