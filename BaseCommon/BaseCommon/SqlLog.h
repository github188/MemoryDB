#ifndef _INCLUDE_SQLLOG_H_
#define _INCLUDE_SQLLOG_H_

#include "FieldIndex.h"
//-------------------------------------------------------------------------
// MYSQL ��־
//-------------------------------------------------------------------------
class MySqlLogThread;
class NiceData;

class BaseCommon_Export SQLLog
{
public:
	SQLLog(NiceData *pInitParam);
	~SQLLog();

public:
	// ��־��ʽ "Data1 d2 d3", �ֱ��ӦDB���е� �ֶ� 2 3 4 ��Ӧ������
	void log( const char* lpszFormat, ... );


	void logVa( va_list &argList,const char* lpszFormat );

protected:
	MySqlLogThread		*mpMySqlLogThread;

	AutoField			mLogField;
	AString				mDBTableName;
};

//-------------------------------------------------------------------------

#endif //_INCLUDE_SQLLOG_H_