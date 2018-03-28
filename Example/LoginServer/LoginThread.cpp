#include "LoginThread.h"

#include "MeshedNetNodeData.h"
#include "TableManager.h"
//#include "CommonDefine.h"

#include "LG_NodeNetEvent.h"
#include "ServerIPInfo.h"
#include "LoginEvent.h"
#include "AccountNetEvent.h"
#include "CSVTableLoader.h"
#include "SDK.h"
#include "NetIndexPacket.h"
#include <windows.h>
#include "BaseServer.h"
#include "FileDataStream.h"

#include "WebGameNet.h"
#include "WebPlayer.h"
#include "LGWebNet.h"
//-------------------------------------------------------------------------*/

void LGNodeNet::OnAppendNetNode(NetNodeConnectData *nodeData)
{
	//if (nodeData->mNodeConnect)
	//{
	//	// �����������¼�
	//	nodeData->mRequestCheckNetEvent = nodeData->mNodeConnect->StartEvent("GG_RequestOnlinePlayerCount");
	//	nodeData->mRequestCheckNetEvent->DoEvent();
	//}
}

void LGNodeNet::RemoveNode(NetNodeConnectData *pNode)
{
	MeshedNodeNet::RemoveNode(pNode);
	mThread->UpdateLeastOnlineGS();

	// ֪ͨAG, ��ǰGS����
	mThread->NotifyGSCountToAG();
}

void LGNodeNet::RegisterNodeEvent( AutoEventCenter serverCenter, AutoEventCenter connectCenter )
{
	serverCenter->RegisterEvent("GL_NotifyOnlineInfo", MEM_NEW EventFactory<GL_NotifyOnlineInfo_R>());

	serverCenter->RegisterEvent("GL_UploadLoginInfoTable", MEM_NEW EventFactory<GL_UploadLoginInfoTable_R>());
	serverCenter->RegisterEvent("GL_RequestLGOnlineInfo", MEM_NEW EventFactory<GL_RequestLGOnlineInfo_R>());
	serverCenter->RegisterEvent("GL_RequestLGUpdateVersion", MEM_NEW EventFactory<GL_RequestLGUpdateVersion_R>());
	serverCenter->RegisterEvent("GL_RequestLGUpdateServerState", MEM_NEW EventFactory<GL_RequestLGUpdateServerState_R>());
	
	// ������Դ��Ϣ
	serverCenter->RegisterEvent("RG_RequestAllUpdateResources", MEM_NEW EventFactory<RG_RequestAllUpdateResources_R>());
	serverCenter->RegisterEvent("RS_ReqeustCloseServer", MEM_NEW EventFactory<RS_ReqeustCloseServer_LR>());

	// ����GS׼��ӭ�ӿͻ��˽���
	connectCenter->RegisterEvent("LG_RequestReadyLogin", MEM_NEW EventFactory<LG_RequestReadyLogin>());
	connectCenter->RegisterEvent("LG_RequestPayEvent", MEM_NEW EventFactory<LG_RequestPayEvent>());

	serverCenter->RegisterEvent("GL_GetBetaTestDiamond", MEM_NEW EventFactory<GL_GetBetaTestDiamond_R>());
	serverCenter->RegisterEvent("GL_UseCDKey", MEM_NEW EventFactory<GL_UseCDKey_R>());
	serverCenter->RegisterEvent("GL_CreateCDKey", MEM_NEW EventFactory<GL_CreateCDKey_R>());
}

void LGNodeNet::DumpAllConnect()
{
	MeshedNodeNet::DumpAllConnect();

	int num[eServerTypeMax] = { 0, 0, 0, 1 };
	int disconnectNum = 0;

	for (size_t i=0; i<mRequestConnectList.size(); ++i)
	{
		Hand<NetNodeConnectData> e = mRequestConnectList.get(i);
		if (e)
		{
			int type = e->get(SERVER_TYPE_KEY);
			if (type>=0 && type<eServerTypeMax)
			{
				if (e->IsConnected())
					++num[type];
				else
					++disconnectNum;
			}
		}
	}
	bool bOk= true;
	for (int i=eUnknow+1; i<eServerTypeMax; ++i)
	{
		if (num[i]<=0)
			bOk = false;
	}
	TableTool::yellow();
	printf("Run state [%s] [GS:%d] [LS:%d] [RS:%d] [Unknow>%d] [Disconnect>%d]\r\n"
		, bOk?"����":"X�쳣"
		, num[eGameServer]
		, num[eLoginServer]
	, num[eResourceServer]
	, num[eUnknow]
	, disconnectNum
		);
	TableTool::white();
}

void LGNodeNet::OnRequestNode( NetNodeConnectData *nodeData, AutoEvent &requestEvt )
{
	requestEvt[SERVER_TYPE_KEY] = eLoginServer;	
}

void LGNodeNet::OnResponseNode( GSKEY otherPartyGSKey, AutoEvent &requstEvt, AutoEvent &respEvt )
{
	respEvt[SERVER_TYPE_KEY] = eLoginServer;
	respEvt["INFO"] = NodeInfo();
	respEvt["SERVER_ID"] = mThread->GetServerID();
	respEvt["SERVER_NAME"] = mThread->GetServerName();
    respEvt["GATE_NET_KEY"] = mThread->mGateNetKey;
}

