#ifndef _INCLUDE_LOGINTHREAD_H_
#define _INCLUDE_LOGINTHREAD_H_

#include "BaseThread.h"
#include "MeshedNodeNet.h"
#include "CommonDefine.h"
#include "DBNodeManager.h"
#include "WebGameNet.h"

#define LOGIN_CONFIG_TABLE	"./RunConfig/LoginConfig.csv"

#define LOGIN_SERVER_VERSION_TABLE "./RunConfig/ServerLoginVersion.csv"

class UDPLog;
class ServerUDPNet;
class LoginServerNet;
class SDK;
//-------------------------------------------------------------------------*/
class LoginThread : public BaseThread
{
	friend class GL_RequestLGOnlineInfo_R;
public:
	virtual void Process(void*);
	virtual void OnStart(void*);
	virtual void OnStop(void*) {}

	virtual bool NotifyThreadClose() override;

#if DEVELOP_MODE
	virtual int OnceTime(void) const{ return 20; }
#else
	//virtual void OnOnceLoopEnd() override {}
	virtual int OnceTime(void) const{ return 1; }
#endif

	virtual tDBManager* GetDBWork() override { return mAccountDB.getPtr(); }

public:
	GSKEY GetLeastOnlinePlayerGS();
	void UpdateLeastOnlineGS();

	// NOTE: 此功能执行在主线，需要加锁同步
    bool UpdateAnnouncementTable(AutoData annoData);


	AString& GetServerName(){ return mGameServerName; }
	int GetServerID(){ return mServerID; }

	void UpdateServerInfo(AutoData serverInfo);

	// 资源更新信息部分
	void AppendResourcesServer( GSKEY resNetKey, NetNodeConnectData *pNetNodeData);

	void UpdateResourceMD5(AString androidMD5, AString iosResMd5)
	{
		mResourcesMD5Android = androidMD5;
		mResourcesMD5IOS = iosResMd5;
	}

	bool UpdateServerVersion(const AString &versionString);

	bool UpdateServerStateData(int state);

	void UpdateTileInfo();

	bool CheckUpdateResource(const AString &clientMD5, bool bIOS)
	{
		if (bIOS)
			return clientMD5==mResourcesMD5IOS;
		else
			return clientMD5==mResourcesMD5Android;
	}

	bool GetRandResourcesServer( AString &resIP, int &nPort );

	void RefreshCheckResourceServer();

	HandConnect GetLeastGSConnect();
	HandConnect _GetLeastGSConnect();

	HandConnect GetAnyGSConnect();

	int GetGSCount();
	int GetOnlineCount();

	void NotifyGSCountToAG();
	void NotifyOnlineCountToAG();

	AutoData GetAnnoDataBySDK(int sdkType);

    void OnPlayerConnected(tNetConnect *pPlayerConnect);
    void OnPlayerLeaveGS(GSKEY netKey);

    void RequestStartGame(HandConnect webPlayer);

public:
	LoginThread();
	~LoginThread();

public:
	AutoNet					mLGServerNet;				// 客户端登陆连接网络
    AutoNet                 mPlayerWebNet;
	UDPLog					*mGlog;
	int						mGlogRegionID;
	int						mGlogServerID;
	UInt64					mResForeNetKey;
    GSKEY                   mGateNetKey;

public:
	HandDB					mAccountDB;					// 帐号DB服务器连接端
	AutoNet					mLGNodeNet;					// 与GS之间连接的网络
	AString					mGameServerName;
	AutoEvent				mWaitSyncOnlineEvent;

public:
	AutoData				mServerInfoData;
	int						mServerID;
	UInt64					mLeastGSKey;
	AString					mLoginDNS;

	// ??? SDK 需要重新规划设计, 付费等, 考虑使用WEB支持
	SDK							*mSDK;					

public:
	// 更新部分
	SERVER_SHOW_STATE		mServerShowState;		// 服务区在客户端显示的状态, 后台设定
	AString						mServerVersion;
	AString						mResourcesMD5Android;
	AString						mResourcesMD5IOS;
	//??? 资源服务器断线后, 必须从列表中移除, 且移除时遍历判断数据是否相等, 不可直接使用KEY, 可能此时已经是最新的连接
	EasyMap<GSKEY, Hand<NetNodeConnectData>>	mResourcesServerList;  

	EasyHash<int, AutoData>		mAnnoDataList;


};
//-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*/
class LGNodeNet : public MeshedNodeNet
{
public:
	virtual void RegisterNodeEvent(AutoEventCenter serverCenter, AutoEventCenter connectCenter) override;

public:
	LGNodeNet(LoginThread *pThread, const char *szServerIp, int nServerPort, int safeCheckCode)
		: MeshedNodeNet(szServerIp, nServerPort, safeCheckCode, 1)
		, mThread(pThread)
	{

	}

