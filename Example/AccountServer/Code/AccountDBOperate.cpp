#include "AccountDBOperate.h"
#include "MemoryDBNode.h"
#include "Md5Tool.h"
#include "TimeManager.h"
#include "AccountDBNode.h"
#include "CommonDefine.h"
#include "GameStruct.h"
#include "GameDefine.h"

//NOTE: 此文件使用中文注释,出现调试代码行不对应问题

int f_create_account::Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData)
{
	AString account = paramData["ACCOUNT"];

	if (account=="")
	{
		mResultData["RESULT"] = eAccountIsNull;
		return eNoneError;
	}

	AString pass = paramData["PASSWORD"];
	if (pass=="")
	{
		mResultData["RESULT"] = ePasswordIsNull;
		return eNoneError;
	}

	AutoTable t = pDBNode->GetTable(szTable);
	if (!t)
		return eTableNoExist;		


	if (t->GetRecord(account))
	{
		mResultData["RESULT"] = eAccountExist;
		return eNoneError;
	}

	// 目前数据记录直接创建在同一节点上
	ARecord dataRe = pDBNode->GetTable(DB_USER_TABLE)->GrowthNewRecord(NULL);
	AutoNice d = MEM_NEW NiceData();
	dataRe->set("SYNC_DATA_MD5", d);

	ARecord re = t->CreateRecord(account, true);

	re["DBID"] = (int)dataRe[0];
	dataRe["ACCOUNT"] = account;

	MD5 md(pass.c_str());
	re["PASSWORD"] = pass;
	re["PASS_MD5"] = pass; //??? md.toString().c_str();
	re["DATA_TIME"] = TimeManager::ToDataTime(TimeManager::Now());
	re["REGI_TIME"] = (int)TimeManager::Now();
	re["PHONE"] = paramData["PHONE"].string();
	re["VERSION"] = paramData["VERSION"].string();
	re["IP"] = paramData["IP"].string();
	re["PLATFORM"] = paramData["PLATFORM"].string();
	mResultData["RESULT"] = eCreateAccountSucceed;
	mResultData["DBID"] = (int)dataRe[0];

	return eNoneError;
}

int f_login_check::Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData)
{
	AString account = paramData["ACCOUNT"];

	if (account=="")
	{
		mResultData["RESULT"] = eAccountIsNull;
		return eNoneError;
	}

	AString pass = paramData["PASS_MD5"];
	if (pass=="")
	{
		mResultData["RESULT"] = ePasswordIsNull;
		return eNoneError;
	}

	AutoTable t = pDBNode->GetTable(szTable);
	if (!t)
		return eTableNoExist;		

	ARecord re = t->GetRecord(account);

	if (!re)
	{
		mResultData["RESULT"] = eAccountNoExist;
		return eNoneError;
	}

	AString nowPass = re["PASS_MD5"];
	if (nowPass!=pass)
	{
		mResultData["RESULT"] = ePasswordError;
		return eNoneError;
	}

	ARecord dataRe = pDBNode->GetTable(DB_USER_TABLE)->GetRecord((int)re["DBID"]);

	// 更新登陆时间, 并把登陆时间,保存在cooki, 用于每个请求的验证
	UInt64 now = TimeManager::Now();
	re["LOGIN_TIME"] = (int)now;

	mResultData["LOGIN_TIME"] = (int)now;
	mResultData["RESULT"] = eLoginSucceed;
	mResultData["DBID"] = (int)re["DBID"];
	mResultData["PHOTO_MD5"] = dataRe["PHOTO_MD5"].string();

	AutoNice md5Nice = (tNiceData*)dataRe["SYNC_DATA_MD5"];
	if (!md5Nice)
	{
		md5Nice = MEM_NEW NiceData();
		dataRe->set("SYNC_DATA_MD5", md5Nice);
	}
	mResultData["SYNC_MD5_NICE"]  = md5Nice.getPtr();

	return eNoneError;
}

int f_check_login_state::Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData)
{
	uint loginTime = (uint)(int)paramData["LOGIN_TIME"];

	AutoTable t = pDBNode->GetTable(szTable);
	if (!t)
		return eTableNoExist;		

	ARecord re = t->GetRecord(recordKey);
	if (!re)
	{
		mResultData["RESULT"] = eAccountNoExist;
		return eNoneError;
	}

	uint nowTime = (uint)(int)re["LOGIN_TIME"];
	if (loginTime!=nowTime)
	{
		mResultData["RESULT"] = eCheckLoginTimeFail;
		return eNoneError;
	}

	UInt64 now = TimeManager::Now();
	if (nowTime+LOGIN_STATE_OVER_TIME <now)
	{
		mResultData["RESULT"] = eLoginStateOverTime;
		return eNoneError;
	}

	// 更新登陆时间, 并把登陆时间,保存在cooki, 用于每个请求的验证
	re["LOGIN_TIME"] = (int)now;
	mResultData["LOGIN_TIME"] = (int)now;
	mResultData["RESULT"] = eCheckStateSuceed;

	return eNoneError;
}

