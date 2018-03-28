
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
		ERROR_LOG("ERROR: 必须指定正确的ID分配区间　[%llu ~ %llu]", minID, maxID);
		return false;
	}
	// 判断分配区间大小，如果太大，分配失败
	if (maxID-minID>ID_RANGE_LIMIT_MAX_COUNT)
	{
		TABLE_LOG("ERROR: 指定的ID分配区间　[%llu ~ %llu] 大于最大允许范围　[%d]", minID, maxID, ID_RANGE_LIMIT_MAX_COUNT);
		return false;
	}

	// 直接分配出索引数据, 如果失败会报出内存错误
	try{
		mRecordList.resize( (size_t)(maxID-minID+1) );
		mMinKey = minID;
		mMaxKey = maxID;
		return true;
	}
	catch(...)
	{
		TABLE_LOG("ERROR: 分配索引数据空间失败，可能内存不够>指定的ID分配区间　[%llu ~ %llu] ", minID, maxID);
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
		TABLE_LOG("ERROR: [%llu] 不能保存在当前数组范围内　[%llu]~[%llu]", (UInt64)key, (UInt64)mMinKey, mMaxKey);
		return false;
	}
	
	mRecordList[id] = scrRecord;

	// 更新当前使用范围
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


