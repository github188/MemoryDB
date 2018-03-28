/********************************************************************
	created:	2011/07/07
	created:	7:7:2011   23:04
	filename: 	e:\Work\Common\DataBase\TableLog.h
	file path:	e:\Work\Common\DataBase
	file base:	TableLog
	file ext:	h
	author:		Yang Wenge
	
	purpose:	表格日志
*********************************************************************/

#ifndef _INCLUDE_TABLELOG_H_
#define _INCLUDE_TABLELOG_H_

#include "BaseCommon.h"

#ifdef __WINDOWS__
#include <vadefs.h>
#endif

#include "AutoString.h"

// 表格日志
class BaseCommon_Export SaveTableLog
{
public:
	virtual ~SaveTableLog()
	{
	}
	virtual	void log(const char* info,...);
	virtual void logVa(va_list &va,const char*info) = 0;
	virtual void logVa(bool bPrint, va_list &va,const char*info) { logVa(va, info); }
};


class BaseCommon_Export TableLog : public SaveTableLog
{
public:
	TableLog(const char* logFileName, AString mode = "wt");
	~TableLog();

	virtual void logVa(va_list &vaList, const char*info);
	virtual void logVa(bool bPrint, va_list &va,const char*info) override;
protected:
	void		*mLog;
};


#endif //_INCLUDE_TABLELOG_H_