int f_get_beta_test_diamond::Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData)
{
	AutoTable buyDiamondTable = pDBNode->GetTable("t_buydiamond");
	if (!buyDiamondTable)
	{
		// 充值返钻表不存在
		return eTableNoExist;
	}

	// ??不删档测试需要改成双倍返钻
	AString account = paramData["ACCOUNT"];
	AutoRecord buyDiamondRecord = buyDiamondTable->GetRecord(account.c_str());
	if (buyDiamondRecord)
	{
		int buyDiamond = buyDiamondRecord["DIAMOND"];
		if (buyDiamond > 0)
		{
			mResultData["DIAMOND"] = buyDiamond;
			buyDiamondRecord["DIAMOND"] = 0;
		}
	}
	return eNoneError;
}

int f_use_cdkey::Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData)
{
	AutoTable cdKeyTable = pDBNode->GetTable(CD_KEY_TABLE);
	if (!cdKeyTable)
	{
		return eTableNoExist;
	}

	AutoIndex cdKeyIndex = cdKeyTable->GetIndex("CDKEY");
	if (!cdKeyIndex)
	{
		return eTableIndexNull;
	}

	AutoRecord cdKeyRecord = cdKeyIndex->GetRecord(paramData["CDKEY"].string());
	if (!cdKeyRecord)
	{
		// 无效的CDKEY
		mResultData["RESULT"] = (int)ERROR_INVALID_CDKEY;
		return eNoneError;
	}

	int playerID = cdKeyRecord["USER_ID"];
	if (playerID > 10000000)
	{
		// 已经被使用过的CDKEY
		mResultData["RESULT"] = (int)ERROR_CDKEY_ALREADY_USED;
		return eNoneError;
	}

	int expireTime = cdKeyRecord["EXPIRE_TIME"];
	if (expireTime < (int)TimeManager::Now())
	{
		// CDKEY已过时
		mResultData["RESULT"] = (int)ERROR_CDKEY_TIME_OUT;
		return eNoneError;
	}

	int maxPlayer = cdKeyRecord["MAX_PLAYER"];
	if (maxPlayer > 1)
	{
		// 多人使用的CDKEY不检查类型
		AutoTable userTable = cdKeyRecord->GetFieldTable("USER_LIST");
		AutoRecord userRecord = userTable->GetRecord((int)paramData["PLAYER_ID"]);
		if (userRecord)
		{
			mResultData["RESULT"] = (int)ERROR_CDKEY_ALREADY_USED;
			return eNoneError;
		}

		int userNum = userTable->GetRecordCount();
		if (userNum >= maxPlayer)
		{
			mResultData["RESULT"] = (int)ERROR_CDKEY_MAX_NUM;
			return eNoneError;
		}

		userRecord = userTable->CreateRecord((int)paramData["PLAYER_ID"], false);
		userRecord["USE_TIME"] = (int)TimeManager::Now();
	}
	else
	{
		AutoIndex userIndex = cdKeyTable->GetIndex("USER_ID");
		if (!userIndex)
		{
			return eTableIndexNull;
		}

		AutoRecord myUsedRecord = userIndex->GetRecord((int)paramData["PLAYER_ID"]);
		if (myUsedRecord)
		{
			// 已经使用了同样类型的CDKEY
			mResultData["RESULT"] = (int)ERROR_CDKEY_SAME_TYPE;
			return eNoneError;
		}

		cdKeyRecord["USER_ID"] = (UInt64)paramData["PLAYER_ID"];
		cdKeyRecord["USE_TIME"] = (int)TimeManager::Now();
	}
	mResultData["RESULT"] = (int)ERROR_NONE;
	return eNoneError;
}

AString f_create_cdkey::genCDKeyString(char * buff, char c1, int n)
{
	char metachar[] = "abcdefghijklmnopqrstuvwxyz0123456789";
	buff[0] = c1;
	for (int i = 1; i < n - 1; i++)
	{
		buff[i] = metachar[rand() % 36]; 
	}
	buff[n - 1] = '\0';
	return buff;
}

