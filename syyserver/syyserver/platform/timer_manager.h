#ifndef __TIMERMANAGER_H__8923lkjsk__
#define __TIMERMANAGER_H__8923lkjsk__

#include <platform/platform_shared/BiboFrame/BiboInterfaces.h>
#include <platform/network/net.h>

#include <map>
#ifdef _HASH_MAP_
#include <unordered_map>
#endif


class CTimerManager : public INoticeCallback
{
	enum{BEAT_TICK = 10};
	typedef int64 TrigTime;
	TrigTime m_cycle_count;

	tick_t m_last_update;
	struct timer_t
	{
		int flags;
		unsigned int after_ms;
		ITimerCallback* cb;
		void* user_ptr;
	};

#ifdef _HASH_MAP_
	typedef std::unordered_map<int64, timer_t> EntryMap;
#else
	typedef std::map<int64, timer_t> EntryMap;
#endif
	EntryMap m_entry_map;

	typedef std::multimap<TrigTime, int64> TriggerMap;
	TriggerMap m_trig_map;

	int64 m_id_counter;

	DWORD m_thread_id;

public:
	CTimerManager() : m_cycle_count(0), m_last_update(get_tick_count()), m_id_counter(10000), m_thread_id(-1)
	{
	}
public:
	virtual void OnNotice(void* user_ptr)
	{
		Update();
	}
	int64 AddTimer(unsigned int after_ms, int flags, ITimerCallback* cb, void* user_ptr)
	{
#ifdef _DEBUG
		DWORD curr_thread_id = BiboGetCurrentThreadID();
		alwaysAssert(m_thread_id == -1 || m_thread_id == curr_thread_id);
#endif

		timer_t timer;
		timer.flags = flags;
		timer.after_ms = after_ms;
		timer.cb = cb;
		timer.user_ptr = user_ptr;

		int64 id = m_id_counter++;
		m_entry_map[id] = timer;

		if(     (flags&IEventSelector::TIMER_CYCLE)
			&& !(flags&IEventSelector::TIMER_WAIT) )
		{
			after_ms = 0;
		}
		ActivateTimer(after_ms, id);
		return id;
	}
	void ActivateTimer(unsigned int after_ms, int64 timer_id)
	{
		tick_t t = get_tick_count();
		TrigTime cur_cycle = m_cycle_count + (t-m_last_update)/BEAT_TICK;
		TrigTime cycle = cur_cycle + after_ms/BEAT_TICK + 1;

		m_trig_map.insert(std::make_pair(cycle, timer_id));
	}
	void CancelTimer(int64 key)
	{
#ifdef _DEBUG
		DWORD curr_thread_id = BiboGetCurrentThreadID();
		alwaysAssert(m_thread_id == -1 || m_thread_id == curr_thread_id);
#endif

		EntryMap::iterator ei = m_entry_map.find(key);
		if(ei!=m_entry_map.end())
		{
			ei->second.cb->ReleaseTimerCallback();
			m_entry_map.erase(ei);
		}
	}
	void NotifyThreadStart()
	{
#ifdef _DEBUG
		m_thread_id = BiboGetCurrentThreadID();
#endif // _DEBUG
	}
	void NotifyThreadStop()
	{
#ifdef _DEBUG
		m_thread_id = -1;
#endif // _DEBUG
	}
	void Update()
	{
#ifdef _DEBUG
		DWORD curr_thread_id = BiboGetCurrentThreadID();
		alwaysAssert(m_thread_id == -1 || m_thread_id == curr_thread_id);
#endif

		tick_t tick = get_tick_count();
		int64 last_cycle_count = m_cycle_count;
		
		while(tick-m_last_update >= BEAT_TICK)
		{
			++m_cycle_count;
			m_last_update += BEAT_TICK;  //这个用于匀速
			//m_last_update = tick; //这个用于不匀速

		}
		if(last_cycle_count != m_cycle_count)
		{
			DoUpdate(m_cycle_count);
		}
	}
	int NextEventTick()
	{
		if(m_trig_map.empty())
			return 0x7fffffff;
		TrigTime cycle = m_trig_map.begin()->first;

		tick_t tick = get_tick_count();
		int64 dt = BEAT_TICK*(cycle-m_cycle_count)-(tick-m_last_update);
		if(dt<0)
			dt = 0;

		return (int)dt;
	}
protected:
	void DoUpdate(int64 cycle)
	{
		for(;;)
		{
			//寻找可以被触发的ID
			TriggerMap::iterator ti=m_trig_map.begin();
			if(ti==m_trig_map.end())
				break;
			if(ti->first > cycle)
				break;
			int64 entry_id = ti->second;
			m_trig_map.erase(ti);

			//执行这个ID
			EntryMap::iterator ei = m_entry_map.find(entry_id);
			if(ei!=m_entry_map.end())
			{
				const timer_t& timer = ei->second;
				if(timer.cb)
				{
					timer.cb->OnTimer(timer.user_ptr);
				}

				if(timer.flags & IEventSelector::TIMER_CYCLE)
				{
					//如果是循环的，再次加入
					ActivateTimer(timer.after_ms, entry_id);
				}
				else
				{
					//否则，删除这个entry
					CancelTimer(entry_id);
				}
			}
		}
	}
};

#endif
