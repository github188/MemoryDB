/********************************************************************
	created:	2014/07/25
	created:	25:7:2014   0:28
	filename: 	H:\RemoteGame\BaseCommon\BaseCommon\IDHashIndex.h
	file path:	H:\RemoteGame\BaseCommon\BaseCommon
	file base:	IDHashIndex
	file ext:	h
	author:		Yang Wenge
	
	purpose:	ID 分布索引，用于分布式保存大型DB表，且提供快速检索
				设定最小与最大ID, 即分表ID段, 实现ID = 最小ID＋KEYID
	NOTE:		使用前必须要分配ID区间，且不可与其他分表存在交集情况
				此索引只用于主索引进行高效且大型分布式数据表格
*********************************************************************/
#ifndef _INCLUDE_IDARRAYINDEX_H_
#define _INCLUDE_IDARRAYINDEX_H_

#include "RecordIndex.h"
#include "StringTool.h"
#include "Array.h"

//-------------------------------------------------------------------------
#define ID_RANGE_LIMIT_MAX_COUNT	(100000000)		// 800M used of index

typedef Array<ARecord>		ArrayRecordList;
//-------------------------------------------------------------------------
class BaseCommon_Export IDArrayIndex : public RecordIndex
{
	friend class ArrayIndexRecordIt;

public:
	IDArrayIndex()
		: mMinKey(0)
		, mMaxKey(0)
		, mMinUseID(-1)
		, mMaxUseID(-1)
	{

	}

	bool SetKeyRangeLimit(UInt64 minID, UInt64 maxID);

public:
	virtual ARecord GetRecord(Int64 nIndex) 
	{		
		int id = ToID(nIndex);		
		if (id>=0 && id<(int)mRecordList.size())
			return mRecordList[id];

		return ARecord();
	}

	virtual ARecord GetRecord(float fIndex)
	{
		return GetRecord( (Int64)fIndex );
	}

	virtual ARecord GetRecord(const char* szIndex)
	{
		return GetRecord(TOINT64(szIndex));
	}

	virtual ARecord GetRecord(const Data &nIndex)
	{
		return GetRecord( (Int64)(UInt64)nIndex );
	}

	virtual bool ExistRecord(Int64 nIndex) { return GetRecord(nIndex); }
	virtual bool ExistRecord(float fIndex) { return GetRecord(fIndex); }
	virtual bool ExistRecord(const char* szIndex) { return GetRecord(szIndex); }

	virtual bool InsertRecord(ARecord scrRecord);
	virtual bool InsertLast(ARecord scrRecord);

	virtual bool RemoveRecord(Int64 nIndex)
	{
		int id = ToID(nIndex);
		if (id<0)
			return false;
		ARecord re = mRecordList[id];
		if ( mRecordList[id] )
		{
			mRecordList[id].setNull();
			return re;
		}
		return false;
	}
	virtual bool RemoveRecord(float fIndex)
	{
		return RemoveRecord((Int64)fIndex);
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

		UInt64 key = d;
		return RemoveRecord((Int64)key);
	}

public:
	virtual ARecordIt GetRecordIt();

	virtual ARecordIt GetRecordIt(ARecord targetRecord);
	virtual ARecordIt GetLastRecordIt();


	virtual UInt64 GetCount()
	{ 
		if (mMinUseID<0 /*|| !mRecordList[mMinUseID]*/)
			return 0;

		int count = 0;
		for (int i=mMinUseID; i<=mMaxUseID; ++i)
		{			
			if (mRecordList[i].getPtr()!=NULL)
				++count;
		}
		return count;
		//return mMaxUseID-mMinUseID + 1; 
	}

	// 转化为本地数组下标
	int ToID(Int64 key)
	{
		if (key<0)
			return -1;

		key -= mMinKey;
		if (key<0 || key>(Int64)mMaxKey)
			return -1;

		return (int)key;
	}

	Int64 ToKey(int id)
	{
		if (id>=0 && id<=mMaxKey)
			return mMinKey + id;

		return -1;
	}

	virtual Int64 GetNextGrowthKey()
	{
		return mMaxUseID + 1 + mMinKey;
		return 0; 
	}

	virtual ARecord GetLastRecord()
	{
		if (mMaxUseID>=0 && mMaxUseID<mRecordList.size())
			return mRecordList[mMaxUseID];

		return ARecord();
	}

	virtual ARecord GetRandRecord()
	{
		//if (mRecordList.empty())
		//	return ARecord();

		size_t randPos = CRand::RandUInt(mMaxUseID-mMinUseID);
		return mRecordList[randPos];
	}

