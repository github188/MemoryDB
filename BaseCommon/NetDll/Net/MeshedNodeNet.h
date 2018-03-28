#ifndef _INCLUDE_MESHEDNODENET_H_
#define _INCLUDE_MESHEDNODENET_H_

#include "IOCPServerNet.h"
#include "IOCPConnect.h"
#include "EventCenter.h"
//-------------------------------------------------------------------------
#define DEFALUT_MESH_NODE_SAFE_CODE		(-200)

using namespace Logic;

typedef EasyMap<GSKEY, AutoEvent>		NetNodeList;
/*/-------------------------------------------------------------------------
// 网状网络节点Net
// 实现每个节点互连, 如所有 GameServer 之间建立
// 连接如果中断后, 支持重连处理, 如通过 World中的列表, 整理网络连接(增加或移除)

// NOTE: 1 一个事件中心内, 只能设置一个集合网络
//		 2 事件发送, 使用 _getTarget() 得到网络ID后发送
//		 3 此网络的事件中心定的所有请求事件都必须使用 SR_RequestEventFactory 注册
//		   目的支持同一事件发送及接收 同名接收事件名称 后加 _R 如 CS_NetTestEvent_R
		 4 由于NodeNet 是一对多服务网络，所以消息ID不可能得到统一，当前节点内，不支持ID数组索引消息优化
   实现：
		1 接收其他节点的连接 AutoNet				mServerNet
		2 连接线程 ConnectNetThread
		3 连接数据结构 NetNodeConnectData，处理连接成功失败, 且保存连接对象
		4 连接其他节点的连接由 MeshedNodeNet 基类对象创建，相当于一个集合连接网络
		5 设置协议同时设置了连接及接受连接的服务网络mServerNet
		
//-------------------------------------------------------------------------*/
class NetNodeConnectData;

class Net_Export MeshedNodeNet : public IOCPBaseNet
{
public:
	MeshedNodeNet(const char *szServerIp, int port, int nSafeCheck, int threadNum = 2);
	~MeshedNodeNet();

	virtual const char* NodeInfo(){ return "THIS"; }

public:
	// 当主动连接对方Node，成功后, 通知对方事件发送之前，可携带其他数据
	virtual void OnRequestNode(NetNodeConnectData *nodeData, AutoEvent &requestEvt){}

	// 作为被连接者，接收到连接者连接后的请求事件, 主要告诉连接者当前整个节点群的信息
	virtual void OnResponseNode(GSKEY otherPartyGSKey, AutoEvent &requstEvt, AutoEvent &respEvt){ respEvt["INFO"] = NodeInfo(); }

	// 对方回复数据
	virtual void OnReceiveResponse(NetNodeConnectData *nodeData, AutoEvent &requstEvt, AutoEvent &respEvt);

	void OnOtherNodeRequestConnect(HandConnect connect, GSKEY otherServerNetKey);
	virtual NetNodeConnectData* RequestConnectOtherNode(GSKEY netKey);

	virtual void OnAppendNetNode(NetNodeConnectData *nodeData){}
	NetNodeList& GetNetNodeList(){ return mRequestConnectList; }

public:
	//virtual bool StartNet()
	//{
	//	return mServerNet->StartNet(mServerIp.c_str(), mServerPort);
	//}
	// NOTE: 主节点必须设置主节点地址, 否则其他节点异常断开后,会启动连接同一程序其他的节点
	void SetMainNode(const char *szIP, int port);
	virtual bool StartNet(const char *szIP, int port);
	virtual void StopNet();
	virtual void NotifyClose();

	virtual Logic::tEventCenter* GetServerCenter(){ return mServerNet->GetEventCenter(); }
	virtual Logic::tEventCenter* GetEventCenter(void) const override
	{
		return (tEventCenter*)mEventCenter.getPtr();
	}

	virtual void SetEventCenter(AutoEventCenter serverCenter, AutoEventCenter connectCenter);

	virtual void RegisterNodeEvent(AutoEventCenter serverCenter, AutoEventCenter connectCenter){}

	virtual int GetSafeCode() { return mSafeCheckCode; }
	GSKEY GetNetKey() const { return mServerNetKey; }

	// 异常断开后，尝试重新连接次数，0 不进行重新连接，直接移除 -1 一直尝试重连
	virtual int GetTryConnectCount(void) override { return 3; }

	// 尝试重连间隔时间
	virtual int GetConnectOverTime(void) override { return 6; }

public:
	virtual bool Connect(const char *szIp, int nPort, int overmilSecond);

