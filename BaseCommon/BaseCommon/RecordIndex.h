/********************************************************************
	created:	2014/07/24
	created:	24:7:2014   10:59
	filename: 	E:\RemoteCode\BaseCommon\BaseCommon\RecordIndex.h
	file path:	E:\RemoteCode\BaseCommon\BaseCommon
	file base:	RecordIndex
	file ext:	h
	author:		Yang Wenge
	
	purpose:	记录索引, 也可以作为记录容器, 区分 单键和多键
*********************************************************************/
#ifndef _INCLUDE_RECORDINDEX_H_
#define _INCLUDE_RECORDINDEX_H_

#include "BaseCommon.h"
#include "Auto.h"
#include "BaseRecord.h"

#include "RandTool.h"

#define		INT_INDEX_TYPE		Int64
//-------------------------------------------------------------------------
// 记录迭代
// 迭代记录 for (ARecord r, ARecordIt it = t.GetRecordIt(); *it; r = ++it) 
// NOTE: r may be NULL
//-------------------------------------------------------------------------
class RecordIndex;
class tBaseTable;
class RecordIt : public AutoBase
{
public:
	virtual ARecord GetRecord() = 0;
	virtual ARecord Begin() = 0;
	virtual bool operator  ++() = 0;
	virtual bool operator  ++(int) { return ++(*this); }

	virtual bool operator  --() = 0;
	virtual bool operator  --(int) { return --(*this); }

	virtual operator bool () = 0;
	virtual operator ARecord () { return GetRecord(); }

	virtual void erase() = 0;

	virtual Auto<RecordIt> Copy() = 0;

	virtual bool GetKey(Int64 &key){ return false; }
};
typedef Auto<RecordIt>	ARecordIt;
//-------------------------------------------------------------------------
class BaseCommon_Export_H RecordIndex : public AutoBase // Base<RecordIndex>
{
	friend class tBaseTable;
	friend class BaseTable;
	friend class MemoryDBTable;
	friend class MemoryDBIndexTable;

public:
	virtual ARecord GetRecord(Int64 nIndex) = 0;
	virtual ARecord GetRecord(float fIndex) = 0;
	virtual ARecord GetRecord(const char* szIndex) = 0;
	virtual ARecord GetRecord(const AString &str) { return GetRecord(str.c_str()); }
	virtual ARecord GetRecord(const Data &nIndex) = 0;
	virtual ARecord GetRecord(int nIndex) { return GetRecord((Int64)nIndex); }

	virtual bool ExistRecord(Int64 nIndex) = 0;
	virtual bool ExistRecord(float fIndex) = 0;
	virtual bool ExistRecord(const char* szIndex) = 0;

	virtual bool InsertRecord(ARecord scrRecord) = 0;

protected:
	virtual bool InsertLast(ARecord scrRecord)
	{
		ERROR_LOG("ERROR: 当前索引[%s]不支持自增创建记录", typeid(*this).name());
		return false; 
	}

	virtual bool RemoveRecord(float fIndex) = 0;
	virtual bool RemoveRecord(const char* szIndex) = 0;
	virtual bool RemoveRecord(Int64 nIndex) = 0;

	virtual bool RemoveRecord(ARecord record) = 0;	
	virtual bool RemoveRecord(const AString &str) { return RemoveRecord(str.c_str()); }
	// 用于确定删除多键索引中指定记录
	virtual bool RemoveRecord(Int64 nIndex, ARecord record)
	{
		ARecord r = GetRecord(nIndex);
		if (r==record)
			return RemoveRecord(nIndex);
		return false;
	}
	virtual bool RemoveRecord(const char *nIndex, ARecord record)
	{
		ARecord r = GetRecord(nIndex);
		if (r==record)
			return RemoveRecord(nIndex);
		return false;
	}
	
public:
	// 迭代记录 for (ARecord r = t.Begin(); t; r = ++t) NOTE: r may be NULL
	virtual ARecordIt GetRecordIt() = 0;
	virtual ARecordIt GetRecordIt(ARecord targetRecord) = 0;
	virtual ARecordIt GetLastRecordIt() = 0;

	virtual bool MultipleKey() { return false; }
	virtual const char* IndexFieldName() const { return mIndexFieldName.c_str(); }
	virtual void SetIndexField(const char *szFieldName){ mIndexFieldName = szFieldName; }

	virtual UInt64 GetCount() = 0;

	// 返回从大到小的排序位置
	virtual UInt64 GetSortPos(ARecord record) 
	{
		AssertEx(0, "WARN: 使用了未实现的功能 GetSqrtPos");
		return NULL_POS; 
	}

	virtual bool IsStringHashKey() { return false; }
	virtual bool IsMultiple() const { return false; }

	virtual Int64 GetNextGrowthKey(){ return 0; }
	virtual ARecord GetLastRecord() = 0;

	virtual ARecord GetRandRecord() = 0;

	virtual void ClearAll(tBaseTable *pOwnerTable) = 0;

public:
	virtual ~RecordIndex() {}

	Auto<RecordIndex> GetSelf() { return Auto<RecordIndex>(this); }

protected:
	EasyString		mIndexFieldName;
};

typedef Auto<RecordIndex>	AutoIndex;
//-------------------------------------------------------------------------
#endif //_INCLUDE_RECORDINDEX_H_