/********************************************************************
created:	2013/06/07
created:	7:6:2013   11:05
filename: 	C:\Work\BaseCommon\EventCore\Core\ResponseEvent.h
file path:	C:\Work\BaseCommon\EventCore\Core
file base:	ResponseEvent
file ext:	h
author:		Wenge Yang

purpose:	Ӧ���¼����ȴ��¼��ظ����¼�
*********************************************************************/
#ifndef _INCLUDE_RESPONSEEVENT_H_
#define _INCLUDE_RESPONSEEVENT_H_

#include "CEvent.h"

namespace Logic
{
	class tServerEvent;
	class EventCoreDll_Export ResponseEvent : public CEvent
	{
		typedef CEvent base;
	public:
		ResponseEvent()
			: mID(0)
		{
			setLog(false);
		}
		virtual ~ResponseEvent()
		{

		}

	public:
		virtual bool _Serialize(DataStream *destData)
		{
			destData->write(mID);
			return base::_Serialize(destData);
		}
		virtual bool _Restore(DataStream *scrData)
		{
			if (!scrData->read(mID))
				return false;
			return base::_Restore(scrData);
		}

		virtual void InitData() override
		{
			base::InitData();
			mID = 0;
		}

	public:
		virtual bool _DoEvent();

		virtual void Dump()
		{
			AString s = "From > [";
			s += get("_SERVER_EVENT_").string();
			s += "]";
			Log( s.c_str() );
			base::Dump();
		}

		virtual AString GetEventNameInfo() const
		{
			AString nameInfo = base::GetEventNameInfo();
			nameInfo += "*";
			if (GetData().exist("_SERVER_EVENT_"))
				nameInfo += GetData().get("_SERVER_EVENT_").string();
			else
				nameInfo += "UNKOWN";
			return nameInfo;
		}

	public:
		EVENT_ID	mID;
	};
    /*/----------------------------------------------------------------------
     ID ���ɼ�Ӧ���Ż�
     1 ��4�ֽ�32λ�޷�������
     2 ��12λ����ʱ���ϴ�����1
     3 ��20λ��ʾ�������е�λ��, ����ͬʱ���ֻ�ܱ���2��20�η����¼�,��ͬʱ�ȴ��¼����ɳ���2��20�η�
     4 �ձ��е�ջ�ڱ����ͷŵ�ID
     ����: ���ͷź�,�ٴ�ʹ�õ�20λ��ʾ��λ��, ���12λ����1��, ���ID�仯, ���ٻ���ԭ����ͬ
     �������,���ٷ����ͷ�4096�κ�,��ʱ��ID���и���һ��, ����Ӧ����������ϲ�����
    //---------------------------------------------------------------------*/
	class EventCoreDll_Export ResponseEventFactory : public CEventFactory
	{
	public:
		virtual AutoEvent NewEvent()
		{
			AutoEvent evt = MEM_NEW ResponseEvent();
			//evt->setLog(true);
			return evt;
		}

		virtual void AllotEventID(tServerEvent *serverEvent);

		virtual void FreeServerEvent( tServerEvent *serverEvent );

		AutoEvent FindServerEvent(EVENT_ID id);

	public:
		ResponseEventFactory()
		{
			mEventList.push_back(NULL);
            SetState(eFactoryNoProtocolSave, true);
		}

	protected:
		Array<tServerEvent*>		mEventList;
		EasyStack<UINT>				mIDStack;
	};

}
#endif