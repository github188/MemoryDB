/********************************************************************
	created:	2011/11/23
	created:	23:11:2011   0:09
	filename: 	h:\Work\Common\DataBase\AutoKey.h
	file path:	h:\Work\Common\DataBase
	file base:	AutoKey
	file ext:	h
	author:		ÑîÎÄ¸ë
	
	purpose:	
*********************************************************************/
#ifndef _INCLUDE_AUTOKEY_H_
#define _INCLUDE_AUTOKEY_H_

#include "BaseCommon.h"
#include "Array.h"

typedef UInt64	KEY_TYPE;

class BaseCommon_Export AutoKey
{
public:
	AutoKey();

public:
	KEY_TYPE AlloctKey();
	void ReleaseKey(KEY_TYPE freeKey);

protected:
	UInt64			mNowSecond;
	USHORT			mTimePeriod;
	USHORT			mCode;
};
#endif //_INCLUDE_AUTOKEY_H_
