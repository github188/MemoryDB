
#include "DBNodeManager.h"
#include <stdarg.h>
#include "ServerIPInfo.h"
#include "RandTool.h"

#include "DBNetPacket.h"

//-------------------------------------------------------------------------
DBNodeManager::DBNodeManager() 	
{
	mDBClient = MEM_NEW MainNodeClient(this);
	//!!! 如果在子类构造内释放  mDBClient, 由于 mNetSucceedCallBack释放时，使用本实例计数减少到1，造成当前构造的实例立即被释放
	//mDBClient->mNetSucceedCallBack = DBCallBack(&DBNodeManager::OnConnectedMainDB, this); 
}

DBNodeManager::DBNodeManager( bool bCreateClient )
{
	if (bCreateClient)
		mDBClient = MEM_NEW MainNodeClient(this);
}

void DBNodeManager::Log( const char* szInfo, ... )
{
#if DEVELOP_MODE
	va_list va;
	va_start(va, szInfo);
	mLog.logVa(va, szInfo);
#endif
}

bool DBNodeManager::InitDB( NiceData *initParam )
{
	mDBClient->mNetSucceedCallBack = DBCallBack(&DBNodeManager::OnConnectedMainDB, this); 

	TABLE_LOG(initParam->dump().c_str());
	mDBName = initParam->get("DBNAME").string();
	if (mDBName!="")
	{
		AString fileName;
		fileName.Format("DBLog/%s_DB.log", mDBName.c_str());
		mLog.setFile(fileName.c_str(), "w");
	}

	AString ip = initParam->get(DBIP);
	Data port = initParam->get(DBPORT);

	if (ip.empty() || port.empty())
	{
		Log(initParam->dump().c_str());
		Log("ERROR: 未设定DB ip 或 port");
		return false;
	}
	InitNodeConnectNet(mDBClient->GetConnectNet().getPtr());
	return mDBClient->Start(ip.c_str(), (int)port);

}

void DBNodeManager::RegisterDBOperate( const char *szOperateType,Logic::tEventFactory *pTaskFactory )
{
	mDBClient->GetEventCenter()->RegisterEvent(szOperateType, pTaskFactory);
}

void DBNodeManager::StartLoadDBTableList( const char* listTableName, DBCallBack finishCallBack )
{
	HandDBOperate op = mDBClient->StartOperate("DB_LoadAllTableField");
	op->mCallBack = finishCallBack;
	op->DoEvent();
}

void DBNodeManager::LoadTable( const char *szTableIndex, const char* dbTableName, DBCallBack finishCallBack )
{
	Hand<DBOperate> request = mDBClient->StartOperate("DB_LoadTableField");
	request->mTableIndex = szTableIndex;
	request->mCallBack = finishCallBack;
	request->DoEvent();
}

DBOperate* DBNodeManager::GetRecord( const char* szTableName, const AString &whereCondition, DBCallBack finishCallBack )
{
	Hand<DBOperate> request = StartDBOperate("DB_FindRecord", szTableName, whereCondition.c_str());
	if (!request)
		request = mDBClient->StartOperate("DB_FindRecord");
	request->mTableIndex = szTableName;
	request->mRecordKey = whereCondition;
	request->mCallBack = finishCallBack;
	request->DoEvent();
	return request.getPtr();
}

void DBNodeManager::CreateRecord( const char* tableName, const AString &whereCondition, DBCallBack finishCallBack )
{
	ABaseTable t = mDBClient->GetTable(tableName);
	if (!t)
	{
		finishCallBack.run(NULL, false);
		return;
	}
	ARecord r;
	Hand<RequestDBSaveEvent> saveRecord;

	if (whereCondition!="")
	{		
		r = t->CreateRecord(whereCondition.c_str(), true);		
		saveRecord = StartDBOperate("RequestDBSaveEvent", tableName, whereCondition.c_str());
	}
	else
		r = t->NewRecord();
	if (!saveRecord)
		saveRecord = mDBClient->StartOperate("RequestDBSaveEvent");

	if (!saveRecord)
	{
		finishCallBack(NULL, false);
		return;
	}

	saveRecord->mbNewInsert = true;
	saveRecord->mbReplace = true;
	saveRecord->mbGrowthKey = whereCondition=="";
	saveRecord->mbNeedResponse = true;
	saveRecord->mResultRecord = r;
	saveRecord->mTableIndex = tableName;
	saveRecord->mCallBack = finishCallBack;
	saveRecord->DoEvent();

	//saveRecord->set("GROWTH_KEY", whereCondition=="");
	//saveRecord->mResultRecord = r;
	//saveRecord->mTableIndex = tableName;
	//saveRecord->mCallBack = finishCallBack;
	//saveRecord->DoEvent();
}