int f_create_cdkey::Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData)
{
	AutoTable keyTable = pDBNode->GetTable(szTable);
	if (!keyTable)
		return eTableNoExist;

	UInt64 now = TimeManager::NowTick();
	int cdkeyDay = paramData["TIME"];
	int startNum = paramData["START_NUM"];
	int maxNum = paramData["NUM"];
	int maxPlayer = paramData["MAX_PLAYER"];
	NOTE_LOG("WARN: DB Need init create cdkey record, count >[%d], may be use long time, now waiting ...", maxNum);
	int expireTime = (int)(TimeManager::Now() + 86400 * cdkeyDay);
	{
		srand((int)time(NULL));
		char r[10];
		for (int i = 0; i < maxNum; i ++)
		{
			AString str = genCDKeyString(r, '0'+startNum, 9);
			AutoRecord re = keyTable->GrowthNewRecord(NULL);
			re[1] = str;
			re[2] = (int)re->getIndexData();
			re[4] = expireTime;
			re[5] = maxPlayer;
		}
		NOTE_LOG("NOTE: Cdkey finish [%d]", maxNum);
	}
	NOTE_LOG("Finish init create cdkey, use time [%u] ms", TimeManager::NowTick()-now);
	mResultData["RESULT"] = ERROR_NONE;
	return eNoneError;
}

//-------------------------------------------------------------------------*/
AccountThread* tLogicBaseMsg::GetThread(MemoryDBNode *pNode)
{
	return dynamic_cast<AccountDBNode*>(pNode)->mpLogicDBThread;
}

void tLogicBaseMsg::BroadCastMsg(MemoryDBNode *pNode, AutoEvent evt)
{
	Hand<IOCPServerNet> net = pNode->GetDBServerNet();
	ConnectList &connList = net->GetConnectList();
	for (int i=0; i<connList.size(); ++i)
	{
		if (connList[i])
			connList[i]->SendEvent(evt.getPtr());
	}
}
/*/-------------------------------------------------------------------------/
NOTE: 如果ServerID为0 表示只提供全局分区信息(无GS)
 ??? 同步游戏区信息优化: 可只广播到ServerID为0的Login (前端从分区Login获取分区信息时,必须从AG临时获取)
//-------------------------------------------------------------------------*/
int e_notify_serverinfo::Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData)
{
	AString name = paramData["SERVER_NAME"];
	int nID = paramData["ID"];
	AString lgIp = paramData["LG_IP"];
	int gsCount = paramData["GS_COUNT"];
	int onlineCount = paramData["ONLINE"];
	AString ver = paramData["VER"];
	AString dns = paramData["LG_DNS"];
	int state = paramData["STATE"];
	UInt64 mainNodeNetKey = paramData["MAIN_NODE_NETKEY"];
	if (mainNodeNetKey<=0)
	{
		ERROR_LOG("e_notify_serverinfo 未提供 MAIN_NODE_NETKEY");
		mResultData["ERROR"] = "e_notify_serverinfo 未提供 MAIN_NODE_NETKEY";
		return eNoneError;
	}

	Hand<AccountDBNode::DBConnect> conn = mRequstConnect;
	if (!conn)
	{
		ERROR_LOG("e_notify_serverinfo 只能由LG 执行 DBConnect");
		mResultData["ERROR"] = "e_notify_serverinfo 只能由LG 执行 DBConnect";
		return eNoneError;
	}

	// 查找存在的连接是不是请求的连接，如果不是一样报告重复
	HandConnect existConn = GetThread(pDBNode)->GetServerLGConnectByIp(nID, lgIp);
	if (existConn && existConn!=conn)
	{
		AString info;
		info.Format("已经存在相同区<%d>且相同登陆IP %s  >%s", nID, ServerIPInfo::GetAddrInfo(TOUINT64(lgIp.c_str())).c_str(), AString::getANIS(name.c_str()).c_str() );
		ERROR_LOG(info.c_str());
		mResultData["ERROR"] = info;
		return eNoneError;
	}

	conn->mServerID = nID;
	conn->mLoginServerIP = lgIp;
	conn->mConnectType = eLGConnect;
	conn->mLGVersionFlag = paramData["VER_FLAG"].string();

	if (nID<=0)
	{
		mResultData["RESULT"] = true;
		return eNoneError;
	}
	if (GetThread(pDBNode)->ExistLoginNode(nID, mainNodeNetKey))
	{
		AString info;
		info.Format("已经存在游戏分区 <%d>%s", nID, AString::getANIS(name.c_str()).c_str());		
		ERROR_LOG(info.c_str());
		mResultData["ERROR"] = info;
		return eNoneError;
	}
	conn->mLgMainNodeNetKey = mainNodeNetKey;

	bool bModify = false;

	AutoTable serverTable = GetThread(pDBNode)->mServerInfoTable;

	for (TableIt tIt(serverTable); tIt; ++tIt)
	{
		ARecord r = tIt.getCurrRecord();
		if (r)
		{
			if ( (int)r[0]!=nID )
			{
				AutoNice loginIPList = (tNiceData*)r["LG_IP"];
				if (loginIPList)
				{
					NiceIt ipIt = loginIPList->begin();
					for (; ipIt->have(); ipIt->next())
					{
						if (ipIt->key()==lgIp)
						{
							AString info;
							info.Format("已经存在登陆IP <%d>%s, 新建游戏区失败<%d>%s", (int)r[0], ServerIPInfo::GetAddrInfo(TOUINT64(lgIp.c_str())).c_str(), nID, AString::getANIS(name.c_str()).c_str() );
							ERROR_LOG(info.c_str());
							mResultData["ERROR"] = info;
							return eNoneError;
						}
					}
				}
			}
		}
	}
	ARecord areaInfo = serverTable->GetRecord(nID);
	if (!areaInfo)
	{
		areaInfo = serverTable->CreateRecord(nID, true);
		areaInfo["SERVER_NAME"] = name;
		
		bModify = true;
	}
	else if (name!=areaInfo["SERVER_NAME"].string())
	{
		NOTE_LOG("警告: [%d]服务区名称不一至 now [%s] 当前修改为 [%s]", nID, AString::getANIS(name.c_str()).c_str(), AString::getANIS(areaInfo["SERVER_NAME"].string().c_str()).c_str());
		areaInfo["SERVER_NAME"] = name;
		bModify = true;
	}
	areaInfo["GS_COUNT"] = gsCount;
	areaInfo["ONLINE"] = onlineCount;
	areaInfo["VER"] = ver;
	areaInfo["STATE"] = state;
	AutoNice d = (tNiceData*)areaInfo["LG_IP"];
	if (!d)
	{
		d = MEM_NEW NiceData();
		areaInfo["LG_IP"] = d.getPtr();
	}

	//??? NOTE: 支持域名, 值表示域名,  正式封测时使用
	if (!d->exist(lgIp.c_str()))	
	{
		d[lgIp.c_str()] = dns;
		bModify = true;
		NOTE_LOG("Append [%d] %s %s [%d:%d] DNS %s", nID, AString::getANIS(name.c_str()).c_str(), ServerIPInfo::GetAddrInfo(TOUINT64(lgIp.c_str())).c_str(), gsCount, onlineCount, dns.c_str());
	}

	if (bModify)
	{
		GetThread(pDBNode)->SyncServerInfoToLG();
	}
	mResultData["RESULT"] = true;
	return eNoneError;
}

