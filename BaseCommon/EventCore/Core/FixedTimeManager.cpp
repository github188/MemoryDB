/********************************************************************
	created:	2012/11/02
	created:	2:11:2012   3:16
	filename: 	C:\NewGame\Common\EventCore\Core\FixedTimeManager.cpp
	file path:	C:\NewGame\Common\EventCore\Core
	file base:	FixedTimeManager
	file ext:	cpp
	author:		Wenge Yang
	
	purpose:	

	
*********************************************************************/
#include "FixedTimeManager.h"
#include "ServerEvent.h"
#include "TimeManager.h"

#if __LINUX__
#	include <sys/time.h>
#else
#	include "time.h"
#	include <Windows.h>
#endif

 
#define DUMP_INFO_LOG		0
#define FIXED_TIME_KEY	"__FixedTime__"


namespace Logic
{
	//----------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------
	FixedTimeManager::FixedTimeManager() 	
		: mMinIntervalTime(20)
	{
#if SECORDS_TIME_MODE		
		::time(&mCurrentTime);
#else
		mCurrentTime = Now();
#endif
		mUpdateEventList.ReadyNodePool(mWillUpdateEventList.GetNodePool());

		mTempWaitList.ReadyNodePool(mWaitEventList.GetNodePool());
		mWillWaitTimeEventList.ReadyNodePool(mWaitEventList.GetNodePool());
	}
	//----------------------------------------------------------------------------------------------------
	void FixedTimeManager::_OnStartUpdateEvent( AutoEvent hEvent, float spaceTime )
	{
		if (spaceTime*1000<mMinIntervalTime)
		{
			WARN_LOG("[%s]Update space time [%f] < min >[%d] mil, then set to", hEvent->GetEventNameInfo().c_str(), spaceTime, mMinIntervalTime);
			spaceTime = mMinIntervalTime*0.001f;
		}

		UpdateEvent updateEvt;
#if UPDATE_USE_SPACE_TIME
		updateEvt.mSpaceMilSecond = (unsigned short)(spaceTime*1000);
#else
        updateEvt.mSpaceMilSecond = (unsigned int)(spaceTime*1000);
        updateEvt.mStartTime = Now();
#endif
		updateEvt.mUpdateEvent = hEvent ;

		mWillUpdateEventList.insert(updateEvt);
	}
	//----------------------------------------------------------------------------------------------------
	void FixedTimeManager::ClearAllEvent()
	{
		//mWaitEventList.clear();
		//mUpdateEventList.clear();

		mWillUpdateEventList.clear();
		mWillWaitTimeEventList.clear();

		// 不可以直接清空链表，可能会在Process 中调用些功能，造成链表被破坏错误
		for (auto it = mWaitEventList.begin(); it!=mWaitEventList.end(); ++it)
		{
			 WaitEvent &d = *it;
			 if (d.mWaitEvent)
				 d.mWaitEvent->setUseData(NULL);
		}

		for (auto it = mUpdateEventList.begin(); it!=mUpdateEventList.end(); ++it)
		{
			UpdateEvent &d = *it;
			if (d.mUpdateEvent)
				d.mUpdateEvent->setUpdateInfo(NULL);
		}
	}
	//----------------------------------------------------------------------------------------------------

	void FixedTimeManager::ReaydyListNodePool( AutoPtr<NodePool> hPool)
	{
		//mWaitEventList.ReadyNodePool(hPool);
		mUpdateEventList.ReadyNodePool(hPool);

		mWillWaitTimeEventList.ReadyNodePool(mWaitEventList.GetNodePool());

		//mRemoveWaitList.ReadyNodePool(mRemoveUpdateList.GetNodePool());
	}


	//----------------------------------------------------------------------------------------------------

