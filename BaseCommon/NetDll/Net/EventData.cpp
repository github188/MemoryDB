#include "EventData.h"
#include "TimeManager.h"

EasyStack<EventData*> EventDataPool::mPoolList;
CsLock EventDataPool::mLock;
EasyStack<PacketData*> PacketDataPool::mPoolList;
CsLock PacketDataPool::mLock;

AutoEvent tNetEventFactory::StartEvent( void )
{		
	AutoEvent hEvent = mFreeEventPool.pop();
	if (hEvent)
	{
		Hand<tNetEvent> e = hEvent;
		e->__setFactory(this);
#if DEVELOP_MODE
		AssertEx(hEvent->mbNowFree, "严重错误，[%s]事件当前应该为释放状态", GetEventName());
		hEvent->mbNowFree = false;
#endif

#if USE_SAFT_RELEASE_EVENT
		hEvent->initData();		
#endif
		//AssertEx(!hEvent->getFinished(), "[%s]事件未正确初始数据", hEvent->GetEventName());
#if SAVE_DEBUG_INFO			
		hEvent->mCode = tEvent::gCode++;
#endif

#if SHOW_EVENT_TICK
		mStartTick = TimeManager::NowTick();
		mCount ++;
#endif
		return hEvent;
	}

	hEvent = NewEvent();

#if DEVELOP_MODE
	hEvent->mName = mEventName;
#endif
	Hand<tNetEvent> e = hEvent;
	e->__setFactory(this);

	return hEvent;
}