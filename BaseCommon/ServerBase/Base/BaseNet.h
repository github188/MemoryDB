
/********************************************************************
	created:	2012/06/26
	created:	26:6:2012   1:48
	filename: 	d:\New\Common\ServerBase\Base\BaseNet.h
	file path:	d:\New\Common\ServerBase\Base
	file base:	BaseNet
	file ext:	h
	author:		杨文鸽
	
	purpose:	
*********************************************************************/
#ifndef _INCLUDE_BASENET_H_
#define _INCLUDE_BASENET_H_

#include "EventCenter.h"
#include "ServerBaseHead.h"

#include "MemBase.h"
#include "BaseNetHandle.h"
#include "CEvent.h"
#include "NetConnect.h"

#include "TCPThreadNet.h"
#include "ClientNet.h"

#include "TCPThreadNet.h"
#include "ClientEvent.h"

#include "IOCPServerNet.h"

#define DEFAULT_USE_THREAD_CLIENT_NET		1
#define USE_IOCP_SERVER_NET					1

class tClientConnect;
class tServerNet;

class ServerBase_Dll_Export ClientNetHandle : public BaseNetHandle
{
public:
	tClientConnect* GetClientNet();	

public:
	ClientNetHandle(BaseThread *pBaseThread);
	~ClientNetHandle();

	virtual bool InitNet();

	virtual bool StartNet(void)
	{
		return InitNet()
			&& TryReady();
	}

	virtual void Process(void);

	virtual bool OnAddConnect( tNetConnect *pConnect )
	{
		return true;
	}

	virtual void OnCloseConnect(tNetConnect *pConnect) {}

public:
	virtual tNetConnect* GetConnect(int netID);

	virtual bool IsOk(void);

	virtual bool TryReady(void);

public:
	AConnect			mClientNet;

};

class ServerBase_Dll_Export DefaultClientNetHandle : public ClientNetHandle
{
public:
	DefaultClientNetHandle(BaseThread *pThread, const char *szGsIp, int nPort)
		: ClientNetHandle(pThread)		
	{
		mIP = szGsIp;
		mPort = nPort;
	}

public:
	virtual const char* GetIp(void) const{ return mIP.c_str(); }
	virtual int		GetPort(void) const { return mPort; }

protected:
	AString mIP;
	int		mPort;
};


class ServerBase_Dll_Export DefaultServerNetHandle : public TCPThreadServerNet
{
public:
	DefaultServerNetHandle(BaseThread *pThread, const char *szGsIp, int nPort)				
	{
		mIP = szGsIp;
		mPort = nPort;
	}

public:
	virtual const char* GetIp(void) const{ return mIP.c_str(); }
	virtual int		GetPort(void) const { return mPort; }

protected:
	AString mIP;
	int		mPort;

};

class ServerBase_Dll_Export TryConnectHandle : public ClientNetHandle
{
public:
	TryConnectHandle(BaseThread *pBaseThread);

public:
	virtual bool StartNet(void);

};



class BaseThread;
// 间隔一段时间尝试连接指定服务

class ServerBase_Dll_Export TryConnentEvent : public Logic::CEvent
{
public:
	TryConnentEvent();
	bool _DoEvent();

	bool _OnTimeOver();

	void _ConnectOnce();

	void _OnFinish();

	void SetConnectInfo(tNetHandle *pServerTool, BaseThread *pBaseThread, int overTime, int tryCount );

	AutoNet GetNetTool();

protected:
	tNetHandle				*mNetServerTool;
	int								mOverTime;
	int								mTryCount;

public:
	BaseThread				*mBaseThread;


};
//-------------------------------------------------------------------------------------------------------------------

// 进行连接一次, 连接成功后, 执行成功的处理, 超时,进行失败处理
class ServerBase_Dll_Export NetConnectEvent : public Logic::CEvent
{
public:
	virtual void OnConnectSucceed() = 0;
	virtual void OnConnectOverTime() = 0;

	virtual void SetConnectNet(AutoNet net, int overTime)
	{
		mNet = net;
		mOverTimeSecond = overTime;
	}

public:
	virtual bool _DoEvent();

public:
	virtual bool Update(float);

	virtual bool _OnTimeOver(void);

protected:
	AutoNet		mNet;

	int			mOverTimeSecond;
};

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

