#include "MemoryDBRecord.h"
#include "MemoryDBTable.h"

#include "TimeManager.h"
//-------------------------------------------------------------------------*/
MemoryDBRecord::MemoryDBRecord( ABaseTable hOwnerTable ) 
	: IndexDBRecord(hOwnerTable)
	, mbNewInsert(false)
	, mbInUpdateList(false)
	, mExtValue(0)
{
	mUpdateTime = TimeManager::NowTick() + rand()%RECORD_UPDATE_TIME;
}
//-------------------------------------------------------------------------

void MemoryDBRecord::OnNeedUpdate(bool bNeedUpdate)
{
	if (mbInUpdateList)
		return;

	MemoryDBTable *pTable = dynamic_cast<MemoryDBTable*>(GetTable());
	AssertEx(pTable!=NULL, "Owner table MemoryDBTable Must exist");
	pTable->AppendUpdateRecord(this);
}
