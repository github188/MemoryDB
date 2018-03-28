
#include "GameServerEvent.h"
#include "NetConnect.h"
#include "EventCenter.h"
//#include "Common.h"

bool SC_NodifyClientConnectOk_C::_DoEvent( void )
{
	Data id = get("ID");
	if (!id.empty())
	{
		mNetConnect->SetNetID((int)id);
		tConnect *p = dynamic_cast<tConnect*>(mNetConnect.getPtr());
		if (p!=NULL)
			p->SetOK();
		mNetConnect->GetNetHandle()->OnAddConnect(mNetConnect.getPtr());
		Log(">>>>连接成功[%s], NetID>>>[%d]************", mNetConnect->GetNetHandle()->GetIp(), (int)id);
	} 
	else
		Log("Error : no exist ID respones");
	Finish();

	return true;
}