	void FixedTimeManager::_OnWaitEvent( AutoEvent hEvent, float waitTimeSecond, EventCallBack callBack)
	{
		if (!hEvent ) // || waitTimeSecond<=0)
			return;

		//for (auto it=mRemoveWaitList.begin(); it!=mRemoveWaitList.end(); )
		//{
		//	// 清除掉移除列表中的, 如果移除后，又加入的, 此时还在移除列表中
		//	if (*it==hEvent)
		//		it = mRemoveWaitList.erase(it);
		//	else
		//		++it;
		//}

#if DUMP_INFO_LOG
		hEvent->setLog(true);
		hEvent->Log("~~~~~~~~~~>装备进行等待 >>> [%f]", waitTimeSecond);
#endif

#if SECORDS_TIME_MODE

        TIME_VAL   currentTime;
		::time(&currentTime);
#else
		// NOTE: 当以秒为单位和毫秒为单位时间长度不同, 所以换成毫秒
		waitTimeSecond *= TIME_UNIT;
		TIME_VAL currentTime = Now();
#endif
		// Note : must remove before Do over time, because may be repeat wait time in over time process.
		// then will remove this event, so after romove event is other event.
		// 注意: 当在超时事件中, 重新进行重置时间时, 可能会对当前的it 作删除操作, 会使用it失效, 
		// 因此, 必须使用额外列表缓存,进行执行超时
		mWillWaitTimeEventList.push_back(WaitEvent(currentTime+(unsigned int)waitTimeSecond, hEvent, callBack));

	}
	//----------------------------------------------------------------------------------------------------
	// NOTE: 当以秒为单位和毫秒为单位时间长度不同
	void FixedTimeManager::Process()
	{
		//-----------------------------------------------------------------
		TIME_VAL currentTime = Now();
		int cycleMilSec = (int)(currentTime - mCurrentTime);

		if (cycleMilSec<mMinIntervalTime)
			return;

		float cycleSecond = cycleMilSec * TIME_UNIT_SECOND;		

		mCurrentTime = currentTime;
		//-----------------------------------------------------------------

		if (!mWillWaitTimeEventList.empty())
		{
			mTempWaitList.swap(mWillWaitTimeEventList);
			for( WaitEventList::iterator willWaitIt=mTempWaitList.begin(); willWaitIt!=mTempWaitList.end(); ++willWaitIt)
			{
				WaitEvent &willEvent = *willWaitIt;

				if (!willEvent.mWaitEvent)
					continue;

				// Note: Check event is exist when insert before, because wait time will change, so list is not sqrt state. change bad.
				// so must delete already exist this event.
				// 特别注意: 当要进行加入的事情已经存在时, 对事件进行重新设置等待时间, 会破坏掉列表的排序状态.
				// 所以, 必须删除掉已经存在的事件
				// 杨文鸽 2012.11.02

				// Note: 优化插入速度，使用事件中保存节点指针方式用于是否为最后一次等待
				// Yang wenge 2016.2.24

				bool bInsert = false;

				for( WaitEventList::iterator it=mWaitEventList.begin(); it!=mWaitEventList.end(); ++it)
				{
					WaitEvent &waited = *it;
					// 找到第一个大于将要加入的事件，然后插入在前面
					// 如果相等，要加入到其后面，先等待先执行
					if (waited.mOverTime>willEvent.mOverTime)
					{ 
						// 插入到当前节点的前面, 所以不会被处理到
						it = mWaitEventList.insert(it, willEvent);
						willEvent.mWaitEvent->setUseData(it.getNode());
#if DUMP_INFO_LOG						
						willEvent.mWaitEvent->Log(" ++++++++++> 进行等待...");
						_Dump();
#endif												
						bInsert = true;

						break;
					}
				}
	
				if (!bInsert)
				{
					void *pData = mWaitEventList.push_back(willEvent);
					willEvent.mWaitEvent->setUseData(pData);
#if DUMP_INFO_LOG
					willEvent.mWaitEvent->Log(" **********> 进行等待...");
					_Dump();
#endif										
				}
			}
			mTempWaitList.clear();
		}

		if (!mWaitEventList.empty())
		{		
			for( WaitEventList::iterator it=mWaitEventList.begin(); it!=mWaitEventList.end();  )
			{
				AutoEvent hE = (*it).mWaitEvent;
				if ((!hE 
					|| hE->needFree())
					|| (hE->getFinished() && (*it).mWaitCallBack.Nothing() )
					|| hE->getUseData()==NULL
					)
				{
#if DUMP_INFO_LOG
					if (hE)
						hE->Log(" -------> 移除等待, 已经完成, 或者设置为移除");
#endif
					it = mWaitEventList.erase(it);
					if (hE)
						hE->setUseData(NULL);
					continue;
				}

				if ( (*it).mOverTime>currentTime )
				{
					break;
				}
				else
				{
					bool bIsThis = (hE->getUseData()==it.getNode());
					
					EventCallBack callBack = (*it).mWaitCallBack;
					// Note : must remove before Do over time, because may be repeat wait time in over time process.
					// then will remove this event, so after romove event is other event.
					// 注意: 当在超时事件中, 重新进行重置时间时, 可能会对当前的it 作删除操作, 会使用it失效, 
					// 因此, 必须使用额外列表缓存,进行执行超时
					it = mWaitEventList.erase(it);

					if (bIsThis)
					{
#if DUMP_INFO_LOG
						hE->Log(" =========> 等待时间完成, 执行等待完成事件...");
#endif
						if (callBack.Nothing())
							hE->DoTimeOver();						
						else
							callBack(hE.getPtr());
					}
					else
					{
#if DUMP_INFO_LOG
						hE->Log(" ##### 等待事件不是最后一次等待操作");
#endif
					}

					continue;
				}

				++it;
			}		

		}

		// 处理时间更新事件
		if (!mWillUpdateEventList.empty())
		{
			for( UpdateEventList::iterator willUpdateIt=mWillUpdateEventList.begin(); willUpdateIt!=mWillUpdateEventList.end(); ++willUpdateIt)
			{
				UpdateEvent &up = *willUpdateIt;
				//if (!mUpdateEventList.exist(*willUpdateIt))
				if (up.mUpdateEvent)
				{
#if UPDATE_USE_SPACE_TIME
					void *pData = mUpdateEventList.push_back(up);
					up.mUpdateEvent->setUpdateInfo(pData);
#else
                    _AppendUpdate(up);
#endif
					//if ((*willUpdateIt).mUpdateEvent->get("_DEBUG_"))
					//	INFO_LOG("[%s] append update", (*willUpdateIt).mUpdateEvent->GetEventNameInfo());
				}
			}
			mWillUpdateEventList.clear();
		}

		if (!mUpdateEventList.empty())
		{			
			for (UpdateEventList::iterator it=mUpdateEventList.begin(); it!=mUpdateEventList.end(); )
			{
				UpdateEvent &upEvent = *it;
				if (
					!upEvent.mUpdateEvent 
					|| upEvent.mUpdateEvent->getFinished()
					|| upEvent.mUpdateEvent->needFree()
					|| upEvent.mUpdateEvent->getUpdateInfo()==NULL
					|| upEvent.mUpdateEvent->getUpdateInfo()!=it.getNode()
					)
				{
					//if (upEvent.mUpdateEvent->get("_DEBUG_"))
					//	INFO_LOG("[%s] stop update", upEvent.mUpdateEvent->GetEventNameInfo());

					it = mUpdateEventList.erase(it);
					continue;
				}
				else
				{
					UpdateEvent &up = *it;
#if UPDATE_USE_SPACE_TIME

					up.mTotalMilSecond += (unsigned short)cycleMilSec;
					if ( up.mTotalMilSecond>=up.mSpaceMilSecond )
					{
						up.mTotalMilSecond -= up.mSpaceMilSecond;

						if ( !up.mUpdateEvent->Update( up.mSpaceMilSecond * 0.001f ) )
						{
							// NOTE: 有可能在Update中把智能对象强行释放掉
							if (up.mUpdateEvent)
								up.mUpdateEvent->setUpdateInfo(NULL);
							it = mUpdateEventList.erase(it);
							continue;
						}
					}
#else
                    if (up.mWaitSecond>currentTime)
                        break;

					if ( !up.mUpdateEvent->Update(up.mSpaceMilSecond* 0.001f) )
					{
                        if (up.mUpdateEvent)
                            up.mUpdateEvent->setUpdateInfo(NULL);                       
					}
                    else
                    {
                        //NOTE_LOG("append will >%s", up.mUpdateEvent->GetEventName());
                        ++up.mAlreayCount;
                        mWillUpdateEventList.push_front(up);
                    }
                    it = mUpdateEventList.erase(it);
                    continue;
#endif
				}

				++it;
			}
		}
	}

