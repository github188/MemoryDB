#ifndef _INCLUDE_IOCPCLIENTSETNET_H_
#define _INCLUDE_IOCPCLIENTSETNET_H_

#include "IOCPServerNet.h"

#define CLIENT_SET_MAX_COUNT		100


//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
// �ͻ������Ӽ���
// NOTE: 1 һ���¼�������, ֻ������һ����������
//		 2 �¼�����, ʹ�� _getTarget() �õ�����ID����

// ��ͻ�����������, �����齨�ڵ�������ϵ
// �ͻ������������ӳɹ���,����ΨһID, ����ͨ��ID, �ӹ����л�ȡ

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