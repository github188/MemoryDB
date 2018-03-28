#ifndef _INCLUDE_MEMORYDBRECORD_H_
#define _INCLUDE_MEMORYDBRECORD_H_

#include "MemoryDBHead.h"
#include "IndexDBRecord.h"
#include "MemoryTableFieldIndex.h"

//-------------------------------------------------------------------------*/
class MemoryDB_Export MemoryDBRecord : public IndexDBRecord
{
public:
	MemoryDBRecord(ABaseTable hOwnerTable);

	virtual ~MemoryDBRecord()
	{
		InitData();
	}

	virtual void InitData() override
	{
		IndexDBRecord::InitData();
		mExtValue = 0;
	}

public:
	virtual bool FullFromString( const char *scrData, bool bHaveKey, bool bNeedCheck = false )
	{
		if (IndexDBRecord::FullFromString(scrData, bHaveKey, bNeedCheck))
		{
			FullAllUpdate(true);
			return true;
		}
		return false;
	}

	virtual bool restoreData( DataStream *scrData )
	{
		if (IndexDBRecord::restoreData(scrData))
		{
			FullAllUpdate(true);
			return true;
		}
		return false;
	}

	virtual void SetNewInsert(bool bInsert){ mbNewInsert = bInsert; if (bInsert) FullAllUpdate(true); }
	virtual bool IsNewInsert() const { return mbNewInsert; }
	virtual bool InUpdate() const { return mbInUpdateList; }
	virtual void OnNeedUpdate(bool bNeedUpdate);

	void* _getRecordData() { return _getData(); }

public:
	bool		mbNewInsert;
	bool		mbInUpdateList;
	UInt64		mUpdateTime;
	UInt64		mExtValue;
};
//-------------------------------------------------------------------------

#endif //_INCLUDE_MEMORYDBRECORD_H_