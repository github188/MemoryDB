#ifndef _INCLUDE_IOCPCLIENTSETNET_H_
#define _INCLUDE_IOCPCLIENTSETNET_H_

#include "IOCPServerNet.h"

#define CLIENT_SET_MAX_COUNT		100


//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
// 客户端连接集合
// NOTE: 1 一个事件中心内, 只能设置一个集合网络
//		 2 事件发送, 使用 _getTarget() 得到网络ID后发送

// 多客户端连接网络, 用于组建节点网络体系
// 客户端连接在连接成功后,设置唯一ID, 可以通过ID, 从管理中获取

//-------------------------------------------------------------------------
class ConnectNetThread;

class Net_Export IOCPClientSetNet : public IOCPBaseNet
{	
typedef PoolList<ConnectNetThread*> ConnectThreadList;

public:
	IOCPClientSetNet()
		: IOCPBaseNet(CLIENT_SET_MAX_COUNT)
	{		
		//IOCPBaseNet::StartNet();
	}

public:
	virtual bool Connect(const char *szIp, int nPort, int overmilSecond);

	virtual void OnConnected(tNetConnect *pConnect) = 0;
	virtual void OnConnectFail(const char *szIp, int nPort) = 0;

	virtual void OnConnected(){  }
	virtual void OnConnectFail(){}
	virtual void OnCloseConnect(tNetConnect *pConnect) {}

	virtual bool IsConnected(){ return GetClientConnect(); }

public:
	virtual bool StartNet(const char *szIP, int port){ return Connect(szIP, port, GetConnectOverTime()); }
	//virtual bool StartNet(void) { return true; }

	virtual void StopNet(void);

	//virtual tNetConnect* GetConnect(int netID) { return IOCPBaseNet::GetConnect(0); }
	virtual const char* GetIp(void) const{ AssertEx(0, "More client"); return "MORE IP"; }
	virtual int	GetPort(void) const { AssertEx(0, "More client"); return -1; }
	virtual bool IsOk(void) { return true; }
	virtual bool TryReady(void){ return false; }

	virtual void Process();

	virtual void _SendSafeCode(IOCPConnect *pRecConnect);

	virtual void _OnConnectStart(tNetConnect *pConnect) override;

	virtual HandConnect CreateConnect();

protected:
	ConnectThreadList	mWaitConnectThreadList;
};


#endif //_INCLUDE_IOCPCLIENTSETNET_H_