void DBNodeManager::DeleteRecord( const char* tableIndex, const char* szWhere, DBCallBack finishCallBack )
{
	Hand<DBOperate> request = StartDBOperate("DB_DeleteRecord", tableIndex, szWhere);
	if (!request)
		request = mDBClient->StartOperate("DB_DeleteRecord");
	request->mTableIndex = tableIndex;
	request->mRecordKey = szWhere;
	request->DoEvent();
}

void DBNodeManager::DeleteRecord( AutoRecord deleteRecord, DBCallBack finishCallBack )
{
	Hand<DBOperate> request = StartDBOperate("DB_DeleteRecord", deleteRecord->GetTable()->GetTableName(), deleteRecord->getIndexData().string());
	if (!request)
		request = mDBClient->StartOperate("DB_DeleteRecord");
	//Hand<DBOperate> request = mDBClient->StartOperate("DB_DeleteRecord");
	request->mTableIndex = deleteRecord->GetTable()->GetTableName();
	request->mRecordKey = deleteRecord->getIndexData().string();
	request->DoEvent();
}

void DBNodeManager::DeleteSubRecord( DBCallBack callBack, const char *szTable, const char *szRecordKey, const char *subTableFieldName, ... )
{
	AString info;
	va_list va;
	va_start(va, subTableFieldName);
	info.Format(va, subTableFieldName);

	AutoNice paramData = MEM_NEW NiceData();
	paramData->set("SAVE_MODE", (int)eRecordDelete);
	paramData->set("TARGET_INFO", info.c_str());

	ExeSqlFunction(callBack, szTable, szRecordKey, "ModifySubRecord", paramData);
}

void DBNodeManager::DeleteSubRecord( DBCallBack callBack, const char *szTable, UInt64 recordKey, const char *subTableFieldName, ... )
{
	AString info;
	va_list va;
	va_start(va, subTableFieldName);
	info.Format(va, subTableFieldName);

	DeleteSubRecord(callBack, szTable, STRING(recordKey), info.c_str());
}

void DBNodeManager::InsertRecord( AutoRecord hRecord, bool bReplace, bool bGrowth, DBCallBack finishCallBack )
{
	//Hand<DBOperate> saveRecord = StartDBOperate("DB_SaveRecord", hRecord->GetTable()->GetTableName(), hRecord->getIndexData().string());
	//if (!saveRecord)
	//	saveRecord = mDBClient->StartOperate("DB_SaveRecord");
	////Hand<DBOperate> saveRecord = mDBClient->StartOperate("DB_SaveRecord");
	//saveRecord->set("REPLACE", bReplace);
	//saveRecord->set("GROWTH_KEY", bGrowth);
	//saveRecord->mResultRecord = hRecord;
	//saveRecord->mTableIndex = hRecord->GetTable()->GetTableName();
	//saveRecord->mCallBack = finishCallBack;
	//saveRecord->DoEvent();

	Hand<RequestDBSaveEvent> saveRecord = StartDBOperate("RequestDBSaveEvent", hRecord->GetTable()->GetTableName(), hRecord->getIndexData().string());
	if (!saveRecord)
		saveRecord = mDBClient->StartOperate("RequestDBSaveEvent");
	if (!saveRecord)
	{
		finishCallBack(NULL, false);
		return;
	}
	//Hand<DBOperate> saveRecord = mDBClient->StartOperate("DB_SaveRecord");
	saveRecord->mbNewInsert = true;
	saveRecord->mbReplace = bReplace;
	saveRecord->mbGrowthKey = bGrowth;
	saveRecord->mbNeedResponse = true;
	saveRecord->mResultRecord = hRecord;
	saveRecord->mTableIndex = hRecord->GetTable()->GetTableName();
	saveRecord->mCallBack = finishCallBack;
	saveRecord->DoEvent();
}

