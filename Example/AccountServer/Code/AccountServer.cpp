
#include "AccountServer.h"
//#include "DBTableManager.h"
#include "TableManager.h"
#include "MySqlDBSaver.h"
#include "MemoryDBNode.h"

#include "AccountDBOperate.h"

#include "AccountDBNode.h"

#include <Windows.h>
#include "psapi.h"

#include "ResoursePack.h"
#include "ServerEvent.h"
#include "CommonDefine.h"

#include <string>
using namespace std;
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
void AccountThread::RegisterDBOperate()
{

	Hand<MemoryDBNode> dbNode = mMemoryDB;
	
	mMemoryDB->RegisterDBOperate("f_create_account", MEM_NEW EventFactory<f_create_account>());
	mMemoryDB->RegisterDBOperate("f_login_check", MEM_NEW EventFactory<f_login_check>());
	mMemoryDB->RegisterDBOperate("f_check_login_state", MEM_NEW EventFactory<f_check_login_state>());
	mMemoryDB->RegisterDBOperate("f_sdk_try_create_account", MEM_NEW EventFactory<f_sdk_try_create_account>());
	mMemoryDB->RegisterDBOperate("f_web_pay_event", MEM_NEW EventFactory<f_web_pay_event>());
	mMemoryDB->RegisterDBOperate("f_get_beta_test_diamond", MEM_NEW EventFactory<f_get_beta_test_diamond>());
	mMemoryDB->RegisterDBOperate("f_use_cdkey", MEM_NEW EventFactory<f_use_cdkey>());
	mMemoryDB->RegisterDBOperate("f_create_cdkey", MEM_NEW EventFactory<f_create_cdkey>());
	mMemoryDB->RegisterDBOperate("f_update_server_run_state", MEM_NEW EventFactory<f_update_server_run_state>());
	
	mMemoryDB->RegisterDBOperate("AL_RequestPayEvent", MEM_NEW EventFactory<AL_RequestPayEvent>());
	mMemoryDB->RegisterDBOperate("AL_RequestResForeNetKey", MEM_NEW EventFactory<AL_RequestResForeNetKey>());
	mMemoryDB->RegisterDBOperate("AL_RequestAllLoginGSIP", MEM_NEW EventFactory<AL_RequestAllLoginGSIP>());
	
	// 逻辑相关, 使用执行DB操作方式处理逻辑消息 
	mMemoryDB->RegisterDBOperate("e_notify_serverinfo", MEM_NEW EventFactory<e_notify_serverinfo>());
	mMemoryDB->RegisterDBOperate("e_notify_gs_count", MEM_NEW EventFactory<e_notify_gs_count>());
	mMemoryDB->RegisterDBOperate("e_notify_online_count", MEM_NEW EventFactory<e_notify_online_count>());
	mMemoryDB->RegisterDBOperate("e_notify_version_changed", MEM_NEW EventFactory<e_notify_version_changed>());
	mMemoryDB->RegisterDBOperate("e_request_modify_server_state", MEM_NEW EventFactory<e_request_modify_server_state>());
	mMemoryDB->RegisterDBOperate("f_request_total_pay_amount", MEM_NEW EventFactory<f_request_total_pay_amount>());
	mMemoryDB->RegisterDBOperate("f_update_anno_info", MEM_NEW EventFactory<f_update_anno_info>());
	mMemoryDB->RegisterDBOperate("f_request_res_fore_net_key", MEM_NEW EventFactory<f_request_res_fore_net_key>());
	mMemoryDB->RegisterDBOperate("f_request_all_login_gs_ip", MEM_NEW EventFactory<f_request_all_login_gs_ip>());
	
	mMemoryDB->RegisterDBOperate("f_upload_photo", MEM_NEW EventFactory<f_upload_photo>());
	mMemoryDB->RegisterDBOperate("f_load_photo", MEM_NEW EventFactory<f_load_photo>());

	mMemoryDB->RegisterDBOperate("f_load_db_data", MEM_NEW EventFactory<f_load_db_data>());
	mMemoryDB->RegisterDBOperate("f_update_db_data", MEM_NEW EventFactory<f_update_db_data>());

	mMemoryDB->RegisterDBOperate("f_save_news_feed", MEM_NEW EventFactory<f_save_news_feed>());
	mMemoryDB->RegisterDBOperate("f_load_news_feed", MEM_NEW EventFactory<f_load_news_feed>());
	mMemoryDB->RegisterDBOperate("f_load_news_feed_big_pic", MEM_NEW EventFactory<f_load_news_feed_big_pic>());
	
}



