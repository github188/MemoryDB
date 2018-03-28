#ifndef _INCLUDE_TCPTHHREADNET_H_
#define _INCLUDE_TCPTHHREADNET_H_

#include "BaseEventNet.h"
#include "TCPListenThread.h"
#include "TCPReceiveThread.h"
#include "NetHead.h"


class ServerThreadConnect : TCPThreadReceiveConnect
{

};

class Net_Export TCPThreadServerNet : public tBaseEventNet, public TCPListenThread
{
public:
	virtual Logic::tEventCenter* GetEventCenter(void) const = 0; 

	// Run main thread
	virtual HandConnect CreateConnect(){ return MEM_NEW TCPThreadReceiveConnect(); }

	virtual bool RemoveConnect(tNetConnect *pConnect)
	{
		return mReceiveThread.RemoveConnect(pConnect);
	}

	//virtual bool StartNet(void) { InitStart(GetIp(), GetPort()); return true; }
	virtual void OnStop(){}
	virtual void OnCloseConnect(tNetConnect *pConnect) { }
	virtual const char* GetIp(void) const { return NULL; }
	virtual int		GetPort(void) const { return  0; }
	virtual bool IsOk(void) { return true; }
	virtual bool TryReady(void) { return true; }

	virtual void Process(){ mReceiveThread.Process(); TCPListenThread::Process(); }

	virtual int GetConnectCount(){ return mReceiveThread.GetConnectCount(); }

	virtual EasyList<HandConnect>& GetConnectList() 
	{
		return mReceiveThread.GetConnectList(); 
	}

	virtual tNetConnect* GetConnect(int netID) {AssertEx(0, "此网络不使用此功能"); return NULL; }

public:
	virtual bool StartNet(const char* szIp, int nPort);

public:
	// NOTE: in main thread.
	virtual void OnAcceptConnect(HandConnect newConnect);

	// NOTE: in main thread now [modify 2013.09.10]
	virtual HandConnect _CreateNewConnect()
	{
		HandConnect conn = CreateConnect();
		TCPThreadReceiveConnect *pRecConnect = dynamic_cast<TCPThreadReceiveConnect*>(conn.getPtr());
		AssertEx(pRecConnect!=NULL, "创建连接必须继承 TCPThreadReceiveConnect ");
		pRecConnect->SetNetHandle(GetSelf());		
		conn->InitCreate();

		return conn;
	}

	virtual void StopNet();

public:
	TCPReceiveThread	mReceiveThread;

};

class Net_Export TCPThreadClientNet : public tBaseEventNet, public WorkThread
{
public:
	TCPThreadClientNet();

	virtual ~TCPThreadClientNet();

public:
	virtual Logic::tEventCenter* GetEventCenter(void) const = 0; 
	virtual void OnConnected(){ mbDisconnect = false; }
	virtual void OnConnectFail(){}

	//virtual bool StartNet(void) 
	//{
	//	return TryReady();
	//	//if (mThreadConnect->InitCreate())
	//	//{
	//	//	InitThread();
	//	//	return true;
	//	//}
	//	//return false;
	//}

	virtual void StopNet(void){ mThreadConnect->Close(); }

	virtual tNetConnect* GetConnect(int netID) { return mThreadConnect.getPtr(); }
	virtual void OnCloseConnect(tNetConnect *pConnect) {}
	virtual const char* GetIp(void) const{ return mIp.c_str(); }
	virtual int	GetPort(void) const {  return mPort; }
	virtual bool IsOk(void) { return true; }
	virtual bool TryReady(void){ return Connect(GetIp(), GetPort(), false); }

public:
	virtual bool Connect(const char *szIp, int nPort, bool bThread);

	bool IsConnected(){ return mConnected; }

	virtual void Process();

public:
	virtual void backWorkThread(void);

protected:
	Hand<TCPThreadReceiveConnect>		mThreadConnect;
	bool						mConnected;
	bool						mbConnectFail;
	bool						mNeedConnect;
	bool						mAlreadyConnect;

	bool						mbDisconnect;

	EasyString					mIp;
	int							mPort;

	//NOTE: use at back thread
	fd_set					*m_ReadFDs;
	fd_set					*m_WriteFDs;
	fd_set					*m_ExceptFDs;

	timeval					*m_Timeout;
};


#endif