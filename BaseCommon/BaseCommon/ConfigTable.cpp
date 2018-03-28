
#include "ConfigTable.h"
#include "StringHashIndex.h"
#include "StringEasyHashIndex.h"
#include "IntEasyMapIndex.h"
#include "IntEasyHashIndex.h"
#include "Int64HashIndex.h"

AutoIndex ConfigTable::NewRecordIndex(FIELD_TYPE indexKeyType, bool bHash, bool bMultKey)
{
	if (bMultKey)
		ERROR_LOG("[%s] ���ñ��֧�ֶ��ֵ, ��ǰʹ��Ψһ��ֵ����", GetTableName());

	switch (indexKeyType)
	{
		//case FIELD_FLOAT:
	case FIELD_STRING:
		//StringEasyHashIndex�Ѿ��Ż�ΪEasyHash StringHashIndex();
		return MEM_NEW StringEasyHashIndex(); 

	case FIELD_BYTE:
	case FIELD_SHORT:
	case FIELD_INT:
		return MEM_NEW IntEasyHashIndex();	// NOTE: ʹ��EasyHash<int, ARecord> 2017.6.20

	case FIELD_UINT64:
		return MEM_NEW Int64HashIndex();

	default:
		{
			FieldInfo info = FieldInfoManager::getFieldInfo(indexKeyType);
			if (info!=NULL)
			{
				TABLE_LOG("ERROR: [%s]δʵ�� [%s] ���͵�����", GetTableName(), info->getTypeString() );
			}
			else
			{
				TABLE_LOG("ERROR: [%s]�������ַ��ֶ����� [%d]", GetTableName(), indexKeyType);
			}
		}
	}

	return AutoIndex();
}
