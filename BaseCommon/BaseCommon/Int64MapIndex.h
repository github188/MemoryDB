
#ifndef _INCLUDE_INT64MAPINDEX_H_
#define _INCLUDE_INT64MAPINDEX_H_

#include "RecordIndex.h"

#include "StringTool.h"
#include "EasyMap.h"
#include <map>

//-------------------------------------------------------------------------
using namespace std;
//using namespace stdext;

typedef map<INT_INDEX_TYPE, ARecord>		IntMapIndexRecordList;
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
class BaseCommon_Export Int64MapIndex : public RecordIndex
{
	friend class Int64IndexRecordIt;
	friend class Right_HashIntIndexRecordIt;

public:
	virtual ARecord GetRecord(Int64 nIndex) 
	{		
		IntMapIndexRecordList::iterator it = mIntMapRecordList.find((INT_INDEX_TYPE)nIndex);
		if (it!=mIntMapRecordList.end())
			return it->second;

		return ARecord();
	}

	virtual ARecord GetRecord(float fIndex)
	{
		return GetRecord((Int64)fIndex);
	}
	//NOTE: 先找到第一个, 然后再逐个判断字符串KEY
	virtual ARecord GetRecord(const char* szIndex)
	{		
		return GetRecord(TOINT64(szIndex));
	}

	virtual ARecord GetRecord(const Data &nIndex)
	{
		return GetRecord((Int64)nIndex);
	}

	virtual bool ExistRecord(Int64 nIndex) {  return GetRecord(nIndex); }
	virtual bool ExistRecord(float fIndex) { return ExistRecord((Int64)(fIndex)); }
	virtual bool ExistRecord(const char* szIndex) { return ExistRecord(TOINT64(szIndex)); }

protected:
	virtual bool InsertRecord(ARecord scrRecord)
	{
		Data d = scrRecord->get(IndexFieldName());
		if (d.empty())
			return false;

		INT_INDEX_TYPE key = d;

		pair<IntMapIndexRecordList::iterator, bool> p = mIntMapRecordList.insert(IntMapIndexRecordList::value_type(key, scrRecord));
		return p.second;
	}

	virtual bool InsertLast(ARecord scrRecord)
	{
		if (scrRecord->_set(scrRecord->getFieldCol(IndexFieldName()), GetNextGrowthKey()))
			return InsertRecord(scrRecord);
		return false;
	}

	virtual bool RemoveRecord(Int64 nIndex)
	{
		IntMapIndexRecordList::iterator it = mIntMapRecordList.find((INT_INDEX_TYPE)nIndex);
		if (it!=mIntMapRecordList.end())
		{
			mIntMapRecordList.erase(it);
			return true;
		}
		return false;
	}
	virtual bool RemoveRecord(float fIndex)
	{
		return RemoveRecord((Int64)(fIndex));
	}
	virtual bool RemoveRecord(const char* szIndex)
	{		
		return RemoveRecord(TOINT64(szIndex));
	}

	virtual bool RemoveRecord(ARecord record)
	{
		Data d = record->get(IndexFieldName());
		if (d.empty())
			return false;

		return RemoveRecord((Int64)d);
	}

public:
	virtual ARecordIt GetRecordIt();

	virtual ARecordIt GetRecordIt(ARecord targetRecord) override;

	virtual ARecordIt GetLastRecordIt();


	virtual UInt64 GetSortPos(ARecord record) 
	{
		Data d = record->get(IndexFieldName());
		if (d.empty())
			return false;

		INT_INDEX_TYPE key = d;

		UInt64 i = 0;
		IntMapIndexRecordList::iterator it = mIntMapRecordList.find(key);
		for (; it!=mIntMapRecordList.end(); ++it)
		{			
			++i;
		}
		return i;
	}

	virtual UInt64 GetCount(){ return mIntMapRecordList.size(); }
	virtual bool IsStringHashKey() { return false; }

	virtual Int64 GetNextGrowthKey()
	{
		IntMapIndexRecordList::reverse_iterator it = mIntMapRecordList.rbegin();
		if (it!=mIntMapRecordList.rend())
		{
			return it->first + 1;
		}		
		return 1; 
	}

	virtual ARecord GetLastRecord()
	{
		IntMapIndexRecordList::reverse_iterator it =  mIntMapRecordList.rbegin();
		if (it!=mIntMapRecordList.rend())
			return it->second;

		return ARecord();
	}

