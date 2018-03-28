#include "NetHandle.h"

#include "EventCenter.h"
#include "ServerEvent.h"

AutoEvent tNetConnect::StartEvent( const char* szEvent )
{
	AssertEx(GetNetHandle()!=NULL && GetNetHandle()->GetEventCenter()!=NULL, "NetHandle is NULL or Net event center is NULL");
	AutoEvent evt = GetNetHandle()->GetEventCenter()->StartEvent(szEvent);

	Logic::tBaseNetEvent *p = dynamic_cast<Logic::tBaseNetEvent*>(evt.getPtr());
	if (p!=NULL)
		p->mNetConnect = GetSelf();
	else
	{
		TABLE_LOG("WARN:: tNetConnect::StartEvent¡¡Event [%s] is not tBaseNetEvent event, can not send", szEvent);
	}

	return evt;
}
