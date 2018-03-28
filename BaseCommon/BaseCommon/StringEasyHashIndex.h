/********************************************************************
	created:	2014/07/24
	created:	24:7:2014   11:38
	filename: 	E:\RemoteCode\BaseCommon\BaseCommon\StringEasyHashIndex.h
	file path:	E:\RemoteCode\BaseCommon\BaseCommon
	file base:	StringEasyHashIndex
	file ext:	h
	author:		Yang Wenge
	
	purpose:	单键字符串索引
	NOTE:		//??? 已经使用EasyHash替换，需要测试，目前主要用在配置或逻辑表格中 2016.10.23
*********************************************************************/
#ifndef _INCLUDE_STRINGMAPINDEX_H_
#define _INCLUDE_STRINGMAPINDEX_H_

#include "RecordIndex.h"


#include "EasyHash.h"
#include "StringTool.h"
//-------------------------------------------------------------------------
typedef EasyHash<AString, ARecord>		StringEasyHashRecordList;

//-------------------------------------------------------------------------
class StringEasyHashRecordIt : public RecordIt
{
public:
	StringEasyHashRecordIt(StringEasyHashRecordList::iterator it)
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
		return MEM_NEW StringEasyHashRecordIt(mListIt);
	}

protected:
	StringEasyHashRecordList::iterator	mListIt;
};
//-------------------------------------------------------------------------
class BaseCommon_Export StringEasyHashIndex : public RecordIndex
{
public:	
	virtual ARecord GetRecord(Int64 nIndex) 
	{		
		return GetRecord((const char*)STRING(nIndex));
	}
	virtual ARecord GetRecord(float fIndex)
	{
		return GetRecord( (const char*)STRING(fIndex) );
	}
	
	virtual ARecord GetRecord(const char* szIndex)
	{
		return mRecordList.find(szIndex);
	}

	virtual ARecord GetRecord(const Data &nIndex)
	{
		return GetRecord(nIndex.string().c_str());
	}

	virtual bool ExistRecord(Int64 nIndex) { return mRecordList.exist((const char*)STRING(nIndex)); }
	virtual bool ExistRecord(float fIndex) { return mRecordList.exist((const char*)STRING(fIndex)); }
	virtual bool ExistRecord(const char* szIndex) { return mRecordList.exist(szIndex); }

	virtual bool InsertRecord(ARecord scrRecord)
	{
		Data d = scrRecord->get(IndexFieldName());
		if (d.empty())
			return false;
		
		mRecordList.insert(d.string(), scrRecord);
		return true;
	}

	virtual bool RemoveRecord(int nIndex)
	{
		return mRecordList.erase((const char*)STRING(nIndex));
	}
	virtual bool RemoveRecord(float fIndex)
	{
		return mRecordList.erase((const char*)STRING(fIndex));
	}
	virtual bool RemoveRecord(const char* szIndex)
	{
		return mRecordList.erase(szIndex);
	}

	virtual bool RemoveRecord(Int64 nIndex)
	{
		return mRecordList.erase((const char*)STRING(nIndex));
	}

	virtual bool RemoveRecord(ARecord record)
	{
		Data d = record->get(IndexFieldName());
		if (d.empty())
			return false;
		
		return mRecordList.erase(d.string());
	}

public:
	virtual ARecordIt GetRecordIt()
	{
		return MEM_NEW StringEasyHashRecordIt(mRecordList.begin());
	}

	//virtual ARecordIt GetRecordIt(const Data &beginKey)
	//{
	//	if (beginKey.string()=="")
	//		return ARecordIt();

	//	auto it = mRecordList.findIt(beginKey.string().c_str());
	//	if (it)
	//		return MEM_NEW StringEasyHashRecordIt(it);

	//	return ARecordIt();
	//}

	virtual ARecordIt GetLastRecordIt()
	{
		return MEM_NEW StringEasyHashRecordIt(mRecordList.GetLastIterator());
	};

	virtual ARecord GetLastRecord()
	{		
		return mRecordList.back();
	}

	virtual UInt64 GetCount(){ return mRecordList.size(); }

	virtual ARecord GetRandRecord()
	{
		NOTE_LOG("WARN: StringEasyHashIndex 当前未实现，使用此功能无意义");
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
		StringEasyHashRecordList::iterator it = mRecordList.findIt(targetRecord->getIndexData().string().c_str());

		if (it.get()==targetRecord)
			return MEM_NEW StringEasyHashRecordIt(it);

		return ARecordIt();
	}

	virtual bool IsStringHashKey() { return true; }

protected:
	StringEasyHashRecordList		mRecordList;
};


//-------------------------------------------------------------------------


#endif //_INCLUDE_STRINGMAPINDEX_H_