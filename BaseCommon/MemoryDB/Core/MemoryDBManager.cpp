#include "MemoryDBManager.h"

#include <stdarg.h>
#include "DBNetPacket.h"

void MemoryDBManager::Log( const char* szInfo, ... )
{
#if DEVELOP_MODE
	va_list va;
	va_start(va, szInfo);
	mLog.logVa(va, szInfo);
#endif
}

bool MemoryDBManager::InitDB( NiceData *initParam )
{
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

	return mDBClient->Start(ip.c_str(), (int)port);

}

void MemoryDBManager::RegisterDBOperate( const char *szOperateType,Logic::tEventFactory *pTaskFactory )
{
	mDBClient->GetEventCenter()->RegisterEvent(szOperateType, pTaskFactory);
}

void MemoryDBManager::StartLoadDBTableList( const char* listTableName, DBCallBack finishCallBack )
{
	HandDBOperate op = mDBClient->StartOperate("DB_LoadAllTableField");
	op->mCallBack = finishCallBack;
	op->DoEvent();
}

void MemoryDBManager::LoadTable( const char *szTableIndex, const char* dbTableName, DBCallBack finishCallBack )
{
	Hand<DBOperate> request = mDBClient->StartOperate("DB_LoadTableField");
	request->mTableIndex = szTableIndex;
	request->mCallBack = finishCallBack;
	request->DoEvent();
}

DBOperate* MemoryDBManager::GetRecord( const char* szTableName, const AString &whereCondition, DBCallBack finishCallBack )
{
	Hand<DBOperate> request = mDBClient->StartOperate("DB_FindRecord");
	request->mTableIndex = szTableName;
	request->mRecordKey = whereCondition;
	request->mCallBack = finishCallBack;
	request->DoEvent();
	return request.getPtr();
}

void MemoryDBManager::CreateRecord( const char* tableName, const AString &whereCondition, DBCallBack finishCallBack )
{
	ABaseTable t = mDBClient->GetTable(tableName);
	if (!t)
	{
		finishCallBack.run(NULL, false);
		return;
	}
	ARecord r;	
	if (whereCondition!="")
	{		
		r = t->CreateRecord(whereCondition.c_str(), true);		
	}
	else
		r = t->NewRecord();

	Hand<RequestDBSaveEvent> saveRecord = mDBClient->StartOperate("RequestDBSaveEvent");
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
}

void MemoryDBManager::DeleteRecord( const char* tableIndex, const char* szWhere, DBCallBack finishCallBack )
{
	Hand<DBOperate> request = mDBClient->StartOperate("DB_DeleteRecord");
	request->mTableIndex = tableIndex;
	request->mRecordKey = szWhere;
	request->DoEvent();
}

void MemoryDBManager::DeleteRecord( AutoRecord deleteRecord, DBCallBack finishCallBack )
{
	Hand<DBOperate> request = mDBClient->StartOperate("DB_DeleteRecord");
	request->mTableIndex = deleteRecord->GetTable()->GetTableName();
	request->mRecordKey = deleteRecord->getIndexData().string();
	request->DoEvent();
}

void MemoryDBManager::InsertRecord( AutoRecord hRecord, bool bReplace, bool bGrowth, DBCallBack finishCallBack )
{
	//Hand<DBOperate> saveRecord = mDBClient->StartOperate("DB_SaveRecord");
	//saveRecord->set("REPLACE", bReplace);
	//saveRecord->mResultRecord = hRecord;
	//saveRecord->mTableIndex = hRecord->GetTable()->GetTableName();
	//saveRecord->mCallBack = finishCallBack;
	//saveRecord->DoEvent();

	Hand<RequestDBSaveEvent> saveRecord = mDBClient->StartOperate("RequestDBSaveEvent");
	if (!saveRecord)
	{
		finishCallBack(NULL, false);
		return;
	}
	//Hand<DBOperate> saveRecord = mDBClient->StartOperate("DB_SaveRecord");
	saveRecord->mbNewInsert = true;
	saveRecord->mbReplace = bReplace;
	saveRecord->mbGrowthKey = bGrowth;
	saveRecord->mbNeedResponse = NeedSaveUpdateResponse();
	saveRecord->mResultRecord = hRecord;
	saveRecord->mTableIndex = hRecord->GetTable()->GetTableName();
	saveRecord->mCallBack = finishCallBack;
	saveRecord->DoEvent();
}

void MemoryDBManager::UpdateRecord( AutoRecord hRecord, DBCallBack finishCallBack )
{
	//Hand<DBOperate> request = mDBClient->StartOperate("DB_UpdateRecord");
	//request->mTableIndex = hRecord->GetTable()->GetTableName();
	//request->mResultRecord = hRecord;
	//request->mCallBack = finishCallBack;
	//request->DoEvent();

	Hand<RequestDBSaveEvent> saveRecord = mDBClient->StartOperate("RequestDBSaveEvent");
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

void MemoryDBManager::LoadAllTable(DBCallBack finishCallBack)
{
	Hand<DBOperate> request = mDBClient->StartOperate("DB_LoadAllTableField");
	request->mCallBack = finishCallBack;
	request->DoEvent();
}

void MemoryDBManager::SetDBEvent( Logic::tEventCenter *eventCenter, DBCallBack startCallBack, DBCallBack disconnectCallBack )
{
	mDBClient->mNetSucceedCallBack = startCallBack; 
	mDBClient->mNetDisconnectCallBack = disconnectCallBack;
}

DBOperate* MemoryDBManager::ExeSqlFunction( DBCallBack finishCallBack, const char *szTable, const char *szRecordKey, const char *szFunctionOprerate, AutoNice paramData )
{
	Hand<RequestDBOperateEvent> op = mDBClient->StartOperate("RequestDBOperateEvent");
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
	//AssertEx(0, "等待实现");
	return op.getPtr();
}

void MemoryDBManager::ModifyData(DBCallBack callBack, const char *szTable, const char *szRecordKey, eModifyDataMode eMode, const char *limit, const char *szModifyFieldName, const char *destValue, const char *szDestField, ... )
{
	Hand<DBOperate> request = mDBClient->StartOperate("DB_RequestModifyData");

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

bool MemoryDBManager::LoadRecordData( DBCallBack finishCallBack, const char *szTable, const char *szRecordKey, const char *szFieldList )
{
	Hand<DB_RequestRecordData_C> questEvt = StartDBOperate("DB_RequestRecordData");
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
