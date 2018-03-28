/********************************************************************
	created:	2017/06/20
	created:	20:6:2017   11:38
	filename: 	E:\RemoteCode\BaseCommon\BaseCommon\ListRecordIndex.h
	file path:	E:\RemoteCode\BaseCommon\BaseCommon
	file base:	ListRecordIndex
	file ext:	h
	author:		Yang Wenge
	
	purpose:	单键整数无序索引
*********************************************************************/
#ifndef _INCLUDE_INTHASHINDEX_H_
#define _INCLUDE_INTHASHINDEX_H_

#include "RecordIndex.h"

#include <list>
#include "StringTool.h"
#include "BaseCommon.h"
//-------------------------------------------------------------------------


typedef std::list<ARecord>		ListIndexRecordList;

//-------------------------------------------------------------------------
class ListRecordIndex;
class BaseCommon_Export_H ListIndexRecordIt : public RecordIt
{
public:
	ListIndexRecordIt(ListIndexRecordList *pIndex, ListIndexRecordList::iterator it)
		: mpRecordList(pIndex)
	{
		mListIt = it;
	}

public:
	virtual ARecord GetRecord() { return *mListIt; }
	virtual ARecord Begin() { mListIt = mpRecordList->begin(); if (mListIt!=mpRecordList->end()) return *mListIt; return ARecord(); }
	virtual bool operator  ++() { ++mListIt; return mListIt!=mpRecordList->end(); }	
	virtual bool operator  --() { ERROR_LOG("ListRecord 不支持 --"); return false; }
	virtual operator bool () { return mListIt!=mpRecordList->end(); }
	virtual void erase() { mListIt = mpRecordList->erase(mListIt); }
	virtual Auto<RecordIt> Copy()
	{
		return MEM_NEW ListIndexRecordIt(mpRecordList, mListIt);
	}

protected:
	ListIndexRecordList::iterator	mListIt;
	ListIndexRecordList		*mpRecordList;
};
//-------------------------------------------------------------------------
class BaseCommon_Export_H ListRecordIndex : public RecordIndex
{
public:	
	virtual ARecord GetRecord(Int64 nIndex)  override
	{		
		for (auto it=mRecordList.begin(); it!=mRecordList.end(); ++it)
		{
			if ( (Int64) ((*it)[0]) == nIndex)
				return *it;
		}
		return ARecord();
	}
	virtual ARecord GetRecord(float fIndex) override
	{
		return GetRecord( (Int64)(fIndex) );
	}
	
	virtual ARecord GetRecord(const char* szIndex) override
	{
		for (auto it=mRecordList.begin(); it!=mRecordList.end(); ++it)
		{
			if ( (*it)[0].string() == szIndex)
				return *it;
		}
		return ARecord();
	}

	virtual ARecord GetRecord(const Data &nIndex) override
	{
		return GetRecord(nIndex.string().c_str());
	}

	virtual bool ExistRecord(Int64 nIndex) { return GetRecord(nIndex); }
	virtual bool ExistRecord(float fIndex) { return GetRecord(fIndex); }
	virtual bool ExistRecord(const char* szIndex) { return GetRecord(szIndex); }

	// NOTE: 如果已经存在相同的key, 会直接覆盖存在的记录
	virtual bool InsertRecord(ARecord scrRecord)
	{		
		mRecordList.push_back(scrRecord);
		return true;
	}

	virtual bool RemoveRecord(int nIndex)
	{
		return RemoveRecord((Int64)nIndex);
	}
	virtual bool RemoveRecord(float fIndex)
	{
		return RemoveRecord((int)(fIndex));
	}
	virtual bool RemoveRecord(const char* szIndex)
	{
		size_t s = mRecordList.size();
		for (auto it=mRecordList.begin(); it!=mRecordList.end(); )
		{
			if ( (*it)[0].string() == szIndex)	
				it = mRecordList.erase(it);
			else
				++it;
		}
		return mRecordList.size()!=s;
	}

	virtual bool RemoveRecord(Int64 nIndex)
	{
		size_t s = mRecordList.size();
		for (auto it=mRecordList.begin(); it!=mRecordList.end(); )
		{
			if ( (Int64) ((*it)[0]) == nIndex)			
				it = mRecordList.erase(it);
			else
				++it;
		}
		return mRecordList.size()!=s;
	}

	virtual bool RemoveRecord(ARecord record)
	{
		for (auto it=mRecordList.begin(); it!=mRecordList.end(); )
		{
			if ( (*it) == record)			
			{
				it = mRecordList.erase(it);
				return true;
			}
			else
				++it;
		}
		return false;
	}

	virtual bool InsertLast(ARecord scrRecord) override
	{		
		mRecordList.push_back(scrRecord);
		return true; 
	}

public:
	virtual ARecordIt GetRecordIt()
	{
		return MEM_NEW ListIndexRecordIt(&mRecordList, mRecordList.begin());
	}

	//virtual ARecordIt GetRecordIt(const Data &beginKey)
	//{
	//	if (beginKey.string().length()<=0)
	//		return ARecordIt();

	//	for (auto it=mRecordList.begin(); it!=mRecordList.end(); ++it)
	//	{
	//		if ( (*it)[0].string() == beginKey.string())	
	//			return MEM_NEW IntEasyHashIndexRecordIt(&mRecordList, it);
	//	}

	//	return ARecordIt();
	//}

	virtual ARecordIt GetLastRecordIt()
	{
		//return MEM_NEW ListIndexRecordIt(&mRecordList, mRecordList.rbegin());
		ERROR_LOG("ListReocrdIndex 不支持 GetLastRecordIt");
		return ARecordIt();
	};

	virtual ARecord GetLastRecord()
	{		
		return mRecordList.back();
	}

	virtual UInt64 GetCount(){ return mRecordList.size(); }

	virtual ARecord GetRandRecord()
	{
		NOTE_LOG("WARN: ListRecordIndex 当前未实现，使用此功能无意义");
		return ARecord();
	}

	virtual void ClearAll(tBaseTable *pOwnerTable)
	{
		for (auto it=mRecordList.begin(); it!=mRecordList.end(); ++it)
		{
			ARecord re = *it;
			if (re && re->GetTable()==pOwnerTable)
				re._free();
		}
		mRecordList.clear();
	}

	virtual ARecordIt GetRecordIt(ARecord targetRecord)
	{
		for (auto it=mRecordList.begin(); it!=mRecordList.end(); )
		{
			if ( (*it) == targetRecord)			
			{
				return MEM_NEW ListIndexRecordIt(&mRecordList, it);				
			}
		}

		return ARecordIt();
	}
protected:
	ListIndexRecordList		mRecordList;
};


//-------------------------------------------------------------------------


#endif //_INCLUDE_INT64HASHINDEX_H_