
#include "IDArrayIndex.h"
#include "TableTool.h"

ARecordIt IDArrayIndex::GetRecordIt()
{
	return MEM_NEW ArrayIndexRecordIt(this, 0);
}

ARecordIt IDArrayIndex::GetRecordIt( ARecord targetRecord )
{
	AString beginKey = targetRecord->get(IndexFieldName());
	if (beginKey == "")
		return ARecordIt();

	Int64 key = (Int64)(UInt64)beginKey;
	if (ExistRecord((Int64)key))
		return MEM_NEW ArrayIndexRecordIt(this, ToID(key));

	return ARecordIt();
}

bool IDArrayIndex::SetKeyRangeLimit( UInt64 minID, UInt64 maxID )
{
	if (minID<0 || minID>=maxID)
	{
		ERROR_LOG("ERROR: ����ָ����ȷ��ID�������䡡[%llu ~ %llu]", minID, maxID);
		return false;
	}
	// �жϷ��������С�����̫�󣬷���ʧ��
	if (maxID-minID>ID_RANGE_LIMIT_MAX_COUNT)
	{
		TABLE_LOG("ERROR: ָ����ID�������䡡[%llu ~ %llu] �����������Χ��[%d]", minID, maxID, ID_RANGE_LIMIT_MAX_COUNT);
		return false;
	}

	// ֱ�ӷ������������, ���ʧ�ܻᱨ���ڴ����
	try{
		mRecordList.resize( (size_t)(maxID-minID+1) );
		mMinKey = minID;
		mMaxKey = maxID;
		return true;
	}
	catch(...)
	{
		TABLE_LOG("ERROR: �����������ݿռ�ʧ�ܣ������ڴ治��>ָ����ID�������䡡[%llu ~ %llu] ", minID, maxID);
		return false;
	}
}

bool IDArrayIndex::InsertRecord( ARecord scrRecord )
{
	Data d = scrRecord->get(IndexFieldName());
	if (d.empty())
		return false;

	Int64 key = (Int64)(UInt64)d;
	int id = ToID(key);
	if (id<0)
	{
		TABLE_LOG("ERROR: [%llu] ���ܱ����ڵ�ǰ���鷶Χ�ڡ�[%llu]~[%llu]", (UInt64)key, (UInt64)mMinKey, mMaxKey);
		return false;
	}
	
	mRecordList[id] = scrRecord;

	// ���µ�ǰʹ�÷�Χ
	if (id<mMinUseID || mMinUseID<0)
		mMinUseID = id;

	if (id>mMaxUseID || mMaxUseID<0)
		mMaxUseID = id;

	return true;
}

bool IDArrayIndex::InsertLast( ARecord scrRecord )
{
	if (scrRecord->_set(scrRecord->getFieldCol(IndexFieldName()), GetNextGrowthKey()))
		return InsertRecord(scrRecord);
	return false;
}

ARecordIt IDArrayIndex::GetLastRecordIt()
{
	return MEM_NEW ArrayIndexRecordIt(this, mMaxUseID);
}