int e_notify_gs_count::Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData)
{
	int id = paramData["SERVER_ID"];
	int count = paramData["GS_COUNT"];
	ARecord serverInfo = GetThread(pDBNode)->mServerInfoTable->GetRecord(id);
	if (serverInfo)
	{
		serverInfo["GS_COUNT"] = count;
		GetThread(pDBNode)->SyncServerInfoToLG();
	}
	return eNoneError;
}

int e_notify_online_count::Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData)
{
	int id = paramData["SERVER_ID"];
	int count = paramData["ONLINE"];
	ARecord serverInfo = GetThread(pDBNode)->mServerInfoTable->GetRecord(id);
	if (serverInfo)
	{
		serverInfo["ONLINE"] = count;			
	}
	return eNoneError;
}

int f_sdk_try_create_account::Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData)
{
	AString account = paramData["ACCOUNT"];

	if (account=="")
	{
		mResultData["RESULT"] = eAccountIsNull;
		return eNoneError;
	}

	AutoTable t = pDBNode->GetTable(szTable);
	if (!t)
		return eTableNoExist;		


	if (t->GetRecord(account))
	{
		mResultData["RESULT"] = eAccountExist;
		return eNoneError;
	}
	ARecord re = t->CreateRecord(account, true);

	re["PASSWORD"] = "SDK";		
	re["DATA_TIME"] = TimeManager::ToDataTime(TimeManager::Now());
	re["REGI_TIME"] = (int)TimeManager::Now();
	re["PHONE"] = paramData["PHONE"].string();
	re["VERSION"] = paramData["VERSION"].string();
	re["IP"] = paramData["IP"].string();
	re["PLATFORM"] = paramData["PLATFORM"].string();
	mResultData["RESULT"] = eCreateAccountSucceed;

	return eNoneError;
}

