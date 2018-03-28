
#ifndef _INCLUDE_MEMORYDBMANAGER_H_
#define _INCLUDE_MEMORYDBMANAGER_H_

#include "DBManager.h"
#include "LogEvent.h"
#include "DBClientRequest.h"
#include "MemoryDBClient.h"
#include "MemoryDBHead.h"

//-------------------------------------------------------------------------
class MemoryDBClient : public DBClient
{

};
//-------------------------------------------------------------------------
// DB�ͻ���Ӧ��, ������������Ż�, ��ʹ�� DBNodeManager
//-------------------------------------------------------------------------
class MemoryDB_Export MemoryDBManager : public tDBManager
{
public:
	MemoryDBManager()
	{
		mDBClient = MEM_NEW MemoryDBClient();
	}

public:
	virtual void Close() { mDBClient->Close(); }
	virtual void Process() { mDBClient->Process(); }
	virtual Hand<DBOperate> StartDBOperate(const char *operateType) { return mDBClient->StartOperate(operateType); }
	virtual void RegisterDBOperate(const char *szOperateType, Logic::tEventFactory *pTaskFactory);

	virtual bool InitDB( NiceData  *initParam );
	virtual bool InitConfigDB( NiceData  *initParam ) { return false; }
	virtual void StartLoadDBTableList(const char* listTableName, DBCallBack finishCallBack);

	virtual AutoTable CreateTable(const char *szTableIndex){ AssertEx(0, "������, ��ʹ��DB���ߴ������"); return AutoTable(); }
	virtual AutoTable GetTable(const char *szTableIndex)
	{
		return mDBClient->GetTable(szTableIndex);
	}

public:
	virtual void SaveTable(AutoTable scrTable, DBCallBack finishCallBack) {  AssertEx(0, "������, ��ʹ��DB���ߴ������"); }
	virtual void LoadTable(const char *szTableIndex, const char* dbTableName, DBCallBack finishCallBack);
	virtual void DeleteTable(const char *szTableIndex, const char* dbTableName, DBCallBack finishCallBack)
	{
		AssertEx(0, "������, ��ʹ��DB���ߴ������");
	}

	virtual DBOperate* GetRecord(const char* szTableName, const AString &whereCondition, DBCallBack finishCallBack);
	virtual void GetRecordBySql(const char* szTableName, const AString &selectSqlString, DBCallBack finishCallBack)
	{
		AssertEx(0, "��δʵ��");
	}
	virtual void GetMultitermRecord(const AString &selectSqlString, int nNeedCount, DBCallBack finishCallBack)
	{
		AssertEx(0, "��δʵ��");
	}

	virtual void CreateRecord(const char* tableName, const AString &whereCondition, DBCallBack finishCallBack);
	virtual void DeleteRecord(const char* tableIndex, const char* szWhere, DBCallBack finishCallBack);

	virtual void DeleteRecord(AutoRecord deleteRecord, DBCallBack finishCallBack);
	virtual void InsertRecord(AutoRecord hRecord, bool bReplace, bool bGrowth, DBCallBack finishCallBack);
	virtual void UpdateRecord(AutoRecord hRecord, DBCallBack finishCallBack);
	virtual void AutoSaveRecord(AutoRecord hRecord, DBCallBack finishCallBack)
	{
		AssertEx(0, "����ʹ��");
	}

	// �޸�ָ�����ݵ�ֵ
	virtual void ModifyData(DBCallBack callBack, const char *szTable, const char *szRecordKey, eModifyDataMode eMode, const char *limit, const char *szModifyFieldName, const char *destValue, const char *szDestField, ...);

	//-------------------------------------------------------------------------------------
	// ִ��DB���ݿ�Ĵ洢���̻�洢����
	virtual DBOperate* ExeSqlFunction(DBCallBack finishCallBack, const char *szTable, const char *szRecordKey, const char *szFunctionOprerate, AutoNice paramData);
	virtual bool LoadRecordData(DBCallBack finishCallBack, const char *szTable, const char *szRecordKey, const char *szFieldList);

public:
	virtual void Log(const char* szInfo, ...);

	virtual const char* GetDBName(){ return mDBName.c_str(); }
	virtual void SetDBEvent(Logic::tEventCenter *eventCenter, DBCallBack startCallBack, DBCallBack disconnectCallBack);
	virtual void LoadAllTable(DBCallBack finishCallBack);

	virtual void Reconnect(){ mDBClient->Reconnect(); }

public:
	Hand<MemoryDBClient>		mDBClient;

	ThreadLog					mLog;
	EasyString					mDBName;
};
//-------------------------------------------------------------------------

#endif //_INCLUDE_MEMORYDBMANAGER_H_
