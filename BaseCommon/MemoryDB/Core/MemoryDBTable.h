/********************************************************************
	created:	2014/07/26
	created:	26:7:2014   15:19
	filename: 	H:\RemoteGame\BaseCommon\ServerBase\DBWork\MemoryDBTable.h
	file path:	H:\RemoteGame\BaseCommon\ServerBase\DBWork
	file base:	MemoryDBTable
	file ext:	h
	author:		Yang Wenge
	
	purpose:	����Memory DB ���
				1 �ṩ�ͻ���������
				2 �ͻ��޸����ݺ�, ͬ������ؽ��� (ÿ����¼, ����ʱ���ڽ��м��ͬ��)
				3 ��¼ѭ������ͬһѭ�������, Ĭ��һ�����ڴ���10��, ��ָ��, (����ڹ涨ʱ������ɲ���һ����������, �򾯸�)
*********************************************************************/
#ifndef _INCLUDE_MEMORYDBTABLE_H_
#define _INCLUDE_MEMORYDBTABLE_H_

#include "IndexBaseTable.h"
#include "NiceData.h"
#include "IndexDBRecord.h"
#include "MemoryDBHead.h"
#include "LoopList.h"
#include "DBTrigger.h"
#include "EasyList.h"
#include "MemoryDBTable.h"
#include "MemoryDBRecord.h"
//-------------------------------------------------------------------------
class tDataSource;
class MemoryDBTable;
class MemoryDB;
class DBOperate;
class tDBTool;

// �����MySql �Զ�ͬ��
class MemoryDB_Export MemoryDBTable : public SkipBaseTable
{
public:
	MemoryDBTable(MemoryDB *pDB);
	virtual ~MemoryDBTable();

public:
	virtual const char* GetTableType(){ return "MemoryTable"; }

	virtual void SaveRecordData(const char *szKey, NiceData &recordData, bool bNewInsert, DBResultCallBack callBack);
	virtual void LoadRecordData(const char *szKey, AString fieldInfo, DBResultCallBack callBack);

public:
	// void ReadyDBSaver(tDBSaver *pSaver);
	virtual void ReadyDataSource(tDataSource *pSource, NiceData &initParam);
	void OnDeleteRecord( const char *szKey, const char *, int deletedCount );
	virtual bool InKeyRange(const char *key);

	virtual bool LoadAllRecord();

	virtual void OnLoadTableAllRecord( const char*, const char*, int ); //DBOperate *op, bool bSu );

	virtual bool LoadFromDB(tDBTool *pDBTool);

public:
	virtual void Process(int nCount);

	virtual void OnLoopAllEnd();

	//virtual void OnProcessRecord(BaseRecord *record){}	

	virtual void AppendUpdateRecord(BaseRecord *pNeedUpdateRecord);
	void SetStartUpdatePool(bool bStart){ mStartUpdatePool = bStart; } 

#if DEVELOP_MODE
	virtual void OnAddUseCount() override {}
#endif

public:
	virtual AutoIndex NewRecordIndex( FIELD_TYPE indexKeyType, bool bHash, bool bMultKey );
	virtual bool SetMainIndex( int indexCol, bool bHash, bool bMultiple = false );
	virtual bool SetIDMainIndex(UInt64 minKey, UInt64 maxKey);

	virtual bool RemoveRecord(float fIndex);
	virtual bool RemoveRecord(const char* szIndex);
	virtual bool RemoveRecord(Int64 nIndex);

	virtual bool RemoveRecord(ARecord record);
	virtual bool DeleteRecord(ARecord record);

	virtual ARecord GrowthNewRecord( DataStream *recordData );

	//NOTE: ֱ�ӻ��½���¼������ͬ����DB, ����ڼ���DB��¼ʱ, ֱ��ʹ��NewRecord > AppendRecord
	virtual ARecord CreateRecord(const char* szIndex, bool bReplace) override;
	virtual ARecord CreateRecord(Int64 nIndex, bool bReplace) override;

	ARecord NewRecord();

	virtual bool AppendRecord(ARecord scrRecord, bool bReplace, bool bRestore = false) override;

	virtual void ClearAll();

	//NOTE: ֧���޸��������ܣ������ᳫʹ���޸�DB��������
	virtual void OnRecordDataChanged( ARecord re, int col, Int64 scrValue );
	virtual void OnRecordDataChanged( ARecord re, int col, const char *scrValue );

	virtual void OnReplaceRecord(AutoRecord scrRe, AutoRecord destRe)
	{ 
		dynamic_cast<MemoryDBRecord*>(destRe.getPtr())->mExtValue = dynamic_cast<MemoryDBRecord*>(scrRe.getPtr())->mExtValue;
	}

	virtual Int64 MaxRecordIndexID() override { return mLastMaxKey; }

public:
	virtual void OnRecordTrigger(AutoRecord updateRecord);

	virtual bool RegisterTrigger(HandTrigger trigger, bool bAppend);


public:
	MemoryDB		*mDB;
	ARecordIt	mTableRecordIt;		// NOTE: ���ɾ����¼ǰ, ���ж��뵱ǰ������ͬ, �����ͬ, �򽫵����ȸ��µ���һ��, �ٽ���ɾ��
	int			mMainIndexCol;

	tDataSource	*mDataSource;

#if MEMORY_DB_DEBUG
	UInt64		mBeginTime;
	int			mTotalUpdateCount;
	int			mTotalUpdateLoop;
	uint		mBeginTotalTime;
#endif
	bool		mNowUpdateDB;
	bool		mStartUpdatePool;
	LoopPool<AutoRecord>	mUpdateList;
	//std::set<AutoRecord>	mUpdateSet;
	HandTrigger	mRecordTrigger;
	UInt64		mLastProcessTime;
	Int64		mLastMaxKey;				// ��ǰ����������ֵ���͵����KEY

	EasyMap<short, bool>	mKeyHashSlotList;	// �����¼KEY��Ӧ�Ĺ�ϣSLOT
	AString	mLoadLimitField;
	int		mLoadLimitRange;
};

typedef Auto<MemoryDBTable>		AutoDBTable;
//-------------------------------------------------------------------------



#endif //_INCLUDE_MEMORYDBTABLE_H_