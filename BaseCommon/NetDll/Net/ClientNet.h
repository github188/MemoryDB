
#ifndef _INCLUDE_CLIENTNET_H_
#define _INCLUDE_CLIENTNET_H_
#include "NetHead.h"

#include "NetConnect.h"
#include "EasyString.h"
#include "BaseEventNet.h"

class tNetHandle;
class tSafeCheck;

//----------------------------------------------------------------------------------------------
// 客户端网络
//----------------------------------------------------------------------------------------------
class Net_Export tClientConnect : public tConnect
{
public:
	tClientConnect();

	virtual ~tClientConnect();

	virtual bool InitConnect(void);

	virtual void SetNetHandle(tNetHandle *pNetServerTool);
	virtual tNetHandle* GetNetHandle( void ){ return mNetHandle; }	
	

public:
	virtual void OnConnected(void) = 0;
	virtual void OnClose(void) = 0;

public:
	virtual bool Connect(const char *ip, unsigned int port, bool bWait = false);
	virtual void Close(void);

	virtual bool IsConnect(int checkTime);

	virtual AString GetHost(void);
	virtual int GetPort(void);
    
    virtual void BindSocket(Socket *pSocket);


protected:
	tNetHandle			*mNetHandle;

};

class EventNetProtocol;
class ConnectThread;

class Net_Export EventClientNet : public tBaseEventNet
{
public:
	EventClientNet();

public:
	virtual bool StartNet(const char *ip, unsigned int port){ return Connect(ip, port, 0); }
	virtual void OnConnected(void) { mbDisconnect = false; }
	virtual void OnClose(void) {}

public:
	virtual bool Connect(const char *ip, unsigned int port, int overTime);

	// overTime 进行异步连接超时时间, 如果为0, 表示使用同步进行连接
	virtual bool ConnectDNS(const char *dnsName, int port, int overTime = 0);

	virtual bool CheckConnect(bool &bFinish);

	virtual void Process();


	virtual Logic::tEventCenter* GetEventCenter(void) const { return mEventCenter; }
	virtual void SetEventCenter(Logic::tEventCenter *pCenter){ mEventCenter = pCenter; }

protected:
	Hand<tClientConnect>	mClientConnect;
	Logic::tEventCenter		*mEventCenter;
	bool					mbDisconnect;
	Array<ConnectThread*>		mConnectThreadList;

public:
	virtual tNetConnect* GetConnect(int netID) { return mClientConnect.getPtr(); }
	virtual void OnCloseConnect(tNetConnect *pConnect) {}
	virtual const char* GetIp(void) const { return "not saved"; }
	virtual int		GetPort(void) const { return 0; }
	virtual bool IsOk(void) { return mClientConnect && mClientConnect->IsOk(); }
	virtual bool TryReady(void) { return true; }

	virtual bool CanWrite(int checkTime);
	virtual bool CheckConnect();

	virtual void InitReset();

	virtual void _StartConnectThread(const char *szIP, int port, int overTime);

	static AString GetIPByDNS(const char *szDNS);

};

class BaseClientConnect : public tClientConnect
{
public:
	virtual void OnConnected(void) 
	{
		EventClientNet *pNet = dynamic_cast<EventClientNet*>(mNetHandle);
		AssertEx(pNet, "应该是 EventClientNet");
		pNet->OnConnected();
	}
	virtual void OnClose(void) 
	{
		EventClientNet *pNet = dynamic_cast<EventClientNet*>(mNetHandle);
		AssertEx(pNet, "应该是 EventClientNet");
		pNet->OnClose();
	}

	virtual void SetNeedRemove( const char *szInfo ){}

};

#endif //_INCLUDE_CLIENTNET_H_