void DBNodeManager::UpdateRecord( AutoRecord hRecord, DBCallBack finishCallBack )
{
	//Hand<DBOperate> request = StartDBOperate("DB_UpdateRecord", hRecord->GetTable()->GetTableName(), hRecord->getIndexData().string());
	//if (!request)
	//	request = mDBClient->StartOperate("DB_UpdateRecord");
	////Hand<DBOperate> request = mDBClient->StartOperate("DB_UpdateRecord");
	//request->mTableIndex = hRecord->GetTable()->GetTableName();
	//request->mResultRecord = hRecord;
	//request->mCallBack = finishCallBack;
	//request->DoEvent();

	Hand<RequestDBSaveEvent> saveRecord = StartDBOperate("RequestDBSaveEvent", hRecord->GetTable()->GetTableName(), hRecord->getIndexData().string());
	if (!saveRecord)
		saveRecord = mDBClient->StartOperate("RequestDBSaveEvent");
	if (!saveRecord)
	{
		finishCallBack(NULL, false);
		return;
	}
	//Hand<DBOperate> saveRecord = mDBClient->StartOperate("DB_SaveRecord");
	saveRecord->mbNewInsert = false;
	saveRecord->mbReplace = false;
	saveRecord->mbGrowthKey = false;
	saveRecord->mbNeedResponse = NeedSaveUpdateResponse();
	saveRecord->mResultRecord = hRecord;
	saveRecord->mTableIndex = hRecord->GetTable()->GetTableName();
	saveRecord->mCallBack = finishCallBack;
	saveRecord->DoEvent();
}

void DBNodeManager::LoadAllTable(DBCallBack finishCallBack)
{
	Hand<DBOperate> request = mDBClient->StartOperate("DB_LoadAllTableField");
	request->mCallBack = finishCallBack;
	request->DoEvent();
}

void DBNodeManager::SetDBEvent( Logic::tEventCenter *eventCenter, DBCallBack startCallBack, DBCallBack disconnectCallBack )
{
	mConnectedCall = startCallBack;
	
	mDBClient->mNetDisconnectCallBack = disconnectCallBack;
}

DBOperate* DBNodeManager::ExeSqlFunction( DBCallBack finishCallBack, const char *szTable, const char *szRecordKey, const char *szFunctionOprerate, AutoNice paramData )
{
	//Hand<DBOperate> op = StartDBOperate("DB_RunDBOperate", szTable, szRecordKey);
	//if (!op)
	//	op = mDBClient->StartOperate("DB_RunDBOperate");
	//op->set("OPERATE_TYPE", szFunctionOprerate);
	//op->set("PARAM", paramData);
	//op->mTableIndex = szTable;
	//op->mRecordKey = szRecordKey;
	//op->mCallBack = finishCallBack;
	//op->DoEvent();
	////AssertEx(0, "等待实现");
	//return op.getPtr();

	Hand<RequestDBOperateEvent> op = StartDBOperate("RequestDBOperateEvent", szTable, szRecordKey);
	if (!op)
		op = mDBClient->StartOperate("RequestDBOperateEvent");

	if (!op)
	{
		finishCallBack(NULL, false);
		return NULL;
	}
	op->mFunctionName = szFunctionOprerate;
	op->mParamData = paramData;
	op->mTableIndex = szTable;
	op->mRecordKey = szRecordKey;
	op->mCallBack = finishCallBack;
	op->DoEvent();

	return op.getPtr();	
}

void DBNodeManager::ModifyData(DBCallBack callBack, const char *szTable, const char *szRecordKey, eModifyDataMode eMode, const char *limit, const char *szModifyFieldName, const char *destValue, const char *szDestField, ... )
{
	Hand<DBOperate> request = StartDBOperate("DB_RequestModifyData", szTable, szRecordKey);
	if (!request)
		request = mDBClient->StartOperate("DB_RequestModifyData");
	request->mTableIndex = szTable;
	request->mRecordKey = szRecordKey;
	request->mCallBack = callBack;

	AString fieldInfo;
	va_list va;
	va_start(va, szDestField);
	fieldInfo.Format(va, szDestField);

	request->set("FIELD_KEY", fieldInfo.c_str());

	request->set("DEST_FIELD", szModifyFieldName);
	request->set("VALUE", destValue);

	request->set("MODE", (int)eMode);
	if (limit!=NULL)
		request->set("LIMIT", limit);

	request->DoEvent();
}