int f_web_pay_event::Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData)
{
	// 需要中转 AG>LG>GS>DB>GS, 回复 GS>DB>GS>LG>AG
	int serverID = paramData["extras_params"];
	HandConnect conn = dynamic_cast<AccountDBNode*>(pDBNode)->mpLogicDBThread->GetServerLGConnect(serverID);
	if (conn)
	{
		static AutoData d = MEM_NEW DataBuffer(256);
		d->clear(false);
		if (paramData->serialize(d.getPtr(), false))
		{		
			mParamData = paramData;
			mPayTable = pDBNode->GetTable(PAY_SUCCEED_ORDER_TABLE);
			Hand<AL_RequestPayEvent> evt = conn->StartEvent("AL_RequestPayEvent");
			evt["DATA"] = d.getPtr();
			evt->mWaitOperate = GetSelf();
			evt->Start();
			evt->WaitTime(30);
			return eWaitFinish;
		}
		else
		{
			ERROR_LOG("序列保存参数失败 %s", paramData->dump());
		}
	}
	else
		ERROR_LOG("获取服务分区LG连接失败 >[%d]", serverID);
	mResultData["RESULT"] = false;
	return eNoneError;
}

bool f_web_pay_event::_OnEvent(AutoEvent &evt)
{
	if (!evt)
	{
		mResultData["RESULT"] = false;
		Finish(eNoneError);
		return true;
	}
	//NOTE_LOG("接收到 AL_RequestPayEvent 回复 %s", evt->GetData().dump().c_str());
	AutoNice res = (tNiceData*)evt["RESP"];
	//if (res)
	//	res->dump();
	mResultData["RESP"] = res.getPtr();

	// 记录到支付成功DB表
	if (mPayTable && res && (int)res["RESULT"]==ePaySucceed)
	{
		int serverID = mParamData["extras_params"];
		Int64 code = mParamData["game_order"];		
		AString sdkOlder = mParamData["order_no"];
		float amount = (float)mParamData["amount"];
		
		ARecord payRecord = mPayTable->GrowthNewRecord(NULL);
		if (payRecord)
		{
			payRecord["SERVER_ID"] = serverID;
			payRecord["ORDER"] = code;
			payRecord["SDK_ORDER"] = sdkOlder;
			payRecord["AMOUNT"] = amount;
			payRecord["TIME"] = (int)TimeManager::Now();
		}
	}

	Finish(eNoneError);
	return true;
}

int e_notify_version_changed::Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData)
{
	int nID = paramData["SERVER_ID"];
	AString ver = paramData["VER"];

	AutoTable serverTable = GetThread(pDBNode)->mServerInfoTable;
	ARecord areaInfo = serverTable->GetRecord(nID);
	if (areaInfo)
	{	
		areaInfo["VER"] = ver;
		GetThread(pDBNode)->SyncServerInfoToLG();
	}
	else
		ERROR_LOG("未找到需要更新版本的服务区信息");

	return eNoneError;
}

int f_update_server_run_state::Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData)
{
	AutoTable serverTable = GetThread(pDBNode)->mServerInfoTable;
	AssertEx(serverTable, "服务区信息表格应该存在");

	int nID = paramData["SERVER_ID"];
	int state = paramData["STATE"];

	ARecord areaInfo = serverTable->GetRecord(nID);
	if (areaInfo)
	{	
		areaInfo["STATE"] = state;
		GetThread(pDBNode)->SyncServerInfoToLG();
	}
	else
		ERROR_LOG("未找到需要更新版本的服务区信息");	

	return eNoneError;
}

int e_request_modify_server_state::Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData)
{
	int serverID = paramData["SERVER_ID"];
	int state = paramData["STATE"];
	HandConnect conn = GetThread(pDBNode)->GetServerLGConnect(serverID);
	if (conn)
	{
		AutoEvent evt = GetThread(pDBNode)->GetMainEventCenter()->StartDefaultEvent("AL_RequestModifyServerShowState");
		evt["STATE"] = state;
		if (conn->SendEvent(evt.getPtr()))
			mResultData["RESULT"] = true;
		else
			mResultData["ERROR"] = "发送LG请求修改显示信息失败";
	}
	else
	{
		mResultData["ERROR"] = "需要修改的游戏区LG连接不存在 [%d]";
	}
	return eNoneError;
}

