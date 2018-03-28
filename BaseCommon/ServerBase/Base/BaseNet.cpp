

#include "BaseNet.h"
#include "EventDllPlugin.h"
#include "Assertx.h"
#include "BaseThread.h"
#include "NetHandle.h"

#include "ClientNet.h"
#include <stdarg.h>

using namespace Logic;
//Logic::NetDllPlugin		gNetDllPlugin;
////-------------------------------------------------------------------------------------------------------------------
//
//bool NetHandle::LoadNetDll(const char* netDllPluginName)
//{
//	//初始网络服务
//	if (!gNetDllPlugin.LoadPluginDll(netDllPluginName))
//	{
//		AString info = "加载网络模块错误-->";
//		info += netDllPluginName;
//		TableTool::Log(info.c_str());
//		Assert(0 &&"加载网络模块错误-->netDllPluginName\n" );
//		return false;
//	}
//	return true;
//}
////-------------------------------------------------------------------------------------------------------------------
//
//void NetHandle::FreeNetDll(void)
//{
//	gNetDllPlugin.FreeNetDll();
//}
////-------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------

TryConnentEvent::TryConnentEvent() 
: mBaseThread(NULL)
, mOverTime(1)
, mTryCount(3)
{

}
//-------------------------------------------------------------------------------------------------------------------

void TryConnentEvent::_ConnectOnce()
{
	if (--mTryCount <0 )
	{
		Finish();
		return;
	} 
	
	mNetServerTool->TryReady();		
	WaitTime((float)mOverTime);
	Log("Try once for [%d]", mTryCount);
	return;

	Finish();
}
//-------------------------------------------------------------------------------------------------------------------
void TryConnentEvent::SetConnectInfo(tNetHandle *pServerTool, BaseThread *pBaseThread, int overTime, int tryCount )
{
	mNetServerTool = pServerTool;
	mBaseThread = pBaseThread;
	mOverTime = overTime;
	mTryCount = tryCount;
}
//-------------------------------------------------------------------------------------------------------------------

bool TryConnentEvent::_OnTimeOver()
{
	if (mNetServerTool && !(mNetServerTool->IsOk()))
	{
		_ConnectOnce();
	}
	else
	{
		Finish();
		return true;
	}
	return false;
}
//-------------------------------------------------------------------------------------------------------------------

void TryConnentEvent::_OnFinish()
{
	if (mNetServerTool && mNetServerTool->IsOk())
		Log("连接成功");
	else
		Log("尝试连接失败");
}

AutoNet TryConnentEvent::GetNetTool()
{
	return mNetServerTool->GetSelf();
}

bool TryConnentEvent::_DoEvent()
{
	if (mNetServerTool && !(mNetServerTool->IsOk()))
	{
		_ConnectOnce();
	}
	else
		Finish();
	return true;
}

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
TryConnectHandle::TryConnectHandle(BaseThread *pBaseThread)
: ClientNetHandle(pBaseThread)
{
	GetEventCenter()->RegisterEvent("TryConnect", MEM_NEW EventFactory<TryConnentEvent>());
}
//-------------------------------------------------------------------------------------------------------------------

bool TryConnectHandle::StartNet()
{
	ClientNetHandle::InitNet();

	AutoEvent hEvt = GetEventCenter()->StartEvent("TryConnect");
	TryConnentEvent *pEvt = dynamic_cast<TryConnentEvent*>(hEvt.getPtr());
	if (pEvt)
	{
		if (GetConnectFinishEvent())
		{
			AutoEvent hFinish = GetEventCenter()->StartEvent(GetConnectFinishEvent());
			if (hFinish)
				pEvt->set("FINISH_NOTIFY", hFinish);
			else
			{
				TableTool::Log("Error: finish connect event [%s] no exist ", GetConnectFinishEvent());
			}
		}
		pEvt->SetConnectInfo(this, mBaseThread, GetConnectOverTime(), GetTryConnectCount());		
		pEvt->DoEvent(true);
		return true;
	}
	return false;
}
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
class GameClientNet : public tClientConnect
{
public:
	virtual void Log(const char *szInfo, ...) 
	{
		va_list va;
		va_start(va,szInfo);
		TableTool::Log(va,szInfo);

	}

public:
	virtual void OnConnected(void)
	{
		Log("网络连接成功");
	}
	virtual void OnClose(void) 
	{
		Log("网络被断开");
	}
};


//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------

ClientNetHandle::ClientNetHandle(BaseThread *pBaseThread)
: BaseNetHandle(pBaseThread)
{
	tClientConnect *pNet = MEM_NEW GameClientNet();
	pNet->SetNetHandle(this);
	mClientNet = pNet;
}
//-------------------------------------------------------------------------------------------------------------------

ClientNetHandle::~ClientNetHandle()
{
	GetClientNet()->SetNetHandle(NULL);
	mClientNet._free();
	//mClientNet = NULL;
}
//-------------------------------------------------------------------------------------------------------------------

void ClientNetHandle::Process(void)
{
	mClientNet->Process();
}
//-------------------------------------------------------------------------------------------------------------------

tNetConnect* ClientNetHandle::GetConnect( int netID )
{
	return mClientNet.getPtr();
}
//-------------------------------------------------------------------------------------------------------------------