void LGNodeNet::OnReceiveResponse( NetNodeConnectData *nodeData, AutoEvent &requestEvt, AutoEvent &respEvt )
{
	nodeData->set(SERVER_TYPE_KEY, (int)respEvt[SERVER_TYPE_KEY]);
	nodeData->set("_INFO_", respEvt["INFO"].string());
	switch ((int)respEvt[SERVER_TYPE_KEY])
	{
	case eGameServer:
		{
			nodeData->set("LOGIN_GSKEY", respEvt["LOGIN_GSKEY"].string());
			nodeData->set("ONLINE_COUNT", (int)respEvt["ONLINE_COUNT"]);
			nodeData->set("MEM_NOT_ENOUGH", (bool)respEvt["MEM_NOT_ENOUGH"]);
			nodeData->set("VER", respEvt["VER"].string());
			mThread->UpdateLeastOnlineGS();
			mThread->NotifyGSCountToAG();			
		}
		break;

	case eResourceServer:
		{
			// �������Դ���·�����	
			GSKEY key = respEvt->get("RESOURCES_NET_KEY");
			if (key>0)
			{
				mThread->UpdateResourceMD5(respEvt["RES_MD5"], respEvt["RES_MD5_IOS"]);			

				nodeData->set("RESOURCES_NET_KEY", key);
				mThread->AppendResourcesServer(key, nodeData);
			}

			AutoData annoData = (DataStream*)respEvt["ANNO_DATA"];
			if (annoData)
				mThread->UpdateAnnouncementTable(annoData);
			mThread->mResForeNetKey = respEvt["FORE_NET_KEY"];
			nodeData->set("VER", respEvt["VER"].string());
		}
		break;

	case eLoginServer:
		{
            nodeData->set("GATE_NET_KEY", (GSKEY)respEvt["GATE_NET_KEY"]);
		}
		break;
	}
	DumpAllConnect();
}
//-------------------------------------------------------------------------*/
#define  SYNC_ONLINE_TOAS_TIME			(10)			// 10��ͬ��һ������������AG

class TM_WaitSyncOnlineToAG : public CEvent
{
public:
	virtual bool _OnTimeOver() override
	{
		mThread->NotifyOnlineCountToAG();
		WaitTime(SYNC_ONLINE_TOAS_TIME);
		return true;
	}
	virtual bool _DoEvent() override
	{
		WaitTime(SYNC_ONLINE_TOAS_TIME);
		return true;
	}

public:
	TM_WaitSyncOnlineToAG()
		:mThread(NULL)
	{

	}