	virtual void OnConnected(tNetConnect *pConnect);
	virtual void OnConnectFail(AutoEvent waitEvent);
	virtual void OnCloseConnect(tNetConnect *pConnect);
	virtual void OnRemoveNode(NetNodeConnectData *pNode){}

	virtual void _OnConnectNodeFault(AutoEvent hConnect);

	virtual void Process();

	HandConnect GetRequestConnect(GSKEY otherNetKey);
	AutoEvent StartRequestEvent(const char *szEvent, GSKEY targetNetkey);

	NetNodeConnectData* GetNodeConnectData(HandConnect connect);

	virtual void RemoveNode(NetNodeConnectData *pNode);

	virtual void DumpAllConnect();

public:
	virtual HandConnect CreateConnect();

	virtual void _SendSafeCode( IOCPConnect *pRecConnect );

	virtual const char* GetIp(void) const{ return mServerIp.c_str(); AssertEx(0, "More client"); return "MORE IP"; }
	virtual int	GetPort(void) const { return mServerPort; AssertEx(0, "More client"); return -1; }

	virtual void SetNetProtocol(Auto<tNetProtocol> netProtocol)
	{ 
		IOCPBaseNet::SetNetProtocol(netProtocol);
		Hand<tBaseEventNet> net = mServerNet;
		net->SetNetProtocol(netProtocol);
	}

public:
	AutoEventCenter		mEventCenter;
	GSKEY				mServerNetKey;
	GSKEY				mMainNodeKey;

protected:
	AutoNet				mServerNet;			// 接收其他节点服务网络

	AString				mServerIp;
	int					mServerPort;
	int					mSafeCheckCode;
	bool				mbStop;

	NetNodeList			mRequestConnectList;
};
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
class NodeServerConnect : public IOCPServerConnect
{
public:
	NodeServerConnect()
		: mServerNetKey(0)
		, mNetNodeConnectData(NULL)
	{

	}

	virtual void OnConnectSafeCodeError(const char *szIp, int port, int safeCode) override
	{
		ERROR_LOG(" [%s : %d] connect safe code error [%d], now net code [%d]", szIp, port, safeCode, GetNetHandle()->GetSafeCode());
		TableTool::red();
		printf("ERROR: [%s : %d] connect safe code error [%d], now net code [%d]\r\n", szIp, port, safeCode, GetNetHandle()->GetSafeCode());
		TableTool::white();
	}

public:
	GSKEY				mServerNetKey;
	NetNodeConnectData	*mNetNodeConnectData;
};
//-------------------------------------------------------------------------
class NodeServerNet : public IOCPServerNet
{
public:
	NodeServerNet(MeshedNodeNet *pMeshNet, int threadNum)
		: IOCPServerNet(NET_CONNECT_MAX, threadNum)
		, mOwnerMeshNet(pMeshNet)
	{

	}

	~NodeServerNet()
	{
		if (mEventCenter)
			mEventCenter->RemoveAllFactory();
		mEventCenter._free();
	}

public:
	virtual Logic::tEventCenter* GetEventCenter(void) const { return (Logic::tEventCenter*)mEventCenter.getPtr(); }
	virtual int GetSafeCode(){ return mOwnerMeshNet->GetSafeCode(); }

	virtual bool OnAddConnect(tNetConnect *pConnect)
	{

		return true; 
	}

	virtual void OnCloseConnect(tNetConnect *pConnect)
	{
		mOwnerMeshNet->OnCloseConnect(pConnect);
	}

	virtual HandConnect CreateConnect(){ return MEM_NEW NodeServerConnect(); }

	virtual void RemoveNode(NetNodeConnectData *pNode);

	virtual void Process() override
	{
		IOCPServerNet::Process();
		if (mEventCenter)
			mEventCenter->ProcessEvent();
	}

	// NodeNet 一对多服务网络，不支持优化ID数组索引消息, 所以直接返回true
	virtual bool NeedUpdateMsgIndex() const override { return true; }

public:
	MeshedNodeNet			*mOwnerMeshNet;
	AutoEventCenter			mEventCenter;
};
//-------------------------------------------------------------------------
class Net_Export NodeRequestConnect : public IOCPConnect
{
public:
	NodeRequestConnect()
		: mNetNodeConnectData(NULL)
	{

	}

	GSKEY GetTargetNetKey();

public:
	NetNodeConnectData	*mNetNodeConnectData;
};
//-------------------------------------------------------------------------

#endif //_INCLUDE_MESHEDNODENET_H_