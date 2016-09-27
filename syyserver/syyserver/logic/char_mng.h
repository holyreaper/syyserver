#ifndef _CHAR_MNG_H_2016_09_23_H
#define  _CHAR_MNG_H_2016_09_23_H
#include "../platform/_platform_def.h"
#include <hash_map>
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
	void Update(tick_t tick);	
public:

protected:
#ifdef _DEBUG
	std::map<int64,IplayerInterface*> _user_map;
#else
	std::hash_map<int64,IplayerInterface*> _user_map;
#endif
	tick_t _last_update_tick;
};
#endif//_CHAR_MNG_H_2016_09_23_H