
#include "BaseEventNet.h"
#include "TableTool.h"
#include "ServerIPInfo.h"
#include <stdarg.h>
#include "ClientEvent.h"
//----------------------------------------------------------------------------------------------

//bool tBaseEventNet::Send( Logic::tEvent *sendEvent, int target )
//{
//	tNetConnect *p = GetConnect(target);
//	if (p)
//		return Send(sendEvent, p);
//	else
//		sendEvent->Log("发送失败X, 获取不到网络连接[%d]", target);
//
//	return false;
//}
//
//bool tBaseEventNet::Send( Logic::tEvent *sendEvent, tNetConnect *pConnect )
//{
//	AssertEx(sendEvent!=NULL, "发送的事件为空");
//
//	Packet *pPacket = mEventNetProtocol.GenerateEventPacket(sendEvent, NeedZipSendPacket());
//	AssertEx(pPacket, "生成消息包失败");
//	
//	pPacket->SetNeedEncrypt(NeedEncryptPacket());
//
//	if ( pConnect->Send(pPacket) )
//	{
//		pConnect->OnSucceedSendEvent(sendEvent, pPacket);		
//		return true;
//	}
//	else
//	{
//		sendEvent->Log("发送失败 X");
//	}
//
//	return false;
//}

//----------------------------------------------------------------------------------------------


tBaseEventNet::tBaseEventNet()	
{
	mNetProtocol = MEM_NEW EventNetProtocol();
}
//-------------------------------------------------------------------------------------------------------------------

tBaseEventNet::~tBaseEventNet()
{

}


//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------


bool tBaseEventNet::OnReceiveEvent( tNetConnect *revConnect, Logic::tEvent *revEvent )
{
	__ENTER_FUNCTION

	AutoEvent hE = revEvent;
	if (hE)
	{
		hE->_OnBindNet(revConnect);
		hE->Log("成功接收");
//		hE->Log("开始执行网络消息事件...");
		try{
			bool re = hE->DoEvent(true);
			//		hE->Log("执行结果[%s]", re ? "TRUE":"FALSE");

			return re;
		}
		catch (std::exception &e)
		{
			bool bNeedRemove = NeedRemoveOnMsgError();
			if (bNeedRemove)
			{
				ERROR_LOG("[%s]网络事件执行错误, 移除连接>[%d]%s\n Exception>[%s]", hE->GetEventName(), revConnect->GetNetID(), ServerIPInfo::GetAddrInfo(revConnect->GetIPKey()).c_str(), e.what());
				revConnect->Close();			
			}
			else
			{
				ERROR_LOG("[%s]网络事件执行错误, 连接[%d]%s\n Exception>[%s]", hE->GetEventName(), revConnect->GetNetID(), ServerIPInfo::GetAddrInfo(revConnect->GetIPKey()).c_str(), e.what());
				Hand<Logic::tClientEvent> clientEvt = hE;
				hE->Finish();
			}
			bool bLog = hE->needLog();
			hE->setLog(true);
			hE->Dump();
			hE->setLog(bLog);
		}
		catch (...)
		{
			bool bNeedRemove = NeedRemoveOnMsgError();
			if (bNeedRemove)
			{
				ERROR_LOG("[%s]网络事件执行错误, 移除连接>[%d]%s\n", hE->GetEventName(), revConnect->GetNetID(), ServerIPInfo::GetAddrInfo(revConnect->GetIPKey()).c_str());
				revConnect->Close();			
			}
			else
			{
				ERROR_LOG("[%s]网络事件执行错误, 连接[%d]%s\n", hE->GetEventName(), revConnect->GetNetID(), ServerIPInfo::GetAddrInfo(revConnect->GetIPKey()).c_str());
				Hand<Logic::tClientEvent> clientEvt = hE;
				hE->Finish();
			}
			bool bLog = hE->needLog();
			hE->setLog(true);
			hE->Dump();
			hE->setLog(bLog);
		}
	}
	//}
	return false;
	__LEAVE_FUNCTION
		return false;
}

void tBaseEventNet::Log( const char *szInfo, ... )
{
#if DEVELOP_MODE
	va_list va;
	va_start(va, szInfo);
	LOG_TOOL(va, szInfo);
#endif
}

//----------------------------------------------------------------------