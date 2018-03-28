
#include "TableLog.h"
#include <stdarg.h>
#include "LogEvent.h"
#include "MemBase.h"

void SaveTableLog::log( const char* info,... )
{
	va_list va; va_start(va,info); logVa(va,info);
}

TableLog::TableLog( const char* logFileName, AString mode /*= "wt"*/ )
{
	mLog = new ThreadLog(logFileName, mode.c_str());
}

TableLog::~TableLog()
{
	delete (ThreadLog*)mLog;
	mLog = NULL;
}

void TableLog::logVa( va_list &vaList, const char*info )
{
#if __WINDOWS__ || __IOS__ || __SERVER__
	((ThreadLog*)mLog)->logVa(vaList, info);
#endif
}

void TableLog::logVa(bool bPrint, va_list &vaList,const char*info)
{
#if __WINDOWS__ || __IOS__ || __SERVER__
	((ThreadLog*)mLog)->logVa(vaList, info, bPrint);
#endif
}