	virtual void InitData() override
	{
		CEvent::InitData();
		mThread = NULL;
	}

public:
	LoginThread	*mThread;
};

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------*/
void LoginThread::OnStart(void*)
{
	AutoEventCenter center = MEM_NEW EventCenter();
	InitEventCenter(center);

	mSDK->InitThread();

	center->RegisterEvent("TM_WaitSyncOnlineToAG", MEM_NEW EventFactory<TM_WaitSyncOnlineToAG>());
	//mWaitSyncOnlineEvent = center->StartEvent("TM_WaitSyncOnlineToAG");
	//Hand<TM_WaitSyncOnlineToAG> waitEvt = mWaitSyncOnlineEvent;
	//waitEvt->mThread = this;
	//mWaitSyncOnlineEvent->DoEvent(false);

	AutoTable versionTable = TableManager::getSingleton().CreateNewTable(LOGIN_SERVER_VERSION_TABLE);
	bool bLoad = versionTable->LoadCSV(LOGIN_SERVER_VERSION_TABLE);
	AssertNote(bLoad, "�������� [%s]", LOGIN_SERVER_VERSION_TABLE);
	mServerVersion = versionTable->GetValue("VER", "VERSION");
	mServerShowState = (SERVER_SHOW_STATE)(int)versionTable->GetValue("VER", "STATE");
	AssertNote(mServerVersion.length()>0, "�������ð汾�� LoginIp >VERSION");

	AutoTable t = TableManager::getSingleton().CreateNewTable(RUNCONFIG);
	if (!t->LoadCSV(LOGIN_CONFIG_TABLE))
	{
		ERROR_LOG("��������ʧ�� [%s]", LOGIN_CONFIG_TABLE);
		Close();
		return;
	}

	AString szIp = TableManager::getSingleton().GetData(RUNCONFIG, "NodeNet", "IP").string();
	int nPort = TableManager::getSingleton().GetData(RUNCONFIG, "NodeNet", "PORT");
	int nodeKey = TableManager::getSingleton().GetData(RUNCONFIG, "MainGS", "NODE_KEY");
	AssertNote(nodeKey!=0, "�������� [%s] MainGS >NODE_KEY, ����Ϊ0", RUNCONFIG);

	mLGNodeNet = MEM_NEW LGNodeNet(this, szIp.c_str(), nPort, nodeKey);
	Hand<LGNodeNet> net = mLGNodeNet;
	net->SetEventCenter(MEM_NEW EventCenter(), MEM_NEW EventCenter());	
	mLGNodeNet->BindEventCenter();

	ARecord mainGS = TableManager::getSingleton().GetRecord(RUNCONFIG, "MainGS");
	AString szMainGS = mainGS->get("IP").string();
	int nMainPort = mainGS->get("PORT");
	net->SetMainNode(szMainGS.c_str(), nMainPort);
	mLGNodeNet->StartNet(szMainGS.c_str(), nMainPort);

	// �����ʺ�DB������
	Hand<DBNodeManager> db = mAccountDB;
	AccountDBClient *p = dynamic_cast<AccountDBClient*>(db.getPtr());
	mAccountDB->SetDBEvent(GetMainEventCenter().getPtr(), DBCallBack(&AccountDBClient::OnConnected, p), DBCallBack(&AccountDBClient::OnDisconnected, p) );

	AString accountIp = TableManager::getSingleton().GetData(RUNCONFIG, "AccountNet", "IP").string();
	int accountPort = TableManager::getSingleton().GetData(RUNCONFIG, "AccountNet", "PORT");
	NiceData dbParam;
	dbParam.set(DBIP, accountIp);
	dbParam.set(DBPORT, accountPort);	
	dbParam.set(DBNAME, "AccoundDB");
	if (!mAccountDB->InitDB(&dbParam))
		ERROR_LOG("��ʼ�ʺ�DBʧ��");

	{
		ARecord loginIpConfig = TableManager::getSingleton().GetRecord(RUNCONFIG, "LoginIp");
		AssertNote(loginIpConfig, "�������õ�½��ַ LoginIp");
		AString szLoginIp = loginIpConfig->get("IP").string();
		int nPort = loginIpConfig->get("PORT");
		mServerID = loginIpConfig["SERVER_ID"];
		mGameServerName = loginIpConfig["INFO"];		
		mLoginDNS = loginIpConfig["DNS"];

		LoginEvent::RegisterNet(center);

        mPlayerWebNet = MEM_NEW LGWebNet(this);
        if (mPlayerWebNet->StartNet(szLoginIp.c_str(), nPort))
        {
            NOTE_LOG("[%d]�ɹ�������½��������[%s:%d]", mServerID, szLoginIp.c_str(), nPort);
        }
        else
            ERROR_LOG("������½��������ʧ�� >[%s:%d]", szLoginIp.c_str(), nPort);

		AString glogIp = TableManager::getSingleton().GetData(RUNCONFIG, "GLogData", "IP").string();
		int glogPort = TableManager::getSingleton().GetData(RUNCONFIG, "GLogData", "PORT");
		mGlogRegionID = TableManager::getSingleton().GetData(RUNCONFIG, "GLogData", "REGION_ID");
		mGlogServerID = TableManager::getSingleton().GetData(RUNCONFIG, "GLogData", "SERVER_ID");

		mGlog = MEM_NEW UDPLog(glogIp.c_str(), glogPort);

	}

	// ��ȡ���ع�������
	AutoData d = MEM_NEW DataBuffer();
	FileDataStream f("RunConfig/anno_data.bytes", FILE_READ);
	if (f.readData(d.getPtr()))
	{
		NOTE_LOG("�ɹ���ȡ�������� >%s", "RunConfig/anno_data.bytes");
		UpdateAnnouncementTable(d);
	}
	else
		NOTE_LOG("δ�ܶ�ȡ�������� >%s, ���ܹ����ļ�������", "RunConfig/anno_data.bytes");
}

void LoginThread::UpdateServerInfo(AutoData serverInfo)
{
	mServerInfoData = serverInfo;
#if !RELEASE_RUN_VER
	AutoTable t = tBaseTable::NewBaseTable();
	if (mServerInfoData)
	{
		// ѡ��ѹ
		mServerInfoData->seek(0);
		DSIZE scrSize = 0, zipSize = 0;
		mServerInfoData->read(scrSize);
		mServerInfoData->read(zipSize);
		Auto<DataBuffer> d = mServerInfoData;
		AutoData scrData;
		if (!d->UnZipData(scrData, 0, scrSize, sizeof(DSIZE)*2, zipSize))
		{
			ERROR_LOG("��Ϸ���б���Ϣ��ѹʧ��");
			return;
		}
		scrData->seek(0);
		if (t->LoadFromData(scrData.getPtr()))
		{
			printf("Now show server area info \r\n----------------------------------------\r\n");
			for (TableIt tIt(t); tIt; ++tIt)
			{
				ARecord re = tIt.getCurrRecord();
				if (re)
				{
					AString name = re["SERVER_NAME"];
					AString ver = re["VER"];
					int id = re["ID"];
					AutoNice ipInfo = (tNiceData*)re["LG_IP"];
					if (ipInfo && !ipInfo->empty())
					{
						for (auto it = ipInfo->begin(); it->have(); it->next())
						{
							UInt64 ipValue = TOUINT64(it->key().c_str());
							printf("ID %d [%s] > %s [%d:%d] v[%s] [%s]\r\n", id, AString::getANIS(name.c_str()).c_str(), ServerIPInfo::GetAddrInfo(ipValue).c_str(), (int)re["GS_COUNT"], (int)re["ONLINE"], ver.c_str(), SERVER_STATE_TOOL::ToStringServerState(re["STATE"]));
						}
					}
					else
						printf("ERROR: ID %d [%s] = NULL\r\n", id, AString::getANIS(name.c_str()));
				}
			}
			printf("----------------------------------------\r\n");
		}
		else
			NOTE_LOG("�ָ���������Ϣ����");
	}
	else
		NOTE_LOG("��������Ϣ����Ϊ��");
#endif
}
//-------------------------------------------------------------------------*/
// ������Դ����
void LoginThread::AppendResourcesServer(GSKEY resNetKey, NetNodeConnectData *pNetNodeData)
{

	mResourcesServerList.erase(resNetKey);
	mResourcesServerList.insert(resNetKey, pNetNodeData->GetSelf());

	NOTE_LOG("��ǰ������Դ���·����� >[%u]", mResourcesServerList.size());

//#if !DEVELOP_MODE
//	Hand<TestSDKEvent> evt = GetMainEventCenter()->StartDefaultEvent("TestSDKEvent");
//	evt->mStartTime = TimeManager::NowTick();
//	evt->WaitTime(10);
//	mSDK->RequestCheckUCLogin("ssh1game33c62eeda73b491fb447cc3d62f88bb213406", evt, false);
//#endif
}

