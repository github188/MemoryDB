/********************************************************************
	created:	2014/07/24
	created:	24:7:2014   11:38
	filename: 	E:\RemoteCode\BaseCommon\BaseCommon\IntEasyHashIndex.h
	file path:	E:\RemoteCode\BaseCommon\BaseCommon
	file base:	IntEasyHashIndex
	file ext:	h
	author:		Yang Wenge
	
	purpose:	单键64整数索引
	NOTE:		已经使用EasyHash替换，经过内测，目前主要用在配置或逻辑表格中 2017.6.20
					调整应用于MemoryDBTable, 同时增加整数索引		2017.6.20
*********************************************************************/
#ifndef _INCLUDE_INT64HASHINDEX_H_
#define _INCLUDE_INT64HASHINDEX_H_

#include "RecordIndex.h"


#include "EasyHash.h"
#include "StringTool.h"
//-------------------------------------------------------------------------
template<>
size_t EasyHash<Int64, ARecord>::_hashKey(const char *key) const
{
	return TOINT64(key);
}
template<>
bool EasyHash<Int64, ARecord>::CompareKey(const Int64 &key, const char *szKey)
{
	return key==TOINT64(szKey);	
}

typedef EasyHash<Int64, ARecord>		Int64HashIndexRecordList;

//-------------------------------------------------------------------------
class Int64HashIndexRecordIt : public RecordIt
{
public:
	Int64HashIndexRecordIt(Int64HashIndexRecordList::iterator it)
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
		return MEM_NEW Int64HashIndexRecordIt(mListIt);
	}

	virtual bool GetKey(Int64 &key) override
	{
		key = mListIt.key();
		return true;
	}

protected:
	Int64HashIndexRecordList::iterator	mListIt;
};
//-------------------------------------------------------------------------
class BaseCommon_Export Int64HashIndex : public RecordIndex
{
public:	
	virtual ARecord GetRecord(Int64 nIndex) 
	{		
		return mRecordList.find(nIndex);
	}
	virtual ARecord GetRecord(float fIndex)
	{
		return GetRecord( (Int64)(fIndex) );
	}
	
	virtual ARecord GetRecord(const char* szIndex)
	{
		return GetRecord(TOINT64(szIndex));
	}

	virtual ARecord GetRecord(const Data &nIndex)
	{
		return GetRecord(nIndex.string().c_str());
	}

	virtual bool ExistRecord(Int64 nIndex) { return mRecordList.exist(nIndex); }
	virtual bool ExistRecord(float fIndex) { return mRecordList.exist((Int64)(fIndex)); }
	virtual bool ExistRecord(const char* szIndex) { return mRecordList.exist(TOINT64(szIndex)); }

	virtual bool InsertRecord(ARecord scrRecord)
	{
		Data d = scrRecord->get(IndexFieldName());
		if (d.empty())
			return false;
		
		mRecordList.insert((Int64)d, scrRecord);
		return true;
	}

	virtual bool RemoveRecord(int nIndex)
	{
		return mRecordList.erase(nIndex);
	}
	virtual bool RemoveRecord(float fIndex)
	{
		return mRecordList.erase((Int64)(fIndex));
	}
	virtual bool RemoveRecord(const char* szIndex)
	{
		return mRecordList.erase( TOINT64(szIndex));
	}

	virtual bool RemoveRecord(Int64 nIndex)
	{
		return mRecordList.erase(nIndex);
	}

	virtual bool RemoveRecord(ARecord record)
	{
		Data d = record->get(IndexFieldName());
		if (d.empty())
			return false;
		
		return mRecordList.erase((Int64)d);
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
		mRecordList.insert(x, scrRecord);
		return true; 
	}

public:
	virtual ARecordIt GetRecordIt()
	{
		return MEM_NEW Int64HashIndexRecordIt(mRecordList.begin());
	}

	//virtual ARecordIt GetRecordIt(const Data &beginKey)
	//{
	//	if (beginKey.string()=="")
	//		return ARecordIt();

	//	auto it = mRecordList.findIt(beginKey.string().c_str());
	//	if (it)
	//		return MEM_NEW Int64HashIndexRecordIt(it);

	//	return ARecordIt();
	//}

	virtual ARecordIt GetLastRecordIt()
	{
		return MEM_NEW Int64HashIndexRecordIt(mRecordList.GetLastIterator());
	};

	virtual ARecord GetLastRecord()
	{		
		return mRecordList.back();
	}

	virtual UInt64 GetCount(){ return mRecordList.size(); }

	virtual ARecord GetRandRecord()
	{
		NOTE_LOG("WARN: Int64HashIndex 当前未实现，使用此功能无意义");
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
		Int64HashIndexRecordList::iterator it = mRecordList.findIt((Int64)targetRecord->getIndexData());

		if (it.get()==targetRecord)
			return MEM_NEW Int64HashIndexRecordIt(it);

		return ARecordIt();
	}
protected:
	Int64HashIndexRecordList		mRecordList;
};


//-------------------------------------------------------------------------


#endif //_INCLUDE_INT64HASHINDEX_H_