#ifndef _INCLUDE_MULTIPLESTRINGINDEX_H_
#define _INCLUDE_MULTIPLESTRINGINDEX_H_

#include "BaseCommon.h"
#include "RecordIndex.h"
#include <map>

using namespace std;

typedef AString STRING_KEY;

typedef multimap<STRING_KEY, ARecord>		STLMultipleStringList;

class BaseCommon_Export STL_MultipleStringIndex : public RecordIndex
{
	friend class STLMultipleStringIndexRecordIt;

public:
	virtual ARecord GetRecord(Int64 nIndex) 
	{		
		return GetRecord(STRING(nIndex));
	}

	virtual ARecord GetRecord(float fIndex)
	{
		return GetRecord(STRING(fIndex));
	}
	//NOTE: 先找到第一个, 然后再逐个判断字符串KEY
	virtual ARecord GetRecord(const char* szIndex)
	{		
		STLMultipleStringList::iterator it = mStringRecordList.find(szIndex);
		if (it!=mStringRecordList.end())
			return it->second;

		return ARecord();
	}

	virtual ARecord GetRecord(const Data &nIndex)
	{
		return GetRecord(nIndex.string().c_str());
	}

	virtual bool ExistRecord(Int64 nIndex) { return ExistRecord(STRING(nIndex)); }
	virtual bool ExistRecord(float fIndex) { return ExistRecord(STRING(fIndex)); }
	virtual bool ExistRecord(const char* szIndex) { return GetRecord(szIndex); }

protected:
	virtual bool InsertRecord(ARecord scrRecord)
	{
		Data d = scrRecord->get(IndexFieldName());
		if (d.empty())
			return false;

		AString key = d.string();

		mStringRecordList.insert(STLMultipleStringList::value_type(key, scrRecord));
		return true;
	}

	virtual bool RemoveRecord(Int64 nIndex)
	{
		return RemoveRecord(STRING(nIndex));
	}
	virtual bool RemoveRecord(float fIndex)
	{
		return RemoveRecord(STRING(fIndex));
	}
	virtual bool RemoveRecord(const char* szIndex)
	{		
		WARN_LOG("RemoveRecord 目前只移除第一个KEY值对应的记录");	
		STLMultipleStringList::iterator it = mStringRecordList.find(szIndex);
		if (it!=mStringRecordList.end())
		{		
			mStringRecordList.erase(it);
			return true;
		}
		return false;
	}

	virtual bool RemoveRecord(ARecord record)
	{
		Data d = record->get(IndexFieldName());
		if (d.empty())
			return false;
		STRING_KEY key = d.string();
		STLMultipleStringList::iterator it = mStringRecordList.find(key);
		while (it!=mStringRecordList.end() && it->first==key)
		{
			if (it->second==record)
			{
				mStringRecordList.erase(it);
				return true;
			}
			++it;
		}
		return false;
	}

	virtual bool RemoveRecord(Int64 nIndex, ARecord record){ return RemoveRecord(STRING(nIndex)); }
	virtual bool RemoveRecord(const char *szIndex, ARecord record)
	{
		STRING_KEY key = szIndex;
		STLMultipleStringList::iterator it = mStringRecordList.find(key);
		while (it!=mStringRecordList.end() && it->first==key)	
		{			
			if (it->second==record)
			{
				mStringRecordList.erase(it);
				return true;
			}
			++it;				
		}
		return false;
	}

public:
	virtual ARecordIt GetRecordIt();

	virtual ARecordIt GetRecordIt(ARecord targetRecord);

	virtual ARecordIt GetLastRecordIt();

	virtual UInt64 GetCount(){ return mStringRecordList.size(); }
	virtual bool IsStringHashKey() { return false; }
	virtual bool IsMultiple() const override { return true; }

	virtual Int64 GetNextGrowthKey()
	{
		AssertNote(0, "字符串不支持自增长");
		return 0; 
	}

	virtual ARecord GetLastRecord()
	{
		STLMultipleStringList::reverse_iterator it =  mStringRecordList.rbegin();
		if (it!=mStringRecordList.rend())
			return it->second;

		return ARecord();
	}

	virtual ARecord GetRandRecord()
	{
		if (mStringRecordList.empty())
			return ARecord();
		size_t num = mStringRecordList.size();
		uint pos = CRand::RandUInt(num-1);

		uint i=0;
		for (STLMultipleStringList::iterator it=mStringRecordList.begin(); it!=mStringRecordList.end(); ++it, ++i)
		{
			if (i==pos)
				return it->second;
		}

		return ARecord();
	}

	virtual void ClearAll(tBaseTable *pOwnerTable)
	{		
		for (STLMultipleStringList::iterator it=mStringRecordList.begin(); it!=mStringRecordList.end(); ++it)
		{
			ARecord r = it->second;
			if (r && r->GetTable()==pOwnerTable)
				r._free();			
		}
		mStringRecordList.clear();
	}

protected:
	STLMultipleStringList		mStringRecordList;

};
//-------------------------------------------------------------------------

class STLMultipleStringIndexRecordIt : public RecordIt
{
	Auto<STL_MultipleStringIndex> mpOwnerIndex;

public:
	STLMultipleStringIndexRecordIt(STLMultipleStringList::iterator it, STL_MultipleStringIndex  *pIndex)
	{
		mpOwnerIndex = pIndex->GetSelf();

		mListIt = it;
		mEndIt = mpOwnerIndex->mStringRecordList.end();
	}

public:
	virtual ARecord GetRecord() 
	{
		if (mListIt!=mEndIt)
			return mListIt->second; 

		return ARecord();
	}
	virtual ARecord Begin()
	{ 
		if (!mpOwnerIndex->mStringRecordList.empty()) 
			return mpOwnerIndex->mStringRecordList.begin()->second; 
		
		return ARecord(); 
	}
	virtual bool operator  ++() 
	{ 
		if (mListIt!=mEndIt)
		{
			++mListIt; 
			return mListIt!=mEndIt;
		}
		return false; 
	}	
	virtual bool operator  --()
	{
		if (mListIt==mpOwnerIndex->mStringRecordList.begin())
		{
			mListIt = mpOwnerIndex->mStringRecordList.end();
			return false;
		}
		else if (mListIt==mpOwnerIndex->mStringRecordList.end())
			return false;

		--mListIt;			
		return true;
	}
	Auto<RecordIt> Copy()
	{
		return MEM_NEW STLMultipleStringIndexRecordIt(mListIt, mpOwnerIndex.getPtr());
	}

	virtual operator bool () { return mListIt!=mEndIt; }
	virtual void erase()
	{
		if (mListIt!=mEndIt)
			mListIt = mpOwnerIndex->mStringRecordList.erase(mListIt);
	}


protected:
	STLMultipleStringList::iterator	mListIt;
	STLMultipleStringList::iterator	mEndIt;
};
//-------------------------------------------------------------------------

#endif //_INCLUDE_MULTIPLESTRINGINDEX_H_