bool LoginThread::UpdateServerVersion(const AString &versionString)
{
	if (versionString.length()<=0)
	{
		ERROR_LOG("�汾�ַ���Ϊ��");
		return false;
	}
	AutoTable versionTable = TableManager::getSingleton().GetTable(LOGIN_SERVER_VERSION_TABLE);
	AssertNote(versionTable, "����������� [%s]", LOGIN_SERVER_VERSION_TABLE);
	Data d = versionTable->GetValue("VER", "VERSION");
	if (d.empty())
	{
		ERROR_LOG("�汾���ñ����� [%s]", LOGIN_SERVER_VERSION_TABLE);
		return false;
	}
	d = versionString.c_str();
	if (!versionTable->SaveCSV(LOGIN_SERVER_VERSION_TABLE))
	{
		ERROR_LOG("����汾����ʧ�� [%s]", LOGIN_SERVER_VERSION_TABLE);
		return false;
	}
	mServerVersion = versionString;
	NOTE_LOG("��������½�汾����Ϊ [%s]", versionString.c_str());

	// ͬ����AccountServer
	AutoNice param = MEM_NEW NiceData();
	param["SERVER_ID"] = mServerID;
	param["VER"] = mServerVersion;
	GetDBWork()->ExeSqlFunction(DBCallBack(), GLOBAL_AUCCONT_TABLE, NULL, "e_notify_version_changed", param);

	UpdateTileInfo();
	return true;
}

bool LoginThread::UpdateServerStateData(int state)
{
	if (mServerShowState==state)
	{
		printf("��ǰ��ʾ״̬��Ҫ���޸ĵ�һ�� [%d]\r\n", state);
		return true;
	}
	if (state<SHOW_STATE_NONE || state>SHOW_STATE_HOT)
	{
		NOTE_LOG("ERROR: ��Ч����Ϣ��ʾ״̬ [%d]", state);
		return false;
	}
	AutoTable versionTable = TableManager::getSingleton().GetTable(LOGIN_SERVER_VERSION_TABLE);
	AssertNote(versionTable, "����������� [%s]", LOGIN_SERVER_VERSION_TABLE);
	Data d = versionTable->GetValue("VER", "STATE");
	if (d.empty())
	{
		ERROR_LOG("�汾���ñ����� [%s]", LOGIN_SERVER_VERSION_TABLE);
		return false;
	}
	d = state;
	if (!versionTable->SaveCSV(LOGIN_SERVER_VERSION_TABLE))
	{
		ERROR_LOG("����汾����ʧ�� [%s]", LOGIN_SERVER_VERSION_TABLE);
		return false;
	}
	mServerShowState = (SERVER_SHOW_STATE)state;
	NOTE_LOG("��������ʾ״̬��Ϣ����Ϊ [%s]", SERVER_STATE_TOOL::ToStringServerState(state));

	// ͬ����AccountServer
	AutoNice param = MEM_NEW NiceData();
	param["SERVER_ID"] = mServerID;
	param["STATE"] = state;
	GetDBWork()->ExeSqlFunction(DBCallBack(), GLOBAL_AUCCONT_TABLE, NULL, "f_update_server_run_state", param);

	UpdateTileInfo();

	return true;
}


void LoginThread::UpdateTileInfo()
{
	AutoTable configTable = tBaseTable::NewBaseTable(false);
	if (!configTable->LoadCSV("RunConfig/LoginConfig.csv"))
	{
		ERROR_LOG("������������ʧ�� RunConfig/LoginConfig.csv");
		return;
	}
	int serverID = configTable->GetValue("LoginIp", "SERVER_ID");
	char szDir[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH-1, szDir);
	AString tile;
	tile.Format("%s v[%s] <%d>[%s] [%s:%d] [v%s] [%s] %s",
#ifdef _DEBUG
		"LS_Debug"
#else
		"LS_Release"
#endif		
		, SERVER_VERSION_FLAG
		, (int)configTable->GetValue("LoginIp", "SERVER_ID")
		, serverID>0 ? AString::getANIS(configTable->GetValue("LoginIp", "INFO").string().c_str()).c_str() : "��Ϸ����Ϣ"
		, configTable->GetValue("LoginIp", "IP").string().c_str()
		, (int)configTable->GetValue("LoginIp", "PORT")
		,  mServerVersion.c_str()
		, SERVER_STATE_TOOL::ToStringServerState((int)mServerShowState)
		, szDir
		);
	SetConsoleTitle(tile.c_str());
	{	
		UDPNet	tempLog;
		tempLog.Send("127.0.0.1", 999, tile.c_str(), tile.length());
	}
}

