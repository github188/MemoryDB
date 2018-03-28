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
	//	// 在连接人数事件
	//	nodeData->mRequestCheckNetEvent = nodeData->mNodeConnect->StartEvent("GG_RequestOnlinePlayerCount");
	//	nodeData->mRequestCheckNetEvent->DoEvent();
	//}
}

void LGNodeNet::RemoveNode(NetNodeConnectData *pNode)
{
	MeshedNodeNet::RemoveNode(pNode);
	mThread->UpdateLeastOnlineGS();

	// 通知AG, 当前GS数量
	mThread->NotifyGSCountToAG();
}

void LGNodeNet::RegisterNodeEvent( AutoEventCenter serverCenter, AutoEventCenter connectCenter )
{
	serverCenter->RegisterEvent("GL_NotifyOnlineInfo", MEM_NEW EventFactory<GL_NotifyOnlineInfo_R>());

	serverCenter->RegisterEvent("GL_UploadLoginInfoTable", MEM_NEW EventFactory<GL_UploadLoginInfoTable_R>());
	serverCenter->RegisterEvent("GL_RequestLGOnlineInfo", MEM_NEW EventFactory<GL_RequestLGOnlineInfo_R>());
	serverCenter->RegisterEvent("GL_RequestLGUpdateVersion", MEM_NEW EventFactory<GL_RequestLGUpdateVersion_R>());
	serverCenter->RegisterEvent("GL_RequestLGUpdateServerState", MEM_NEW EventFactory<GL_RequestLGUpdateServerState_R>());
	
	// 更新资源信息
	serverCenter->RegisterEvent("RG_RequestAllUpdateResources", MEM_NEW EventFactory<RG_RequestAllUpdateResources_R>());
	serverCenter->RegisterEvent("RS_ReqeustCloseServer", MEM_NEW EventFactory<RS_ReqeustCloseServer_LR>());

	// 请求GS准备迎接客户端进入
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
		, bOk?"正常":"X异常"
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
			// 如果是资源更新服务器	
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
#define  SYNC_ONLINE_TOAS_TIME			(10)			// 10秒同步一次在线人数到AG

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
	AssertNote(bLoad, "必须配置 [%s]", LOGIN_SERVER_VERSION_TABLE);
	mServerVersion = versionTable->GetValue("VER", "VERSION");
	mServerShowState = (SERVER_SHOW_STATE)(int)versionTable->GetValue("VER", "STATE");
	AssertNote(mServerVersion.length()>0, "必须配置版本号 LoginIp >VERSION");

	AutoTable t = TableManager::getSingleton().CreateNewTable(RUNCONFIG);
	if (!t->LoadCSV(LOGIN_CONFIG_TABLE))
	{
		ERROR_LOG("加载配置失败 [%s]", LOGIN_CONFIG_TABLE);
		Close();
		return;
	}

	AString szIp = TableManager::getSingleton().GetData(RUNCONFIG, "NodeNet", "IP").string();
	int nPort = TableManager::getSingleton().GetData(RUNCONFIG, "NodeNet", "PORT");
	int nodeKey = TableManager::getSingleton().GetData(RUNCONFIG, "MainGS", "NODE_KEY");
	AssertNote(nodeKey!=0, "必须设置 [%s] MainGS >NODE_KEY, 不可为0", RUNCONFIG);

	mLGNodeNet = MEM_NEW LGNodeNet(this, szIp.c_str(), nPort, nodeKey);
	Hand<LGNodeNet> net = mLGNodeNet;
	net->SetEventCenter(MEM_NEW EventCenter(), MEM_NEW EventCenter());	
	mLGNodeNet->BindEventCenter();

	ARecord mainGS = TableManager::getSingleton().GetRecord(RUNCONFIG, "MainGS");
	AString szMainGS = mainGS->get("IP").string();
	int nMainPort = mainGS->get("PORT");
	net->SetMainNode(szMainGS.c_str(), nMainPort);
	mLGNodeNet->StartNet(szMainGS.c_str(), nMainPort);

	// 连接帐号DB服务器
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
		ERROR_LOG("初始帐号DB失败");

	{
		ARecord loginIpConfig = TableManager::getSingleton().GetRecord(RUNCONFIG, "LoginIp");
		AssertNote(loginIpConfig, "必须配置登陆地址 LoginIp");
		AString szLoginIp = loginIpConfig->get("IP").string();
		int nPort = loginIpConfig->get("PORT");
		mServerID = loginIpConfig["SERVER_ID"];
		mGameServerName = loginIpConfig["INFO"];		
		mLoginDNS = loginIpConfig["DNS"];

		LoginEvent::RegisterNet(center);

        mPlayerWebNet = MEM_NEW LGWebNet(this);
        if (mPlayerWebNet->StartNet(szLoginIp.c_str(), nPort))
        {
            NOTE_LOG("[%d]成功开启登陆服务网络[%s:%d]", mServerID, szLoginIp.c_str(), nPort);
        }
        else
            ERROR_LOG("开启登陆服务网络失败 >[%s:%d]", szLoginIp.c_str(), nPort);

		AString glogIp = TableManager::getSingleton().GetData(RUNCONFIG, "GLogData", "IP").string();
		int glogPort = TableManager::getSingleton().GetData(RUNCONFIG, "GLogData", "PORT");
		mGlogRegionID = TableManager::getSingleton().GetData(RUNCONFIG, "GLogData", "REGION_ID");
		mGlogServerID = TableManager::getSingleton().GetData(RUNCONFIG, "GLogData", "SERVER_ID");

		mGlog = MEM_NEW UDPLog(glogIp.c_str(), glogPort);

	}

	// 读取本地公告数据
	AutoData d = MEM_NEW DataBuffer();
	FileDataStream f("RunConfig/anno_data.bytes", FILE_READ);
	if (f.readData(d.getPtr()))
	{
		NOTE_LOG("成功读取公告数据 >%s", "RunConfig/anno_data.bytes");
		UpdateAnnouncementTable(d);
	}
	else
		NOTE_LOG("未能读取公告数据 >%s, 可能公告文件不存在", "RunConfig/anno_data.bytes");
}

