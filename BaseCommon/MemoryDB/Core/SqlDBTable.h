
#ifndef _INCLUDE_SQLDBTABLE_H_
#define _INCLUDE_SQLDBTABLE_H_

#include "MemoryDBTable.h"
//-------------------------------------------------------------------------
// 直接操作MYSQL DB 表格
// 处理数据较大, 且不要求并发处理时间的数据, 如: 战斗记录
// NOTE: 此表与冷却表比较类似，在加载时，只加载记录的索引，不加载记录内容
//-------------------------------------------------------------------------

class MemoryDB_Export SqlDBTable : public MemoryDBTable
{
public:
	SqlDBTable(MemoryDB *pDB)
		: MemoryDBTable(pDB)
	{
		SetStartUpdatePool(false);
	}
	virtual const char* GetTableType(){ return "SQLTable"; }

	virtual bool LoadAllRecord()
	{
		mTempRecord = NewRecord(); 
		//OnLoadTableAllRecord(NULL, true);
		SetStartUpdatePool(false);
		WARN_LOG("[%s] is SqlDBTable, use all record load from sql", GetTableName());
		return MemoryDBTable::LoadAllRecord();
	}

	virtual bool NeedTrySaveRecord() const { return true; }

	ARecord GrowthNewRecord( DataStream *recordData ){ return ARecord(); }

public:
	virtual void SaveRecordData(const char *szKey, AutoNice reData, bool bNewInsert, DBResultCallBack callBack);
	virtual void LoadRecordData(const char *szKey, AString fieldInfo, DBResultCallBack callBack);

	AutoRecord	mTempRecord;

	
};


//-------------------------------------------------------------------------



#endif //_INCLUDE_SQLDBTABLE_H_