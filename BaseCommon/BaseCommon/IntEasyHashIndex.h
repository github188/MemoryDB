/********************************************************************
	created:	2017/06/20
	created:	20:6:2017   11:38
	filename: 	E:\RemoteCode\BaseCommon\BaseCommon\IntEasyHashIndex.h
	file path:	E:\RemoteCode\BaseCommon\BaseCommon
	file base:	IntEasyHashIndex
	file ext:	h
	author:		Yang Wenge
	
	purpose:	单键整数索引
	NOTE:		已经使用EasyHash替换，经过内测，目前主要用在配置或逻辑表格中 2017.6.20
					调整应用于MemoryDBTable, 同时增加整数索引		2017.6.20
*********************************************************************/
#ifndef _INCLUDE_INTHASHINDEX_H_
#define _INCLUDE_INTHASHINDEX_H_

#include "RecordIndex.h"

#include "EasyHash.h"
#include "StringTool.h"
//-------------------------------------------------------------------------
template<>
size_t EasyHash<int, ARecord>::_hashKey(const char *key) const
{
	return TOINT(key);
}
template<>
bool EasyHash<int, ARecord>::CompareKey(const int &key, const char *szKey)
{
	return key==TOINT(szKey);	
}

typedef EasyHash<int, ARecord>		IntEasyHashIndexRecordList;

//-------------------------------------------------------------------------
class IntEasyHashIndexRecordIt : public RecordIt
{
public:
	IntEasyHashIndexRecordIt(IntEasyHashIndexRecordList::iterator it)
	{
		mListIt = it;
	}

public:
	virtual ARecord GetRecord() { return mListIt.get(); }
	virtual ARecord Begin() { mListIt.reset(); return *mListIt; }
	virtual bool operator  ++() { return mListIt.next(); }	
	virtual bool operator  --() { return mListIt.previou(); }
	virtual operator bool () { return mListIt.have(); }
	virtual void erase() { mListIt.erase(); }
	virtual Auto<RecordIt> Copy()
	{
		return MEM_NEW IntEasyHashIndexRecordIt(mListIt);
	}

	virtual bool GetKey(Int64 &key) override
	{
		key = mListIt.key();
		return true;
	}

protected:
	IntEasyHashIndexRecordList::iterator	mListIt;
};
//-------------------------------------------------------------------------
class BaseCommon_Export IntEasyHashIndex : public RecordIndex
{
public:	
	virtual ARecord GetRecord(Int64 nIndex)  override
	{		
		return mRecordList.find((int)nIndex);
	}
	virtual ARecord GetRecord(float fIndex) override
	{
		return GetRecord( (Int64)(fIndex) );
	}
	
	virtual ARecord GetRecord(const char* szIndex) override
	{
		return GetRecord(TOINT64(szIndex));
	}

	virtual ARecord GetRecord(const Data &nIndex) override
	{
		return GetRecord(nIndex.string().c_str());
	}

	virtual bool ExistRecord(Int64 nIndex) { return mRecordList.exist((int)nIndex); }
	virtual bool ExistRecord(float fIndex) { return mRecordList.exist((int)(fIndex)); }
	virtual bool ExistRecord(const char* szIndex) { return mRecordList.exist((int)TOINT64(szIndex)); }

	// NOTE: 如果已经存在相同的key, 会直接覆盖存在的记录
	virtual bool InsertRecord(ARecord scrRecord)
	{
		Data d = scrRecord->get(IndexFieldName());
		if (d.empty())
			return false;
		
		mRecordList.insert((int)d, scrRecord);
		return true;
	}

	virtual bool RemoveRecord(int nIndex)
	{
		return mRecordList.erase(nIndex);
	}
	virtual bool RemoveRecord(float fIndex)
	{
		return mRecordList.erase((int)(fIndex));
	}
	virtual bool RemoveRecord(const char* szIndex)
	{
		return mRecordList.erase( (int)TOINT64(szIndex));
	}

	virtual bool RemoveRecord(Int64 nIndex)
	{
		return mRecordList.erase((int)nIndex);
	}

	virtual bool RemoveRecord(ARecord record)
	{
		Data d = record->get(IndexFieldName());
		if (d.empty())
			return false;
		
		return mRecordList.erase((int)d);
	}

	virtual bool InsertLast(ARecord scrRecord) override
	{
		Int64 x = 1;
		if (!mRecordList.empty())
		{		
			x = 0XFFFFFFFF;
			x =-x;
			for (auto it = mRecordList.begin(); it; ++it)
			{
				if (x<it.key())
					x = it.key();
			}
			++x;
		}
		scrRecord->_set(scrRecord->getFieldCol(IndexFieldName()), x);
		mRecordList.insert((int)x, scrRecord);
		return true; 
	}

	virtual Int64 GetNextGrowthKey() override
	{
		Int64 x = 1;
		if (!mRecordList.empty())
		{		
			x = 0XFFFFFFFF;
			x =-x;
			for (auto it = mRecordList.begin(); it; ++it)
			{
				if (x<it.key())
					x = it.key();
			}
			++x;
		}
		return x;
	}

public:
	virtual ARecordIt GetRecordIt()
	{
		return MEM_NEW IntEasyHashIndexRecordIt(mRecordList.begin());
	}

	//virtual ARecordIt GetRecordIt(ARecord record) override
	//{
	//	if (!record)
	//		return ARecordIt();
	//	Data beginKey = record[0];
	//	if (beginKey.string().length()<=0)
	//		return ARecordIt();

	//	auto it = mRecordList.findIt(beginKey.string().c_str());
	//	if (it)
	//		return MEM_NEW IntEasyHashIndexRecordIt(it);

	//	return ARecordIt();
	//}

	virtual ARecordIt GetLastRecordIt()
	{
		return MEM_NEW IntEasyHashIndexRecordIt(mRecordList.GetLastIterator());
	};

	virtual ARecord GetLastRecord()
	{		
		return mRecordList.back();
	}

	virtual UInt64 GetCount(){ return mRecordList.size(); }

	virtual ARecord GetRandRecord()
	{
		NOTE_LOG("WARN: IntEasyHashIndex 当前未实现，使用此功能无意义");
		return ARecord();
	}

	virtual void ClearAll(tBaseTable *pOwnerTable)
	{
		for (auto it=mRecordList.begin(); it; ++it)
		{
			ARecord re = *it;
			if (re && re->GetTable()==pOwnerTable)
				re._free();
		}
		mRecordList.clear(false);
	}

	virtual ARecordIt GetRecordIt(ARecord targetRecord)
	{
		IntEasyHashIndexRecordList::iterator it = mRecordList.findIt((int)targetRecord->getIndexData());

		if (it.get()==targetRecord)
			return MEM_NEW IntEasyHashIndexRecordIt(it);

		return ARecordIt();
	}
protected:
	IntEasyHashIndexRecordList		mRecordList;
};


//-------------------------------------------------------------------------


#endif //_INCLUDE_INT64HASHINDEX_H_