bool DBNodeManager::LoadRecordData( DBCallBack finishCallBack, const char *szTable, const char *szRecordKey, const char *szFieldList )
{
	Hand<DB_RequestRecordData_C> questEvt = StartDBOperate("DB_RequestRecordData", szTable, szRecordKey);
	if (!questEvt)
		questEvt = StartDBOperate("DB_RequestRecordData");

	Array<AString> field;
	AString::Split(szFieldList, field, " ", -1);
	if (field.size()<=0)
		return false;
	for (size_t i=0; i<field.size(); ++i)
	{
		questEvt->mFieldList.push_back(field[i].c_str());
	}
	questEvt->mCallBack = finishCallBack;
	questEvt->mTableIndex = szTable;
	questEvt->mRecordKey = szRecordKey;
	questEvt->DoEvent();

	return true;
}

bool DBNodeManager::LoadRecordData( DBCallBack finishCallBack, const char *szTable, const char *szRecordKey, const char *szFieldList, const char *szDestFieldInfo, ... )
{
	AString info;
	va_list va;
	va_start(va, szDestFieldInfo);
	info.Format(va, szDestFieldInfo);

	Hand<DB_RequestRecordData_C> questEvt = StartDBOperate("DB_RequestRecordData", szTable, szRecordKey);
	if (!questEvt)
		questEvt = StartDBOperate("DB_RequestRecordData");

	Array<AString> field;
	AString::Split(szFieldList, field, " ", -1);
	if (field.size()<=0)
		return false;
	for (size_t i=0; i<field.size(); ++i)
	{
		questEvt->mFieldList.push_back(field[i].c_str());
	}
	questEvt->set("TARGET_INFO", info.c_str());
	questEvt->mCallBack = finishCallBack;
	questEvt->mTableIndex = szTable;
	questEvt->mRecordKey = szRecordKey;
	questEvt->DoEvent();

	return true;
}

Hand<DBClient> DBNodeManager::_CreateDBClinet(GSKEY nodeIpPort)
{
	Hand<DBClient> clientDB = MEM_NEW DBClient();
	Hand<tBaseEventNet> net = clientDB->GetConnectNet();
	net->SetNetProtocol(mDBClient->GetConnectNet()->GetNetProtocol());

	InitNodeConnectNet(net.getPtr());

	AString ip;
	int port = 0, port2=0;
	ip = ServerIPInfo::Num2IP(nodeIpPort, port, port2);
	clientDB->Start(ip.c_str(), port);

	return clientDB;
}

