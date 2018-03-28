#include "MeshedNodeNet.h"
#include "ServerEvent.h"
#include "ServerIPInfo.h"
#include "IOCPCommon.h"
#include "NodeNetEvent.h"
#include "MeshedNetNodeData.h"

using namespace Logic;

//-------------------------------------------------------------------------



HandConnect MeshedNodeNet::CreateConnect()
{
	return MEM_NEW NodeRequestConnect();
}

void MeshedNodeNet::OnReceiveResponse(NetNodeConnectData *nodeData, AutoEvent &requstEvt, AutoEvent &respEvt)
{
	nodeData->set("_INFO_", respEvt["INFO"].string());
}

void MeshedNodeNet::OnOtherNodeRequestConnect( HandConnect connect, GSKEY otherServerNetKey )
{	
	Hand<NodeServerConnect> serverConnect = connect;
	serverConnect->mServerNetKey = otherServerNetKey;

	Hand<NetNodeConnectData> evt = mRequestConnectList.find(otherServerNetKey);
	if (evt)
	{
		if ( evt->mNodeConnect && !evt->mNodeConnect->IsRemove() )
		{
			int p1, p2;
			AString ip = ServerIPInfo::Num2IP(otherServerNetKey, p1, p2);
			TABLE_LOG("已经存在连接 [%s:%d]", ip.c_str(), p1);
		}

		//evt->mResponseConnect = connect;
		serverConnect->mNetNodeConnectData = evt.getPtr();

	}	
	else
	{
		// 请求连接
		NetNodeConnectData* pInfo = RequestConnectOtherNode(otherServerNetKey);
		pInfo->mServerNetKey = otherServerNetKey;
		serverConnect->mNetNodeConnectData = pInfo;
		//int p1, p2;
		//AString ip = ServerIPInfo::Num2IP(otherServerNetKey, p1, p2);
		//Connect(ip.c_str(), p1, GetConnectOverTime());
	}
}

void MeshedNodeNet::OnConnected( tNetConnect *pConnect )
{
	//NodeRequestConnect *p = dynamic_cast<NodeRequestConnect*>(pConnect);
	//AssertEx(p!=NULL, "必须为 NodeRequestConnect ");
	//p->mOtherServerKey = ServerIPInfo::IP2Num(pConnect->GetIp(), pConnect->GetPort(), 0);
	//TABLE_LOG( "连接成功>[%s:%d]", pConnect->GetIp(), pConnect->GetPort() );

	//Hand<TM_ConnectNodeServerEvent> evt = mRequestConnectList.find(p->mOtherServerKey);
	//evt->mNodeConnect = p->GetSelf();

	DumpAllConnect();

	// 发送连接请求
	AutoEvent evtQuest = pConnect->StartEvent("Node_RequestConnect");
	evtQuest->_setTarget(pConnect->GetNetID());
	evtQuest->DoEvent();
}

bool MeshedNodeNet::Connect( const char *szIp, int nPort, int overmilSecond )
{
	if (szIp==NULL || nPort<=0)
	{
		NOTE_LOG("ERROR: MeshedNodeNet::Connect >未提供正常的连接IP或端口[%s:%d]", szIp, nPort);
		return false;
	}

	if (mServerIp==szIp && mServerPort==nPort)
	{
		TABLE_LOG("WARN: 连接自己的服务器节点, 放弃连接");
		return false;
	}

	GSKEY  netKey = ServerIPInfo::IP2Num(szIp, nPort, 0);
	if (netKey==mServerNetKey)
	{
		TABLE_LOG("WARN: 连接自己的服务器节点, 放弃连接");
		return false;
	}
	if (mRequestConnectList.exist(netKey))
	{
		TABLE_LOG("当前已经存在 [%s:%d;, 不需要再次连接", szIp, nPort);
		return false;
	}

	RequestConnectOtherNode(netKey);

	return true;
}

HandConnect MeshedNodeNet::GetRequestConnect( GSKEY otherNetKey )
{
	Hand<NetNodeConnectData> evt = mRequestConnectList.find(otherNetKey);
	if (evt)
		return evt->mNodeConnect;

	return HandConnect();
}

//-------------------------------------------------------------------------


class Node_RequestConnect : public tNodeRequestEvent
{
public:
	virtual bool _DoEvent()
	{
		WaitTime(10);
		AutoNet net = GetEventCenter()->_GetNetTool(0);
		AssertEx(net, "中心必须设定了网络");

		MeshedNodeNet *p = dynamic_cast<MeshedNodeNet*>( net.getPtr() );
		AssertEx (p!=NULL, "必定 MeshedNodeNet");
		p->OnRequestNode(dynamic_cast<NodeRequestConnect*>(mNetConnect.getPtr())->mNetNodeConnectData, GetSelf());
		set("SERVER_KEY", p->mServerNetKey);			
		 
		return true;
	}
	 
