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
	// �Ż���Ѱ�ٶȣ� ��Ҫ���ַ���KEYʹ�ù�ϣ�ַ�������������KEY��ͨ��ModifyIDIndex ת��ΪID��������
	virtual AutoIndex NewRecordIndex(FIELD_TYPE indexKeyType, bool bHash, bool bMultKey);
};


//-------------------------------------------------------------------------*

#endif //_INCLUDE_CONFIGTABLE_H_