	void FixedTimeManager::_Dump()
	{
		INFO_LOG("---------------------------start list wait event ---------------------------");
		for (WaitEventList::iterator it = mWaitEventList.begin(); it!=mWaitEventList.end(); ++it)
		{
			const char *sz = (*it).mWaitEvent->getUseData()==it.getNode() ? "[Y]" : "[X]";
			(*it).mWaitEvent->Log(" %s Wait >>> [%llu]", sz, (*it).mOverTime);
		}
		INFO_LOG("---------------------------finish list wait event ---------------------------");
	}

//-------------------------------------------------------------------------*/

	TIME_VAL FixedTimeManager::Now()
	{
#if SECORDS_TIME_MODE
		TIME_VAL     currentTime;
		::time(&currentTime);

		return currentTime;
#else
#	ifdef __LINUX__
		long long time_last;    
		time_last = time(NULL);     //总秒数  
		struct timeb t1;    
		ftime(&t1);   
#	else		
		return GetTickCount64();
#	endif //__LINUX__
#endif //SECORDS_TIME_MODE
	}


	void FixedTimeManager::RemoveEvent(AutoEvent hEvent)
	{
		RemoveWaitEvent(hEvent);
		RemoveUpdateEvent(hEvent);
	}



	void FixedTimeManager::RemoveWaitEvent(AutoEvent hEvent)
	{
		// 移除等待事件，只需要将验证指针设置为空，即超时后不再执行即可
		if (hEvent)
			hEvent->setUseData(NULL);

	}