	virtual void _OnResp(AutoEvent &resp)
	{
		//TABLE_LOG(resp->GetData().dump().c_str());
		// 如果回复即表示连接成功
		TABLE_LOG("成功连接到 [%s:%d]", mNetConnect->GetIp(), mNetConnect->GetPort());

		Hand<MeshedNodeNet> net = GetMeshedNodeNet();
		net->OnReceiveResponse(dynamic_cast<NodeRequestConnect*>(mNetConnect.getPtr())->mNetNodeConnectData, GetSelf(), resp);
		net->DumpAllConnect();
		int count = resp->get("COUNT");
		for (int i=0; i<count; ++i)
		{
			GSKEY ssKey = resp->get(STRING(i));
			int p1 = 0, p2 = 0;
			AString ip = ServerIPInfo::Num2IP(ssKey, p1, p2);
			net->StartNet(ip.c_str(), p1);
		} 
		Finish();
	}
};
//-------------------------------------------------------------------------
class Node_RequestConnect_R : public tNodeResponseEvent
{
public:
	virtual bool _DoEvent()
	{
		Data serverKey = get("SERVER_KEY");
		if (serverKey.empty())
		{
			TABLE_LOG("ERROR: 未提供服务网络KEY");
			mNetConnect->SetRemove(true);
			return false;
		}
		
		// 加入到网络
		GetMeshedNodeNet()->OnOtherNodeRequestConnect(mNetConnect, serverKey);
		AutoEvent respEvt = GetResponseEvent();
		GetMeshedNodeNet()->OnResponseNode(serverKey, GetSelf(), respEvt);

		NetNodeList &list = GetMeshedNodeNet()->GetNetNodeList();
		for (size_t i=0; i<list.size(); ++i)
		{
			GSKEY ssKey = list.getKey(i);
			respEvt->set(STRING((int)i), ssKey);
		}
		respEvt->set("COUNT", (int)list.size());

		Finish();

		return true;
	}
};
//-------------------------------------------------------------------------*/
// 接收处理通知节点关闭
class Node_NotifyClose_R : public tBaseNetEvent
{
public:
	virtual bool _DoEvent() override
	{
		NodeServerNet *pNet = dynamic_cast<NodeServerNet*>(mNetConnect->GetNetHandle());
		AssertEx(pNet!=NULL, "必须为 NodeServerNet");

		Hand<NodeServerConnect> conn = mNetConnect;
		if (conn && conn->mNetNodeConnectData!=NULL)
		{
			NetNodeConnectData *e = conn->mNetNodeConnectData;			
			AString info = ServerIPInfo::GetAddrInfo(e->mServerNetKey);
						
			if (e->IsMainNode())
			{
				NOTE_LOG( "主节点主动关闭, 尝试继续连接 <%s> KEY %s", e->get("_INFO_").string().c_str(), info.c_str());
			}
			else
			{
				NOTE_LOG( "对方节点主动关闭 <%s> KEY %s", e->get("_INFO_").string().c_str(), info.c_str());
				pNet->mOwnerMeshNet->RemoveNode(conn->mNetNodeConnectData);
				pNet->mOwnerMeshNet->DumpAllConnect();
			}
		}
		return true;
	}
};
//-------------------------------------------------------------------------

MeshedNodeNet::MeshedNodeNet( const char *szServerIp, int port, int nSafeCheck, int threadNum ) 
	: IOCPBaseNet(NET_CONNECT_MAX, threadNum)
	, mServerIp(szServerIp)
	, mServerPort(port)
	, mSafeCheckCode(nSafeCheck)
	, mbStop(false)
	, mMainNodeKey(0)
{
	//IOCPBaseNet::StartNet();

	mServerNetKey = ServerIPInfo::IP2Num(szServerIp, port, 0);
	mServerNet = MEM_NEW NodeServerNet(this, threadNum);
	if (mServerPort>0)
		mServerNet->StartNet(mServerIp.c_str(), mServerPort);
	else
		NOTE_LOG("WARN: [%s:%d] Node server port <= 0, then node stoping", szServerIp, port);
}

MeshedNodeNet::~MeshedNodeNet()
{
	StopNet();
	if (mEventCenter)
	{
		mEventCenter->RemoveFactory("TM_ConnectNodeServerEvent");
		mEventCenter->RemoveFactory("Node_RequestConnect");
	}

	if (mServerNet && mServerNet->GetEventCenter()!=NULL)
	{
		mServerNet->GetEventCenter()->RemoveFactory("Node_RequestConnect");
		mServerNet->GetEventCenter()->RemoveFactory("Node_NotifyClose");
	}
	mServerNet._free();
}

