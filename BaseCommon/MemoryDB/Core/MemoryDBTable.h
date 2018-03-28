/********************************************************************
	created:	2014/07/26
	created:	26:7:2014   15:19
	filename: 	H:\RemoteGame\BaseCommon\ServerBase\DBWork\MemoryDBTable.h
	file path:	H:\RemoteGame\BaseCommon\ServerBase\DBWork
	file base:	MemoryDBTable
	file ext:	h
	author:		Yang Wenge
	
	purpose:	用于Memory DB 表格
				1 提供客户检索数据
				2 客户修改数据后, 同步到落地进程 (每条记录, 配置时间内进行检测同步)
				3 记录循环不在同一循环内完成, 默认一个周期处理10个, 可指定, (如果在规定时间内完成不了一次完整遍历, 则警告)
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

// 可完成MySql 自动同步
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

	//NOTE: 直接会新建记录并插入同步到DB, 如果在加载DB记录时, 直接使用NewRecord > AppendRecord
	virtual ARecord CreateRecord(const char* szIndex, bool bReplace) override;
	virtual ARecord CreateRecord(Int64 nIndex, bool bReplace) override;

	ARecord NewRecord();

	virtual bool AppendRecord(ARecord scrRecord, bool bReplace, bool bRestore = false) override;

	virtual void ClearAll();

	//NOTE: 支持修改主键功能，但不提倡使用修改DB主键操作
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
	ARecordIt	mTableRecordIt;		// NOTE: 如果删除记录前, 先判断与当前迭代相同, 如果相同, 则将迭代先更新到下一个, 再进行删除
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
	Int64		mLastMaxKey;				// 当前表格内最大数值类型的最大KEY

	EasyMap<short, bool>	mKeyHashSlotList;	// 保存记录KEY对应的哈希SLOT
	AString	mLoadLimitField;
	int		mLoadLimitRange;
};

typedef Auto<MemoryDBTable>		AutoDBTable;
//-------------------------------------------------------------------------



#endif //_INCLUDE_MEMORYDBTABLE_H_