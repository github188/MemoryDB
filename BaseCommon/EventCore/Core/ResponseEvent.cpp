
#include "ResponseEvent.h"
#include "ServerEvent.h"
#include "TimeManager.h"
#include "EventCenter.h"
#include "FixedTimeManager.h"

namespace Logic
{

	bool ResponseEvent::_DoEvent()
	{
		if (mID<=0)
			return false;

		ResponseEventFactory *f = dynamic_cast<ResponseEventFactory*>(GetEventCenter()->GetResponseEventFactory().getPtr());
		Hand<tServerEvent> hWaitEvt = f->FindServerEvent(mID); //GetEventCenter()->GetTimeEventManager()->GetWaitEvent(mID);
		if (hWaitEvt)
		{
			hWaitEvt->StopWaitTime();
			if (hWaitEvt->getFinished() )
				hWaitEvt->Log("Warn: 接收到回复时, 已经完成");
			else
				hWaitEvt->_OnEvent(GetSelf());

			// 优化: 直接清除此事件, 有利于事件被重用。 理论上当前还在时间列表中,不清除时，会等待时间超时后再移除处理
			// NOTE: 如果只在时间管理内等待，没有再被引用将直接移除，1个自身的 + 时间列表中的+当前被占用 = 3
			if (hWaitEvt.getUseCount()==3)
			{
				hWaitEvt._free();
			}
			else
			{
				GREEN_LOG("Wait use ServerEvent >[%s] [%llu]", hWaitEvt->GetEventNameInfo().c_str(), hWaitEvt->GetEventID());
			}
		}
		else
		{
			NOTE_LOG("WARN: 回复时, 等待事件不再存在 [%llu] > [%s]", mID, get("_SERVER_EVENT_").string().c_str());
			TABLE_LOG(GetData().dump().c_str());
		}
		Finish();
		return true;
	}
#define _MOVE_BIT 20
    static const UINT msMaxPosValue = ~((0xFFFFFFFF>>_MOVE_BIT)<<_MOVE_BIT);
	void ResponseEventFactory::AllotEventID( tServerEvent *serverEvent )
	{
		if (serverEvent->mID>0)
			return;

		UINT x = 0;
		if (!mIDStack.empty())
		{
			x = mIDStack.pop();
            // 自增高12位
            UINT pos = x &msMaxPosValue;            
            UINT code = (x>>_MOVE_BIT)+1;            
            x = (code<<_MOVE_BIT)+pos;
			AssertNote(mEventList[pos]==NULL, "Event no free id %u", pos)	;
			mEventList[pos] = serverEvent;
			INFO_LOG(" [%s] Start wait pos [%u], code [%u]", serverEvent->GetEventNameInfo().c_str(), pos, code); 
		}
		else
		{
            AssertNote (mEventList.size()<msMaxPosValue, "Max wait event count must less 104万");
            
			x = mEventList.size();				
			mEventList.push_back(serverEvent);
			INFO_LOG(" [%s] Start wait pos [%u], code [%u]", serverEvent->GetEventNameInfo().c_str(), mEventList.size()-1, 0); 
		}
		serverEvent->mID = x;
	}

	void ResponseEventFactory::FreeServerEvent( tServerEvent *serverEvent )
	{
		if (serverEvent->mID<=0)
			return;

		UINT pos = serverEvent->mID&msMaxPosValue;

		if (pos>=0 && pos<mEventList.size())
		{
			tServerEvent *p = mEventList[pos];
			if (p==serverEvent)
			{
				mEventList[pos] = NULL;
				mIDStack.push(serverEvent->mID);
				INFO_LOG(" [%s] Free wait pos [%u], code [%u]", serverEvent->GetEventNameInfo().c_str(), pos, serverEvent->mID>>_MOVE_BIT); 
			}
		}
	}

	AutoEvent ResponseEventFactory::FindServerEvent( EVENT_ID id )
	{
		UINT x = id & msMaxPosValue;
		if (x>0 && x<mEventList.size())
		{
			tServerEvent *p = mEventList[x];
			if (p!=NULL && p->mID==id)
			{
				return p->GetSelf();
			}
		}
		return AutoEvent();
	}



}