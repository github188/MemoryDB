/********************************************************************
	created:	2014/07/24
	created:	24:7:2014   10:37
	filename: 	E:\RemoteCode\BaseCommon\BaseCommon\BaseTable.h
	file path:	E:\RemoteCode\BaseCommon\BaseCommon
	file base:	BaseTable
	file ext:	h
	author:		Yang Wenge
	
	purpose:	DB 基本表格, 主要功能管理记录及记录操作
*********************************************************************/
#ifndef _INCLUDE_BASETABLE_H_
#define _INCLUDE_BASETABLE_H_

#include "Auto.h"
#include "Hand.h"
#include "BaseRecord.h"
#include "RecordIndex.h"
#include "DBResultCallBack.h"
//-------------------------------------------------------------------------
// 最大记录内存池记录数量数支持 32 * 0xFFFE, DBServer指定为1000，约6500万
#define RECORD_BLOCK_INIT_NUM		(32)		
//-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------
class tBaseTable;
// 读取加载表格方法,用于客户端从资源管理或资源包中读取表格 或 从数据库中读取
class TableLoader : public AutoBase
{
public:
	virtual ~TableLoader(){}
	virtual bool LoadTable(tBaseTable *pTable, DataStream *scrData) = 0;
	virtual bool SaveTable(tBaseTable *pTable, DataStream *resultData, const char *szIndexField = NULL) = 0;
};

typedef Auto<TableLoader>	AutoLoader;
//-------------------------------------------------------------------------*/
// 用来记录保存表格中间件, 目的是表格也保存这个对象，不会让记录与表格互相包含死锁
class TablePtr : public AutoBase
{
public:
	AutoField	mField;
	tBaseTable	*mpTable;

	TablePtr() : mpTable(NULL) {}
};
typedef Auto<TablePtr> ATablePtr;
//-------------------------------------------------------------------------*/
struct TableCommonConfig
{
public:
	int		mRecordMemPoolBlockRecordCount;

	TableCommonConfig()
		: mRecordMemPoolBlockRecordCount(RECORD_BLOCK_INIT_NUM){	}
};
//-------------------------------------------------------------------------
// 内存表格
//-------------------------------------------------------------------------
class BaseCommon_Export tBaseTable : public AutoBase
{
public:
	static Auto<tBaseTable> NewBaseTable(bool bUseRecordPool = false);
	static Auto<tBaseTable> NewFieldTable(bool bCreateField = true);
	static void SetCommonConfig(const TableCommonConfig &config){ msCommonConfig = config; }

	Data GetValue(const char* recordkey, const char *szFieldName);
	Data GetValue(int recordkey, const char *szFieldName);

	Data GetValue(const char* recordkey, int field);
	Data GetValue(int recordkey, int field);

public:
	virtual ARecord GetRecord(Int64 nIndex) = 0;
	virtual ARecord GetRecord(float fIndex) = 0;
	virtual ARecord GetRecord(const char* szIndex) = 0;
	virtual ARecord GetRecord(int nIndex) { return GetRecord((Int64)nIndex); }
	virtual ARecord GetRecord(UInt64 nIndex) { return GetRecord((Int64)nIndex); }
	virtual ARecord GetRecord(const AString &strKey){ return GetRecord(strKey.c_str()); } 

	virtual ARecord CreateRecord(Int64 nIndex, bool bReplace) = 0;
	virtual ARecord CreateRecord(float fIndex, bool bReplace) = 0;
	virtual ARecord CreateRecord(const char* szIndex, bool bReplace) = 0;
	virtual ARecord CreateRecord(const AString &strIndex, bool bReplace) { return CreateRecord(strIndex.c_str(), bReplace); }
	virtual ARecord CreateRecord(int nIndex, bool bReplace) { return CreateRecord((Int64)nIndex, bReplace); }

	virtual bool AppendRecord(ARecord scrRecord, bool bReplace, bool bRestore = false) = 0;

