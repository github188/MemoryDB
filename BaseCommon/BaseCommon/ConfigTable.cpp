
#include "ConfigTable.h"
#include "StringHashIndex.h"
#include "StringEasyHashIndex.h"
#include "IntEasyMapIndex.h"
#include "IntEasyHashIndex.h"
#include "Int64HashIndex.h"

AutoIndex ConfigTable::NewRecordIndex(FIELD_TYPE indexKeyType, bool bHash, bool bMultKey)
{
	if (bMultKey)
		ERROR_LOG("[%s] 配置表格不支持多键值, 当前使用唯一键值索引", GetTableName());

	switch (indexKeyType)
	{
		//case FIELD_FLOAT:
	case FIELD_STRING:
		//StringEasyHashIndex已经优化为EasyHash StringHashIndex();
		return MEM_NEW StringEasyHashIndex(); 

	case FIELD_BYTE:
	case FIELD_SHORT:
	case FIELD_INT:
		return MEM_NEW IntEasyHashIndex();	// NOTE: 使用EasyHash<int, ARecord> 2017.6.20

	case FIELD_UINT64:
		return MEM_NEW Int64HashIndex();

	default:
		{
			FieldInfo info = FieldInfoManager::getFieldInfo(indexKeyType);
			if (info!=NULL)
			{
				TABLE_LOG("ERROR: [%s]未实现 [%s] 类型的索引", GetTableName(), info->getTypeString() );
			}
			else
			{
				TABLE_LOG("ERROR: [%s]不存在字符字段类型 [%d]", GetTableName(), indexKeyType);
			}
		}
	}

	return AutoIndex();
}
