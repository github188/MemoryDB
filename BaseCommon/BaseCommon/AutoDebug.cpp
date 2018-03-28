/********************************************************************
	created:	2011/05/27
	created:	27:5:2011   1:55
	filename: 	e:\服务器\EventSystem_5.1\AutoDebug.cpp
	file path:	e:\服务器\EventSystem_5.1
	file base:	AutoDebug
	file ext:	cpp
	author:		杨文鸽
	
	purpose:	
*********************************************************************/

#include "AutoDebug.h"
#include "AutoPtr.h"

#define	DEBUG_CHAR_STRING		1

//UseInfoEx* UseInfoEx::New(){ NEW_USE_LOG return MEM_NEW UseInfoEx; }
//void UseInfoEx::Release(){ FREE_USE_LOG delete this; }

// 智能对象使用的空间大小，由于使用地址前一个字节保存空间索引，所以空间大8字节
struct _HandUse { void *p; int coount; };
Bucket		mUseMemPool(sizeof(_HandUse), 2048);


AutoDebugLog::~AutoDebugLog()
{
	for (PtrInfoMap::iterator it = mInfoMap.begin(); it!=mInfoMap.end(); ++it)
	{
		//if (it->second>0)
			//gAutoDebugLog.log("尚存在对象:[%d]-->[%s]",it->first, it->second);
		//else if (it->second<0)
		//	gAutoDebugLog.log("严重错误-->对象:[%d]-->[%s]",it->first.c_str(),it->second.c_str());
	}
#if DEBUG_CHAR_STRING
	for (STRINGMAP::iterator it = gDebugAutoString.begin(); it!=gDebugAutoString.end(); ++it)
	{
		#pragma warning(push)
		#pragma warning(disable:4312)
		if (it->second>0)
			gAutoDebugLog.log("=======尚存在字符串对象:[%s]-->[%d]",(char*)(it->first), it->second);
		else if (it->second<0)
			gAutoDebugLog.log("严重错误-->字符串对象:[%s]-->[%d]",(char*)(it->first), it->second);
		#pragma warning(pop)
	}
#endif
	gAutoDebugLog.log("创建Use数量[%u]，释放Use数量[%u]", gNewUseCount, gFreeUseCount);
}

AutoDebugLog::AutoDebugLog() 
	: gNewCount(0)
	, gFreeCount(0)
	, gNewUseCount(0)
	, gFreeUseCount(0)
{
	gAutoDebugLog.setFile( "Auto_DebugInfo.txt","wt" );
	
}

void AutoDebugLog::NewLog( const type_info &classinfo, void *ptr )
{
	CsLockTool l(mLock);
	++gNewCount; 
	//gAutoDebugLog.log("创建[%s] 创建个数为:[%u] 释放个数[%u] 当前个数[%u]\n", 
	//	classinfo.name(), gNewCount, gFreeCount, gNewCount-gFreeCount);

	PtrInfoMap::iterator it = mInfoMap.find(ptr);
	if (it!=mInfoMap.end())
		gAutoDebugLog.log("严重错误-->记录对象错误");
		//it->second ++;
	else
		mInfoMap[ptr] = classinfo.name();

#if DEBUG_CHAR_STRING
	if (typeid(char)==classinfo)
	{
		#pragma warning(push)
		#pragma warning(disable:4311)
		STRINGMAP::iterator it = gDebugAutoString.find((size_t)ptr);
		if (it!=gDebugAutoString.end())
		{
			it->second ++;
			gAutoDebugLog.log("严重错误-->字符串对象:[%s]-->重复创建AUTO[%d]",(char*)ptr,it->second);
		}
		else
		{
			gDebugAutoString[(size_t)ptr] = 1;
			gAutoDebugLog.log("=====字符串对象:[%s]\n",(char*)ptr);

		}
		#pragma warning(pop)
	}
#endif
}

void AutoDebugLog::FreeLog( const type_info &classinfo, void *ptr )
{
	CsLockTool l(mLock);
	++gFreeCount; 
	//gAutoDebugLog.log("释放[%s] 创建个数为:[%u] 释放个数[%u] 当前个数[%u]\n", 
	//	classinfo.name(), gNewCount, gFreeCount, gNewCount-gFreeCount);
	if (!mInfoMap.empty())
	{
		PtrInfoMap::iterator it = mInfoMap.find(ptr);
		if (it!=mInfoMap.end())
			mInfoMap.erase(it);
			//it->second--;
		else
			gAutoDebugLog.log("错误:没有这个对象的创建[%s]",classinfo.name());
	}
#if DEBUG_CHAR_STRING
	if (typeid(char)==classinfo)
	{
		#pragma warning(push)
#pragma warning(disable:4311)
		if (!gDebugAutoString.empty())
		{
			STRINGMAP::iterator it2 = gDebugAutoString.find((size_t)ptr);
			if (it2!=gDebugAutoString.end())
			{
				--it2->second;
				if (0==it2->second)
					gDebugAutoString.erase((size_t)ptr);
			}
			else
				gAutoDebugLog.log("错误:没有这个字符的创建[%llu]",(size_t)ptr);
		}
#pragma warning(pop)
	}
#endif
}