void MeshedNodeNet::OnConnectFail(AutoEvent waitEvent )
{
	Hand<NetNodeConnectData> evt = waitEvent;
	int p1, p2;
	AString ip = ServerIPInfo::Num2IP(evt->mServerNetKey, p1, p2);
	TABLE_LOG( "连接失败>[%s:%d]", ip.c_str(), p1);	
	
	// 连接失败时, 会由事件控制重连处理
	_OnConnectNodeFault(waitEvent);	
}

void MeshedNodeNet::OnCloseConnect( tNetConnect *pConnect )
{
	NOTE_LOG( "Disconnect>[%s:%d]", pConnect->GetIp(), pConnect->GetPort());	
	//GSKEY  netKey = ServerIPInfo::IP2Num(pConnect->GetIp(), pConnect->GetPort(), 0);

	if (mbStop)
		return;

	for (size_t i=0; i<mRequestConnectList.size(); )
	{
		Hand<NetNodeConnectData> evt = mRequestConnectList.get(i);
		if (!evt)
		{
			mRequestConnectList._remove(i);
			continue;
		}
		++i;
		if (evt->mNodeConnect.getPtr()==pConnect)
		{
			if (GetTryConnectCount()==0)
				RemoveNode(evt.getPtr());
			else
			{
				evt->mNodeConnect.setNull();
				evt->setFinished(false);
				// 进行重连
				evt->DoEvent(false);
				_OnConnectNodeFault(evt);
			}

			DumpAllConnect();
			break;
		}
	}

}

void MeshedNodeNet::_OnConnectNodeFault(AutoEvent waitEvent)
{	
	Hand<NetNodeConnectData> evt = waitEvent;
	TABLE_LOG("WARN: 连接到 %s 发生故障, 当前作移除处理, 等待重连", ServerIPInfo::GetAddrInfo(evt->mServerNetKey).c_str());

		//DumpAllConnect();
		//evt->Finish();
		//mRequestConnectList.erase(netKey);
		//DumpAllConnect();
	
}

void MeshedNodeNet::DumpAllConnect()
{
	AString info;
	info += ( "\r\n======ALL CONNECT==============================\r\n");
	AString temp;
	temp.Format( "<%s> KEY [%s:%d] THIS\r\n", NodeInfo(), mServerIp.c_str(), mServerPort );
	info += temp;
	for (size_t i=0; i<mRequestConnectList.size(); ++i)
	{
		GSKEY netKey = mRequestConnectList.getKey(i);
		int p1, p2;
		AString ip = ServerIPInfo::Num2IP(netKey, p1, p2);

		Hand<NetNodeConnectData> e = mRequestConnectList.get(i);
		if (e->mNodeConnect)
		{
			temp.Format( "<%s> KEY [%s:%d] => [%s:%d]\r\n", e->get("_INFO_").string().c_str(), GetIp(), GetPort(), e->mNodeConnect->GetIp(), e->mNodeConnect->GetPort());
		}
		else
		{
			temp.Format( "<%s> KEY [NULL] => [%s:%d]\r\n", e->get("_INFO_").string().c_str(), ip.c_str(), p1);
		}
		info += temp;
	}
	info += ( "===============================================\r\n");
	TableTool::green();
	printf(info.c_str());
	TableTool::white();
}

AutoEvent MeshedNodeNet::StartRequestEvent( const char *szEvent, GSKEY targetNetkey )
{
	HandConnect conn = GetRequestConnect(targetNetkey);
	if (conn)
		return conn->StartEvent(szEvent);

	TABLE_LOG("ERROR: 不存在网络连接　%s", ServerIPInfo::GetAddrInfo(targetNetkey).c_str());
	return AutoEvent();
}

void MeshedNodeNet::_SendSafeCode( IOCPConnect *pConnect )
{
	int safe = GetSafeCode();
	if (safe!=0)
	{
        pConnect->_Send((const CHAR*)&safe, sizeof(int));
	}
	
	pConnect->_SendTo();
}

//-------------------------------------------------------------------------


NetNodeConnectData* MeshedNodeNet::RequestConnectOtherNode( GSKEY netKey )
{
	AutoEvent hEvt = mRequestConnectList.find(netKey);
	if (hEvt)
	{
		hEvt->Finish();
		hEvt._free();
	}

	ConnectNetThread *pWaitConnectThread = MEM_NEW ConnectNetThread();

	int p1, p2;
	AString ip = ServerIPInfo::Num2IP(netKey, p1, p2);
	pWaitConnectThread->StartConnect(ip.c_str(), p1, GetConnectOverTime());

	Hand<NetNodeConnectData> hConnectEvt = GetEventCenter()->StartEvent("TM_ConnectNodeServerEvent");
	hConnectEvt->mConnectNodeNetThread = pWaitConnectThread;
	hConnectEvt->mServerNetKey = netKey;
	hConnectEvt->mMeshedNet = GetSelf();	

	mRequestConnectList.erase(netKey);
	mRequestConnectList.insert(netKey, hConnectEvt);
	
	hConnectEvt->DoEvent(false);	

	return hConnectEvt.getPtr();
}

