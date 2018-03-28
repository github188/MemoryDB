
#ifndef _INCLUDE_MEMORYTABLEFIELDINDEX_H_
#define _INCLUDE_MEMORYTABLEFIELDINDEX_H_

#include "IndexBaseTable.h"
#include "RecordMemPool.h"
#include "EasyStack.h"
/*/-------------------------------------------------------------------------/
NOTE: 缓存释放的记录, 优化记录restore时的速度
	记录在新建时，会对每个字段进行一次初始，构建时较耗时, 所有直接使用缓存的记录
//-------------------------------------------------------------------------*/

#define DEBUG_RECORD_POOL		0

class MemoryTableFieldIndex : public BaseFieldIndex
{
	friend class MemoryRecord;
public:
	MemoryTableFieldIndex(tBaseTable *pTable)
        : mpTable(pTable)
#if DEBUG_RECORD_POOL
		, mUseCount(0)
		, mFreeCount(0)
#endif
	{

	}
	~MemoryTableFieldIndex();

	virtual BaseRecord* TryGetFreeRecord() override
	{
#if DEBUG_RECORD_POOL
		BaseRecord *p = mFreeRecordPool.pop();
		if (p!=NULL)
			++mUseCount;
		return p;
#else
		return mFreeRecordPool.pop();
#endif
	}

public:
	virtual void* _allotRecordData(uint size) override
	{
		if (size<=mRecordMemPool.getBlockSize())
			return mRecordMemPool.newPtr();
		AssertNote(0, "分配空间[%u]超过记录池设定大小[%llu]", size, mRecordMemPool.getBlockSize());
		return NULL;
	}

	virtual void _freeRecordData(void *pRecordData) override
	{
		mRecordMemPool.deletePtr((unsigned char*)pRecordData);
	}

	// NOTE: 在执行之前，一定要把所有相关的记录释放
	virtual void _updateInfo() override
	{		
        AssertNote(mpTable==NULL || mpTable->GetRecordCount()<=0, "Clear record data error : table exist record");
		_ClearFreeRecord();
		BaseFieldIndex::_updateInfo();
		mRecordMemPool.clear();
		mRecordMemPool.setBlockSize(getDataLength()+(getCount()+7)/8);
	}

protected:
	virtual void FreeRecord(BaseRecord *pRecord) override;
	void _ClearFreeRecord();

protected:
	RecordMemPool			mRecordMemPool;
	EasyStack<BaseRecord*>	mFreeRecordPool;
    tBaseTable                      *mpTable;

#if DEBUG_RECORD_POOL
	int						mFreeCount;
	int						mUseCount;
#endif
};


#endif //_INCLUDE_MEMORYTABLEFIELDINDEX_H_