string& StringReplace(string&   str,const   string&   old_value,const   string&   new_value) 
{
	string::size_type   pos(0);     

	if( (pos=str.find(old_value))!=string::npos   )     
		str.replace(pos,old_value.length(),new_value);   

	return str;
}

void AccountThread::SyncServerInfoToLG()
{
	static AutoData infoData = MEM_NEW DataBuffer(1024);
	infoData->clear(false);

	if (mServerInfoTable->SaveData(infoData.getPtr()))
	{
		// 压缩
		Auto<DataBuffer> d = infoData;
		AutoData zipData = MEM_NEW DataBuffer(128);
		//UInt64 now = TimeManager::NowTick();
		DSIZE zipSize = d->ZipData(zipData, sizeof(DSIZE)*2, 0, 0, MAX_SPEED);		
		if (zipSize<=0)
		{
			ERROR_LOG("压缩游戏区列表失败");
			return;
		}
		zipData->seek(0);
		zipData->write(infoData->dataSize());
		zipData->write(zipSize);
		//NOTE_LOG("SERVER List data size >[%d], Zip size [%d], use time [%llu]", infoData->dataSize(), zipData->dataSize(), TimeManager::NowTick()-now);
		AutoEvent evt = GetMainEventCenter()->StartDefaultEvent("AL_SyncServerInfo");	

		evt["INFO"] = zipData.getPtr();

		Hand<IOCPServerNet> net = GetDB()->GetDBServerNet();
		ConnectList &connList = net->GetConnectList();
		for (int i=0; i<connList.size(); ++i)
		{
			Hand<AccountDBNode::DBConnect> conn = connList[i];
			if (conn && conn->mConnectType==eLGConnect)
				connList[i]->SendEvent(evt.getPtr());
		}
	}
	else
		ERROR_LOG("游戏分区信息表格序列失败");
}

void AccountThread::SyncAnnoInfoToLG(AutoData annoData)
{
	AutoEvent evt = GetMainEventCenter()->StartDefaultEvent("AL_RequestUpdateAnno");
	evt["ANNO_DATA"] = annoData.getPtr();

	Hand<IOCPServerNet> net = GetDB()->GetDBServerNet();
	ConnectList &connList = net->GetConnectList();
	for (int i=0; i<connList.size(); ++i)
	{
		Hand<AccountDBNode::DBConnect> conn = connList[i];
		if (conn && conn->mConnectType==eLGConnect)
			connList[i]->SendEvent(evt.getPtr());
	}
}

bool AccountThread::ExistLoginNode(int serverID, UInt64 mainNodeKey)
{
	Hand<IOCPServerNet> net = GetDB()->GetDBServerNet();
	ConnectList &connList = net->GetConnectList();
	for (int i=0; i<connList.size(); ++i)
	{
		Hand<AccountDBNode::DBConnect> conn = connList[i];
		if (conn && conn->mConnectType==eLGConnect && conn->mServerID==serverID && conn->mLgMainNodeNetKey!=0 && conn->mLgMainNodeNetKey!=mainNodeKey)
			return true;
	}
	return false;
}

HandConnect AccountThread::GetServerLGConnect(int serverID)
{
	Hand<IOCPServerNet> net = GetDB()->GetDBServerNet();
	ConnectList &connList = net->GetConnectList();
	for (int i=0; i<connList.size(); ++i)
	{
		Hand<AccountDBNode::DBConnect> conn = connList[i];
		if (conn && conn->mConnectType==eLGConnect && conn->mServerID==serverID)
			return conn;
	}
	return HandConnect();
}

//-------------------------------------------------------------------------


//-------------------------------------------------------------------------*/
AccountThread::AccountThread( const char *threadIndexName ) 
	: BaseThread(threadIndexName)
{
	mServerInfoTable = tBaseTable::NewBaseTable();
	mServerInfoTable->AppendField("ID", FIELD_INT);			// 服务区编号
	mServerInfoTable->AppendField("SERVER_NAME", FIELD_STRING);
	mServerInfoTable->AppendField("LG_IP", FIELD_NICEDATA);	
	mServerInfoTable->AppendField("GS_COUNT", FIELD_SHORT);
	mServerInfoTable->AppendField("ONLINE", FIELD_SHORT);
	mServerInfoTable->AppendField("VER",  FIELD_STRING);
	mServerInfoTable->AppendField("STATE",  FIELD_BYTE);
}

 
void RegisterGSNetEvent(AutoEventCenter center)
{
	//center->RegisterEvent("GD_NotifyReloadConfigData", MEM_NEW EventFactory<GD_NotifyReloadConfigData_R>());
}


