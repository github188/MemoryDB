#include "BaseNetHandle.h"
#include "EventProtocol.h"
#include <stdarg.h>

//bool tBaseNetHandle::Send( Logic::tEvent *sendEvent, int target )
//{
//	tNetConnect *p = GetConnect(target);
//	if (p)
//		return Send(sendEvent, p);
//	else
//		sendEvent->Log("发送失败X, 获取不到网络连接[%d]", target);
//
//	return false;
//}

bool tBaseNetHandle::Send( Logic::tEvent *sendEvent, tNetConnect *pConnect )
{
	AssertEx(sendEvent!=NULL, "发送的事件为空");

	EventNetProtocol *pNetProtocol = dynamic_cast<EventNetProtocol*>(mpNetProtocol);
	if (pNetProtocol==NULL)
	{
		Log("ERRRO: Send fail, net protocol is not EventNetProtocol");
		return false;
	}
    
	HandPacket pPacket = pNetProtocol->GenerateEventPacket(pConnect, sendEvent, NeedZipSendPacket());
	AssertEx(pPacket, "生成消息包失败");

	//pPacket->SetNeedEncrypt(NeedEncryptPacket());
	bool bEncrypt = NeedEncryptPacket() && sendEvent->GetEventFactory()->HasState(ePacketStateEncrypt);
	if ( pConnect->Send(pPacket.getPtr(), bEncrypt) )
	{
		pConnect->OnSucceedSendEvent(sendEvent, pPacket.getPtr());		
		return true;
	}
	else
	{
		sendEvent->Log("发送失败 X");
	}

	return false;
}

bool tBaseNetHandle::OnReceiveEvent( tNetConnect *revConnect, Logic::tEvent *revEvent )
{
	__ENTER_FUNCTION

		AutoEvent hE = revEvent;
	if (hE)
	{
		hE->_OnBindNet(revConnect);
		hE->Log("成功接收");
		//		hE->Log("开始执行网络消息事件...");
		bool re = hE->DoEvent(true);
		//		hE->Log("执行结果[%s]", re ? "TRUE":"FALSE");

		return re;
	}
	//}
	return false;
	__LEAVE_FUNCTION
		return false;
}

void tBaseNetHandle::Log( const char *szInfo, ... )
{
#if DEVELOP_MODE
	va_list va;
	va_start(va, szInfo);
	LOG_TOOL(va, szInfo);
#endif
}