int f_request_total_pay_amount::Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData)
{
	AutoTable payTable = pDBNode->GetTable(PAY_SUCCEED_ORDER_TABLE);
	if (!payTable)
		return eTableNoExist;
	int col = payTable->GetField()->getFieldCol("AMOUNT");
	if (col<0)
		return eFieldNoExist;

	double amount = 0;
	for (ARecordIt it = payTable->GetRecordIt(); *it; ++(*it))
	{
		ARecord re = it->GetRecord();
		if (re)
			amount += (float)re[col];
	}
	mResultData["RESULT"] = (int)amount;

	return eNoneError;
}

int f_update_anno_info::Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData)
{
	AutoData d = (DataStream*)paramData["ANNO_DATA"];
	if (d)
	{
		GetThread(pDBNode)->SyncAnnoInfoToLG(d);		
	}
	return eNoneError;
}

int f_request_res_fore_net_key::Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData)
{
	int serverID = paramData["SERVER_ID"];
	if (serverID<=0)
	{
		mResultData["RESULT"] = "服务区号为0";
		return eNoneError;
	}
	HandConnect conn = GetThread(pDBNode)->GetServerLGConnect(serverID);
	if (conn)
	{
		Hand<AL_RequestResForeNetKey> evt = conn->StartEvent("AL_RequestResForeNetKey");		
		evt->mWaitOperate = GetSelf();
		evt->Start();
		evt->WaitTime(30);
		return eWaitFinish;
	}

	mResultData["RESULT"] = "AG未找到服务区的LG连接";
	return eNoneError;
}

bool f_request_res_fore_net_key::_OnEvent(AutoEvent &evt)
{
	if (evt)
	{
		mResultData["FORE_NET_KEY"] = evt["FORE_NET_KEY"].string();
	}
	Finish(eNoneError);
	return true;
}

int f_request_all_login_gs_ip::Execute( MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData )
{
	mpNode = pDBNode;
	Hand<IOCPServerNet> net = GetThread(pDBNode)->GetDB()->GetDBServerNet();
	ConnectList &connList = net->GetConnectList();

	for (int i=0; i<connList.size(); ++i)
	{
		Hand<AccountDBNode::DBConnect> conn = connList[i];
		if (conn && conn->mConnectType==eLGConnect)
		{
			Hand<AL_RequestAllLoginGSIP> evt = conn->StartEvent("AL_RequestAllLoginGSIP");		
			evt->mWaitOperate = GetSelf();
			mRequestList.push_back(evt);
			evt->Start();
			evt->WaitTime(10);
		}
	}
	if (mRequestList.empty())
	{
		mResultData["RESULT"] = "AG未找到服务区的LG连接";
		return eNoneError;
	}
	WaitTime(20);
	return eWaitFinish;
}

void f_request_all_login_gs_ip::ResponseFinish()
{
	NOTE_LOG("Now finish");
	AutoTable infoTable;
	DataBuffer d(128);
	for (int i=0; i<mRequestList.size(); ++i)
	{
		Hand<AL_RequestAllLoginGSIP> evt = mRequestList[i];
		if (evt && evt->mResponeEvent)
		{
			if (!infoTable)
				infoTable = (tBaseTable*)evt->mResponeEvent["INFO"];
			else
			{
				AutoTable t = (tBaseTable*)evt->mResponeEvent["INFO"];
				for (TableIt tIt(t); tIt; ++tIt)
				{
					ARecord r = tIt.getCurrRecord();
					d.clear(false);
					if (r->saveData(&d))
					{
						d.seek(0);
						if (!infoTable->GrowthNewRecord(&d))
						{
							mResultData["RESULT"] = "Append ip record fail";
							ERROR_LOG("Append ip record fail");
						}
					}
					else
					{
						mResultData["RESULT"] = "Save ip record data fail";
						ERROR_LOG("Save ip record data fail");
					}
				}
			}
		}
		evt._free();
	}
	if (infoTable)
	{
		// 加上LG
		for (TableIt tIt(GetThread(mpNode)->mServerInfoTable); tIt; ++tIt)
		{
			ARecord re = tIt.getCurrRecord();
			if (re)
			{
				//AString name = re["SERVER_NAME"];
				AString ver;
				int id = re["ID"];
				Hand<AccountDBNode::DBConnect> conn = GetThread(mpNode)->GetServerLGConnect(id);
				if (conn)
					ver = conn->mLGVersionFlag;
				AutoNice ipInfo = (tNiceData*)re["LG_IP"];
				if (ipInfo && !ipInfo->empty())
				{
					for (auto it = ipInfo->begin(); it->have(); it->next())
					{
						UInt64 ipValue = TOUINT64(it->key().c_str());
						ARecord r = infoTable->GrowthNewRecord(NULL);
						r["SERVER_ID"] = id;
						r["IP"] = ipValue;
						r["TYPE"] = LOGIN_IP_LG;
						r["VER"] = ver;
					}
				}
			}
		}
		//infoTable->SaveCSV("f:/dd.csv");
	}
	mResultData["INFO"] = infoTable.getPtr();
	Finish(eNoneError);
}