bool ClientNetHandle::InitNet()
{
	Logic::tEventCenter  *pCenter = GetEventCenter();
	BaseNetHandle::InitNet();
	return GetClientNet()->InitConnect();
}

//-------------------------------------------------------------------------------------------------------------------

bool ClientNetHandle::TryReady( void )
{
	if (mClientNet->TryConnect())
	{
		mClientNet->SetOK();
		return true;
	}
	return false;
}

bool ClientNetHandle::IsOk( void )
{
	return mClientNet && mClientNet->IsOk();
}

tClientConnect* ClientNetHandle::GetClientNet()
{
	return (tClientConnect *)mClientNet.getPtr();
}


//-------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------

bool NetConnectEvent::_DoEvent()
{
	mNet->StartNet();
	mNet->TryReady();
	WaitTime((float)mOverTimeSecond);
	StartUpdate(0.1f);
	return true;
}

bool NetConnectEvent::Update( float )
{
	if (mNet->IsOk())
	{
		OnConnectSucceed();
		Finish();
		return true;
	}
	return false;
}

bool NetConnectEvent::_OnTimeOver( void )
{
	OnConnectOverTime();
	Finish();
	return true;
}




//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------

void DefaultThreadReceiveConnect::OnStartNet()
{
	Hand<TM_WaitNetCheckEvent> hEvt = GetNetHandle()->GetEventCenter()->StartEvent("TM_WaitNetCheckEvent");

	hEvt->mNetConnect = GetSelf();
	hEvt->DoEvent();

	mWaitNetCheckEvent = hEvt;
}

void DefaultThreadReceiveConnect::SetRemove( bool bNeedRemove )
{
	base::SetRemove(bNeedRemove);

	if (bNeedRemove)
		mWaitNetCheckEvent._free();
}

void DefaultThreadReceiveConnect::OnWaitCheckNetOverTime()
{
	Log("等待网络检查事件超时, 可能连接已断");
	DefaultTcpThreadServerNet *pNet = dynamic_cast<DefaultTcpThreadServerNet*>(GetNetHandle());
	AssertEx(pNet!=NULL, "网络应该是 DefaultTcpThreadServerNet");
	pNet->OnConnectCheckFail(GetSelf());
}

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------

Logic::tEventCenter* DefaultTcpThreadServerNet::GetEventCenter( void ) const
{
	return mBaseThread->GetMainEventCenter().getPtr();
}

//tNetConnect* DefaultTcpThreadServerNet::GetConnect( int netID )
//{
//	//AssertEx(0, "不可使用此功能"); return NULL;
//	base::GetConnect(netID);
//}

bool DefaultTcpThreadServerNet::StartNet( void )
{
	bool re = base::StartNet();

	RegisterEvent(GetEventCenter()->GetSelf());

	return re;
}

bool DefaultTcpThreadServerNet::OnAddConnect( tNetConnect *pConnect )
{
	base::OnAddConnect(pConnect);	
	DefaultThreadReceiveConnect  *p = dynamic_cast<DefaultThreadReceiveConnect*>(pConnect);
	p->OnStartNet();

	return true;
}

void DefaultTcpThreadServerNet::RegisterEvent( AutoEventCenter hCenter )
{
	hCenter->RegisterEvent("TM_WaitNetCheckEvent", MEM_NEW EventFactory<TM_WaitNetCheckEvent>());
	hCenter->RegisterEvent("QuestCheckNet_R", MEM_NEW EventFactory<S_QuestCheckNet>());
}


//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------

Logic::tEventCenter* DefaultTCPThreadClientNet::GetEventCenter() const
{
	return mBaseThread->GetMainEventCenter().getPtr();
}

void DefaultTCPThreadClientNet::OnConnected()
{
	_TcpClientNet::OnConnected();
	
	mNetCheckEvent = GetEventCenter()->StartEvent("QuestCheckNet");
	Hand<C_QuestCheckNet> e = mNetCheckEvent;
	e->mbConnected = true;
	e->mClientNet = GetSelf();
	mNetCheckEvent->DoEvent();
}

void DefaultTCPThreadClientNet::StopNet( void )
{
	parent::StopNet();
	if (mNetCheckEvent)
	{
		mNetCheckEvent->Finish();
		mNetCheckEvent.setNull();
	}
}

bool DefaultTCPThreadClientNet::TryReady()
{
	bool b = Connect(mIp.c_str(), mPort, 0);

	return b;
}

void DefaultTCPThreadClientNet::OnConnectFail()
{
	_TcpClientNet::OnConnectFail();

	mNetCheckEvent = GetEventCenter()->StartEvent("QuestCheckNet");
	Hand<C_QuestCheckNet> e = mNetCheckEvent;
	e->mbConnected = false;
	e->mClientNet = GetSelf();
	mNetCheckEvent->DoEvent();
}

void DefaultTCPThreadClientNet::RegisterEvent( AutoEventCenter hCenter )
{
	hCenter->RegisterEvent("QuestCheckNet", MEM_NEW Logic::SR_RequestEventFactory<C_QuestCheckNet>());
}

bool DefaultTCPThreadClientNet::StartNet()
{
	RegisterEvent(GetEventCenter()->GetSelf());
	return _TcpClientNet::StartNet();
}


//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