//-------------------------------------------------------------------------
bool DBNodeManager::RefreshNodeDistributionData( ABaseTable keyInfoTable )
{
	ClearDistribution();
	for (ARecordIt tIt= keyInfoTable->GetRecordIt(); *tIt; ++(*tIt))
	{
		ARecord r = *tIt;
		AString tableName = r->getIndexData();
		// 将表格恢复到主节点中, 直接进行使用, 不再需要加载所有表格信息
		AutoField field = MEM_NEW BaseFieldIndex();
		if (!field->FullFromString(r->get("FIELD_DATA").string()) || !field->check())
		{
			ERROR_LOG("DB下传[%s]表格字段数据错误, 字段恢复失败, 或者检验失败", tableName.c_str());
			continue;
		}

		ABaseTable newTable = mDBClient->CreateTable(tableName.c_str());
		newTable->InitField(field);
		
		TABLE_LOG("成功接收DB表格信息>[%s]", tableName.c_str());

		AutoDBTableDistribution dist = mTableDistributionDataList.find(tableName);
		if (!dist)
		{
			dist = MEM_NEW DBTableDistribution();
			dist->mbStringKey = r->get("IS_STRING_KEY");
			mTableDistributionDataList.insert(tableName, dist);
		}
		else
		{
			if (dist->mbStringKey!=(bool)r->get("IS_STRING_KEY"))
			{
				ERROR_LOG("严重错误, 表格KEY类型不一至, 当前为 [%s] 类型", dist->mbStringKey ? "哈希字符串":"整数" );
				return false;
			}
		}
		AutoTable infoTable = r->GetFieldTable("INFO_TABLE");
		for (ARecordIt it=infoTable->GetRecordIt(); *it; ++(*it))
		{
			ARecord re = *it;

			Hand<DBClient> clientDB;

			if (re->get("IS_MAIN_NODE"))
			{
				clientDB = mDBClient;
				// 如果是当前主连接节点的表格, 忽略加入到分布信息中, 默认如果找不到分布, 会直接通过主连接操作
				//continue;
			}
			else
			{			
				UInt64 ipKey = (UInt64)re->getIndexData();
				clientDB = mDBClientList.find(ipKey);
				if (!clientDB)
				{
					clientDB = _CreateDBClinet(ipKey);
					if (!clientDB)
					{
						ERROR_LOG("严重错误, %s DBClient创建失败", ServerIPInfo::GetAddrInfo(ipKey));
						continue;
					}
					mDBClientList.insert(ipKey, clientDB);
				}
			}
			//Hand<DBClient> clientDB = MEM_NEW DBClient();
			//Hand<tBaseEventNet> net = clientDB->GetConnectNet();
			//net->SetNetProtocol(mDBClient->GetConnectNet()->GetNetProtocol());

			//InitNodeConnectNet(net.getPtr());

			//AString ip;
			//int port = 0, port2=0;
			//ip = ServerIPInfo::Num2IP((UInt64)re->getIndexData(), port, port2);
			//clientDB->Start(ip.c_str(), port);
			if (!clientDB->GetTable(tableName.c_str()) )
			{
				ABaseTable t = clientDB->CreateTable(tableName.c_str());
				t->InitField(field);
			}

			AutoData slotData = (DataStream*)re["SLOT_INFO"];
			bool bRe = dist->AppendNodeDistributionData(clientDB, slotData, clientDB==mDBClient);
			if (bRe)
			{
				bool bAllOk = dist->CheckAllHashSlotDB();
				NOTE_LOG("成功设定表格分区信息: NET: %s for table [%s] > Now slot DB [%s]", 
					ServerIPInfo::GetAddrInfo((UInt64)re->getIndexData()).c_str(),
					tableName.c_str(), 
					bAllOk ? "已完整" : "尚缺");
				//LOG_YELLOW;
				//TABLE_LOG("成功设定表格分区信息: NET: %s for table [%s] > [%s] ID Range: [%s]~[%s]", 
				//	ServerIPInfo::GetAddrInfo((UInt64)re->getIndexData()).c_str(),
				//	tableName.c_str(), 
				//	(dist->mbStringKey ? "字符哈希KEY":"整数KEY"),
				//	re->get("MIN_KEY").string().c_str(),
				//	re->get("MAX_KEY").string().c_str()
				//	);
				//LOG_WHITE;
			}
			else
			{
				ERROR_LOG("错误:设定表格分区信息失败: NET: %s for table [%s] > [%s] ", 
					ServerIPInfo::GetAddrInfo((UInt64)re->getIndexData()).c_str(),
					tableName.c_str(), 
					(dist->mbStringKey ? "字符哈希KEY":"整数KEY")
					//re->get("MIN_KEY").string().c_str(),
					//re->get("MAX_KEY").string().c_str()
					)
			}
		}		
	}
	LOG_GREEN
		NOTE_LOG("=============DB 分布检查 [%llu]=============", mTableDistributionDataList.size());
	for (auto it=mTableDistributionDataList.begin(); it; ++it)
	{
		AutoDBTableDistribution dist = it.get();
		if (dist)
		{
			bool bAllOk = dist->CheckAllHashSlotDB();
			if (bAllOk)
			{
				LOG_GREEN
				NOTE_LOG("√ [%s] DB table hash slot [完整]", it.key().c_str())
				LOG_WHITE
			}
			else
			{
				LOG_RED
				NOTE_LOG("× [%s] DB table hash slot [尚缺], DB 当前不可正常使用", it.key().c_str());
				LOG_WHITE
			}
		}
	}
	NOTE_LOG("=========================================", mTableDistributionDataList.size());
	LOG_GREEN;
	TABLE_LOG("成功刷新DB分布信息");
	LOG_WHITE;

	return true;
}