bool LoginThread::GetRandResourcesServer(AString &resIP, int &nPort)
{
	while (true)
	{	
		if (mResourcesServerList.empty())
		{
			WARN_LOG("��Դ���·�����Ϊ��");
			return false;
		}

		int nRand = rand()%mResourcesServerList.size();
		Hand<NetNodeConnectData> res = mResourcesServerList.get(nRand);
		if (res)
		{
			int p2 = 0;
			AString ip = ServerIPInfo::Num2IP(mResourcesServerList.getKey(nRand), nPort, p2);
			resIP = ip;
			return true;
		}	
		mResourcesServerList._remove(nRand);
	}
	return false;
}

void LoginThread::RefreshCheckResourceServer()
{
	// �����ǰ����MD5�����еĲ����, ����յ�ǰ������Դ������
	for (size_t i=0; i<mResourcesServerList.size(); )
	{
		Hand<NetNodeConnectData> netData = mResourcesServerList.get(i);
		if (!netData || !netData->IsConnected())
		{
			AString ipInfo = ServerIPInfo::GetAddrInfo(mResourcesServerList.getKey(i));
			ERROR_LOG("��Դ������ %s ����ر�, ��ǰ�Ƴ�", ipInfo.c_str());
			mResourcesServerList._remove(i);
		}
		else
			++i;
	}

	if (mResourcesServerList.empty())
	{
		ERROR_LOG("��ǰ�����ڿ��õ���Դ������");
	}
}

HandConnect LoginThread::GetLeastGSConnect()
{
	HandConnect conn = _GetLeastGSConnect();
	if (!conn)
	{
		UpdateLeastOnlineGS();
		conn = _GetLeastGSConnect();
	}
	if (!conn)
	{
		ERROR_LOG("���ش���: ��ǰ�����ڿ��õ�GS");
	}
	return conn;
}

HandConnect LoginThread::_GetLeastGSConnect()
{
	Hand<MeshedNodeNet> net = mLGNodeNet;
	NetNodeList &netNodeList = net->GetNetNodeList();
	for (int i=0; i<netNodeList.size(); ++i)
	{
		Hand<NetNodeConnectData> node = netNodeList.get(i);

		if (!node || (bool)node->get("NOW_CLOSE") || !node->IsConnected() || !IS_GS  )
			continue;

		// �ų��ڴ治����GS
		if (node->get("MEM_NOT_ENOUGH"))
			continue;

		if (mLeastGSKey==(UInt64)node["LOGIN_GSKEY"])
		{
			return node->mNodeConnect;
		}
	}
	return HandConnect();
}

HandConnect LoginThread::GetAnyGSConnect()
{
	Hand<MeshedNodeNet> net = mLGNodeNet;
	NetNodeList &netNodeList = net->GetNetNodeList();
	for (int i=0; i<netNodeList.size(); ++i)
	{
		Hand<NetNodeConnectData> node = netNodeList.get(i);

		if (!node || (bool)node->get("NOW_CLOSE") || !node->IsConnected() || !IS_GS  )
			continue;

		// �ų��ڴ治����GS
		if (node->get("MEM_NOT_ENOUGH"))
			continue;

		return node->mNodeConnect;			
	}
	return HandConnect();
}

int LoginThread::GetGSCount()
{
	Hand<MeshedNodeNet> net = mLGNodeNet;
	NetNodeList &netNodeList = net->GetNetNodeList();	
	int x = 0;
	for (int i=0; i<netNodeList.size(); ++i)
	{
		Hand<NetNodeConnectData> node = netNodeList.get(i);

		if (!node || (bool)node->get("NOW_CLOSE") || !node->IsConnected() || !IS_GS  )
			continue;

		++x;
	}
	return x;
}

int LoginThread::GetOnlineCount()
{
	Hand<MeshedNodeNet> net = mLGNodeNet;
	NetNodeList &netNodeList = net->GetNetNodeList();	
	int x = 0;
	for (int i=0; i<netNodeList.size(); ++i)
	{
		Hand<NetNodeConnectData> node = netNodeList.get(i);

		if (!node || (bool)node->get("NOW_CLOSE") || !node->IsConnected() || !IS_GS  )
			continue;

		x += (int)node["ONLINE_COUNT"] - 1;
	}
	return x;
}

void LoginThread::NotifyGSCountToAG()
{
	AutoNice d = MEM_NEW NiceData();
	d["SERVER_ID"] = mServerID;
	d["GS_COUNT"] = GetGSCount();

	GetDBWork()->ExeSqlFunction(DBCallBack(), GLOBAL_AUCCONT_TABLE, NULL, "e_notify_gs_count", d);
}

