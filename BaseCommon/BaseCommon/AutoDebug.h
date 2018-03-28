/********************************************************************
	created:	2011/05/23
	created:	23:5:2011   0:31
	filename: 	h:\2011_02_27\Code\common\AutoDebug.h
	file path:	h:\2011_02_27\Code\common
	file base:	AutoDebug
	file ext:	h
	author:		杨文鸽
	
	purpose:	监视Auto工作,目前具有数量验证,可以加入具体对象的分析
*********************************************************************/
#ifndef _INCLUDE_AUTODEBUG_H_
#define _INCLUDE_AUTODEBUG_H_

#include "BaseCommon.h"
#include "LogEvent.h"
#include <map>
#include <string>

#if __LINUX__
#include <typeinfo>
using namespace std;
#endif

#include "Lock.h"

class BaseCommon_Export AutoDebugLog 
{
public:
	ThreadLog	gAutoDebugLog;
	size_t gNewCount;
	size_t gFreeCount;

	size_t gNewUseCount;
	size_t gFreeUseCount;

	typedef std::map<void*, std::string> PtrInfoMap;
	PtrInfoMap	mInfoMap;

public:
	AutoDebugLog();
	~AutoDebugLog();

	void NewLog(const type_info &classinfo, void *ptr);
	void FreeLog(const type_info &classinfo, void *ptr);

	void NewUse()
	{
		gNewUseCount++;
	}

	void FreeUse()
	{
		gFreeUseCount++;
	}

	static AutoDebugLog & GetMe()
	{
		static AutoDebugLog	gLog;
		return gLog;
	}

protected:
	typedef  std::map<size_t, int>	STRINGMAP;

	STRINGMAP		gDebugAutoString;

	CsLock			mLock;
};

#endif //_INCLUDE_AUTODEBUG_H_