AutoEvent DBNodeManager::StartDBOperate( const char *szOperateName, const char *szTable, Int64 key )
{
	AutoDBTableDistribution keyInfo = mTableDistributionDataList.find(szTable);
	if (keyInfo)
	{
		Hand<DBClient> nodeData = keyInfo->FindNode(key);
		if (nodeData)
			return nodeData->StartOperate(szOperateName);
	}
	//TABLE_LOG("WARN: [%s] 发现不存在当前设定范围内, table [%s]", STRING(key), szTable);
	return AutoEvent();
}

AutoEvent DBNodeManager::StartDBOperate( const char *szOperateName, const char *szTable, const char *szKey )
{
	AutoDBTableDistribution keyInfo = mTableDistributionDataList.find(szTable);
	if (keyInfo)
	{
		if (szKey==NULL)
		{
			if (mDBClient)
				return mDBClient->StartOperate(szOperateName);
			else
				return AutoEvent();
		}
		Hand<DBClient> nodeData = keyInfo->FindNode(szKey);
		if (nodeData)
			return nodeData->StartOperate(szOperateName);
	}
	//TABLE_LOG("WARN: [%s] 发现不存在当前设定范围内, table [%s]", szKey, szTable);
	return AutoEvent();
}

void DBNodeManager::OnConnectedMainDB( DBOperate *op, bool bSu )
{
	if (bSu)
	{
		HandDBOperate request = mDBClient->StartOperate("DB_RequestTableDistribution");
		request->mCallBack = DBCallBack(&DBNodeManager::OnResponseDistribution, this);
		request->DoEvent();
	}
	mConnectedCall.run(NULL, bSu);
}

void DBNodeManager::OnResponseDistribution( DBOperate *op, bool bSu )
{
	if (bSu && op->mResultTable)
	{	
		RefreshNodeDistributionData(op->mResultTable);
	}
}

void DBNodeManager::Process()
{
	mDBClient->Process();
	for (auto it = mTableDistributionDataList.begin(); it.have(); it.next())
	{
		AutoDBTableDistribution tableDist = it.get();
		for(size_t n=0; n<tableDist->mDistributionList.size(); ++n)
		{
			tableDist->mDistributionList[n].mNode->Process();
		}
	}
}

void DBNodeManager::Close()
{
	mDBClient->Close();
	ClearDistribution();
}

void DBNodeManager::ClearDistribution()
{
	for (auto it = mTableDistributionDataList.begin(); it.have(); it.next())
	{
		AutoDBTableDistribution tableDist = it.get();
		//for(size_t n=0; n<tableDist->mDistributionList.size(); ++n)
		//{
		//	tableDist->mDistributionList[n].mNode->Close();
		//}
		tableDist->mDistributionList.clear();
	}
	mTableDistributionDataList.clear();

	for (auto it=mDBClientList.begin(); it; ++it)
	{
		it.get()->Close();
	}
	mDBClientList.clear(false);
}

void DBNodeManager::OnNotifyDistribution( AutoTable distTable )
{
	if (distTable)
		RefreshNodeDistributionData(distTable);
}

void DBNodeManager::RecordOperate( DBCallBack callBack, eRecordOperateMode mode, const char *szTable, const char *szRecordKey, AutoRecord dataRecord, const char *subTableFieldName, ... )
{
	AString info;
	va_list va;
	va_start(va, subTableFieldName);
	info.Format(va, subTableFieldName);

	AutoNice paramData = MEM_NEW NiceData();
	paramData->set("SAVE_MODE", (int)mode);
	paramData->set("TARGET_INFO", info.c_str());

	if (mode!=eRecordDelete && dataRecord)
	{
		AutoData recordData = MEM_NEW SaveRecordDataBuffer();
		bool b = false;
		if (mode==eRecordUpdate)
		{
			dataRecord->Update();
			b = dataRecord->saveUpdateData(recordData.getPtr());
		}
		else
			b = dataRecord->saveData(recordData.getPtr());

		AssertEx(b, "记录保存到数据失败");

		paramData->set("RECORD_DATA", recordData);
	}
	ExeSqlFunction(callBack, szTable, szRecordKey, "ModifySubRecord", paramData);
}

