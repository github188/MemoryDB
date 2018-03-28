/********************************************************************
	created:	2013/07/25
	created:	25:7:2013   13:38
	filename: 	C:\Work\BaseCommon\ServerBase\Base\DBManager.h
	file path:	C:\Work\BaseCommon\ServerBase\Base
	file base:	DBManager
	file ext:	h
	author:		Wenge Yang
	
	purpose:	DB操作管理
				1 异步存取DBSQL
				2 远程异步DB操作
				3 每个玩家在初始时, 都会指定出DB实例, 决定DB数据源
*********************************************************************/

#ifndef _INCLUDE_DBMANAGER_H_
#define _INCLUDE_DBMANAGER_H_

#include "DBCallBack.h"
#include "Hand.h"
#include "AutoString.h"
#include "BaseTable.h"
#include "DBOperate.h"

namespace Logic
{
	class tEventCenter;
}

class DBOperate;
class TaskFactory;
class NiceData;

class tDBManager : public Base<tDBManager>
{
public:
	virtual bool IsOk(){ return true; }

	virtual void Close() = 0;
	virtual void Process() = 0;
	virtual Hand<DBOperate> StartDBOperate(const char *operateType) = 0;
	virtual void RegisterDBOperate(const char *szOperateType, Logic::tEventFactory *pTaskFactory) = 0;

	virtual bool InitDB( NiceData  *initParam ) = 0;
	virtual bool InitConfigDB( NiceData  *initParam ) = 0;
	virtual void StartLoadDBTableList(const char* listTableName, DBCallBack finishCallBack) = 0;

	virtual AutoTable CreateTable(const char *szTableIndex) = 0;
	virtual AutoTable GetTable(const char *szTableIndex) = 0;

public:
	virtual void SaveTable(AutoTable scrTable, DBCallBack finishCallBack) = 0;
	virtual void LoadTable(const char *szTableIndex, const char* dbTableName, DBCallBack finishCallBack) = 0;
	virtual void DeleteTable(const char *szTableIndex, const char* dbTableName, DBCallBack finishCallBack) = 0;

	virtual DBOperate* GetRecord(const char* szTableName, const AString &whereCondition, DBCallBack finishCallBack) = 0;
	virtual void GetRecordBySql(const char* szTableName, const AString &selectSqlString, DBCallBack finishCallBack) = 0;
	virtual void GetMultitermRecord(const AString &selectSqlString, int nNeedCount, DBCallBack finishCallBack) = 0;

	virtual void CreateRecord(const char* tableName, const AString &whereCondition, DBCallBack finishCallBack) = 0;
	virtual void DeleteRecord(const char* tableIndex, const char* szWhere, DBCallBack finishCallBack) = 0;

	virtual void DeleteRecord(AutoRecord deleteRecord, DBCallBack finishCallBack) = 0;
	virtual void InsertRecord(AutoRecord hRecord, bool bReplace, bool bGrowth, DBCallBack finishCallBack) = 0;
	virtual void UpdateRecord(AutoRecord hRecord, DBCallBack finishCallBack) = 0;
	virtual void AutoSaveRecord(AutoRecord hRecord, DBCallBack finishCallBack) = 0;

	virtual void ModifyData(DBCallBack callBack, const char *szTable, const char *szRecordKey, eModifyDataMode eMode, const char *limit, const char *szModifyFieldName, const char *destValue, const char *szDestFieldKey, ...) = 0;
	
	virtual void RecordOperate(DBCallBack, eRecordOperateMode mode, const char *szTable, const char *szRecordKey, AutoRecord dataRecord, const char *subTableFieldName, ...){}
	virtual void RecordOperate(DBCallBack, eRecordOperateMode mode, const char *szTable, UInt64 recordKey, AutoRecord dataRecord, const char *subTableFieldName, ...){}
	virtual void RecordOperate(DBCallBack, eRecordOperateMode mode, const char *szTable, const char *szRecordKey, AutoRecord dataRecord, int recordCountLimit, const char *subTableFieldName, ...){}

	virtual void DeleteSubRecord(DBCallBack, const char *szTable, const char *szRecordKey, const char *subTableFieldName, ...){}
	virtual void DeleteSubRecord(DBCallBack, const char *szTable, UInt64 recordKey, const char *subTableFieldName, ...){}
	//-------------------------------------------------------------------------------------
	// 执行DB数据库的存储过程或存储函数
	virtual DBOperate* ExeSqlFunction(DBCallBack finishCallBack, const char *szTable, const char *szRecordKey, const char *szFunctionOprerate, AutoNice paramData) = 0;

	// 获取记录的部分字段数据
	virtual bool LoadRecordData(DBCallBack finishCallBack, const char *szTable, const char *szRecordKey, const char *szFieldList){ AssertEx(0, "未实现"); return false; }
	virtual bool LoadRecordData(DBCallBack finishCallBack, const char *szTable, const char *szRecordKey, const char *szFieldList, const char *szTarget, ...){ AssertEx(0, "未实现"); return false; }

public:
	virtual void Log(const char* szInfo, ...) = 0;

	virtual const char* GetDBName(){ return "UNKOW"; }
	virtual void SetDBEvent(Logic::tEventCenter *eventCenter, DBCallBack startCallBack, DBCallBack disconnectCallBack){}

	virtual void LoadAllTable(DBCallBack callBack){}

	virtual void Reconnect(){}

	virtual void OnNotifyDistribution(AutoTable distTable){}

	virtual bool NeedSaveUpdateResponse() const { return true; }

public:
	virtual ~tDBManager(){}
};

typedef Hand<tDBManager>		HandDB;

#endif