
#include "ClientEvent.h"
#include "EventCenter.h"
#include "NetHandle.h"
#include "ResponseEvent.h"

#define  NET_MAIN	0

namespace Logic
{
	//----------------------------------------------------------------------
	tClientEvent::tClientEvent() 
		: mID(0)
	{

	}

	//----------------------------------------------------------------------
	void tClientEvent::Finish()
	{
		if (!hasState(STATE_ALREADY_RESPONSE))
		{
			setState(STATE_ALREADY_RESPONSE, true);

			if (!CheckOnResponse())
			{
				Log("Error: 准备回复时, 检查失败");
				return;
			}
			EVENT_ID respID = mID; // get(RESP_ID_KEY);
			// 如果没有分配回复ID, 默认为不需要回复
			if (respID!=0) 
			{						
				AutoEvent hResp = GetResponseEvent();

				//!!! WARN: 在下面函数内如果对当前事件进行了设置数据, 会使用 RESP_ID_KEY 对应的值变化, 原因尚未查出
				SetRespData(hResp);
				//hResp->set(RESP_ID_KEY, respID); 
				Hand<ResponseEvent> r = hResp;
				r->mID = mID;
				//hResp->set(RESP_KEY_TIME, (int)time);
#if DEVELOP_MODE
				hResp->set("_SERVER_EVENT_", GetEventNameInfo().c_str());
#endif

				SendResponce(hResp);
			}			
			else
			{
				LOG_RED;
				Log("ERROR: 逻辑错误, 请求事件中应该存在 RESP_ID_KEY");
				LOG_WHITE;
			}
		}
		parent::Finish();
	}
	//----------------------------------------------------------------------

	EVENT_ID tClientEvent::GetEventID()
	{
		return mID;
	}

	AutoEvent tClientEvent::GetResponseEvent()
	{
		if (!mResponseEvt)
			mResponseEvt = GetEventCenter()->GetResponseEventFactory()->StartEvent(); 

		return mResponseEvt;
	}

	void tClientEvent::SendResponce( AutoEvent hRespEvent )
	{
		if (mNetConnect)
		{
			if (!mNetConnect->SendEvent(hRespEvent.getPtr()) )
			{
				Log("Error: 回复发送失败!");
			}			
		}
		else
		{
			hRespEvent->Send(NET_MAIN, _getTarget());
			Log("客户端事件网络发送对象为空 >>> 可能_onReciveRun() 功能被覆盖");
		}
	}


	//----------------------------------------------------------------------
	//----------------------------------------------------------------------


}