void LoginThread::NotifyOnlineCountToAG()
{
	int onlineCount = GetOnlineCount();
	AutoNice d = MEM_NEW NiceData();
	d["SERVER_ID"] = mServerID;
	d["ONLINE"] = onlineCount;

	GetDBWork()->ExeSqlFunction(DBCallBack(), GLOBAL_AUCCONT_TABLE, NULL, "e_notify_online_count", d);

	if (onlineCount > 0)
	{
		int serverId = mGlogServerID;					// serverId	��Ϸ���id	Int(11)	Y	
		int cu = GetOnlineCount();			// cu	��������	Int(11)	Y	
		int regionID = mGlogRegionID;					// regionID	����id	Int(11)	Y	
		AString dt = TimeManager::ToDataTime(TimeManager::Now());					// dt	ʱ��	String	Y	
		int platID = 1;						// platID	�ն˲���ϵͳ	Int(2)	Y	0Ϊios��1Ϊandroid
		int iMaxCapacity = 2000;				// iMaxCapacity	�������������	Int(15)	Y	
		int channel = 78;					// channel	����id	Int(11)	Y	
		int AdSubChannel = 0;				// AdSubChannel	���������	Int(11)	N	
		mGlog->Log("log_cu\n%d|%d|%d|%s|%d|%d|%d|%d\n",
			serverId,
			cu,
			regionID,
			dt.c_str(),
			platID,
			iMaxCapacity,
			channel,
			AdSubChannel);
	}
}

AutoData LoginThread::GetAnnoDataBySDK(int sdkType)
{
	if (sdkType<=0)
		return mAnnoDataList.find(0); 

	AutoData d = mAnnoDataList.find(sdkType);
	if (!d)
		return mAnnoDataList.find(0); 

	return d;
}

void LoginThread::OnPlayerConnected(tNetConnect *pPlayerConnect)
{
    // ��½�����ȷ��䵽GS������֤����ȡ��DB key, ���ߴ���һ����KEY�� ÿ��KEY��Ӧ������Ϸ��ļ�¼���綷��������һ��DB���齫��һ��DB��
    // ���ݱ����ʺű���ͬһȫ��DB����
    Hand<WebPlayer> p = pPlayerConnect;
    
}

void LoginThread::OnPlayerLeaveGS(GSKEY netKey)
{
    // ֪ͨ�ͻ����˳�����

}

void LoginThread::RequestStartGame(HandConnect webPlayer)
{
    Hand<WebPlayer> p = webPlayer;

}

//-------------------------------------------------------------------------*/
LoginThread::LoginThread()
	: BaseThread("Login")
	, mLeastGSKey(0)
	, mServerID(0)
	, mSDK(NULL)
	, mGlog(NULL)
	, mGlogRegionID(0)
	, mGlogServerID(0)
	, mResForeNetKey(0)
{
	//mLoginServerNet = new LoginServerNet(this);
	mAccountDB = MEM_NEW AccountDBClient(this);

	mSDK = MEM_NEW SDK();
}

LoginThread::~LoginThread()
{
	mLGNodeNet._free();
	mAccountDB._free();
    mPlayerWebNet._free();
	SAFE_DELETE(mGlog);
	SAFE_DELETE(mSDK);
}

void LoginThread::Process(void*)
{
	mLGNodeNet->Process();
	mAccountDB->Process();

    mPlayerWebNet->Process();

	mSDK->Process();
}


bool LoginThread::NotifyThreadClose()
{
	mAccountDB->Close();
    mPlayerWebNet->StopNet();

	Hand<MeshedNodeNet> net = mLGNodeNet;
	net->NotifyClose();
	Process(NULL);
	TimeManager::Sleep(1000);
	Process(NULL);

	mLGNodeNet->StopNet();
	Process(NULL);

	Process(NULL);

	GetMainEventCenter()._free();

	mSDK->Close();

	CanClose();

	return true;
}

#define LOG_SYNC_ONLINE {}