bool f_request_all_login_gs_ip::_OnEvent( AutoEvent &evt )
{
	for (int i=0; i<mRequestList.size(); ++i)
	{
		Hand<AL_RequestAllLoginGSIP> evt = mRequestList[i];
		if (evt && !evt->mResponeEvent)
			return true;
	}
	ResponseFinish();
	return true;
}

int f_upload_photo::Execute( MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData )
{
	AutoTable userTable = pDBNode->GetTable(szTable);
	if (!userTable)
		return eTableNoExist;

	ARecord re = userTable->GetRecord(recordKey);
	if (!re)
		return eRecordNoExist;

	AutoData d = (DataStream*)paramData["DATA"];
	if (!d)
	{
		mResultData["ERROR"] = eNullData;
		return eNoneError;
	}
	re["PHOTO"] = d.getPtr();
	re["PHOTO_MD5"] = paramData["MD5"].string();
	mResultData["RESULT"] = eOk;
	return eNoneError;
}

int f_load_photo::Execute( MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData )
{	
	AutoTable userTable = pDBNode->GetTable(szTable);
	if (!userTable)
		return eTableNoExist;

	if (paramData["SELF"])
	{
		ARecord r = userTable->GetRecord(recordKey);
		AutoData dx = (DataStream*)r["PHOTO"];
		if (!dx)
		{
			return eNoneError;
		}
		mResultData["DATA"] = dx.getPtr();
		return eNoneError;
	}
	int nowID = paramData["NOW_ID"];
	int id = TOINT(recordKey);
	RandTool<ARecord> t;
	auto lastIt = userTable->GetRecordIt();
	for (; *lastIt; ++(*lastIt))
	{
		ARecord re = lastIt->GetRecord();
		if (re)
		{
			int key = re[0];
			if (key!=nowID && key!=id)
				t.Push(re);
		}
	}
	ARecord re;
	t.RandPop(re);
	AutoData d = (DataStream*)re["PHOTO"];
	if (!d)
	{
		return eNoneError;
	}
	mResultData["DATA"] = d.getPtr();
	mResultData["NOW_ID"] = (int)re[0];
	return eNoneError;
}

int f_load_db_data::Execute( MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData )
{
	AutoTable userTable = pDBNode->GetTable(szTable);
	if (!userTable)
		return eTableNoExist;

	ARecord r = userTable->GetRecord(recordKey);
	if (!r)
		return eRecordNoExist;

	AString fieldName = paramData["FIELD"];
	AString key = paramData["KEY"];
	if (key.length()>0)
	{
		AutoNice data = (tNiceData*)r[fieldName];
		if (data)
		{			
			mResultData["VALUE"].setData(data[key]);				
		}
	}
	else
	{
		Data d = r[fieldName];
		mResultData["VALUE"].setData(d);
#if DEVELOP_MODE
		AutoNice test = (tNiceData*)d;
		if (test)
			NOTE_LOG("%s_%s data:\r\n%s", fieldName.c_str(), key.c_str(), test->dump().c_str());
#endif
	}
	NOTE_LOG("*** Response %s_%s data:\r\n%s", fieldName.c_str(), key.c_str(), mResultData->dump().c_str());
	return eNoneError;
}

