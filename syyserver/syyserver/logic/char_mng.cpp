//#include "stdafx.h"

#ifndef PLATFORM_WIN32
#define  UPDATE_INTERVAL 1000
#else
#define  UPDATE_INTERVAL 1000*60*5

#endif
#include "char_mng.h"
#
CCharManager::CCharManager():_last_update_tick(0)
{

}
CCharManager::~CCharManager()
{

}
void CCharManager::Update(tick_t tick)
{
	if (tick < _last_update_tick + UPDATE_INTERVAL)
		return ;
	auto it = _user_map.begin();
	for (; it != _user_map.end(); it++)//更新玩家数据
	{
		//it->second->LogOut
	}
}

