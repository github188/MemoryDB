#ifndef _INCLUDE_MEMORYDBINDEXTABLE_H_
#define _INCLUDE_MEMORYDBINDEXTABLE_H_

#include "MemoryDBTable.h"
//-------------------------------------------------------------------------
// �����������ܵ�DB��
//-------------------------------------------------------------------------
class MemoryDB_Export MemoryDBIndexTable : public MemoryDBTable
{
public:
	MemoryDBIndexTable(MemoryDB *pDB)
		: MemoryDBTable(pDB)
	{

	}
	~MemoryDBIndexTable()
	{
		for (int i=0; i<mRecordIndexList.size(); ++i)
		{
			mRecordIndexList[i]._free();
		}
		mRecordIndexList.clear(true);
	}

public:
	virtual ARecord NewRecord();

	virtual const char* GetTableType() { return "IndexTable"; }


public:
	virtual AutoIndex GetIndex(const char *szFieldName){ return GetIndex(GetField()->getFieldCol(szFieldName)); }	
	virtual bool SetIndexField( const char *szFieldName, bool bHash );
	virtual AString GetIndexFieldsInfo();

	AutoIndex GetIndex(int col)
	{
		if (col>=0 && col<mRecordIndexList.size())
			return mRecordIndexList[col];

		return AutoIndex();
	}

public:
	//virtual ARecord GrowthNewRecord( DataStream *recordData );
	bool AppendRecord( ARecord scrRecord, bool bReplace );

	bool RemoveRecord( ARecord record );
	 

	void ClearAll()
	{
		MemoryDBTable::ClearAll();
		for (int i=0; i<mRecordIndexList.size(); ++i)
		{
			if (mRecordIndexList[i])
				mRecordIndexList[i]->ClearAll(this);
		}
	}

	virtual void OnRecordDataChanged(ARecord re, int col, Int64 scrValue);
	virtual void OnRecordDataChanged(ARecord re, int col, const char *scrValue);

public:
	Array<AutoIndex>	mRecordIndexList;	// �����ֶ�����
};
//-------------------------------------------------------------------------

class MemoryDBIndexRecord : public MemoryDBRecord
{
public:
	MemoryDBIndexRecord(ABaseTable hOwnerTable)
		: MemoryDBRecord(hOwnerTable)
	{
		 
	}

public:
	virtual bool restoreData( DataStream *scrData );
};

//-------------------------------------------------------------------------
// ֻ�������ڴ��е���ʱ��񣬲��������
class TempMemoryTable : public MemoryDBIndexTable
{
public:
	TempMemoryTable(MemoryDB *pDB)
		: MemoryDBIndexTable(pDB)
	{

	}

	virtual const char* GetTableType() override { return "TempTable"; }

public:
	virtual void AppendUpdateRecord(BaseRecord *pNeedUpdateRecord){}
	virtual void Process(int nCount){}
	bool DeleteRecord( ARecord record ){ return RemoveRecord(record); }

	virtual bool RegisterTrigger(HandTrigger trigger, bool bAppend)
	{
		ERROR_LOG("��ʱ�ڴ���δʵ��ֵ�仯����");
		return false;
	}
};
//-------------------------------------------------------------------------
#endif //_INCLUDE_MEMORYDBINDEXTABLE_H_