int f_update_db_data::Execute( MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData )
{
	AutoTable userTable = pDBNode->GetTable(szTable);
	if (!userTable)
		return eTableNoExist;

	ARecord r = userTable->GetRecord(recordKey);
	if (!r)
		return eRecordNoExist;

	AString fieldName = paramData["FIELD"];
	AString key = paramData["KEY"];
	Data value = paramData["VALUE"];

#if DEVELOP_MODE
	AutoNice test = (tNiceData*)value;
	if (test)
		NOTE_LOG("%s_%s data:\r\n%s", fieldName.c_str(), key.c_str(), test->dump().c_str());
#endif

    bool bSave = false;
	if (key.length()>0)
	{
		AutoNice data = (tNiceData*)r[fieldName];
		if (data)
		{			
			if (data[key].setData(value))
            {
                r->NotifyChanged(fieldName.c_str());
                bSave = true;
				mResultData["RESULT"] = true;
            }
			else
				ERROR_LOG("Save field %s, key %s, value %s  fail", fieldName.c_str(), key.c_str(), value.string().c_str());
		}
		else
			ERROR_LOG("Field %s no exist in table %s", fieldName.c_str(), szTable);
	}
	else
	{
		if (r[fieldName].setData(value))
        {
            bSave = true;
			mResultData["RESULT"] = true;
        }
		else
			ERROR_LOG("Set data fail,  Field %s in table %s", fieldName.c_str(), szTable);
	}
    if (bSave)
    {
        AString md5Key;
        md5Key.Format("%s_%s", fieldName, key.c_str());
        AutoNice md5Data = (tNiceData*)r["SYNC_DATA_MD5"];
        md5Data[md5Key] = paramData["MD5"].string();
        r->NotifyChanged("SYNC_DATA_MD5");
    }
	return eNoneError;
}

int f_save_news_feed::Execute( MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData )
{
	AutoTable userTable = pDBNode->GetTable(szTable);
	if (!userTable)
		return eTableNoExist;

	ARecord r = userTable->GetRecord(recordKey);
	if (!r)
		return eRecordNoExist;

	AString text = paramData["TEXT"];
	AutoData picData = (DataStream*)paramData["PIC_DATA"];
	AutoData smallData = (DataStream*)paramData["SMALL_DATA"];

	AutoTable newsTable = r->GetFieldTable("NEWS_FEED");
	AutoRecord newsRe = newsTable->GrowthNewRecord(NULL);

	newsRe["USER"] = (int)r[0];
	newsRe["TEXT"] = text;
	newsRe["PHOTO_1"] = picData.getPtr();
	newsRe["SMALL_PHOTO_1"] = smallData.getPtr();
	newsRe["TIME"] = (int)TimeManager::Now();

	mResultData["RESULT"] = true;

	return eNoneError;
}

int f_load_news_feed::Execute( MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData )
{
	AutoTable userTable = pDBNode->GetTable(szTable);
	if (!userTable)
		return eTableNoExist;

	ARecord r = userTable->GetRecord(recordKey);
	if (!r)
		return eRecordNoExist;

	AutoTable newsTable = pDBNode->GetTable("db_t_userdata_NEWS_FEED");
	if (!newsTable)
		return eTableNoExist;

	ARecord newRe;

	int lastID = r["FLUSH_NEWS_ID"];

	int requestID = paramData["REQUEST_ID"];
	int localLastID = paramData["LAST_ID"];
	if (localLastID<lastID)
	{
		lastID = localLastID;
		 r["FLUSH_NEWS_ID"] = localLastID;
	}

	Int64 maxID = newsTable->MaxRecordIndexID();

	// 只要是大于记录的ID, 则发送最后一条, REQUEST_ID 为零,表示只尝试是否存在新的动态
	if (maxID>0 && maxID>lastID )
	{			
		newRe = newsTable->GetRecord(maxID);	
		if (newRe)
			r["FLUSH_NEWS_ID"] = maxID;
		else
			ERROR_LOG("严重错误, 最大记录未找到");		
	}
	else if (requestID>0)
	{
		newRe = newsTable->GetRecord(requestID);
		if (!newRe)
			ERROR_LOG("不存在动态记录 %d", requestID);
	}

	if (newRe)
	{
		mResultData["ID"] = (int)newRe[0];
		mResultData["SENDER"] =  (int)newRe["USER"];
		mResultData["TEXT"] = newRe["TEXT"].string();
		mResultData["PIC_DATA"] = (DataStream*)newRe["SMALL_PHOTO_1"];
		mResultData["TIME"] = (int)newRe["TIME"];
		NOTE_LOG(" >>> %d Response %d %s", (int)newRe[0], requestID, mResultData->dump().c_str());
	}
	else
	{
		mResultData["ID"] = (int)maxID;
		NOTE_LOG(" >>> %d Response NULL news feed %d", maxID, requestID);
	}

	return eNoneError;
}

int f_load_news_feed_big_pic::Execute( MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData )
{
	AutoTable userTable = pDBNode->GetTable(szTable);
	if (!userTable)
		return eTableNoExist;

	ARecord r = userTable->GetRecord(recordKey);
	if (!r)
		return eRecordNoExist;

	int index = paramData["PIC_INDEX"];
	++index;
	AString key = "PHOTO_";
	key += index;

	AutoData d = (DataStream*)r[key];
	mResultData["DATA"] = d.getPtr();

	return eNoneError;
}