void DBNodeManager::RecordOperate( DBCallBack callBack, eRecordOperateMode mode, const char *szTable, UInt64 recordKey, AutoRecord dataRecord, const char *subTableFieldName, ... )
{
	AString info;
	va_list va;
	va_start(va, subTableFieldName);
	info.Format(va, subTableFieldName);
	RecordOperate(callBack, mode, szTable, STRING(recordKey), dataRecord, info.c_str());
}

void DBNodeManager::RecordOperate(DBCallBack callBack, eRecordOperateMode mode, const char *szTable, const char *szRecordKey, AutoRecord dataRecord, int recordCountLimit, const char *subTableFieldName, ... )
{
	AString info;
	va_list va;
	va_start(va, subTableFieldName);
	info.Format(va, subTableFieldName);

	
	AutoNice paramData = MEM_NEW NiceData();

	paramData->set("SAVE_MODE", (int)mode);
	paramData->set("TARGET_INFO", info.c_str());
	paramData->set("RECORD_COUNT_LIMIT", recordCountLimit);

	if (mode!=eRecordDelete && dataRecord)
	{
		AutoData recordData = MEM_NEW SaveRecordDataBuffer();
		bool b = false;
		if (mode==eRecordUpdate)
		{
			dataRecord->Update();
			b = dataRecord->saveUpdateData(recordData.getPtr());
		}
		else
			b = dataRecord->saveData(recordData.getPtr());

		AssertEx(b, "记录保存到数据失败");

		paramData->set("RECORD_DATA", recordData);
	}
	ExeSqlFunction(callBack, szTable, szRecordKey, "ModifySubRecord", paramData);
}

HandConnect DBNodeManager::GetDBConnect( const char *szDBTable, int dbKey )
{
	AutoDBTableDistribution keyInfo = mTableDistributionDataList.find(szDBTable);
	if (keyInfo)
	{
		Hand<DBClient> nodeData = keyInfo->FindNode(dbKey);
		if (nodeData)
			return nodeData->GetConnectNet()->GetClientConnect();
	}
	return mDBClient->GetConnectNet()->GetClientConnect();
}

HandConnect DBNodeManager::GetRandDBConnect(const char *szDBTable)
{
	RandTool<HandConnect>	randTool;
	AutoDBTableDistribution keyInfo = mTableDistributionDataList.find(szDBTable);
	if (keyInfo)
	{
		for (size_t i=0; i<keyInfo->mDistributionList.size(); ++i)
		{
			DBTableDistribution::KeyRange &info = keyInfo->mDistributionList[i];
			Hand<DBClient> nodeData = info.mNode;
			if (nodeData)
				randTool.Push(nodeData->GetConnectNet()->GetClientConnect());
		}
	}		

	if (mDBClient->GetTable(szDBTable))
		randTool.Push(mDBClient->GetConnectNet()->GetClientConnect());

	HandConnect conn;
	randTool.RandPop(conn);

	return conn;
}


//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
Hand<DBClient> DBTableDistribution::FindNode( Int64 key )
{
	if (mbStringKey)
		return FindNode(STRING(key));

	return _FindNode((UInt64)key);
}

Hand<DBClient> DBTableDistribution::FindNode( const char *szKey )
{
	if (mbStringKey) 
	{
		short slotIndex = STRHASH_SLOT(szKey);
		return mDBSlotIndexList[slotIndex];
		//UInt64 key = (UInt64)(uint)(MAKE_INDEX_ID(szKey));
		//return _FindNode(key);
	}

	return FindNode( TOINT64(szKey) );
}

