#include "LGWebNet.h"

#include "LoginThread.h"
#include "WebPlayer.h"
#include "ClientEvent.h"

#include "WebNetMessage.h"
#include "BigMsgEvent.h"


bool LGWebNet::OnAddConnect(tNetConnect *pConnect)
{
    mpLoginThread->OnPlayerConnected(pConnect);
    return true;
}

LGWebNet::LGWebNet(LoginThread *pThread) 
    : mpLoginThread(pThread)
{   
	WEBMsg::Register(this);

	RegisterMsg(WEB_SEND_BIG_MSG, MEM_NEW EventFactory<WEB_SendDataPartEvent>());
	RegisterMsg(WEB_REV_BIG_MSG, MEM_NEW EventFactory<WEB_RevDataPartEvent>());
}
HandConnect LGWebNet::CreateConnect()
{
    return MEM_NEW WebPlayer();
}
