
#include "MultipleStringIndex.h"


ARecordIt STL_MultipleStringIndex::GetRecordIt()
{
	return MEM_NEW STLMultipleStringIndexRecordIt(mStringRecordList.begin(), this);
}

ARecordIt STL_MultipleStringIndex::GetRecordIt(ARecord targetRecord)
{
	Data d = targetRecord->get(IndexFieldName());
	if (d.empty())
		return false;
	STRING_KEY key = d.string();

	STLMultipleStringList::iterator it = mStringRecordList.find(key);
	while (it!=mStringRecordList.end() && it->first==key)
	{
		if (it->second==targetRecord)
		{
			return MEM_NEW STLMultipleStringIndexRecordIt(it, this);			
		}
		++it;
	}
	return ARecordIt();
}

ARecordIt STL_MultipleStringIndex::GetLastRecordIt()
{
	if (mStringRecordList.empty())
		return ARecordIt();

	return MEM_NEW STLMultipleStringIndexRecordIt(--mStringRecordList.end(), this);
}
