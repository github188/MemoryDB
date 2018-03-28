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

	// NOTE: �˹���ִ�������ߣ���Ҫ����ͬ��
    bool UpdateAnnouncementTable(AutoData annoData);


	AString& GetServerName(){ return mGameServerName; }
	int GetServerID(){ return mServerID; }

	void UpdateServerInfo(AutoData serverInfo);

	// ��Դ������Ϣ����
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
	AutoNet					mLGServerNet;				// �ͻ��˵�½��������
    AutoNet                 mPlayerWebNet;
	UDPLog					*mGlog;
	int						mGlogRegionID;
	int						mGlogServerID;
	UInt64					mResForeNetKey;
    GSKEY                   mGateNetKey;

public:
	HandDB					mAccountDB;					// �ʺ�DB���������Ӷ�
	AutoNet					mLGNodeNet;					// ��GS֮�����ӵ�����
	AString					mGameServerName;
	AutoEvent				mWaitSyncOnlineEvent;

public:
	AutoData				mServerInfoData;
	int						mServerID;
	UInt64					mLeastGSKey;
	AString					mLoginDNS;

	// ??? SDK ��Ҫ���¹滮���, ���ѵ�, ����ʹ��WEB֧��
	SDK							*mSDK;					

public:
	// ���²���
	SERVER_SHOW_STATE		mServerShowState;		// �������ڿͻ�����ʾ��״̬, ��̨�趨
	AString						mServerVersion;
	AString						mResourcesMD5Android;
	AString						mResourcesMD5IOS;
	//??? ��Դ���������ߺ�, ������б����Ƴ�, ���Ƴ�ʱ�����ж������Ƿ����, ����ֱ��ʹ��KEY, ���ܴ�ʱ�Ѿ������µ�����
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

	// ���ڵ㣬�ڵ�Ͽ��󣬲��ٳ������������ڵ�
	virtual int GetTryConnectCount(void) override 
	{ 
		if (mServerNetKey==mMainNodeKey)
			return 0; 
		return MeshedNodeNet::GetTryConnectCount();
	}

public:
	// ���������ӶԷ�Node���ɹ���, ֪ͨ�Է��¼�����֮ǰ����Я����������
	// ֪ͨ��ǰ��ʲô���͵ķ�����, ����������
	virtual void OnRequestNode(NetNodeConnectData *nodeData, AutoEvent &requestEvt) override;

	// ��Ϊ�������ߣ����յ����������Ӻ�������¼�, ��Ҫ���������ߵ�ǰ�����ڵ�Ⱥ����Ϣ
	// ʶ����������ͣ��������ͻظ������Ϣ
	virtual void OnResponseNode(GSKEY otherPartyGSKey, AutoEvent &requstEvt, AutoEvent &respEvt) override;
	 
	// �Է��ظ�����
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
// ���Ÿ��ͻ������ӵ�½�ķ�������
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
				ERROR_LOG("������ӳ���[%d] ��δ�յ�������Ϣ, ��ǰ�Ͽ���½����", LOGIN_CONNECT_OVERTIME);
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
//�ʺ�DB���Ӷ�
class AccountDBClient : public DBNodeManager
{
public:
	virtual void Close() override { mWaitReconnectEvent._free(); DBNodeManager::Close(); }
	virtual void InitNodeConnectNet(tNetHandle *pDBConnectNet);
	virtual bool NeedSaveUpdateResponse() const override { return false; }

	virtual int GetDBNetSaftCode() const { return ACCOUNT_NET_SAFECODE; }

	void OnConnected(DBOperate *op, bool bConnectSucceed);

	// ���Ӻ�֪ͨLogin���ظ�����״̬
	void OnDBResponseServerState(DBOperate *op, bool bSu);

	void OnDisconnected(DBOperate *op, bool)
	{
		NOTE_LOG("�ʺ�DB�������Ͽ�����, 10������³�������");
		StartWaitReconnect();
	}

	bool OnStartReconnect(tEvent *p)
	{
		NOTE_LOG("��ʼ��������AG");
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