	virtual ARecord GetRandRecord()
	{
		if (mIntMapRecordList.empty())
			return ARecord();
		size_t num = mIntMapRecordList.size();
		uint pos = CRand::RandUInt(num-1);

		uint i=0;
		for (IntMapIndexRecordList::iterator it=mIntMapRecordList.begin(); it!=mIntMapRecordList.end(); ++it, ++i)
		{
			if (i==pos)
				return it->second;
		}

		return ARecord();
	}

	virtual void ClearAll(tBaseTable *pOwnerTable)
	{		
		for (IntMapIndexRecordList::iterator it=mIntMapRecordList.begin(); it!=mIntMapRecordList.end(); ++it)
		{
			ARecord r = it->second;
			if (r && r->GetTable()==pOwnerTable)
				r._free();
		}
		mIntMapRecordList.clear();
	}

protected:
	IntMapIndexRecordList		mIntMapRecordList;
};
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
class Int64MapIndex;

class Int64IndexRecordIt : public RecordIt
{
	Auto<Int64MapIndex>	mpOwnerIndex;

public:
	Int64IndexRecordIt(IntMapIndexRecordList::iterator it, Int64MapIndex *pIndex);

public:
	virtual ARecord GetRecord();
	virtual ARecord Begin();
	virtual bool operator  ++() ; 
	virtual bool operator  --()
	{
		if (mListIt==mpOwnerIndex->mIntMapRecordList.begin())
		{
			mListIt = mpOwnerIndex->mIntMapRecordList.end();
			return false;
		}
		else if (mListIt==mpOwnerIndex->mIntMapRecordList.end())
			return false;

		--mListIt;			
		return true;
	}

	virtual operator bool ();
	virtual void erase();

	Auto<RecordIt> Copy()
	{
		return MEM_NEW Int64IndexRecordIt(mListIt, mpOwnerIndex.getPtr());
	}

protected:
	IntMapIndexRecordList::iterator	mListIt;
};



//-------------------------------------------------------------------------