//-------------------------------------------------------------------------*/
// ���ڳ��ֹ��������������֪ͨ��ʹ����������Ϊ0����ÿ�ζ���ʹ�õ�һ��Ϊ0��GS, �����������������BUG
// ��������: ��������κ�һ����������ҵ�GS(��������Ϊ1��ʾ��Чͬ��), ��ֱ�Ӵ����п���GS�����
//-------------------------------------------------------------------------*/
GSKEY LoginThread::GetLeastOnlinePlayerGS()
{
	int minCount = -1;
	GSKEY gsKey = 0;
	RandTool<GSKEY>	mEmptyGS;
	Hand<MeshedNodeNet> net = mLGNodeNet;
	NetNodeList &netNodeList = net->GetNetNodeList();
	LOG_SYNC_ONLINE("Now begin check \r\n================================");
	for (int i=0; i<netNodeList.size(); ++i)
	{
		Hand<NetNodeConnectData> node = netNodeList.get(i);

		if (!node || (bool)node->get("NOW_CLOSE") || !node->IsConnected() || !IS_GS  )
			continue;

		// �ų��ڴ治����GS
		if (node->get("MEM_NOT_ENOUGH"))
			continue;

		//??? �ų���������ʱ�䳬ʱ��GS
		if (TimeManager::Now() - (UInt64)node->get("LAST_TIME") > CHECK_MEMORY_ONCE_TIME+6)
			continue;

		GSKEY ipKey = node["LOGIN_GSKEY"];
		int	onlineCount = node["ONLINE_COUNT"];
		LOG_SYNC_ONLINE("%s > [%d]", ServerIPInfo::GetAddrInfo(ipKey).c_str(), onlineCount);

		// �ų��������ķ�����
		if (onlineCount > SCENE_THEAD_MAX_PLAYER_COUNT-6)
			continue;

		// ���GL֮�仹δ�ɹ����ӣ���ʱonlineCount=0,�����ų���GS
		if (onlineCount<1)
			continue;
		else
			mEmptyGS.Push(ipKey);

		if (minCount<0 || onlineCount<minCount )
		{
			minCount = onlineCount;
			gsKey = node["LOGIN_GSKEY"];

			LOG_SYNC_ONLINE(" ### %s > [%d]", ServerIPInfo::GetAddrInfo(ipKey).c_str(), onlineCount);
		}
	}
#if RELEASE_RUN_VER
	//!!! �������κ�һ������ҵ�GSʱ����ֱ�Ӵӿ���GS�����
	if (minCount<=1)
	{
		if (!mEmptyGS.RandPop(gsKey))
		{
			NOTE_LOG("XXX ERROR: ���ش���, δ��⵽����GS������");
		}
		else
			LOG_SYNC_ONLINE("===== Rand result %s > [%d]\r\n-----------------------------------------", ServerIPInfo::GetAddrInfo(gsKey).c_str(), minCount);
	}
	else
#endif
		LOG_SYNC_ONLINE("***** Result %s > [%d]\r\n-----------------------------------------", ServerIPInfo::GetAddrInfo(gsKey).c_str(), minCount);

	//mLeastOnlineGSIp = ServerIPInfo::Num2IP(gsKey, mLeastOnlineGSPort, p2);
	//ERROR_LOG(" *** Min Server > [%s : %d] > player online [%d]", mLeastOnlineGSIp.c_str(), mLeastOnlineGSPort, minCount);
	return gsKey;
}

void LoginThread::UpdateLeastOnlineGS()
{
	GSKEY gs = GetLeastOnlinePlayerGS();

	if (gs==0)
	{
		NOTE_LOG("��ǰδ���ֿ��õ� GameServer, �뼰ʱ������������");
	}
	mLeastGSKey = gs;

}

bool LoginThread::UpdateAnnouncementTable(AutoData annoData)
{
	if (!annoData || annoData->dataSize()<4)
	{
		ERROR_LOG("��������Ϊ��");
		return false;
	}

	DSIZE size = annoData->dataSize();

	annoData->seek(size-4);
	DSIZE scrSize = 0;
	if (!annoData->read(scrSize))
	{
		ERROR_LOG("��ȡ��������ʧ��");
		return false;
	}

	AutoData scrData = MEM_NEW DataBuffer(1024);
	Auto<DataBuffer> d = annoData;
	if (!d->UnZipData(scrData, 0, scrSize))
	{
		ERROR_LOG("��ѹʧ��");
		return false;
	}
	scrData->seek(0);
	AutoTable annoTable = tBaseTable::NewBaseTable();
	CSVTableLoader loader;
	if (!loader.LoadFromData(annoTable.getPtr(), scrData.getPtr()))
	{
		ERROR_LOG("�ָ����ʧ��");
		return false;
	}

	FileDataStream f("RunConfig/anno_data.bytes", FILE_CREATE);
	if (f.writeData(annoData.getPtr(), annoData->dataSize()))
		NOTE_LOG("�ɹ����湫������ >%s", "RunConfig/anno_data.bytes")
	else
		ERROR_LOG("ʧ�ܱ��湫������ >%s", "RunConfig/anno_data.bytes");

	annoTable->SaveCSV("TableLog/Login_AnnoTable.csv");

	NOTE_LOG("�ɹ����¹�����Ϣ [%s]", ANNO_DB_RESOURCE);

	mAnnoDataList.clear();

	EasyHash<int, AutoTable> tempAnnoList;

	for (TableIt tIt(annoTable); tIt; ++tIt)
	{
		ARecord annoRe = tIt.getCurrRecord();
		int id = annoRe[0];

		int typeIndex = id/1000;

		AutoTable t = tempAnnoList.find(typeIndex);
		
		if (!t)
		{
			t = tBaseTable::NewBaseTable();
			t->InitField(annoTable->GetField());
		}
		t->AppendRecord(annoRe, true);

		tempAnnoList.insert(typeIndex, t);
	}

	for (auto it=tempAnnoList.begin(); it.have(); ++it)
	{
		AutoTable t = it.get();

		AString tableName;
		tableName.Format("TableLog/Login_AnnoTable_%d.csv", it.key());
		t->SaveCSV(tableName.c_str());

		AutoData d = MEM_NEW DataBuffer(1024);
		if (t->SaveData(d.getPtr()))
		{                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
			mAnnoDataList.insert(it.key(), d);
		}
		else
			ERROR_LOG("���й���������ʧ��");
	}


	return false;
}




HandConnect LGServerNet::CreateConnect()
{
	return MEM_NEW LoginClientConnect();
}

//-------------------------------------------------------------------------*/
class TM_WaitSendHeartBeat : public CEvent
{
public:
	virtual bool _DoEvent() override
	{
		WaitTime(NET_HEARTBEAT_ONCE_TIME);

		return true;
	}