class ServerBase_Dll_Export DefaultThreadReceiveConnect : public IOCPServerConnect
{
	typedef IOCPConnect base;

public:
	DefaultThreadReceiveConnect()
	{

	}
	~DefaultThreadReceiveConnect()
	{
		mWaitNetCheckEvent._free();
	}

	virtual void OnStartNet();

	virtual void SetRemove(bool bNeedRemove);

	virtual void OnWaitCheckNetOverTime();

	virtual void ResetWaitCheckEvent(){ if (mWaitNetCheckEvent) mWaitNetCheckEvent->DoEvent(); }

	virtual void OnDisconnect(){ if (mWaitNetCheckEvent) mWaitNetCheckEvent->Finish(); mWaitNetCheckEvent.setNull(); }

public:
	AutoEvent			mWaitNetCheckEvent;
};

//-------------------------------------------------------------------------------------------------------------------
// 多线程TCP网络
// 多线程接收TCP服务网络
//-------------------------------------------------------------------------------------------------------------------

class ServerBase_Dll_Export DefaultTcpThreadServerNet : public IOCPServerNet
{
	typedef IOCPServerNet base;

public:
	DefaultTcpThreadServerNet(BaseThread  *pThread)
		: mBaseThread(pThread)
	{

	}	

	DefaultTcpThreadServerNet(BaseThread  *pThread, const char *szIp, int port)
		: mBaseThread(pThread)		
	{
		BindIpPort(szIp, port);
	}	

	virtual bool StartNet(void);

	virtual void OnConnectCheckFail(HandConnect hConnect){ if (hConnect) hConnect->SetRemove(true); }
	
	virtual void RegisterEvent(AutoEventCenter hCenter);

public:
	virtual HandConnect CreateConnect(){ return MEM_NEW DefaultThreadReceiveConnect(); }


	virtual void BindIpPort(const char *szIp, int port)
	{
		mIp = szIp;
		mPort = port;
	}

	virtual void SetEventCenter(AutoEventCenter eventCenter){}

	virtual BaseThread* GetBaseThread(){ return mBaseThread; }

	Logic::tEventCenter* GetEventCenter( void ) const;

public:
	virtual const char* GetIp(void) const { return mIp.c_str(); }
	virtual int		GetPort(void) const { return mPort; }

	//virtual tNetConnect* GetConnect(int netID);

	virtual bool OnAddConnect(tNetConnect *pConnect);

	virtual int	GetWaitCheckNetOverTime(){ return 30; }

protected:
	EasyString	mIp;
	int			mPort;


	BaseThread *mBaseThread;

};


//-------------------------------------------------------------------------------------------------------------------
// 多线程TCP网络
// 多线程接收TCP客户端网络 TCPThreadClientNet
//-------------------------------------------------------------------------------------------------------------------
#if DEFAULT_USE_THREAD_CLIENT_NET
	typedef IOCPClientNet _TcpClientNet;
#else
class _TcpClientNet : public EventClientNet 
{
public:
	virtual const char* GetIp(void) const { return mIp; }
	virtual int		GetPort(void) const { return mPort; }

	virtual bool StartNet(void) 
	{
		RegisterEvent(GetEventCenter()->GetSelf());
		return TryReady();
	}

	virtual void OnConnectFail(){}

public:
	AString mIp;
	int		mPort;
};
#endif
class ServerBase_Dll_Export DefaultTCPThreadClientNet : public _TcpClientNet
{
	typedef  _TcpClientNet parent;
public:
	DefaultTCPThreadClientNet(BaseThread *pBaseThread)
		: mBaseThread(pBaseThread)
	{

	}

	DefaultTCPThreadClientNet(BaseThread *pBaseThread, const char *szIp, int port)
		: mBaseThread(pBaseThread)
	{
		mIp = szIp;
		mPort = port;
	}

	virtual int GetCheckNetOnceTime(){ return 10; }

	virtual void RegisterEvent(AutoEventCenter hCenter);

public:
	virtual void OnConnectCheckFail()
	{
		//TryReady();
	}

	virtual Logic::tEventCenter* GetEventCenter() const;

	virtual void OnConnected();
	virtual void OnConnectFail();

	virtual BaseThread* GetBaseThread(){ return mBaseThread; }

	virtual bool StartNet();
	virtual void StopNet(void);

