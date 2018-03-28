#ifndef _INCLUDE_CONFIGTABLE_H_
#define _INCLUDE_CONFIGTABLE_H_

#include "IndexDBRecord.h"
#include "IndexBaseTable.h"

//-------------------------------------------------------------------------*
class BaseCommon_Export ConfigTable : public SkipBaseTable
{
public:
	ConfigTable()
		: SkipBaseTable(eInitCreateField){}
	// 优化查寻速度， 主要是字符串KEY使用哈希字符串索引，数字KEY，通过ModifyIDIndex 转换为ID数组索引
	virtual AutoIndex NewRecordIndex(FIELD_TYPE indexKeyType, bool bHash, bool bMultKey);
};


//-------------------------------------------------------------------------*

#endif //_INCLUDE_CONFIGTABLE_H_