	virtual bool RemoveRecord(Int64 nIndex) = 0;
	virtual bool RemoveRecord(float fIndex) = 0;
	virtual bool RemoveRecord(const char* szIndex) = 0;
	virtual bool RemoveRecord(int nIndex) { return RemoveRecord((Int64)nIndex); }

	virtual bool RemoveRecord(ARecord record) = 0;	

	// 删除DB数据库中的记录, 且只能通过此功能删除
	virtual bool DeleteRecord(ARecord record) = 0;

	virtual ARecord NewRecord() = 0;

public:
	virtual ARecordIt GetRecordIt() = 0;
	virtual ARecordIt GetRecordIt(ARecord targetRecord) = 0;
	virtual AutoIndex GetMainIndex() = 0;

	virtual const char* GetTableType() = 0;

public:
	virtual bool Save(TableLoader *pLoader, DataStream *resultStream)
	{
		AssertEx(pLoader!=NULL, "Table loader is null");
		return pLoader->SaveTable(this, resultStream);
	}

	virtual bool Load(TableLoader *pLoader, DataStream *scrDatao)
	{
		AssertEx(pLoader!=NULL, "Table loader is null");
		return pLoader->LoadTable(this, scrDatao);
	}
    
	virtual bool SaveCSV(const char *szCSVFilePathName, bool bTabDelimited = false);
	virtual bool LoadCSV(const char *szCSVFilePathName, bool bTabDelimited = false);

	virtual bool SaveData(DataStream *resultStream);
	virtual bool LoadFromData(DataStream *scrStream);

	// NOTE: 为了保存的数据和C#一至, 统一使用字符串升序方法保存
	virtual bool SortSaveData(DataStream *destData, bool bSaveTableIndex = false);

	// 对于子表, 删除记录后, 必须加入到删除列表, 当同步序列化数据时, 保存删除信息
	virtual bool SaveDeleteInfo(DataStream *resultStream, bool bClearDeleteList){ return true; }
	virtual bool ApplyDeleteInfo(DataStream *scrData){ return true; }
	virtual bool HasDeleteRecord(){ return false; }
	virtual void ClearDeleteRecordList(){}

	virtual void SaveRecordData(const char *szKey, AutoNice recordData, bool bNewInsert, DBResultCallBack callBack){ ERROR_LOG("未实现[SaveRecordData]"); callBack(NULL, false); }
	virtual void SaveRecordData(KEY nKey, AutoNice recordData, bool bNewInsert, DBResultCallBack callBack){  SaveRecordData(STRING(nKey), recordData, bNewInsert, callBack); }		
	virtual void LoadRecordData(const char *szKey, AString fieldInfo, DBResultCallBack callBack){ ERROR_LOG("未实现[LoadRecordData]"); callBack(NULL, false); }
	virtual void LoadRecordData(KEY nKey, AString fieldInfo, DBResultCallBack callBack){  LoadRecordData(STRING(nKey), fieldInfo, callBack); }		

	virtual bool NeedTrySaveRecord() const { return false; }
    // NOTE: 用于DB子表时不再检查每条记录的更新状态
    virtual bool NeedCheckRecordUpdateState() const { return true; }

public:
	virtual bool InitField(AutoField scrField);
	virtual void Init();
	virtual AutoField GetField() const { return mOwnerPtr->mField; }
	virtual void ReadyField(){ if (mOwnerPtr->mField) mOwnerPtr->mField->_updateInfo(); }
	virtual FieldInfo AppendField(const char *szFieldName, FIELD_TYPE type){ return SetField(szFieldName, type, GetField()->getCount());}
	virtual FieldInfo AppendField(const char *szFieldName, const char *szType){ return SetField(szFieldName, szType, GetField()->getCount());}
	virtual FieldInfo SetField(const char *szFieldName, FIELD_TYPE type, int col);

