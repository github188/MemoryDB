
#include "GameServerConnectNet.h"

#include "GameServerEvent.h"

#include "ServerEvent.h"
//-------------------------------------------------------------------------------------------------------
// 专用于GS之前的接收数据特殊处理, 目的解决发送与接收同名事件消息问题
// 注意: 在LOGIN 或 WORLD中也必须使用
//		凡是连接GS的内部网络事件消息, 都要遵从这个规则
//-------------------------------------------------------------------------------------------------------

void GSClinetNet::RegisterGSEvent( Logic::tEventCenter *center )
{
	center->RegisterEvent("NodifyClientConnectOk_GR", MEM_NEW EventFactory<SC_NodifyClientConnectOk_C>());
	AString respName = RESP_EVENT_NAME;
	respName += "_GR";
	center->RegisterDefaultEvent( respName.c_str() );

}
//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------


void ExecuteEventPacket(tNetHandle *pNet, Packet *pPacket, tConnect *pConnect , Logic::tEventCenter *pEventCenter)
{
	__ENTER_FUNCTION
	AssertEx(pEventCenter!=NULL && pPacket!=NULL && pConnect!=NULL , "事件中心为空或包为空, 或连接为空");

	EventPacket* p = dynamic_cast<EventPacket*>(pPacket);
	if (p)
	{
		AutoData evtData = p->GetData();
		evtData->seek(0);
		//-------------------------------------------------------------------------
		// 恢复事件, 这里使用特殊处理, 即对接受到的事件名后面加_R, 以区分发送事件
		AutoEvent hE;
		AString eventName;
		if (evtData->readString(eventName))
		{			
			eventName += "_GR";   //!!! 统一方式
			hE = pEventCenter->StartEvent(eventName);
			if (hE)
			{
				if (!hE->_Restore(evtData))
				{
					TableTool::Log("Error: restor event [%s] form data stream", eventName.c_str());
					return;
				}					
			}
			else
			{
				TableTool::Log("Error: restor event form data stream, [%s] no exist", eventName.c_str());
				return;
			}
		}
		else
		{
			TableTool::Log("Error: restor event form data stream");
			return;
		}
		//--------------------------------------------------------------------------
		if (hE)
		{
			hE->_setTarget(pConnect->GetNetID());
			hE->_OnReceive(pNet, pConnect);
			hE->Log("开始执行网络消息事件...");
			bool re = hE->DoEvent(true);
			hE->Log("执行结果[%s]", re ? "TRUE":"FALSE");

			//成功接收到网络事件
			pConnect->OnSucceedReceiveEvent(hE.getPtr());
		}
	}
	else
	{
		TableTool::Log("XXX Error: 不是事件类型的消息包");
	}

	__LEAVE_FUNCTION
}

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
void GSClinetNet::ExecutePacket( Packet *pPacket, tConnect *pConnect )
{
	ExecuteEventPacket(this, pPacket, pConnect, GetEventCenter());
}


//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
void GSNetHandle::ExecutePacket( Packet *pPacket, tConnect *pConnect )
{
	ExecuteEventPacket(this, pPacket, pConnect, GetEventCenter());
}
//-------------------------------------------------------------------------------------------------------
bool GSNetHandle::OnAddConnect( tConnect *pConnect )
{
	// 加入到GS信息中, 优化单向连接, 即 GS间仅使用一条连接, 目前, 不作此优化处理
	// 1 连接端, 成功连接后, 即发送等待客户端IP
	// 2 接受连接端, 接收到 IP连接注册后, 即整理连接信息, 加入到GS, 
	// 3 可能会有重复情况, 比如同时相互连接, 此时不在考虑
	// 4 连接被断开后, 在 OnCloseConnect 内, 整理GS信息

#if !NEED_NET_SAFT_CHECK

	AutoEvent hE = GetEventCenter()->StartEvent("NodifyClientConnectOk");
	hE->set("ID", pConnect->GetNetID());
	Send(hE.getPtr(), pConnect);
	hE->Finish();	
	return true;
#endif
	return false;
}
//-------------------------------------------------------------------------------------------------------

void GSNetHandle::OnCloseConnect( tConnect *pConnect )
{
	// 清理 GS 信息
}

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------