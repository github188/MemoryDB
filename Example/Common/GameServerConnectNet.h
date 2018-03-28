/********************************************************************
	created:	2012/12/26
	created:	26:12:2012   18:26
	filename: 	C:\NewGame\Server\Server\MainServer\GameServerConnect.h
	file path:	C:\NewGame\Server\Server\MainServer
	file base:	GameServerConnect
	file ext:	h
	author:		Yang Wenge
	
	purpose:	GS服务器之间的连接, 理论上此连接不会断开, 且相互通用
				一但对方连接成功后, 彼此都会创建此对象, 
				特点: 1 连接是多人共用方案, 就是GS与GS之前只创建一条此连接
						因此, 要求多人共同使用一条连接
					2 消息包内区别发送方与接收方的ID
*********************************************************************/
#ifndef _INCLUDE_GAMESERVERCONNECTNET_H_
#define _INCLUDE_GAMESERVERCONNECTNET_H_

#include "NetHandle.h"
#include "NetConnect.h"
#include "EasyList.h"

#include "BaseNet.h"

#include "ServerIPInfo.h"
// GS 网络服务器内的接受创建的连接
//class GSConnect : public ServerConnect
//{
//
//};

//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------
template<typename T>
class GSSeverEventFactory : public tServerEventFactory
{
public:
	GSSeverEventFactory()
	{
		AString respName = RESP_EVENT_NAME;
		respName += "_GR";
		AppendTriggerEventName(respName.c_str());
		mRespEventID_GS = MAKE_INDEX_ID(respName.c_str());
	}


public:
	virtual bool _HasRelation(AutoEvent &hEvent)
	{
		return hEvent->GetNameIndex()==mRespEventID_GS;
	}

	virtual AutoEvent NewEvent()
	{
		AutoEvent hE = MEM_NEW T();
		return hE;
	}

protected:
	int		mRespEventID_GS;
};
//-------------------------------------------------------------------------------------------------------

// GS 连接 管理, (网络服务器)
class GSNetHandle : public ServerNetHandle
{
public:
	GSNetHandle(BaseThread *pThread, const char *szGsIp, int nPort)
		: ServerNetHandle(pThread)		
	{
		mIP = szGsIp;
		mPort = nPort;
	}

public:
	virtual bool OnAddConnect( tConnect *pConnect );
	virtual void OnCloseConnect(tConnect *pConnect);

	virtual void ExecutePacket( Packet *pPacket, tConnect *pConnect );

public:
	virtual void RegisterEvent( void );


	virtual const char* GetIp(void) const{ return mIP.c_str(); }
	virtual int		GetPort(void) const { return mPort; }

protected:
	AString mIP;
	int		mPort;
};
//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------

class GSClinetNet : public ClientNetHandle
{


public:
	GSClinetNet(BaseThread *pThread)
		: ClientNetHandle(pThread)		
		, mWaitClientPort(0)
		, mGsPort(0)
	{

	}

	static void RegisterGSEvent(Logic::tEventCenter *center);

public:
	void SetGSKey(GSKEY gsIPKey)
	{
		mIPPortGSkey = gsIPKey;
		mGsIp = ServerIPInfo::Num2IP(gsIPKey, mWaitClientPort, mGsPort );
	}

	
	void ClearUp(void)
	{

	}

public:
	virtual const char* GetIp(void) const
	{
		return mGsIp.c_str();
	}
	virtual int		GetPort(void) const
	{
		return mGsPort;
	}

public:
	void OnNotifyRemove(void){}
	//void AppendUsePlayer(AutoPlayer  player)
	//{
	//	mUsePlayerMap.insert(player);
	//}

	//void RemoveUsePlayer(AutoPlayer player)
	//{
	//	for (EasyList<AutoPlayer>::iterator it = mUsePlayerMap.begin(); it!=mUsePlayerMap.end(); )
	//	{
	//		if ( *it == player )
	//			it = mUsePlayerMap.erase(it);
	//		else
	//			++it;
	//	}
	//}

public:
	void ExecutePacket( Packet *pPacket, tConnect *pConnect );


public:
	AString mGsIp;
	int		mGsPort;
	int		mWaitClientPort;

	GSKEY						mIPPortGSkey;

	//EasyList<AutoPlayer>		mUsePlayerMap;		// 使用到此连接的玩家列表, 目的用与当连接出现断开后, 进行处理
};
//-------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------

typedef Hand<GSClinetNet>		HandGSConnectNet;
//-------------------------------------------------------------------------------------------------------

#endif