	virtual const char* NodeInfo(){ return "-LS"; }

	virtual void DumpAllConnect() override;

	// 主节点，节点断开后，不再尝试连接其他节点
	virtual int GetTryConnectCount(void) override 
	{ 
		if (mServerNetKey==mMainNodeKey)
			return 0; 
		return MeshedNodeNet::GetTryConnectCount();
	}

public:
	// 当主动连接对方Node，成功后, 通知对方事件发送之前，可携带其他数据
	// 通知当前是什么类型的服务器, 及在线人数
	virtual void OnRequestNode(NetNodeConnectData *nodeData, AutoEvent &requestEvt) override;

	// 作为被连接者，接收到连接者连接后的请求事件, 主要告诉连接者当前整个节点群的信息
	// 识别服务器类型，根据类型回复相关信息
	virtual void OnResponseNode(GSKEY otherPartyGSKey, AutoEvent &requstEvt, AutoEvent &respEvt) override;
	 
	// 对方回复数据
	virtual void OnReceiveResponse(NetNodeConnectData *nodeData, AutoEvent &requestEvt, AutoEvent &respEvt) override;

public:
	virtual void OnAppendNetNode( NetNodeConnectData *nodeData ) override;

	virtual void _OnConnectNodeFault( AutoEvent waitEvent ) override
	{
		MeshedNodeNet::_OnConnectNodeFault(waitEvent);

		//mThread->UpdateLeastOnlineGS();
		mThread->RefreshCheckResourceServer();
	}

	virtual void RemoveNode(NetNodeConnectData *pNode) override;

public:
	LoginThread		*mThread;
};

//-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*/
// 开放给客户端连接登陆的服务网络
class LGServerNet : public IOCPServerNet
{
public:
	virtual int GetSafeCode() override
	{
		return NET_SAFT_CHECK_CODE;
	}

	HandConnect CreateConnect();

	virtual tEventCenter* GetEventCenter() const override
	{
		return mpLoginThread->GetMainEventCenter().getPtr();
	}

	void BindEventCenter();

public:
	LGServerNet(LoginThread *pThread)
		: mpLoginThread(pThread){}

	~LGServerNet()
	{
		if (mSendHeartEvent)
			mSendHeartEvent._free();
	}

public:
	LoginThread			*mpLoginThread;

	AutoEvent	mSendHeartEvent;
};
//-------------------------------------------------------------------------*/
#define LOGIN_CONNECT_OVERTIME		90

class LoginClientConnect : public IOCPServerConnect
{
public:
	LoginClientConnect()
	{
		mLastReceiveTime = TimeManager::Now();
	}

	virtual void OnReceivePacket(Packet *pPacket) override
	{
		mLastReceiveTime = TimeManager::Now();

		pPacket->Execute(this);
	}

	virtual bool Process() override
	{
		if (TimeManager::Now()-mLastReceiveTime>LOGIN_CONNECT_OVERTIME)
		{
			if (!IsRemove())
			{
#if !RELEASE_RUN_VER
				ERROR_LOG("玩家连接超过[%d] 秒未收到网络消息, 当前断开登陆连接", LOGIN_CONNECT_OVERTIME);
#endif
				SetRemove(true);
			}			
		}
		return IOCPServerConnect::Process();
	}

public:
	UInt64 mLastReceiveTime;
};
//-------------------------------------------------------------------------*/
//帐号DB连接端
class AccountDBClient : public DBNodeManager
{
public:
	virtual void Close() override { mWaitReconnectEvent._free(); DBNodeManager::Close(); }
	virtual void InitNodeConnectNet(tNetHandle *pDBConnectNet);
	virtual bool NeedSaveUpdateResponse() const override { return false; }

	virtual int GetDBNetSaftCode() const { return ACCOUNT_NET_SAFECODE; }

	void OnConnected(DBOperate *op, bool bConnectSucceed);

	// 连接后通知Login并回复服务状态
	void OnDBResponseServerState(DBOperate *op, bool bSu);

	void OnDisconnected(DBOperate *op, bool)
	{
		NOTE_LOG("帐号DB服务器断开连接, 10秒后重新尝试连接");
		StartWaitReconnect();
	}

	bool OnStartReconnect(tEvent *p)
	{
		NOTE_LOG("开始重新连接AG");
		Reconnect();
		return true;
	}

	void StartWaitReconnect();

public:
	AccountDBClient(LoginThread *pThread)
		: mpLoginThread(pThread)
	{
		
	}

	LoginThread		*mpLoginThread;
	AutoEvent		mWaitReconnectEvent;
};

//-------------------------------------------------------------------------*/

#endif //_INCLUDE_LOGINTHREAD_H_