void AccountThread::OnStart( void* )
{
	AutoTable config = TableManager::getSingleton().GetTable("DBConfig");
	AssertEx(config, "DB配置表格[DBConfig]不存在, 或加载失败");
	//config->SaveCSV("d:/db_config.csv");

	AutoRecord dbConfig = config->GetRecord("DBNode");
	AString szNodeIP = dbConfig->getData("STRING").string();
	int nodePort = dbConfig->getData("VALUE");

	AccountDBNode *pDBNode = MEM_NEW AccountDBNode(this, szNodeIP.c_str(), nodePort);
	//Hand<tBaseEventNet> eventNet = pDBNode->GetNodeNet();

	mMemoryDB = pDBNode;
	Hand<tBaseEventNet> net = mMemoryDB->GetDBServerNet();

	RegisterGSNetEvent(net->GetEventCenter());	

	RegisterDBOperate();	

	mDBStartTime = TimeManager::Now();

	AutoRecord re = config->GetRecord("DataDB");
	NiceData initDBParam;
	initDBParam.set(DBIP, re->getData("STRING").string());
	initDBParam.set(DBPORT, (int)re->getData("VALUE"));
	initDBParam.set(DBUSER, re->getData("STRING2"));
	initDBParam.set(DBPASSWORD, re->getData("STRING3"));
	initDBParam.set(DBBASE, re->getData("STRING4"));
	initDBParam.set(COOLTIME, (int)re->getData("VALUE2"));
	

	AutoRecord serverDBConfig = config->GetRecord("DBServer");
	initDBParam.set("DBSERVER_IP", serverDBConfig->get("STRING"));
	initDBParam.set("DBSERVER_PORT", serverDBConfig->get("VALUE"));
	initDBParam.set(ISLOCALDB, (AString)serverDBConfig["STRING3"]=="LOCAL_DB");

	AString szList = serverDBConfig->get("STRING2").string();
	initDBParam.set("TABLE_LIST", serverDBConfig->get("STRING2").string());

	AutoRecord mainDB = config->GetRecord("MainDBNode");
	initDBParam.set("MAIN_IP", mainDB->getData("STRING"));
	initDBParam.set("MAIN_PORT", mainDB->getData("VALUE"));

	AutoRecord backConfig = config->GetRecord("DBBack");
	AutoRecord backInfo = config->GetRecord("BackDataDB");
	if (backConfig && backInfo)
	{
		initDBParam.set("BACK_IP", backConfig->get("STRING").string());
		initDBParam.set("BACK_PORT", (int)backConfig->get("VALUE"));

		AutoNice backDB = MEM_NEW NiceData();
		backDB->set(DBIP, backInfo->get("STRING").string());
		backDB->set(DBPORT, (int)backInfo->getData("VALUE"));
		backDB->set(DBUSER, backInfo->getData("STRING2"));
		backDB->set(DBPASSWORD, backInfo->getData("STRING3"));
		backDB->set(DBBASE, backInfo->getData("STRING4"));

		backDB->set(DBNAME, serverDBConfig->get("STRING2").string());

		initDBParam.set("BACK_INFO", backDB);
	}

	Log(initDBParam.dump().c_str()); 

	if (mMemoryDB->InitDB(initDBParam)) 
	{
		PROCESS_MEMORY_COUNTERS memoryUseInfo;
		// 内存使用分析
		GetProcessMemoryInfo(
			GetCurrentProcess(),
			&memoryUseInfo,
			sizeof(memoryUseInfo)
			);

		MEMORYSTATUS sysMemory;
		::GlobalMemoryStatus(&sysMemory);	

		LOG_YELLOW;
		TABLE_LOG("当前内存使用[%.3f]M, 可用系统内存 [%.3f]M", 
			(float)memoryUseInfo.WorkingSetSize/1024/1024, 
			(float)sysMemory.dwAvailPhys/1024/1024
			);

		LOG_GREEN;
		//printf("\r\n");
		NOTE_LOG("\r\n------------------------------------------------------------------"
		"\r\nDB 启动成功\r\n"
		"------------------------------------------------------------------");		

		bool bNeedCreateDB = false;
		if (dbConfig["STRING2"].string()=="YES")
		{					
			if (mMemoryDB->GetTable(DB_ACCOUNT_TABLE))
				ERROR_LOG("配置需要重建DB表格, 但当前存在表格[%s]", DB_ACCOUNT_TABLE)			
			else
				bNeedCreateDB = true;
		}
		else
		{
			if (!mMemoryDB->GetTable(DB_ACCOUNT_TABLE))
			{
				bNeedCreateDB = true;
				ERROR_LOG("不存在DB %s, 尝试新建", DB_ACCOUNT_TABLE)
			}
		}

		if (bNeedCreateDB)
			{
				// 创建 帐号DB 表
				ABaseTable accountTable = mMemoryDB->CreateNewDBTable(DB_ACCOUNT_TABLE, "MemoryTable");				
				accountTable->AppendField("ACCOUNT", FIELD_STRING);
				accountTable->AppendField("PASS", FIELD_STRING);
				accountTable->AppendField("PASS_MD5", FIELD_STRING);				
				accountTable->AppendField("REGISTER_TIME", FIELD_INT);
				accountTable->AppendField("LOGIN_TIME", FIELD_INT);
				accountTable->AppendField("DBID", FIELD_INT);

				_NewCreateDBTable(DB_ACCOUNT_TABLE, accountTable);

				// 创建用户数据DB表, 帐号索引
				ABaseTable userTable = mMemoryDB->CreateNewDBTable(DB_USER_TABLE, "MemoryTable");
				userTable->AppendField("ID", FIELD_INT);
				userTable->AppendField("ACCOUNT", FIELD_STRING);

				// 资料数据				
				userTable->AppendField("NAME", FIELD_STRING);
				userTable->AppendField("VIP", FIELD_SHORT);
				userTable->AppendField("GOLD", FIELD_INT);
				userTable->AppendField("BIRTHDAY", FIELD_INT);
				userTable->AppendField("SYNC_DATA_MD5", FIELD_NICEDATA); // 同步数据的MD5, key : 数据事件索引, value : 客户端将数据序列后转换为BASE64后生成的MD5
				userTable->AppendField("BASE_DATA", FIELD_NICEDATA);	// key : 数据名, value : 字符串数据
				userTable->AppendField("EXT_DATA", FIELD_NICEDATA);		// key : 数据名, value : NiceData 的数据 (复杂二级结构数据)
				userTable->AppendField("PHOTO", FIELD_DATA)->setMaxLength(512*1024);
				userTable->AppendField("PHOTO_MD5", FIELD_STRING);
				userTable->AppendField("PHOTO_1", FIELD_DATA)->setMaxLength(256*1024);
				userTable->AppendField("PHOTO_2", FIELD_DATA)->setMaxLength(256*1024);
				userTable->AppendField("PHOTO_3", FIELD_DATA)->setMaxLength(256*1024);
				userTable->AppendField("PHOTO_4", FIELD_DATA)->setMaxLength(256*1024);
				userTable->AppendField("PHOTO_5", FIELD_DATA)->setMaxLength(256*1024);
				userTable->AppendField("FLUSH_NEWS_ID", FIELD_INT);		// 动态刷新的最后ID, 下次刷新从此ID开始获取

				// 邮件消息
				userTable->AppendField("MAIL", FIELD_TABLE)->setAttribute(_DB_FIELD_TABLE);

				AutoTable mailTable = tBaseTable::NewBaseTable(false);
				mailTable->AppendField("ID", FIELD_INT);
				mailTable->AppendField("TYPE", FIELD_SHORT);
				mailTable->AppendField("SENDER", FIELD_INT);
				mailTable->AppendField("TITLE", FIELD_INT);
				mailTable->AppendField("TEXT", FIELD_STRING)->setMaxLength(4*1024);
				mailTable->AppendField("PHOTO", FIELD_DATA)->setMaxLength(256);
				mailTable->AppendField("TIME", FIELD_INT);
				mailTable->AppendField("IS_READ", FIELD_BOOL);

				userTable->SetFieldTable("MAIL", mailTable.getPtr());

				// 动态, 审核后, 索引到另一全局动态表格中, 刷新时从索引表中获取对应ID
				userTable->AppendField("NEWS_FEED", FIELD_TABLE)->setAttribute(_DB_FIELD_TABLE);

				AutoTable newsFeedTable = tBaseTable::NewBaseTable(false);
				newsFeedTable->AppendField("ID", FIELD_INT);
				newsFeedTable->AppendField("USER", FIELD_INT);
				newsFeedTable->AppendField("NAME", FIELD_STRING);
				newsFeedTable->AppendField("TEXT", FIELD_STRING)->setMaxLength(512);
				newsFeedTable->AppendField("TIME", FIELD_INT);
				newsFeedTable->AppendField("PHOTO_1", FIELD_DATA)->setMaxLength(256*1024);
				newsFeedTable->AppendField("PHOTO_2", FIELD_DATA)->setMaxLength(256*1024);
				newsFeedTable->AppendField("PHOTO_3", FIELD_DATA)->setMaxLength(256*1024);
				newsFeedTable->AppendField("PHOTO_4", FIELD_DATA)->setMaxLength(256*1024);
				newsFeedTable->AppendField("SMALL_PHOTO_1", FIELD_DATA);
				newsFeedTable->AppendField("SMALL_PHOTO_2", FIELD_DATA);
				newsFeedTable->AppendField("SMALL_PHOTO_3", FIELD_DATA);
				newsFeedTable->AppendField("SMALL_PHOTO_4", FIELD_DATA);

				userTable->SetFieldTable("NEWS_FEED", newsFeedTable.getPtr());
				
				_NewCreateDBTable(DB_USER_TABLE, userTable);

			}		

			OnDBStartSucceed();
	}
	else
	{
		NOTE_LOG("DB 启动失败"); 
	}
	LOG_WHITE;
}



