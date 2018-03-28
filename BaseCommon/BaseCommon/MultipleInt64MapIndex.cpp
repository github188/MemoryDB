
#include "MultipleInt64MapIndex.h"
#include "BaseTable.h"

#	if TABLE_USE_STL_INDEX

ARecord HashMultiIntIndexRecordIt::Begin()
{
	mListIt = mpOwnerIndex->mHashRecordList.begin();
	if (mListIt!=mEndIt) 
		return mListIt->second;

	return ARecord();
}

void HashMultiIntIndexRecordIt::erase()
{
	if (mListIt!=mEndIt)
	{
		mListIt = mpOwnerIndex->mHashRecordList.erase(mListIt);
	}
}



//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

ARecordIt MultipleInt64MapIndex::GetRecordIt()
{
	return MEM_NEW HashMultiIntIndexRecordIt(mHashRecordList.begin(), this);
}

ARecordIt MultipleInt64MapIndex::GetRecordIt( ARecord targetRecord )
{
	INT_INDEX_TYPE key = targetRecord->get(IndexFieldName());
	HashMultiIntIndexRecordList::iterator it = mHashRecordList.find(key);
	while (it!=mHashRecordList.end() && it->first==key)	
	{		
		if (it->second==targetRecord)
			return MEM_NEW HashMultiIntIndexRecordIt(it, this);		
		++it;					
	}
	return ARecordIt();
}

ARecordIt MultipleInt64MapIndex::GetLastRecordIt()
{
	if (mHashRecordList.empty())
		return ARecordIt();

	return MEM_NEW HashMultiIntIndexRecordIt(--mHashRecordList.end(), this);
}

#endif //TABLE_USE_STL_INDEX