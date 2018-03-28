/********************************************************************
	created:	2014/01/10
	created:	10:1:2014   17:29
	filename: 	E:\HomeGame\BaseCommon\EventCore\Core\ExeEvent.h
	file path:	E:\HomeGame\BaseCommon\EventCore\Core
	file base:	ExeEvent
	file ext:	h
	author:		Wenge Yang
	
	purpose:	事件执行管理, 实现事件脚本式执行
*********************************************************************/
#ifndef _INCLUDE_EXEEVENT_H_
#define _INCLUDE_EXEEVENT_H_

#include "EventCallBack.h"
#include "EasyHash.h"

class EventSet
{
public:
	virtual ~EventSet(){}


public:
	void Register(const char *szIndex, EventCallBack eventCallBack)
	{
		EventCallBack *p = mEventCallBack.findPtr(szIndex);
		if (p!=NULL)
		{
			Log("WARN: [%s] already exist, now replace");

			*p = eventCallBack;
		}
		else
			mEventCallBack.insert(szIndex, eventCallBack);
	}

	bool Remove(const char *szIndex)
	{
		return mEventCallBack.erase(szIndex)>0;
	}

	bool Exe(const char *szIndex)
	{
		EventCallBack *p = mEventCallBack.findPtr(szIndex);
		if (p==NULL)
		{
			Log("WARN: [%s] not exist", szIndex);
			return false;
		}		
		(*p)();		
		return true;
	}

protected:
	EasyHash<EasyString, EventCallBack>		mEventCallBack;
};


#endif //_INCLUDE_EXEEVENT_H_