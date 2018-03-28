/********************************************************************
created:	2011/11/23
created:	23:11:2011   12:25
filename: 	e:\01_Langrisser\third\DataTable\EventCore\ClientEvent.h
file path:	e:\01_Langrisser\third\DataTable\EventCore
file base:	ClientEvent
file ext:	h
author:		Wenge Yang
description:客户端执行事件
purpose:	
*********************************************************************/
#ifndef _INCLUDE_CLIENTEVENT_H_
#define _INCLUDE_CLIENTEVENT_H_

#include "EventCore.h"

#include "ServerEvent.h"
#include "LogicEvent.h"

//#define RESPONSE_EVENT_KEY	"_Response_Event_"

class tNetHandle;
class tNetConnect;

namespace Logic
{
	//----------------------------------------------------------------------
	// 客户端由服务器驱动的逻辑事件, 继承自脚本逻辑事件体, 
	// 本身具有逻辑行为树功能, 即所有逻辑执行完成后, 再答复给服务器结束事件
	//----------------------------------------------------------------------
	class EventCoreDll_Export tClientEvent : public tBaseNetEvent /*public LogicEvent*/
	{
		typedef tBaseNetEvent parent;
	public:
		tClientEvent();

		virtual void InitData() override
		{
			parent::InitData();
			if (mResponseEvt)
				mResponseEvt->initData();
			mID = 0;
		}

		virtual bool _Serialize(DataStream *destData)
		{
			destData->write(mID);
			return parent::_Serialize(destData);
		}
		virtual bool _Restore(DataStream *scrData)
		{
			if (!scrData->read(mID))
				return false;
			return parent::_Restore(scrData);
		}

	public:
		virtual void SendResponce(AutoEvent hRespEvent);
		virtual bool CheckOnResponse() const { return true; }

	public:
		virtual void Finish(void);		

		virtual void SetRespData(AutoEvent &hResp){};

		virtual EVENT_ID GetEventID();

		virtual AutoEvent GetResponseEvent();

	public:
		EVENT_ID		mID;
		AutoEvent	mResponseEvt;
	};

	//-------------------------------------------------------------------------


#define ClientEventFactory	EventFactory
	
}


#endif //_INCLUDE_CLIENTEVENT_H_