	void FixedTimeManager::RemoveUpdateEvent(AutoEvent hEvent)
	{
		// 直接设置更新信息数据为空
		if (hEvent)
			hEvent->setUpdateInfo(NULL);
	}
	//-------------------------------------------------------------------------*/
	class PauseEventInfo : public NiceData
	{
	public:
		float mLastWaitTime;
		AutoEvent mWaitEvent;

		EventCallBack mCallBack;
	};
	//-------------------------------------------------------------------------*/
	void FixedTimeManager::PauseEvent(AutoEvent hEvent)
	{
		for (auto it=mWaitEventList.begin(); it!=mWaitEventList.end(); ++it)
		{
			if ((*it).mWaitEvent==hEvent)
			{
				PauseEventInfo *pInfo = MEM_NEW PauseEventInfo();
				pInfo->mWaitEvent = hEvent;
				pInfo->mCallBack = (*it).mWaitCallBack;				
				UInt64 t = Now();
				if ((*it).mOverTime>t)
					pInfo->mLastWaitTime = ((*it).mOverTime - t) * TIME_UNIT_SECOND;
				else
					pInfo->mLastWaitTime = 0;
				
				AutoNice hInfo = pInfo;
				hEvent->set("__WaitTimeInfo__", hInfo);
			}
		}

		for (auto it = mWillWaitTimeEventList.begin(); it!=mWillWaitTimeEventList.end(); ++it)
		{
			if ((*it).mWaitEvent==hEvent)
			{
				PauseEventInfo *pInfo = MEM_NEW PauseEventInfo();
				pInfo->mWaitEvent = hEvent;
				pInfo->mCallBack = (*it).mWaitCallBack;				
				UInt64 t = Now();
				if ((*it).mOverTime>t)
					pInfo->mLastWaitTime = ((*it).mOverTime - t) * TIME_UNIT_SECOND;
				else
					pInfo->mLastWaitTime = 0;

				AutoNice hInfo = pInfo;
				hEvent->set("__WaitTimeInfo__", hInfo);
			}
		}

		for (auto it=mUpdateEventList.begin(); it!=mUpdateEventList.end(); ++it)
		{
			if ( (*it).mUpdateEvent==hEvent)
			{					
				hEvent->set("__NeedUpdate__", true);
				hEvent->set("__UpdateTime__", (*it).mSpaceMilSecond);
				
				break;
			}
		}

		for (auto it=mWillUpdateEventList.begin(); it!=mWillUpdateEventList.end(); ++it)
		{
			if ( (*it).mUpdateEvent==hEvent)
			{					
				hEvent->set("__NeedUpdate__", true);
				hEvent->set("__UpdateTime__", (*it).mSpaceMilSecond);

				break;
			}
		}

		RemoveWaitEvent(hEvent);
		RemoveUpdateEvent(hEvent);
	}

	void FixedTimeManager::ContinueEvent(AutoEvent hEvent)
	{
		AutoNice hInfo;
		if (hEvent->get("__WaitTimeInfo__", hInfo))
		{
			PauseEventInfo *pInfo = dynamic_cast<PauseEventInfo*>(hInfo.getPtr());
			if (pInfo!=NULL)
			{
				hEvent->WaitTime(pInfo->mLastWaitTime, pInfo->mCallBack);
			}
			hEvent->removeData("__WaitTimeInfo__");
		}
		
		if (hEvent->get("__NeedUpdate__"))
		{
			hEvent->StartUpdate((float)hEvent->get("__UpdateTime__") * 0.001f);
			hEvent->removeData("__NeedUpdate__");
			hEvent->removeData("__UpdateTime__");
		}
	}
 #if !UPDATE_USE_SPACE_TIME
    void FixedTimeManager::_AppendUpdate(UpdateEvent &updateEvt)
    {
        updateEvt.mWaitSecond = (updateEvt.mAlreayCount+1)*updateEvt.mSpaceMilSecond + updateEvt.mStartTime;
        bool bInsert = false;
        for (auto it=mUpdateEventList.begin(); it!=mUpdateEventList.end(); ++it)
        {
            if ( (*it).mWaitSecond>updateEvt.mWaitSecond )
            {
                bInsert = true;
                it = mUpdateEventList.insert(it, updateEvt);
                updateEvt.mUpdateEvent->setUpdateInfo(it.getNode());
                break;
            }
        }
        if (!bInsert)
        {
            void *pData = mUpdateEventList.push_back(updateEvt);
            updateEvt.mUpdateEvent->setUpdateInfo(pData);
        }
        //NOTE_LOG("*** append update >%s", updateEvt.mUpdateEvent->GetEventName());
    }
#endif
    //----------------------------------------------------------------------------------------------------

}