	virtual bool TryReady();

protected:
	BaseThread		*mBaseThread;

	AutoEvent		mNetCheckEvent;
};
//-------------------------------------------------------------------------------------------------------------------

class S_QuestCheckNet : public Logic::tClientEvent
{
public:
	S_QuestCheckNet()
	{
		setLog(false);
	}

public:
	virtual bool _DoEvent()
	{
		DefaultThreadReceiveConnect *p = dynamic_cast<DefaultThreadReceiveConnect*>(mNetConnect.getPtr());
		AssertEx(p!=NULL, "当前连接不是 DefaultThreadReceiveConnect ");

		if (p->mWaitNetCheckEvent)
			p->mWaitNetCheckEvent->DoEvent();
		else
		{
			Log("Error: 等待验证事件未开始创建");
		}

		Finish();
		return true;
	}

	virtual void SetRespData(AutoEvent &hResp)
	{
		hResp->set("INFO", "Connect normal");
	};
};
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------


class C_QuestCheckNet : public Logic::tServerEvent
{
	friend class DefaultTCPThreadClientNet;
public:
	C_QuestCheckNet()
		: mbReceiveResponse(false) 
		, mbConnected(false)
	{
		setLog(false);
	}

	virtual bool _NeedFinishWhenResponsed() const { return false; }

public:
	virtual bool DoEvent( bool bImmediately = true)
	{
		if (!bImmediately)
			return Logic::tServerEvent::DoEvent(bImmediately);

		mbReceiveResponse = false;

		if (mbConnected)
		{
			DefaultTCPThreadClientNet *pNet = dynamic_cast<DefaultTCPThreadClientNet*>(mClientNet.getPtr());
			AssertEx(pNet!=NULL, "应该是 DefaultTCPThreadClientNet");
			WaitTime((float)pNet->GetCheckNetOnceTime());
			Logic::tServerEvent::DoEvent(true);	
		}
		else
			Logic::CEvent::WaitTime(10);

		return true;
	}

	virtual void _OnResp(AutoEvent &respEvent)
	{
		mbReceiveResponse = true;
	}

	virtual bool _OnEvent(AutoEvent &hEvent)
	{
		bool re = Logic::tServerEvent::_OnEvent(hEvent);
		setFinished(false);
		//DoEvent();
		return re;
	}

	virtual bool _OnTimeOver()
	{
		if (mbReceiveResponse)
			DoEvent();
		else
		{
			setLog(true);

			Log("当前连接异常, 网络检查超时");
			DefaultTCPThreadClientNet  *pNet = dynamic_cast<DefaultTCPThreadClientNet*>(mClientNet.getPtr());
			AssertEx(pNet!=NULL, "不是 DefaultTCPThreadClientNet");
			Finish();
			pNet->OnConnectCheckFail();
		}

		return true;
	}

	virtual bool Send(int nType = 0, int nTarget = 0 )
	{
		return mClientNet->GetConnect(0)->SendEvent(this);
	}

protected:
	bool mbConnected;
	bool mbReceiveResponse;
	AutoNet		mClientNet;
};

//-------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------

class TM_WaitNetCheckEvent : public Logic::CEvent
{
	friend class DefaultThreadReceiveConnect;
public:
	TM_WaitNetCheckEvent()
	{
		//mPlayer = AutoPlayer();
	}

public:
	virtual bool _DoEvent()
	{
		DefaultThreadReceiveConnect *p = dynamic_cast<DefaultThreadReceiveConnect*> (mNetConnect.getPtr());
		DefaultTcpThreadServerNet *pNet = dynamic_cast<DefaultTcpThreadServerNet*>(p->GetNetHandle());
		AssertEx(pNet!=NULL, "应该是 DefaultTcpThreadServerNet");
		WaitTime((float)pNet->GetWaitCheckNetOverTime());
		return true;
	}

	virtual void SetRespData(AutoEvent &hResp)
	{
	}

	virtual bool _OnTimeOver()
	{
		DefaultThreadReceiveConnect *p = dynamic_cast<DefaultThreadReceiveConnect*> (mNetConnect.getPtr());
		if (p!=NULL)
			p->OnWaitCheckNetOverTime();
		else
			Finish();

		return true;
	}

public:
	HandConnect  mNetConnect;
};
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#endif //_INCLUDE_BASENET_H_