	virtual bool _OnTimeOver() override
	{
		ConnectList &list = mGameServerNet->GetConnectList();
		for (size_t i=0; i<list.size(); ++i)
		{
			HandConnect &conn = list[i];
			if (conn && !conn->IsDisconnect())
			{
				try{
					conn->Send(&mHeartBeatPacket, false);
				}
				catch(...){}
			}
		}
		WaitTime(NET_HEARTBEAT_ONCE_TIME);

		return true;
	}

public:
	LGServerNet		*mGameServerNet;
	HeartBeatPacket		mHeartBeatPacket;
};

void LGServerNet::BindEventCenter()
{
	IOCPServerNet::BindEventCenter();

	if (mSendHeartEvent)
		mSendHeartEvent._free();

	if (!GetEventCenter()->_ExistEventFactory("TM_WaitSendHeartBeat"))
	{
		GetEventCenter()->RegisterEvent("TM_WaitSendHeartBeat", MEM_NEW EventFactory<TM_WaitSendHeartBeat>());
	}
	mSendHeartEvent = GetEventCenter()->StartEvent("TM_WaitSendHeartBeat");
	Hand<TM_WaitSendHeartBeat> t = mSendHeartEvent;
	t->mGameServerNet = this;
	mSendHeartEvent->DoEvent();
}

//-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*/
void AccountDBClient::InitNodeConnectNet(tNetHandle *pDBConnectNet)
{
	AutoEventCenter center = pDBConnectNet->GetEventCenter();
	center->RegisterEvent("AL_SyncServerInfo", MEM_NEW EventFactory<AL_SyncServerInfo>());
	center->RegisterEvent("AL_RequestUpdateAnno", MEM_NEW EventFactory<AL_RequestUpdateAnno>());
	center->RegisterEvent("AL_RequestPayEvent", MEM_NEW EventFactory<AL_RequestPayEvent_L>());
	center->RegisterEvent("AL_RequestModifyServerShowState", MEM_NEW EventFactory<AL_RequestModifyServerShowState_L>());
	center->RegisterEvent("AL_RequestResForeNetKey", MEM_NEW EventFactory<AL_RequestResForeNetKey_L>());
	center->RegisterEvent("AL_RequestAllLoginGSIP", MEM_NEW EventFactory<AL_RequestAllLoginGSIP_L>());
}

void AccountDBClient::OnConnected(DBOperate *op, bool bConnectSucceed)
{
	mWaitReconnectEvent._free();
	if (bConnectSucceed)
	{	
		NOTE_LOG("�ɹ������ʺ�DB������");

		ARecord loginIpConfig = TableManager::getSingleton().GetRecord(RUNCONFIG, "LoginIp");
		AssertNote(loginIpConfig, "�������õ�½��ַ LoginIp");
		AString szLoginIp = loginIpConfig->get("IP").string();
		int nPort = loginIpConfig->get("PORT");
		//NOTE_LOG("LG [%s:%d] == [%s:%d]", mpLoginThread->mLGServerNet->GetIp(), mpLoginThread->mLGServerNet->GetPort(), szLoginIp.c_str(), nPort);
		// �ϴ����漰������Ϣ, �ϴ��� AccountServer��ͬ��������LG
		AutoNice d = MEM_NEW NiceData();
		d["LG_IP"] = ServerIPInfo::IP2Num(szLoginIp.c_str(), nPort, 0);
		d["LG_DNS"] = mpLoginThread->mLoginDNS;
		d["ID"] = mpLoginThread->GetServerID();
		d["SERVER_NAME"] = mpLoginThread->GetServerName();
		d["GS_COUNT"] = mpLoginThread->GetGSCount();
		d["ONLINE"] = mpLoginThread->GetOnlineCount();		
		d["VER"] = mpLoginThread->mServerVersion;
		d["STATE"] = mpLoginThread->mServerShowState;
		Hand<MeshedNodeNet> net = mpLoginThread->mLGNodeNet;
		d["MAIN_NODE_NETKEY"] = net->mMainNodeKey;
		d["VER_FLAG"] = SERVER_VERSION_FLAG;
		ExeSqlFunction(DBCallBack(&AccountDBClient::OnDBResponseServerState, this), GLOBAL_AUCCONT_TABLE, NULL, "e_notify_serverinfo", d);
	}
	else
	{
		StartWaitReconnect();
	}
}

void AccountDBClient::OnDBResponseServerState(DBOperate *op, bool bSu)
{
	// ����������ȷ, ͼ����ʾ��ɫ
	if (bSu && op->mResultNiceData)
	{
		if (op->mResultNiceData["RESULT"])
			return;
		else
			ERROR_LOG("e_notify_serverinfo >ERROR : %s", op->mResultNiceData["ERROR"].string().c_str());
	}
	else
		ERROR_LOG("e_notify_serverinfo ִ�д��� >%d", op->mResultType);
	tBaseServer::SetIocn("LG.bmp", STRING(mpLoginThread->GetServerID()), "����", 100, 0x000000);
}

void AccountDBClient::StartWaitReconnect()
{
	mWaitReconnectEvent._free();
	mWaitReconnectEvent = mpLoginThread->GetMainEventCenter()->WaitTime(10, EventCallBack(&AccountDBClient::OnStartReconnect, this));
}

