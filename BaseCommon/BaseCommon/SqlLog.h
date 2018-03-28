#ifndef _INCLUDE_SQLLOG_H_
#define _INCLUDE_SQLLOG_H_

#include "FieldIndex.h"
//-------------------------------------------------------------------------
// MYSQL 日志
//-------------------------------------------------------------------------
class MySqlLogThread;
class NiceData;

class BaseCommon_Export SQLLog
{
public:
	SQLLog(NiceData *pInitParam);
	~SQLLog();

public:
	// 日志格式 "Data1 d2 d3", 分别对应DB表中的 字段 2 3 4 对应的数据
	void log( const char* lpszFormat, ... );


	void logVa( va_list &argList,const char* lpszFormat );

protected:
	MySqlLogThread		*mpMySqlLogThread;

	AutoField			mLogField;
	AString				mDBTableName;
};

//-------------------------------------------------------------------------

#endif //_INCLUDE_SQLLOG_H_