void LoginThread::UpdateServerInfo(AutoData serverInfo)
{
	mServerInfoData = serverInfo;
#if !RELEASE_RUN_VER
	AutoTable t = tBaseTable::NewBaseTable();
	if (mServerInfoData)
	{
		// 选解压
		mServerInfoData->seek(0);
		DSIZE scrSize = 0, zipSize = 0;
		mServerInfoData->read(scrSize);
		mServerInfoData->read(zipSize);
		Auto<DataBuffer> d = mServerInfoData;
		AutoData scrData;
		if (!d->UnZipData(scrData, 0, scrSize, sizeof(DSIZE)*2, zipSize))
		{
			ERROR_LOG("游戏区列表信息解压失败");
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
			NOTE_LOG("恢复服务器信息错误");
	}
	else
		NOTE_LOG("服务器信息数据为空");
#endif
}
//-------------------------------------------------------------------------*/
// 更新资源部分
void LoginThread::AppendResourcesServer(GSKEY resNetKey, NetNodeConnectData *pNetNodeData)
{

	mResourcesServerList.erase(resNetKey);
	mResourcesServerList.insert(resNetKey, pNetNodeData->GetSelf());

	NOTE_LOG("当前可用资源更新服务器 >[%u]", mResourcesServerList.size());

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
		ERROR_LOG("版本字符串为空");
		return false;
	}
	AutoTable versionTable = TableManager::getSingleton().GetTable(LOGIN_SERVER_VERSION_TABLE);
	AssertNote(versionTable, "必须加载配置 [%s]", LOGIN_SERVER_VERSION_TABLE);
	Data d = versionTable->GetValue("VER", "VERSION");
	if (d.empty())
	{
		ERROR_LOG("版本配置表格错误 [%s]", LOGIN_SERVER_VERSION_TABLE);
		return false;
	}
	d = versionString.c_str();
	if (!versionTable->SaveCSV(LOGIN_SERVER_VERSION_TABLE))
	{
		ERROR_LOG("保存版本配置失败 [%s]", LOGIN_SERVER_VERSION_TABLE);
		return false;
	}
	mServerVersion = versionString;
	NOTE_LOG("服务器登陆版本更新为 [%s]", versionString.c_str());

	// 同步到AccountServer
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
		printf("当前显示状态与要求修改的一样 [%d]\r\n", state);
		return true;
	}
	if (state<SHOW_STATE_NONE || state>SHOW_STATE_HOT)
	{
		NOTE_LOG("ERROR: 无效的信息显示状态 [%d]", state);
		return false;
	}
	AutoTable versionTable = TableManager::getSingleton().GetTable(LOGIN_SERVER_VERSION_TABLE);
	AssertNote(versionTable, "必须加载配置 [%s]", LOGIN_SERVER_VERSION_TABLE);
	Data d = versionTable->GetValue("VER", "STATE");
	if (d.empty())
	{
		ERROR_LOG("版本配置表格错误 [%s]", LOGIN_SERVER_VERSION_TABLE);
		return false;
	}
	d = state;
	if (!versionTable->SaveCSV(LOGIN_SERVER_VERSION_TABLE))
	{
		ERROR_LOG("保存版本配置失败 [%s]", LOGIN_SERVER_VERSION_TABLE);
		return false;
	}
	mServerShowState = (SERVER_SHOW_STATE)state;
	NOTE_LOG("服务器显示状态信息更新为 [%s]", SERVER_STATE_TOOL::ToStringServerState(state));

	// 同步到AccountServer
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
		ERROR_LOG("加载运行配置失败 RunConfig/LoginConfig.csv");
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
		, serverID>0 ? AString::getANIS(configTable->GetValue("LoginIp", "INFO").string().c_str()).c_str() : "游戏区信息"
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
			WARN_LOG("资源更新服务器为空");
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
	// 如果当前最新MD5与现有的不相符, 则清空当前所有资源服务器
	for (size_t i=0; i<mResourcesServerList.size(); )
	{
		Hand<NetNodeConnectData> netData = mResourcesServerList.get(i);
		if (!netData || !netData->IsConnected())
		{
			AString ipInfo = ServerIPInfo::GetAddrInfo(mResourcesServerList.getKey(i));
			ERROR_LOG("资源服务器 %s 网络关闭, 当前移除", ipInfo.c_str());
			mResourcesServerList._remove(i);
		}
		else
			++i;
	}

	if (mResourcesServerList.empty())
	{
		ERROR_LOG("当前不存在可用的资源服务器");
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
		ERROR_LOG("严重错误: 当前不存在可用的GS");
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

		// 排除内存不够的GS
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

		// 排除内存不够的GS
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
		int serverId = mGlogServerID;					// serverId	游戏侧服id	Int(11)	Y	
		int cu = GetOnlineCount();			// cu	在线人数	Int(11)	Y	
		int regionID = mGlogRegionID;					// regionID	分区id	Int(11)	Y	
		AString dt = TimeManager::ToDataTime(TimeManager::Now());					// dt	时间	String	Y	
		int platID = 1;						// platID	终端操作系统	Int(2)	Y	0为ios，1为android
		int iMaxCapacity = 2000;				// iMaxCapacity	服务器最大容量	Int(15)	Y	
		int channel = 78;					// channel	渠道id	Int(11)	Y	
		int AdSubChannel = 0;				// AdSubChannel	广告子渠道	Int(11)	N	
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
    // 登陆后，首先分配到GS进行验证并获取到DB key, 或者创建一个新KEY， 每个KEY对应所有游戏表的记录，如斗地主，是一个DB表，麻将是一个DB表
    // 数据表与帐号表都在同一全局DB库中
    Hand<WebPlayer> p = pPlayerConnect;
    
}

