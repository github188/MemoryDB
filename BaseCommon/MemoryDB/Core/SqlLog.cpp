#include "SqlLog.h"

#include "LogThread.h"

#include "MySqlDBTool.h"
#include <stdarg.h>
//-------------------------------------------------------------------------
// 记录到MYSQL中的日志
//-------------------------------------------------------------------------
class MySqlLogThread : public LogThreadManager
{
	friend class SQLLog;

public:
	MySqlLogThread(NiceData *pInitParam)
	{
		mMysqlTool.InitStart(*pInitParam);
	}

public:
	virtual void __writeLog(const char *szLog)
	{
		mMysqlTool.exeSql(szLog, false);
	}

protected:
	MySqlDBTool			mMysqlTool;
};

//-------------------------------------------------------------------------

SQLLog::SQLLog( NiceData *pInitParam )
{
	mpMySqlLogThread = new MySqlLogThread(pInitParam);
	//mpMySqlLogThread->mMysqlTool.InitField(mLogField, true);

	mDBTableName = pInitParam->get(DBNAME).string();
	AString sql;
	sql.Format("Select * from `%s` limit 0", mDBTableName.c_str());
	if (mpMySqlLogThread->mMysqlTool.exeSql(sql, true))
		mpMySqlLogThread->mMysqlTool.InitField(mLogField, true);

	mpMySqlLogThread->InitThread();
}

SQLLog::~SQLLog()
{
	mpMySqlLogThread->Close();
	SAFE_DELETE(mpMySqlLogThread);
}
//-------------------------------------------------------------------------

void SQLLog::log( const char* lpszFormat, ... )
{
	//处理一下格式字符串，用"] ["子串做间隔符,以支持数据中本身存在的空格符
	Array<AString> dataList;
	AString format = lpszFormat;

	//去掉多余的括号
	format.replace(']',' ');
	format.replace('[',' ');

	AString::Split(format.c_str(), dataList, " ", 100);
	int size = dataList.size();

	//重新用"] ["组合
	AString str;
	for(int i = 0; i < size;i++)
	{
		const char* strTemp = dataList[i].c_str();
		if(strlen(strTemp) > 0)
		{
			str += strTemp;
			if(i + 1 < size && strlen(dataList[i + 1].c_str()) > 0 )
				str += "] [";
		}
	}

	va_list argptr;
	va_start(argptr, lpszFormat);
#if __WINDOWS__ || __IOS__ || __SERVER__
	logVa( argptr, str.c_str() );
#endif
}


void SQLLog::logVa( va_list &argList,const char* lpszFormat )
{
	char szData[LOG_INFO_MAX_LENGTH];

	_vsnprintf_s ( szData, LOG_INFO_MAX_LENGTH, LOG_INFO_MAX_LENGTH-1, lpszFormat, argList);	
	szData[LOG_INFO_MAX_LENGTH-1] = '\0';


	Array<AString> dataList;
	AString::Split(szData, dataList, "] [", 100);	//要求数据中不能有"] [" 子串

	//AssertEx(dataList.size() <= mLogField->getCount(), "填制数据列数与指定表格不一致");

	AString sql;
	sql.Format("INSERT into `%s` SET `", mDBTableName.c_str());

	AString strUtf8;
	for (int i=1; i<mLogField->getCount(); ++i)
	{
		if ((int)dataList.size()<i)
			break;

		FieldInfo info = mLogField->getFieldInfo(i);

		if (i>1)
			sql += ", `";

		sql += info->getName().c_str();
		sql += "` = ";

		int fieldType = info->getType();
		if (fieldType == FIELD_STRING || FIELD_DATA)
			sql += "'";

		if(fieldType == FIELD_STRING)
		{
			//必须把字符串值转换为utf8 (列名称本身不用)，否则不能支持中文
			strUtf8 = AString::getUTF8(dataList[i-1].c_str());
			sql += strUtf8;
		}
		else
		{
			sql += dataList[i-1];
		}

		if (fieldType == FIELD_STRING || FIELD_DATA)
			sql += "'";
	}

	printf(sql.c_str());

	LogTask *logTask = mpMySqlLogThread->AlloctLogTask(); //new LogTask();

	char *szLogBuffer = logTask->szLogBuffer;

	strcpy_s(szLogBuffer, LOG_INFO_MAX_LENGTH-1, sql.c_str());

	mpMySqlLogThread->AppendLog(logTask);
}
//-------------------------------------------------------------------------