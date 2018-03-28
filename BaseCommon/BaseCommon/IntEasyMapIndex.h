#ifndef _INCLUDE_INTMAPINDEX_H_
#define _INCLUDE_INTMAPINDEX_H_

#include "RecordIndex.h"

#include "EasyMap.h"
#include "StringTool.h"
//-------------------------------------------------------------------------
typedef EasyMap<int, ARecord>		IntEasyMapRecordList;

//-------------------------------------------------------------------------
class IntEasyMapRecordIt : public RecordIt
{
	friend class IntEasyMapIndex;
public:
	IntEasyMapRecordIt(IntEasyMapRecordList::Iterator it)
	{
		mListIt = it;
	}

public:
	virtual ARecord GetRecord() { return mListIt.get(); }
	virtual ARecord Begin() { return mListIt.begin(); }
	virtual bool operator  ++() { return mListIt.next(); }	
	virtual bool operator  --() { return mListIt.previou(); }
	virtual operator bool () { return mListIt.have(); }

	virtual void erase() { mListIt.erase(); }


	virtual Auto<RecordIt> Copy()
	{
		return MEM_NEW IntEasyMapRecordIt(mListIt);
	}

    virtual bool GetKey(Int64 &key) override
    {
        key = mListIt.key();
        return true;
    }

protected:
	IntEasyMapRecordList::Iterator	mListIt;
};
//-------------------------------------------------------------------------
class BaseCommon_Export IntEasyMapIndex : public RecordIndex
{

public:
	virtual ARecord GetRecord(Int64 nIndex) 
	{		
		return mRecordList.find((int)nIndex);
	}
	virtual ARecord GetRecord(float fIndex)
	{
		return GetRecord((Int64)fIndex);
	}

	virtual ARecord GetRecord(const char* szIndex)
	{
		return GetRecord(TOINT64(szIndex));
	}

	virtual ARecord GetRecord(const Data &nIndex)
	{
		return GetRecord((Int64)nIndex);
	}

	virtual bool ExistRecord(Int64 nIndex) { return mRecordList.exist((int)nIndex); }
	virtual bool ExistRecord(float fIndex) { return mRecordList.exist((int)fIndex); }
	virtual bool ExistRecord(const char* szIndex) { return mRecordList. exist(StringTool::Int(szIndex)); }

	virtual bool InsertRecord(ARecord scrRecord)
	{
		Data d = scrRecord->get(IndexFieldName());
		if (d.empty())
			return false;

		int key = d;
		mRecordList.insert(key, scrRecord);
		return true;
	}

	virtual bool InsertLast(ARecord scrRecord)
	{
		if (scrRecord->_set(scrRecord->getFieldCol(IndexFieldName()), GetNextGrowthKey()))
			return InsertRecord(scrRecord);
		return false;
	}

	virtual bool RemoveRecord(Int64 nIndex)
	{
		return mRecordList.erase((int)nIndex)>0;
	}
	virtual bool RemoveRecord(float fIndex)
	{
		return mRecordList.erase((int)fIndex)>0;
	}
	virtual bool RemoveRecord(const char* szIndex)
	{
		return mRecordList.erase(StringTool::Int(szIndex))>0;
	}

	virtual bool RemoveRecord(ARecord record)
	{
		Data d = record->get(IndexFieldName());
		if (d.empty())
			return false;

		int key = d;
		return mRecordList.erase(key)>0;
	}

public:
	virtual ARecordIt GetRecordIt()
	{
		return MEM_NEW IntEasyMapRecordIt(mRecordList.begin());
	}

	virtual ARecordIt GetRecordIt(const Data &beginKey)
	{
		if (beginKey.string().length()<=0)
			return ARecordIt();

		int key = beginKey;
		if (mRecordList.exist(key))
			return MEM_NEW IntEasyMapRecordIt(mRecordList.begin(key));

		return ARecordIt();
	}

	virtual ARecordIt GetLastRecordIt()
	{
		return MEM_NEW IntEasyMapRecordIt(mRecordList.GetLastIterator());
	};

	virtual UInt64 GetCount(){ return mRecordList.size(); }
	virtual Int64 GetNextGrowthKey()
	{
		const IntEasyMapRecordList::Value *p = mRecordList.last();
		if (p!=NULL)
			return p->mKey+1;

		return 1; 
	}

	virtual UInt64 GetSortPos(ARecord record)
	{
		Data key = record->get(IndexFieldName());

		ARecordIt it = GetRecordIt(key);
		if (it)
		{
			while (true)
			{
				if (*it == record)
				{
					IntEasyMapRecordIt *indexIt = dynamic_cast<IntEasyMapRecordIt*>(it.getPtr());
					AssertEx(indexIt!=NULL, "Must is IntEasyMapRecordIt");
					return indexIt->mListIt.getCurrentPos();
				}
				if (! (++(*it)) )
					break;
			}
		}
		return NULL_POS;
	}

	virtual ARecord GetLastRecord()
	{
		const IntEasyMapRecordList::Value *p = mRecordList.last();
		if (p!=NULL)
			return p->mVal;
		return ARecord();
	}

	virtual ARecord GetRandRecord()
	{
		if (mRecordList.empty())
			return ARecord();

		Int64 randPos = CRand::RandUInt(mRecordList.size()-1);
		return mRecordList.get(randPos);
	}

	virtual void ClearAll(tBaseTable *pOwnerTable)
	{
		for (size_t i=0; i<mRecordList.size(); ++i)
		{
			ARecord re = mRecordList.get(i);
			if (re && re->GetTable()==pOwnerTable)
				re._free();
		}
		mRecordList.clear(false);
	}

	virtual ARecordIt GetRecordIt(ARecord targetRecord)
	{
		IntEasyMapRecordList::Iterator it= mRecordList.begin((int)targetRecord->getIndexData());
		if (it)
			return MEM_NEW IntEasyMapRecordIt(it);

		return ARecordIt();
	}

protected:
	IntEasyMapRecordList		mRecordList;
};

#if TABLE_USE_STL_INDEX
#	include "Int64MapIndex.h"
#	include "MultipleInt64MapIndex.h"

// 直接使用64位的索引
//typedef Int64MapIndex			IntEasyMapIndex;

	typedef MultipleInt64MapIndex	MultipleIntMapIndex;
#else
class MultipleIntMapIndex : public IntEasyMapIndex
{
public:
	virtual bool InsertRecord(ARecord scrRecord)
	{
		Data d = scrRecord->get(IndexFieldName());
		if (d.empty())
			return false;

		mRecordList.insert((int)d, scrRecord);
		return true;
	}

	virtual bool RemoveRecord(ARecord record)
	{
		Data d = record->get(IndexFieldName());
		if (d.empty())
			return false;

		int key = d;
		return mRecordList.erase(key, record)>0;
	}

	virtual bool MultipleKey() { return true; }

public:
	virtual ARecord GetRecord(Int64 nIndex)
	{		
		return mRecordList.findFirst((int)nIndex);
	}

};

#endif

//-------------------------------------------------------------------------

#endif //_INCLUDE_INTMAPINDEX_H_