void AccountThread::InitEventCenter( AutoEventCenter eventCenter )
{
	if (!eventCenter)
		eventCenter = MEM_NEW Logic::EventCenter();
	BaseThread::InitEventCenter(eventCenter);
}

bool AccountThread::NotifyThreadClose()
{
	if (!mMemoryDB->NowDBState())
	{	
		NOTE_LOG("严重警告:当前DB存在需要处理的操作,不允许退出");
		return false;
	}

	mMainEventCenter->Dump();
	mMemoryDB->Close();
	mMemoryDB._free();
	mMainEventCenter._free();

	mServerInfoTable._free();

	TableTool::green();
	printf("\r\nDB清理完成, 请按回车正常退出程序\r\n");
	TableTool::white();
	return BaseThread::NotifyThreadClose();
}

AccountThread::~AccountThread()
{
	mMemoryDB._free();

}

MemoryDBNode* AccountThread::GetDB()
{
	return dynamic_cast<MemoryDBNode*>(mMemoryDB.getPtr());
}

void AccountThread::_NewCreateDBTable(const char *szTableName, AutoTable table)
{
	if (!table->GetField()->check())
	{
		ERROR_LOG("[%s] 创建失败 >字段检查错误", szTableName);
	}
	else
	{				
		AString resultInfo = mMemoryDB->SaveDBTableFieldToDB(szTableName, table, NULL, AutoNice());

		if (!resultInfo.empty())
		{
			ERROR_LOG("ERROR: Create table [%s] to DB Fail >%s", szTableName, resultInfo.c_str());
		}
		ARecord infoRe = mMemoryDB->GetDBTableListTable()->GetRecord(szTableName);
		if (!infoRe)
		{
			NOTE_LOG("[%s]在DB列表中不存在, DB列表信息错误", szTableName);				
		}
		else
		{			
			AString fieldData = infoRe->get("DATA");
			mMemoryDB->InitReadyMemoryDBTable(szTableName, fieldData);
		}
	}
}

HandConnect AccountThread::GetServerLGConnectByIp(int nID, AString ip)
{
	Hand<IOCPServerNet> net = GetDB()->GetDBServerNet();
	ConnectList &connList = net->GetConnectList();
	for (int i=0; i<connList.size(); ++i)
	{
		Hand<AccountDBNode::DBConnect> conn = connList[i];
		if (conn && conn->mConnectType==eLGConnect && conn->mServerID==nID && conn->mLoginServerIP==ip)
			return conn;
	}
	return HandConnect();
}

void AccountThread::OnDBStartSucceed()
{
	mAccountTable = mMemoryDB->GetTable(DB_ACCOUNT_TABLE);
	mUserDataTable = mMemoryDB->GetTable(DB_USER_TABLE);
	mNewsFeedTable = mMemoryDB->GetTable("db_t_userdata_NEWS_FEED");
}

bool AccountServer::IsStop()
{
	return mBaseThread->mbStop;
}