	virtual FieldInfo SetField(const char *szFieldName, const char *szType, int col)
	{
		return SetField(szFieldName, (FIELD_TYPE)FieldInfoManager::GetMe().toFieldType(szType), col);
	}
	bool SetFieldTable( const char *szField, tBaseTable *pSubTable );

	virtual void OnFieldChangeBefore(){	}
	virtual void OnFieldChanged() { }

	virtual bool SetMainIndex(int indexCol, bool bHash, bool bMultiple = false){ return false; }
	virtual bool SetMainIndex(const char *szFieldName, bool bHash, bool bMultiple = false){ return false; }
	virtual bool SetIDMainIndex(UInt64 minKey, UInt64 maxKey){ return false; }
	virtual int  GetMainIndexCol();

	virtual bool SetIndexField(const char *szFieldName, bool bHash){ return false; }
	virtual AutoIndex GetIndex(const char *szFieldName){ return AutoIndex(); }
	virtual AString GetIndexFieldsInfo(){ return AString(); }

	virtual void OnRecordDataChanged(ARecord re, int col, Int64 scrValue) = 0;
	virtual void OnRecordDataChanged(ARecord re, int col, const char *scrValue) = 0;

	virtual bool SetSkipUpdateField(const char *szFieldName, bool bSkip);
	virtual bool SetSkipUpdateField(int nFieldCol, bool bSkip){ return false; }	

public:
	tBaseTable();		
	virtual ~tBaseTable();

public:
	virtual bool Empty(){ return !GetMainIndex() || GetMainIndex()->GetCount()<=0; }
	virtual int GetRecordCount(){ if (!GetMainIndex()) return 0; return (int)GetMainIndex()->GetCount(); }
	virtual void ClearAll(){ /*mField.setNull();*/ }
	virtual const char* GetTableName() const { return "None"; }
	virtual void SetTableName(const char *szTableName){ WARN_LOG("不能保存表格索引名称,可能为了节省空间"); }

	virtual bool InKeyRange(const char *key){ return false;}

	virtual ARecord GrowthNewRecord(DataStream *recordData){ return ARecord(); }

	virtual bool ModifyIDIndex(int limitMaxID, int &maxID){ return false; }

	virtual Int64 MaxRecordIndexID() { if (GetMainIndex()) return GetMainIndex()->GetNextGrowthKey()-1; return 0; }

public:
	virtual void Process(int){}
	ATablePtr GetTablePtr() { return mOwnerPtr; }
	Auto<tBaseTable> GetSelf() { return Auto<tBaseTable>(this); }

protected:
	ATablePtr			mOwnerPtr;
	
#if SAVE_DEBUG_INFO
	static __declspec(thread) int msCount;
#endif

public:
	static TableCommonConfig msCommonConfig;

};

//-------------------------------------------------------------------------
typedef Auto<tBaseTable>	ABaseTable;
typedef ABaseTable					AutoTable;
//-------------------------------------------------------------------------

struct TableIt
{
public:
	ARecordIt	mRecordIt;
	ABaseTable	mTable;

public:
	operator bool () { return _Have(); }
	bool operator  ++() { return _Next(); }
	bool operator  ++(int) { return ++(*this); }
	ARecord operator *() { return getCurrRecord(); }

public:
	TableIt(ABaseTable table)
	{
		mTable = table;
		if (table)
			mRecordIt = table->GetRecordIt();
	}

	bool _Have()
	{
		if (mRecordIt)
			return *mRecordIt;

		return false;
	}

	bool _Next()
	{
		if (mRecordIt)
		{
			return ++(*mRecordIt);
		}
		return false;
	}

	ARecord getCurrRecord()
	{
		if (mRecordIt)
			return *mRecordIt;
		return ARecord();
	}

	bool RemoveRecord()
	{
		if (mRecordIt)
		{
			mRecordIt->erase();
			return true;
		}
		return false;
	}
};
//-------------------------------------------------------------------------*/
#endif //_INCLUDE_BASETABLE_H_