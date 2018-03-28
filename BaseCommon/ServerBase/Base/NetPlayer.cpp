
#include "NetPlayer.h"
#include "BaseNetManager.h"

#include "NetHandle.h"

#include "NetConnect.h"


tNetHandle* NetPlayer::GetNetHandle( void )
{
	if (mConnect)
		return mConnect->GetNetHandle();
	return NULL;
}

void NetPlayer::BindConnect( tNetConnect *pConnect )
{
	mConnect = pConnect;
	//if (mConnect)
	//{
		//mConnect->_SetPlayer(this);
	//}
}

void NetPlayer::Process( void *param )
{
	if (mConnect)
		mConnect->Process();
}

bool NetPlayer::SendEvent( Logic::tEvent *pEvent )
{
	if (mConnect)
		return GetConnect()->SendEvent(pEvent);
	return false;
}