Hand<DBClient> DBTableDistribution::_FindNode( UInt64 key )
{

	return mDBSlotIndexList[HASH_SLOT(key)];
	//for (size_t i=0; i<mDistributionList.size(); ++i)
	//{
	//	KeyRange &keyInfo = mDistributionList[i];		

	//	if (mbStringKey)
	//	{
	//		//!!! NOTE: 当前不对字符串索引进行分表处理, 字符串表格只能使用单节点DB
	//		return keyInfo.mNode;
	//	}

	//	if ( key>=(UInt64)keyInfo.mMin && key<=(UInt64)keyInfo.mMax )
	//		return keyInfo.mNode;
	//}

	return Hand<DBClient>();
}

bool DBTableDistribution::AppendNodeDistributionData( Hand<DBClient> node, AutoData slotData, bool bIsMainDB )
{
	if (!bIsMainDB)
	{
		// 先移除当前相同的节点
		for (size_t i=0; i<mDistributionList.size(); ++i)
		{
			KeyRange &keyInfo = mDistributionList[i];
			if ( node==keyInfo.mNode )
			{
				mDistributionList.remove(i); 
				break;
			}
		}
		// 增加到列表, 目的为了更新DBNet
		KeyRange k;
		//k.mMin = minRange;
		//k.mMax = maxRange;
		k.mNode = node;
		mDistributionList.push_back(k);
	}

	if (!slotData || slotData->dataSize()<=0)
	{
		for (int i=0; i<DB_HASH_SLOT_COUNT; ++i)
		{
			mDBSlotIndexList[i] = node;
		}
	}
	else
	{
		slotData->seek(0);
		short slotIndex  = 0;
		while (true)
		{
			if (slotData->read(slotIndex))
			{
				if (slotIndex>=DB_HASH_SLOT_COUNT)
					return false;
				mDBSlotIndexList[slotIndex] = node;
			}
			else
				break;
		}
	}
	return true;
	//if (maxRange!=-1 && maxRange<=minRange)
	//{
	//	ERROR_LOG("严重错误: 指定区间最大KEY值[%s]不大于最小KEY值[%s]", STRING(maxRange), STRING(minRange));
	//	return false;
	//}	

	//// 先移除当前相同的节点
	//for (size_t i=0; i<mDistributionList.size(); ++i)
	//{
	//	KeyRange &keyInfo = mDistributionList[i];
	//	if ( node==keyInfo.mNode )
	//	{
	//		mDistributionList.remove(i); 
	//		break;
	//	}
	//}
	//// 检查是否存在交集情况, 如果存在, 返回否, 并报告严重错误
	//for (size_t i=0; i<mDistributionList.size(); ++i)
	//{
	//	KeyRange &keyInfo = mDistributionList[i];

	//	if ( (minRange>=keyInfo.mMin && minRange<=keyInfo.mMax)	// 最小值在区间内
	//		|| (maxRange>=keyInfo.mMin && maxRange<=keyInfo.mMax) // 最大值在区间内
	//		)
	//	{ 
	//		ERROR_LOG("严重错误: %s指定区间最大KEY值[%s]不大于最小KEY值[%s], 与当前%s[%s~%s]存在交集",
	//			node->GetConnectNet()->GetIp(),
	//			STRING(maxRange), 
	//			STRING(minRange),
	//			keyInfo.mNode->GetConnectNet()->GetIp(),
	//			STRING(keyInfo.mMin),
	//			STRING(keyInfo.mMax)
	//			);
	//		return false;
	//	}
	//}
	////for (size_t i=0; i<mDistributionList.size(); ++i)
	////{
	////	KeyRange &keyInfo = mDistributionList[i];
	////	if ( node==keyInfo.mNode )
	////	{
	////		if (minRange==keyInfo.mMin && maxRange==keyInfo.mMax)
	////			return true;
	////		TABLE_LOG("WARN: 当前已经存在分布信息, 且数据不一至, 现在进行更新 [%s]~[%s]", STRING(minRange), STRING(maxRange));

	////		keyInfo.mMin = minRange;
	////		keyInfo.mMax = maxRange;
	////		return true;
	////	}
	////}
	//KeyRange k;
	//k.mMin = minRange;
	//k.mMax = maxRange;
	//k.mNode = node;
	//mDistributionList.push_back(k);

	return true;
}

