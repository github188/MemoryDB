
#include "StringHashIndex.h"
#include "BaseTable.h"

#if TABLE_USE_STL_INDEX

ARecord HashStringIndexRecordIt::Begin( )
{
	mListIt = mpOwnerIndex->mHashRecordList.begin();
	if (mListIt!=mEndIt) 
		return mListIt->second;

	return ARecord();
}

void HashStringIndexRecordIt::erase()
{
	if (mListIt!=mEndIt)
	{
		mListIt = mpOwnerIndex->mHashRecordList.erase(mListIt);
	}
}

HashStringIndexRecordIt::HashStringIndexRecordIt( HashStringIndexRecordList::iterator it, StringHashIndex *pIndex )
{
	mpOwnerIndex = pIndex;
	mListIt = it;
	mEndIt = mpOwnerIndex->mHashRecordList.end();
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------


ARecordIt StringHashIndex::GetRecordIt()
{
	return MEM_NEW HashStringIndexRecordIt(mHashRecordList.begin(), this);
}

ARecordIt StringHashIndex::GetRecordIt( ARecord targetRecord )
{
	AString key = targetRecord->get(IndexFieldName()).string();
	HashStringIndexRecordList::iterator it = mHashRecordList.find(key);
	if (it!=mHashRecordList.end())	
	{
		return MEM_NEW HashStringIndexRecordIt(it, this);			
	}
	return ARecordIt();
}

ARecordIt StringHashIndex::GetLastRecordIt()
{
	if (mHashRecordList.empty())
		return ARecordIt();

	return MEM_NEW HashStringIndexRecordIt(--mHashRecordList.end(), this);
}


#endif