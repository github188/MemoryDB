
/********************************************************************
	created:	2015/05/24
	created:	24:5:2015   19:50
	filename: 	G:\HomeGame\Code\BaseCommon\NetDll\Net\UDPbroadcast.h
	file path:	G:\HomeGame\Code\BaseCommon\NetDll\Net
	file base:	UDPbroadcast
	file ext:	h
	author:		YangWenge
	
	purpose:	UDP直播中转网络
				独立工作线程
				只中转指定地址的数据源，且同时只存在一个可中转的源地址
*********************************************************************/
#ifndef _INCLUDE_UDPBROADCAST_H_
#define _INCLUDE_UDPBROADCAST_H_

#include "UDPThreadNet.h"
#include "NetHead.h"
#include "EventCenter.h"
#include "ServerEvent.h"

#include "Easyhash.h"

using namespace Logic;
//-------------------------------------------------------------------------
// 1 启动通知客户端要求向指定验证地址发送验证KEY，目的获取客户端的公网UDP地址
// 2 验证通过后，将广播服务地址发送给客户端，客户端继续发三次UDP消息，以便能接收到广播推送
//-------------------------------------------------------------------------
class Net_Export UDPBroadcast : public  UDPThreadNet
{
public:
	virtual void OnAppendTarget(UInt64 targetAddr, HandConnect tcpConnect, AutoEvent notifyEvent) = 0;

	AutoEvent StartNotifyClient(HandConnect tcpConnect);

	void SetSourceAddr(UInt64 sourceAddr);
	UInt64 GetSourceAddr();

public:
	// NOTE: 参数 revData 只能即时使用, 执行后, 即可能被切换到接收线程使用
	virtual void OnReceiveMsg(UInt64 scrAddr, DataStream *revData, int revSize);

	static void RegisterNetEvent(AutoEventCenter center);

	virtual void Process()
	{
		UDPThreadNet::Process();
		//mEventCenter->ProcessEvent();
	}	

	void _AppendConnect(UInt64 newAddr);

	void _RemoveConnect(UInt64 targetAdd);

	void AppendWaitEvent(UInt64 checkKey, AutoEvent waitEvent)
	{
		mWaitEventList[checkKey] = waitEvent;
	}

protected:
	typedef EasyHash<UInt64, AutoEvent>		WaitEventList;

	WaitEventList	mWaitEventList;

public:
	UDPBroadcast();

	virtual ~UDPBroadcast();

public:
	virtual bool StartNet( const char *szIP, int lowPort, int highPort );
	
	virtual void Close()
	{
		mpBroadcastUDPReceiveThread->_ForceClose(12);
		//mpBroadcastUDPReceiveThread->Close();
		UDPThreadNet::Close();
	}

	AString getIpPort(int &checkPort, int &broadcastServerPort)
	{ 
		mBroadcastUDP.getIpPort(broadcastServerPort); 

		return UDPThreadNet::getIpPort(checkPort);
	}

public:
	// 主线消息处理
	//AutoEventCenter		mEventCenter;
	AString				mServerIP;
	int					mCheckPort;
	int					mBroadcastPort;

protected:
	// NOTE: 线程中执行
	void _ProcessNet();

protected:
	// 线程内使用
	UDPNet			mBroadcastUDP;
	DataBuffer		mBroadcastReceiveBuffer;
	UInt64			mRevBroadcastAddr;

	PoolList<UInt64>	mTargetAddrList;
	
	CsLock			mListLock;
	bool			mbHaveWaitConnect;

	UInt64			mDataSourceAddr;

	UInt64			mSetSourceAddr;
	bool			mbHaveSourceAddrChanged;

	CsLock			mSourceAddLock;

	struct WaitConnect 
	{
		bool	mbAppend;
		UInt64	mTargetAddr;

		WaitConnect()
			: mbAppend(true)
			, mTargetAddr(0)
		{

		}
	};

	PoolList<WaitConnect>	mWaitConnectList;

	class UDPReceiveThread;
	UDPReceiveThread	*mpBroadcastUDPReceiveThread;

	class UDPReceiveThread : public WorkThread
	{
	public:
		UDPReceiveThread(UDPBroadcast *pNet)
			: mpNet(pNet)
		{

		}

		virtual void backWorkThread(void)
		{
			while (IsActive())
			{
				mpNet->_ProcessNet();
			}
		}

		virtual void onBeforeClose(void) 
		{
			_ForceClose(10);		
		};

	public:
		UDPBroadcast *mpNet;
	};
};
//-------------------------------------------------------------------------
//class BoroadcastEventCenter : public EventCenter
//{
//public:
//	BoroadcastEventCenter(UDPBroadcast *pNet)
//		: mpNet(pNet)
//	{
//
//	}
//
//	UDPBroadcast	*mpNet;
//};

//-------------------------------------------------------------------------
class CS_NotityConnectUDP : public tBaseNetEvent
{
public:
	virtual bool _DoEvent();

	bool OnUDPResponce(UInt64 targetAdd, UInt64 checkCode);

	virtual bool _OnTimeOver()
	{
		return true;
	}

public:
	UInt64 mCheckCode;

public:
	Hand<UDPBroadcast>	mUDP;
};
//-------------------------------------------------------------------------


#endif