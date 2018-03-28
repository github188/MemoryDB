
#include "Int64MapIndex.h"
#include "BaseTable.h"


//-------------------------------------------------------------------------
ARecord Int64IndexRecordIt::Begin()
{
	mListIt = mpOwnerIndex->mIntMapRecordList.begin();
	if (mListIt!=mpOwnerIndex->mIntMapRecordList.end()) 
		return mListIt->second;

	return ARecord();
}

void Int64IndexRecordIt::erase()
{
	if (mListIt!=mpOwnerIndex->mIntMapRecordList.end())
	{
		mListIt = mpOwnerIndex->mIntMapRecordList.erase(mListIt);
	}
}

//-------------------------------------------------------------------------
Int64IndexRecordIt::Int64IndexRecordIt( IntMapIndexRecordList::iterator it, Int64MapIndex *pIndex )
{
	mpOwnerIndex = pIndex;
	mListIt = it;
}

Int64IndexRecordIt::operator bool()
{
	return mListIt!=mpOwnerIndex->mIntMapRecordList.end();
}

bool Int64IndexRecordIt::operator  ++() 
{ 
	IntMapIndexRecordList::iterator endIt = mpOwnerIndex->mIntMapRecordList.end();
	if (mListIt!=endIt)
	{ 
		++mListIt; 
		return mListIt!=endIt;
	}
	return false; 
}	

ARecord Int64IndexRecordIt::GetRecord()
{
	if (mListIt!=mpOwnerIndex->mIntMapRecordList.end())
		return mListIt->second; 

	return ARecord();
}


//-------------------------------------------------------------------------
//-------------------------------------------------------------------------


ARecordIt Int64MapIndex::GetRecordIt()
{
	return MEM_NEW Int64IndexRecordIt(mIntMapRecordList.begin(), this);
}

ARecordIt Int64MapIndex::GetRecordIt( ARecord targetRecord )
{
	IntMapIndexRecordList::iterator it = mIntMapRecordList.find((INT_INDEX_TYPE)targetRecord->get(IndexFieldName()));
	if (it!=mIntMapRecordList.end())
		return MEM_NEW Int64IndexRecordIt(it, this);		

	return ARecordIt();
}

ARecordIt Int64MapIndex::GetLastRecordIt()
{
	if (mIntMapRecordList.end()!=mIntMapRecordList.begin())
		return MEM_NEW Int64IndexRecordIt(--mIntMapRecordList.end(), this);

	return ARecordIt();
}

//-------------------------------------------------------------------------


