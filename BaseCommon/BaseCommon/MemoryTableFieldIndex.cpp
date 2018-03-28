#include "MemoryTableFieldIndex.h"
#include "IndexDBRecord.h"


MemoryTableFieldIndex::~MemoryTableFieldIndex()
{
	_ClearFreeRecord();
}

void MemoryTableFieldIndex::FreeRecord(BaseRecord *pRecord)
{
	if (mFreeRecordPool.size()>1000)
	{
		// 避免运行时泄露
		//NOTE_LOG("!!! WARN: 缓存的记录数量过多，可能未能重复使用，请检查设计问题");
		BaseFieldIndex::FreeRecord(pRecord);
		return;
	}
	pRecord->InitData();
	//!!! 一定要把记录中的字段信息清除，否则互相包含死锁不能正常释放
	IndexDBRecord *p = dynamic_cast<IndexDBRecord*>(pRecord);
	AssertEx(p!=NULL, "Must is IndexDBRecord use FreeRecord");
	p->mOwnerTable.setNull();

	mFreeRecordPool.push(pRecord);
#if DEBUG_RECORD_POOL
	++mFreeCount;
#endif
}

void MemoryTableFieldIndex::_ClearFreeRecord()
{
#if DEBUG_RECORD_POOL
	//NOTE_LOG("---Field----\r\n%s\r\n------------", ToString().c_str());
	if (mUseCount>0)
		NOTE_LOG("###### field use [%d], free [%d], now [%llu] ===", mUseCount, mFreeCount, mFreeRecordPool.size());
#endif
	while (!mFreeRecordPool.empty())
	{
		BaseRecord *pRe = mFreeRecordPool.pop();
		IndexDBRecord *p = dynamic_cast<IndexDBRecord*>(pRe);
		//!!! 交给池释放,可能池分配的系统内存 OxFFFF 开头标识
		if (p->mRecordData!=NULL)
		{
			mRecordMemPool.deletePtr((unsigned char*)p->mRecordData);
			p->mRecordData = NULL;
		}
		MEM_DELETE pRe;
	}
}