void MeshedNodeNet::Process()
{
	IOCPBaseNet::Process();
	mEventCenter->ProcessEvent();
	mServerNet->Process();
}

NetNodeConnectData* MeshedNodeNet::GetNodeConnectData( HandConnect connect )
{
	NodeServerConnect *pConnect = dynamic_cast<NodeServerConnect*>(connect.getPtr());
	if (pConnect!=NULL)
		return pConnect->mNetNodeConnectData;
	
	NodeRequestConnect *pConn = dynamic_cast<NodeRequestConnect*>(connect.getPtr());
	if (pConn!=NULL)
		return pConn->mNetNodeConnectData;

	//for (size_t i=0; i<mRequestConnectList.size(); ++i)
	//{
	//	Hand<NetNodeConnectData> evt = mRequestConnectList.get(i);
	//	if (evt->mNodeConnect==connect)
	//		return evt.getPtr();
	//}
	return NULL;
}

void MeshedNodeNet::RemoveNode(NetNodeConnectData *pNode)
{
	OnRemoveNode(pNode);
	INFO_LOG("开始移除节点连接 <%s> %s", pNode->get("_INFO_").string().c_str(), ServerIPInfo::GetAddrInfo(pNode->mServerNetKey).c_str());
	Hand<NodeServerNet> net= mServerNet;
	net->RemoveNode(pNode);
	AutoEvent evt = pNode->GetSelf();
	evt._free();
	for (size_t i = 0; i<mRequestConnectList.size(); )
	{
		AutoEvent node = mRequestConnectList.get(i);
		if (!node || node.getPtr()==pNode)
			mRequestConnectList._remove(i);
		else
			++i;
	}
}

void MeshedNodeNet::SetMainNode(const char *szIP, int port)
{
	mMainNodeKey = ServerIPInfo::IP2Num(szIP, port, 0);
}

bool MeshedNodeNet::StartNet(const char *szIP, int port)
{	
	return Connect(szIP, port, GetConnectOverTime());
}

void MeshedNodeNet::StopNet()
{
	mbStop = true;
	mServerNet->StopNet();
	IOCPBaseNet::StopNet();
	for (size_t i = 0; i<mRequestConnectList.size(); ++i)
	{
		mRequestConnectList.get(i)._free();
	}	
	mRequestConnectList.clear(false);
}

void MeshedNodeNet::NotifyClose()
{
	AutoEvent evt = GetEventCenter()->StartDefaultEvent("Node_NotifyClose");
	for (size_t i = 0; i<mRequestConnectList.size(); ++i)
	{
		Hand<NetNodeConnectData> data = mRequestConnectList.get(i);
		if (data && data->mNodeConnect)
			data->mNodeConnect->SendEvent(evt.getPtr());
	}
}

void MeshedNodeNet::SetEventCenter(AutoEventCenter serverCenter, AutoEventCenter connectCenter)
{
	mEventCenter = connectCenter;

	BindEventCenter();

	mEventCenter->RegisterEvent("TM_ConnectNodeServerEvent", MEM_NEW EventFactory<NetNodeConnectData, true>());
	mEventCenter->RegisterEvent("Node_RequestConnect", MEM_NEW SR_RequestEventFactory<Node_RequestConnect, true>());

	Hand<NodeServerNet> net = mServerNet;
	net->mEventCenter = serverCenter;
	net->BindEventCenter();
	serverCenter->RegisterEvent("Node_RequestConnect", MEM_NEW EventFactory<Node_RequestConnect_R, true>());
	serverCenter->RegisterEvent("Node_NotifyClose", MEM_NEW EventFactory<Node_NotifyClose_R, true>());
	

	RegisterNodeEvent(serverCenter, connectCenter);
}

//-------------------------------------------------------------------------
GSKEY NodeRequestConnect::GetTargetNetKey()
{
	return mNetNodeConnectData->mServerNetKey;
}
//-------------------------------------------------------------------------

void NodeServerNet::RemoveNode(NetNodeConnectData *pNode)
{
	for (int i=0; i<mConnectList.size(); ++i)
	{
		Hand<NodeServerConnect> conn = mConnectList[i];
		if (conn && conn->mNetNodeConnectData==pNode)
		{
			INFO_LOG("正常移除节点连接 %s ==> [%s:%d]", ServerIPInfo::GetAddrInfo(pNode->mServerNetKey).c_str(), conn->GetIp(), conn->GetPort());
			conn->SetRemove(true);
			break;
		}
	}
}