	virtual void ClearAll(tBaseTable *pOwnerTable)
	{
		if (mMinUseID<0)
			return;

		for (int i=mMinUseID; i<=mMaxUseID; ++i)
		{
			ARecord &re = mRecordList[i];
			if (re && re->GetTable()==pOwnerTable)
			{
				re._free();
			}
		}
		mMinUseID = -1;
		mMaxUseID = -1;
	}

protected:
	ArrayRecordList		mRecordList;

	UInt64				mMinKey;
	UInt64				mMaxKey;

	int					mMinUseID;		// 用于优化迭代
	int					mMaxUseID;
};

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
class ArrayIndexRecordIt : public RecordIt
{
public:
	ArrayIndexRecordIt(IDArrayIndex *ownerTable, int beginPos)
		: mCurrentPos(beginPos)
		, mRecordIndex(ownerTable)
	{
		if (beginPos<GetIndex()->mMinUseID)
			mCurrentPos = GetIndex()->mMinUseID;		

		//if (!GetRecord())
		//	++(*this);
	}

public:
	virtual ARecord GetRecord() 
	{
		if (mCurrentPos>=0 && (size_t)mCurrentPos < GetIndex()->mRecordList.size())
			return GetIndex()->mRecordList[mCurrentPos];
		return ARecord(); 
	}
	virtual ARecord Begin() 
	{
		mCurrentPos = 0;
		//if ( !GetRecord() )
		//	++(*this);
		return GetRecord();
	}

	// 移动到下一条记录，直到找到一条不为空的记录返回，或者超出范围
	// 此迭代耗时，每次都会将范围内的ID全部检索一遍　(10000万次约0.001秒)
	virtual bool operator  ++() 
	{
		IDArrayIndex *pIndex = GetIndex();
		ArrayRecordList &recordList = GetIndex()->mRecordList;
		++mCurrentPos;
		if (mCurrentPos<pIndex->mMinUseID || mCurrentPos>pIndex->mMaxUseID || (size_t)mCurrentPos>=recordList.size())
			return false;
		return true;
		//int i = 0;
		//while (i++<10)	// 每次只检索10条
		//{	
		//	++mCurrentPos;
		//	if (mCurrentPos<pIndex->mMinUseID || mCurrentPos>pIndex->mMaxUseID || (size_t)mCurrentPos>=recordList.size())
		//		return false;

		//	if (recordList[mCurrentPos])
		//		return recordList[mCurrentPos];
		//}	
		//return true;
	}	

	virtual bool operator --()
	{
		ArrayRecordList &recordList = GetIndex()->mRecordList;

		if (mCurrentPos>0)
			--mCurrentPos;
		else
		{
			mCurrentPos = NULL_POS;
		}
		if (mCurrentPos<GetIndex()->mMinUseID || mCurrentPos>GetIndex()->mMaxUseID || (size_t)mCurrentPos>=recordList.size())
			return false;
		return true;
	
		//while (true)
		//{	
		//	if (mCurrentPos>0)
		//		--mCurrentPos;
		//	else
		//	{
		//		mCurrentPos = NULL_POS;
		//		break;
		//	}
		//	if (mCurrentPos<GetIndex()->mMinUseID || mCurrentPos>GetIndex()->mMaxUseID || (size_t)mCurrentPos>=recordList.size())
		//		return false;

		//	if (recordList[mCurrentPos])
		//		return recordList[mCurrentPos];
		//}	
		//return false;
	}

	virtual operator bool () 
	{ 
		return mCurrentPos>=mRecordIndex->mMinUseID 
			&& mCurrentPos<=mRecordIndex->mMaxUseID 
			&& (size_t)mCurrentPos < mRecordIndex->mRecordList.size();
	}

	IDArrayIndex* GetIndex(){ return (mRecordIndex); }

	virtual void erase() 
	{ 
		if (mCurrentPos>=0 && (size_t)mCurrentPos < GetIndex()->mRecordList.size())
			GetIndex()->mRecordList[mCurrentPos]._free();
	}

	Auto<RecordIt> Copy()
	{
		return MEM_NEW ArrayIndexRecordIt(mRecordIndex, mCurrentPos);
	}

	virtual bool GetKey(Int64 &key) override
	{
		if (mCurrentPos>=0 && (size_t)mCurrentPos < GetIndex()->mRecordList.size())
		{
			key = mRecordIndex->ToKey(mCurrentPos);
			return true;
		}
		return false; 
	}

protected:
	int				mCurrentPos;
	IDArrayIndex	*mRecordIndex;
};
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------

#endif //_INCLUDE_IDARRAYINDEX_H_