void LoginThread::OnPlayerLeaveGS(GSKEY netKey)
{
    // 通知客户端退出房间

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
// 由于出现过错误的在线人数通知，使用在线人数为0，且每次都会使用第一个为0的GS, 造成在线人数已满的BUG
// 修正策略: 如果存在任何一个无在线玩家的GS(现在最少为1表示有效同步), 则直接从所有可用GS中随机
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

		// 排除内存不够的GS
		if (node->get("MEM_NOT_ENOUGH"))
			continue;

		//??? 排除掉最后更新时间超时的GS
		if (TimeManager::Now() - (UInt64)node->get("LAST_TIME") > CHECK_MEMORY_ONCE_TIME+6)
			continue;

		GSKEY ipKey = node["LOGIN_GSKEY"];
		int	onlineCount = node["ONLINE_COUNT"];
		LOG_SYNC_ONLINE("%s > [%d]", ServerIPInfo::GetAddrInfo(ipKey).c_str(), onlineCount);

		// 排除人数满的服务器
		if (onlineCount > SCENE_THEAD_MAX_PLAYER_COUNT-6)
			continue;

		// 如果GL之间还未成功连接，此时onlineCount=0,所以排除此GS
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
	//!!! 当存在任何一个无玩家的GS时，则直接从可用GS中随机
	if (minCount<=1)
	{
		if (!mEmptyGS.RandPop(gsKey))
		{
			NOTE_LOG("XXX ERROR: 严重错误, 未检测到可用GS服务器");
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
		NOTE_LOG("当前未发现可用的 GameServer, 请及时检查服务器环境");
	}
	mLeastGSKey = gs;

}

bool LoginThread::UpdateAnnouncementTable(AutoData annoData)
{
	if (!annoData || annoData->dataSize()<4)
	{
		ERROR_LOG("公告数据为空");
		return false;
	}

	DSIZE size = annoData->dataSize();

	annoData->seek(size-4);
	DSIZE scrSize = 0;
	if (!annoData->read(scrSize))
	{
		ERROR_LOG("读取公告数据失败");
		return false;
	}

	AutoData scrData = MEM_NEW DataBuffer(1024);
	Auto<DataBuffer> d = annoData;
	if (!d->UnZipData(scrData, 0, scrSize))
	{
		ERROR_LOG("解压失败");
		return false;
	}
	scrData->seek(0);
	AutoTable annoTable = tBaseTable::NewBaseTable();
	CSVTableLoader loader;
	if (!loader.LoadFromData(annoTable.getPtr(), scrData.getPtr()))
	{
		ERROR_LOG("恢复表格失败");
		return false;
	}

	FileDataStream f("RunConfig/anno_data.bytes", FILE_CREATE);
	if (f.writeData(annoData.getPtr(), annoData->dataSize()))
		NOTE_LOG("成功保存公告数据 >%s", "RunConfig/anno_data.bytes")
	else
		ERROR_LOG("失败保存公告数据 >%s", "RunConfig/anno_data.bytes");

	annoTable->SaveCSV("TableLog/Login_AnnoTable.csv");

	NOTE_LOG("成功更新公告信息 [%s]", ANNO_DB_RESOURCE);

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
			ERROR_LOG("序列公告表格数据失败");
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
		NOTE_LOG("成功连接帐号DB服务器");

		ARecord loginIpConfig = TableManager::getSingleton().GetRecord(RUNCONFIG, "LoginIp");
		AssertNote(loginIpConfig, "必须配置登陆地址 LoginIp");
		AString szLoginIp = loginIpConfig->get("IP").string();
		int nPort = loginIpConfig->get("PORT");
		//NOTE_LOG("LG [%s:%d] == [%s:%d]", mpLoginThread->mLGServerNet->GetIp(), mpLoginThread->mLGServerNet->GetPort(), szLoginIp.c_str(), nPort);
		// 上传公告及分区信息, 上传后 AccountServer后同步到所有LG
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
	// 如果结果不正确, 图标显示红色
	if (bSu && op->mResultNiceData)
	{
		if (op->mResultNiceData["RESULT"])
			return;
		else
			ERROR_LOG("e_notify_serverinfo >ERROR : %s", op->mResultNiceData["ERROR"].string().c_str());
	}
	else
		ERROR_LOG("e_notify_serverinfo 执行错误 >%d", op->mResultType);
	tBaseServer::SetIocn("LG.bmp", STRING(mpLoginThread->GetServerID()), "黑体", 100, 0x000000);
}

void AccountDBClient::StartWaitReconnect()
{
	mWaitReconnectEvent._free();
	mWaitReconnectEvent = mpLoginThread->GetMainEventCenter()->WaitTime(10, EventCallBack(&AccountDBClient::OnStartReconnect, this));
}

