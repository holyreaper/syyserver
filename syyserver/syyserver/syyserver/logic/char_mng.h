#ifndef _CHAR_MNG_H_2016_09_23_H
#define  _CHAR_MNG_H_2016_09_23_H
#include "../platform/_platform_def.h"
class ICharManager
{
public:
	virtual int login(int64 uid) =0;
};
#include "player.h"
class CCharManager
{
public:
	CCharManager();
	virtual ~CCharManager();
public:

protected:
#ifdef _DEBUG
	std::map<int64,IplayerInterface*>_user_map;
#else
	hash_map<int64,IplayerInterface*>_user_map;

#endif
};
#endif//_CHAR_MNG_H_2016_09_23_H