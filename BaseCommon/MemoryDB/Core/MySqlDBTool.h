/********************************************************************
	created:	2013/09/07
	created:	7:9:2013   15:24
	filename: 	C:\Work\BaseCommon\BaseCommon\MySqlDBTool.h
	file path:	C:\Work\BaseCommon\BaseCommon
	file base:	MySqlDBTool
	file ext:	h
	author:		Yang WenGe
	
	purpose:	MySQL API 实现DB操作

	NOTE:		1 存储过程中, 已支持BLOB数据返回
				2 当前使用记录处理 BLOB字段支持: 
				3 调取到DataRecord 字段类型为 FIELD_DATA (AutoData)
				4 取出时使用结果集直接取出, 且分析长度
				5 保存时, 进行绑定二进制数据为参数
*********************************************************************/
#ifndef _INCLUDE_MYSQLDBTOOL_H_
#define _INCLUDE_MYSQLDBTOOL_H_

#include "MemoryDBHead.h"
#include "DBTool.h"
#include "BaseTable.h"
#include "EventCenter.h"

#define USE_MYSQL_API				1
#define MYSQL_TRY_RUN_TIME			(600)		// 10分钟进行一次检查连接

//-------------------------------------------------------------------------*/
#if USE_MYSQL_API
struct st_mysql;
struct st_mysql_res;
struct st_mysql_stmt;

struct st_mysql_bind;

class LoadDataCallBack;

class MemoryDB_Export MySqlDBTool : public tDBTool
{
	struct MySQLBindData
	{
		AutoData  mData;
		unsigned long mLength;
	};

public:
	MySqlDBTool();
	~MySqlDBTool();

public:
	virtual bool InitStart(NiceData &param);
	virtual void Stop();
	virtual void clearResult();

public:
	// Load DB table, and all record
	AutoTable LoadDBTable(const char *szTableName, bool bLoadAllRecord = true);
	bool CreateDBTable(const char *szTableName, AutoField tableField, const char *szInfoData = "NONE");

	//防止长时间无操作断开, NOTE: 必须保证在MYSQL使用线程Process, 否则会出现异常DUMP
	void Process();

	void CheckConnect();
	void Reconnect(){ if (!InitStart(mInitParam)) NOTE_LOG("重新连接失败"); }

public:
	virtual bool InitField(AutoField &hField, bool bMustAllField, Array<AString> *selectFieldList = NULL, Array<AString> *excludeFieldList = NULL);

	virtual AString GetFieldData();

	virtual bool LoadRecord(AutoRecord &hRecord);

	virtual bool SaveRecord(BaseRecord *scrRecord, bool bUpdate, bool bTryInsert);

	virtual bool ExeByField(const char *szTable, AutoField &hRecord, const AString &whereString);

	// exe my sql string command, bNeedLoadData param is skip.
	virtual bool exeSql(const AString &sqlString, bool bNeedLoadData, int *affectCount = NULL, bool bNeedAutoCommit = true);

	virtual bool exeSqlNeedLoad( const AString &sqlString )
	{
		return exeSql(sqlString, true);
	}

	virtual bool exeSql(const AString &sqlString, Array<AutoData> &paramDataList, int *pAffectCount = NULL);

	// 存储过程中, 已支持BLOB数据返回
	virtual AutoNice ExeSqlFunction( const AString &sqlString, bool bAutoCommit);

	virtual AutoNice ExeSqlFunction( const AString &sqlString, Array<AutoData> &paramDataList, bool bAutoCommit = false );

	virtual SQL_RESULT LoadBlobData(const char *szTableName, const char *szFieldName, const char *szWhereCondition, DataStream *resultData);

	virtual bool SaveBlobData(const char *szTableName, const char *szFieldName, const char *szWhereCondition, DataStream *saveData, size_t fieldLength, bool bTryInsert);

	virtual const char* _getErrorMsg();

	virtual const char* getErrorMsg(){ return mErrorInfo.c_str(); }

	virtual void Log(const char *szInfo, ...);

	virtual bool LoadData(int keyCol, int dataCol, LoadDataCallBack *loadCallBack, DataStream *pDestData = NULL);

public:
	bool _ExeStmt(const AString &sqlString, st_mysql_bind *pBindList, int &affectCount, bool bNeedAutoCommit, NiceData *pResultData = NULL);

	// To sql and need data, 如果是自增长时, 需要把第一列索引数据设置未更新状态
	static bool _MakeSaveSqlData(AString &resultInsertSQL, Array<AutoData> &resultData, AutoRecord scrRecord, bool bInsert, bool bGrownInsert = false, const char *szTableName = NULL);

protected:
	bool _SaveBlobData( const AString &sqlString, DataStream *saveData, size_t fieldLength );

	SQL_RESULT _LoadBlobData( const AString &sqlString, DataStream *resultData );


	void _LogSqlError(const AString &sqlString, st_mysql_stmt *pStamt);

	bool _getDataResult(st_mysql_stmt *mStmt, NiceData *resultNice);

	void _ClearMysqlResult(st_mysql_res *pResult, st_mysql_stmt *mStmt);

	void _SetAutoCommit(bool bAuto);

protected:
	st_mysql			*mMySql;
	st_mysql_res		*mResult;
	AString				mErrorInfo;

	NiceData			mInitParam;

	UInt64				mLastCheckTime;

public:
	AutoEvent			mWaitTryRunEvent;
};
//-------------------------------------------------------------------------*/
#endif
#endif //_INCLUDE_MYSQLDBTOOL_H_