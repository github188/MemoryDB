
#ifndef _INCLUDE_BASERECORDSET_H_
#define _INCLUDE_BASERECORDSET_H_
//--------------------------------------------------------------------------------------------
#include "BaseCommon.h"
#include "RecordSet.h"
#include "EasyMap.h"
//--------------------------------------------------------------------------------------------
// int index record set
class BaseCommon_Export IntRecordSet : public RecordSet
{
public:
	IntRecordSet();

public:	
	virtual AutoRecord GetRecord(int nIndex)
	{
		return mRecordMap.get(nIndex);
	}
	virtual AutoRecord GetRecord(float fIndex)
	{
		return mRecordMap.get((int)fIndex);
	}
	virtual AutoRecord GetRecord(const char* szIndex)
	{

		return mRecordMap.get( atoi(szIndex) );
	}

	virtual AutoRecord GetRecord(__int64 nIndex)
	{
		return mRecordMap.get((int)(nIndex & 0xFFFFFFFF));
	}

	virtual bool AppendRecord(int nIndexCol, AutoRecord scrRecord, bool bReplace);

	virtual bool ExistRecord(int nIndex) { return mRecordMap.exist(nIndex); }
	virtual bool ExistRecord(float fIndex)  { return mRecordMap.exist((int)fIndex); }
	virtual bool ExistRecord(const char* szIndex)  { return mRecordMap.exist(atoi(szIndex)); }
	virtual bool ExistRecord(__int64 nIndex) { return mRecordMap.exist((int)(nIndex&0xFFFFFFFF)); }

	virtual bool RemoveRecord(int nIndex)
	{
		return mRecordMap.erase(nIndex)>0;
	}

	virtual bool RemoveRecord(float fIndex)
	{
		return RemoveRecord((int)fIndex);
	}

	virtual bool RemoveRecord(const char* szIndex)
	{
		return RemoveRecord(atoi(szIndex));
	}

	virtual bool RemoveRecord(__int64 nIndex)
	{
		return RemoveRecord((int)(nIndex&0xFFFFFFFF));
	}

	virtual bool RemoveRecord(int indexCol, AutoRecord record);

	AutoRecordIt GetRecordIt(void);

	virtual void ClearAllRecord(void)
	{
		mRecordMap.clear();
	}

	virtual bool Empty(void){ return mRecordMap.empty(); }
	virtual size_t GetRecordCount(void) { return mRecordMap.size(); }

protected:
	EasyMap<int, AutoRecord>	mRecordMap;

public:
	friend class IntRecordIt;
};
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------


class BaseCommon_Export_H IntRecordIt : public RecordIt
{
public:
	IntRecordIt(IntRecordSet &pSet)
		: mMapIt(pSet.mRecordMap.GetIterator())
	{
		
	}

public:

	virtual AutoRecord begin()
	{
		return mMapIt.begin();
	}
	virtual bool have()
	{
		return mMapIt.have();
	}
	virtual bool end()
	{
		return mMapIt.end();

	}
	virtual AutoRecord record()
	{
		return mMapIt.get();
	}
	virtual bool next()
	{
		return mMapIt.next();
	}

	virtual AutoRecord nextRecord()
	{
		return mMapIt.nextValue();
	}
	virtual bool remove()
	{
		return mMapIt.remove();
	}

protected:
	EasyMap<int, AutoRecord>::Iterator		mMapIt;
};


//--------------------------------------------------------------------------------------------
// ×Ö·û´®¼ÇÂ¼¼¯
//class BaseCommon_Export IntRecordSet : public RecordSet
//{
//public:
//	IntRecordSet();
//
//public:	
//	virtual AutoRecord GetRecord(int nIndex)
//	{
//		return mRecordMap.get(nIndex);
//	}
//	virtual AutoRecord GetRecord(float fIndex)
//	{
//		return mRecordMap.get((int)fIndex);
//	}
//	virtual AutoRecord GetRecord(const char* szIndex)
//	{
//
//		return mRecordMap.get( atoi(szIndex) );
//	}
//
//	virtual AutoRecord GetRecord(__int64 nIndex)
//	{
//		return mRecordMap.get((int)(nIndex & 0xFFFFFFFF));
//	}
//
//	virtual bool AppendRecord(int nIndexCol, AutoRecord scrRecord, bool bReplace);
//
//	virtual bool ExistRecord(int nIndex) { return mRecordMap.exist(nIndex); }
//	virtual bool ExistRecord(float fIndex)  { return mRecordMap.exist((int)fIndex); }
//	virtual bool ExistRecord(const char* szIndex)  { return mRecordMap.exist(atoi(szIndex)); }
//	virtual bool ExistRecord(__int64 nIndex) { return mRecordMap.exist((int)(nIndex&0xFFFFFFFF)); }
//
//	virtual bool RemoveRecord(int nIndex)
//	{
//		return mRecordMap.erase(nIndex)>0;
//	}
//
//	virtual bool RemoveRecord(float fIndex)
//	{
//		return RemoveRecord((int)fIndex);
//	}
//
//	virtual bool RemoveRecord(const char* szIndex)
//	{
//		return RemoveRecord(atoi(szIndex));
//	}
//
//	virtual bool RemoveRecord(__int64 nIndex)
//	{
//		return RemoveRecord((int)(nIndex&0xFFFFFFFF));
//	}
//
//	virtual bool RemoveRecord(int indexCol, AutoRecord record);
//
//	AutoRecordIt GetRecordIt(void);
//
//	virtual void ClearAllRecord(void)
//	{
//		mRecordMap.clear();
//	}
//
//	virtual bool Empty(void){ return mRecordMap.empty(); }
//	virtual size_t GetRecordCount(void) { return mRecordMap.size(); }
//
//protected:
//	EasyMap<int, AutoRecord>	mRecordMap;
//
//public:
//	friend class IntRecordIt;
//};

#endif