////-------------------------------------------------------------------------
//typedef EasyMap<Int64, ARecord>		Int64EasyMapIndexList;
//
////-------------------------------------------------------------------------
//class Int64EasyMapIndexRecordIt : public RecordIt
//{
//public:
//	Int64EasyMapIndexRecordIt(Int64EasyMapIndexList::Iterator it)
//	{
//		mListIt = it;
//	}
//
//public:
//	virtual ARecord GetRecord() { return mListIt.get(); }
//	virtual ARecord Begin() { return mListIt.begin(); }
//	virtual bool operator  ++() { return mListIt.next(); }	
//	virtual bool operator  --() { return mListIt.previou(); }
//	virtual operator bool () { return mListIt.have(); }
//	virtual void erase() { mListIt.remove(); }
//
//	virtual Auto<RecordIt> Copy() override
//	{
//		return MEM_NEW Int64EasyMapIndexRecordIt(mListIt);
//	}
//
//protected:
//	Int64EasyMapIndexList::Iterator	mListIt;
//};
////-------------------------------------------------------------------------
//class BaseCommon_Export Int64EasyMapIndex : public RecordIndex
//{
//public:
//	virtual ARecord GetRecord(Int64 nIndex) 
//	{		
//		return mRecordList.find(nIndex);
//	}
//	virtual ARecord GetRecord(float fIndex)
//	{
//		return GetRecord( (Int64)fIndex );
//	}
//
//	virtual ARecord GetRecord(const char* szIndex)
//	{
//		return GetRecord(TOINT64(szIndex));
//	}
//
//	virtual ARecord GetRecord(const Data &nIndex)
//	{
//		return GetRecord( (Int64)nIndex );
//	}
//
//	virtual bool ExistRecord(Int64 nIndex) { return mRecordList.exist(nIndex); }
//	virtual bool ExistRecord(float fIndex) { return mRecordList.exist((Int64)fIndex); }
//	virtual bool ExistRecord(const char* szIndex) { return mRecordList. exist(TOINT64(szIndex)); }
//
//	virtual bool InsertRecord(ARecord scrRecord)
//	{
//		Data d = scrRecord->get(IndexFieldName());
//		if (d.empty())
//			return false;
//
//		Int64 key = (Int64)(UInt64)d;		
//		mRecordList.insert(key, scrRecord);
//		return true;
//	}
//
//	virtual bool InsertLast(ARecord scrRecord)
//	{
//		if (scrRecord->_set(scrRecord->getFieldCol(IndexFieldName()), GetNextGrowthKey()))
//			return InsertRecord(scrRecord);
//		return false;
//	}
//
//	virtual bool RemoveRecord(Int64 nIndex)
//	{
//		return mRecordList.erase(nIndex)>0;
//	}
//	virtual bool RemoveRecord(float fIndex)
//	{
//		return mRecordList.erase((UInt64)(fIndex))>0;
//	}
//	virtual bool RemoveRecord(const char* szIndex)
//	{
//		return mRecordList.erase(TOINT64(szIndex))>0;
//	}
//
//	virtual bool RemoveRecord(ARecord record)
//	{
//		Data d = record->get(IndexFieldName());
//		if (d.empty())
//			return false;
//
//		Int64 key = (Int64)(UInt64)d;
//		return mRecordList.erase(key)>0;
//	}
//
//public:
//	virtual ARecordIt GetRecordIt()
//	{
//		return MEM_NEW Int64EasyMapIndexRecordIt(mRecordList.begin());
//	}
//
//	virtual ARecordIt GetRecordIt(ARecord record) override
//	{
//		if (!record)
//			return ARecordIt();
//		Data beginKey = record[0];
//
//		if (beginKey.string().length()<=0)
//			return ARecordIt();
//
//		Int64 key = (Int64)(UInt64)beginKey;
//		if (mRecordList.exist(key))
//			return MEM_NEW Int64EasyMapIndexRecordIt(mRecordList.begin(key));
//
//		return ARecordIt();
//	}
//
//	virtual ARecordIt GetLastRecordIt()
//	{
//		return MEM_NEW Int64EasyMapIndexRecordIt(mRecordList.GetLastIterator());
//	};
//
//	virtual UInt64 GetCount(){ return mRecordList.size(); }
//	virtual Int64 GetNextGrowthKey()
//	{
//		const Int64EasyMapIndexList::Value *p = mRecordList.last();
//		if (p!=NULL)
//			return p->mKey+1;
//		return 1; 
//	}
//
//	virtual ARecord GetLastRecord()
//	{
//		const Int64EasyMapIndexList::Value *p = mRecordList.last();
//		if (p!=NULL)
//			return p->mVal;
//		return ARecord();
//	}
//
//	virtual ARecord GetRandRecord()
//	{
//		if (mRecordList.empty())
//			return ARecord();
//
//		Int64 randPos = CRand::RandUInt(mRecordList.size()-1);
//		return mRecordList.get(randPos);
//	}
//
//	virtual void ClearAll(tBaseTable *pOwnerTable)
//	{
//		for (size_t i=0; i<mRecordList.size(); ++i)
//		{
//			ARecord re = mRecordList.get(i);
//			if (re && re->GetTable()==pOwnerTable)
//				re._free();
//		}
//		mRecordList.clear(false);
//	}
//
//protected:
//	Int64EasyMapIndexList		mRecordList;
//};
//
//class MultipleInt64EasyMapIndex : public Int64EasyMapIndex
//{
//public:
//	virtual bool AppendRecord(ARecord scrRecord, bool bReplace, bool bRestore = false)
//	{
//		Data d = scrRecord->get(IndexFieldName());
//		if (d.empty())
//			return false;
//
//		Int64 key = (Int64)(UInt64)d;
//		if (bReplace)
//		{
//			mRecordList.erase(key);
//		}
//		mRecordList.insert(key, scrRecord);
//		return true;
//	}
//
//	virtual bool RemoveRecord(ARecord record)
//	{
//		Data d = record->get(IndexFieldName());
//		if (d.empty())
//			return false;
//
//		Int64 key = (Int64)(UInt64)d;
//		return mRecordList.erase(key, record)>0;
//	}
//
//	virtual bool MultipleKey() { return true; }
//
//public:
//	virtual ARecord GetRecord(Int64 nIndex)
//	{		
//		return mRecordList.findFirst(nIndex);
//	}
//};



//-------------------------------------------------------------------------


#endif //_INCLUDE_INT64MAPINDEX_H_