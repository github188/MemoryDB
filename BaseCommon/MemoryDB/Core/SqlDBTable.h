
#ifndef _INCLUDE_SQLDBTABLE_H_
#define _INCLUDE_SQLDBTABLE_H_

#include "MemoryDBTable.h"
//-------------------------------------------------------------------------
// ֱ�Ӳ���MYSQL DB ���
// �������ݽϴ�, �Ҳ�Ҫ�󲢷�����ʱ�������, ��: ս����¼
// NOTE: �˱�����ȴ��Ƚ����ƣ��ڼ���ʱ��ֻ���ؼ�¼�������������ؼ�¼����
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