#ifndef _INCLUDE_DBTOOL_H_
#define _INCLUDE_DBTOOL_H_

#include "BaseCommon.h"
#include "AutoString.h"
#include "Auto.h"

#include "NiceData.h"
#include "BaseRecord.h"



enum SQL_RESULT
{
	SQL_SUCCEED,			//无错误
	SQL_PARAM_ERROR,
	SQL_EXE_ERROR,
	SQL_NO_DATA,			//数据不存在
	SQL_DATA_TRUNCATED,		//缓冲太小
	SQL_FIELD_ERROR,
	SQL_TABLE_NO_EXIST,
};

class DataStream;
class DBTableRecord;

//#if USE_MYSQL_API
class BaseCommon_Export_H tDBTool : public MemBase
{
public:
	virtual ~tDBTool(){}

	virtual bool InitStart(NiceData &param) = 0;
	virtual void Stop() = 0;

	virtual void clearResult() = 0;

public:
	virtual bool InitField(AutoField &hField, bool bMustAllField, Array<AString> *selectFieldList = NULL, Array<AString> *excludeFieldList = NULL) = 0;

	virtual bool LoadRecord(AutoRecord &hRecord) = 0;

	virtual bool SaveRecord(BaseRecord *scrRecord, bool bUpdate, bool bTryInsert) = 0;

	virtual bool ExeByField(const char *szTable, AutoField &hField, const AString &whereString) = 0;

	virtual bool exeSql(const AString &sqlString, bool bNeedLoadData, int *affectCount = NULL, bool bNeedAutoCommit = true) = 0;

	virtual bool exeSqlNeedLoad( const AString &sqlString) = 0;

	virtual AutoNice ExeSqlFunction( const AString &sqlString, bool bNeedAutoCommit ) = 0;
	virtual AutoNice ExeSqlFunction( const AString &sqlString, Array<AutoData> &paramDataList ){ return AutoNice(); }

	virtual SQL_RESULT LoadBlobData(const char *szTableName, const char *szFieldName, const char *szWhereCondition, DataStream *resultData) = 0;
	virtual bool SaveBlobData(const char *szTableName, const char *szFieldName, const char *szWhereCondition, DataStream *saveData, size_t fieldLength, bool bTryInsert) = 0;

public:
	virtual const char* getErrorMsg() = 0;
	virtual void Log(const char *szInfo, ...) = 0;


};

typedef Auto<tDBTool>	AutoDBTool;



//#else
//class BaseCommon_Export_H tDBTool
//{
//
//
//public:
//	virtual ~tDBTool(){}
//
//	virtual int getColNum(void) = 0;
//	virtual const char* getFieldName(int col) = 0;
//	virtual bool getFieldType(int col, int &result) = 0;
//	virtual bool getFieldLength(int col, int &length) = 0;
//
//	virtual bool exeSql( const AString &sqlString ) = 0;
//	virtual bool exeSqlNeedLoad( const AString &sqlString) = 0;
//	virtual int affectCount(void) = 0;
//	virtual void clearResult(void) = 0;
//
//	virtual int getRowCount(void) = 0;
//	virtual bool getValue(int row, int col, int &result) = 0;
//	virtual bool getValue(int row, int col, float &result) = 0;
//	virtual bool getValue(int row, int col, bool &result) = 0;
//	virtual bool getValue(int row, int col, AString &result) = 0;
//	virtual bool getValue(int row, int col, UInt64 &result) = 0;
//
//	virtual int getBinaryData(int col, char *destDataBuffer, size_t destSize) = 0;
//
//	virtual bool fetch() = 0;
//
//	virtual const char* getErrorMsg(void) = 0;
//
//	virtual AutoNice ExeSqlFunction(const char *pFormat, ...) = 0;
//
//	virtual SQL_RESULT LoadBlobData(const char *szTableName, const char *szFieldName, const char *szWhereCondition, DataStream *resultData) = 0;
//	virtual bool SaveBlobData(const char *szTableName, const char *szFieldName, const char *szWhereCondition, DataStream *saveData, size_t fieldLength, bool bTryInsert) = 0;
//
//};
//
//typedef Auto<tDBTool>	AutoDBTool;
//
//#endif



#endif //_INCLUDE_DBTOOL_H_