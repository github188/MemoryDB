
#include "BaseRecordSet.h"
#include "TableTool.h"

IntRecordSet::IntRecordSet()
{

}


bool IntRecordSet::AppendRecord(int indexCol, AutoRecord scrRecord, bool bReplace)
{
	int nKey;
	if (scrRecord->get(indexCol, nKey))
	{
		if (bReplace)
		{
			if (mRecordMap.erase(nKey)>0)
			{
				TableTool::Log("Warn: now replace one record >>> [%d]", nKey);
			}
		}
		else
		{
			if ( mRecordMap.exist(nKey) )
				return false;				
		}
		mRecordMap.insert(nKey, scrRecord);
		return true;
	}
	return false;
}


bool IntRecordSet::RemoveRecord(int indexCol, AutoRecord record)
{
	if (record)
	{
		int nKey;
		if (record->get(indexCol, nKey))
		{
			size_t pos;
			if ( mRecordMap.find(nKey, &pos)==record )
			{
				mRecordMap._remove(pos);
				return true;
			}
		}
	}		
	return false;
}


AutoRecordIt IntRecordSet::GetRecordIt(void)
{
	return AutoRecordIt( new IntRecordIt(*this) );
}