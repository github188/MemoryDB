/********************************************************************
	created:	2014/07/27
	created:	27:7:2014   1:20
	filename: 	H:\RemoteGame\BaseCommon\ServerBase\DBWork\DataSource.h
	file path:	H:\RemoteGame\BaseCommon\ServerBase\DBWork
	file base:	DataSource
	file ext:	h
	author:		Yang Wenge
	
	purpose:	MemoryDB 数据源, 数据持久保存
	NOTE:		使用任务方式异步处理数据
*********************************************************************/
#ifndef _INCLUDE_DATASOURCE_H_
#define _INCLUDE_DATASOURCE_H_

#include "TaskManager.h"
#include "LoadDataCallBack.h"
#include "BaseTable.h"
#include "DBCallBack.h"

#include "NiceData.h"
#include "NetHandle.h"
#include "MemoryDBHead.h"
#include "DBResultCallBack.h"
#include "MySqlDBTool.h"

// 是否开启DB本地文件异步操作(保存,删除)
#define LOCAL_DB_USE_THREAD		0

#define LOAD_RECORD_CALLBACK LoadDataCallBack

//-------------------------------------------------------------------------
class MemoryDB_Export tDataSource : public AutoBase
{
public:
	virtual ~tDataSource(){}
	virtual bool InitDataSource(NiceData &initParam){ return false; }
	virtual void Process() = 0;

	virtual bool ReadyCreateDBTable(const char* szTableIndex, tBaseTable *table, AString tableInfoData) = 0;

	virtual int GetOperateCount() = 0;
	virtual int GetErrorCount(){ return 0; }

public:
	// WARN: 参数表格,不可被主线使用, 只有完成后, 才可以继续使用
	virtual bool LoadAllRecord(ABaseTable destTable, LOAD_RECORD_CALLBACK callBack) = 0;

	// NOTE: 异步时, 必须保证投递执行顺序, 当删除后再插入同样KEY的值时, 才不会进行误删新的记录
	virtual void DelectData(const char *szKeyID, UInt64 extValue, LoadDataCallBack callBack) = 0;

	virtual bool SaveRecordData(const char *szKey, BaseRecord *pRe) = 0;

	virtual bool ReloadRecord(const char *szKey, AutoRecord destRecord) = 0;

	virtual bool ReadyReloadTool(NiceData &initParam){ return false; }

};
typedef Auto<tDataSource> AutoSource;

//-------------------------------------------------------------------------
class tDBSaver;
//-------------------------------------------------------------------------


class MemoryDB_Export DataSource : public tDataSource, public TaskManager
{
	friend class DataTask;
public:
	DataSource();
	~DataSource();

	virtual tDBSaver* CreateSaver();

	virtual void ClearAuto() { TaskManager::Close(); }

	virtual int GetOperateCount() { return TaskManager::GetNowTaskCount(); }

	virtual void AddErrorCount(int errorCount)
	{
		mErrorCountLock.lock();
		mErrorCount += errorCount;
		mErrorCountLock.unlock();
	}

	virtual int GetErrorCount()
	{
		int count = 0;
		mErrorCountLock.lock();
		count = mErrorCount;
		mErrorCountLock.unlock();
		return count;
	}

public:
	virtual const char* GetDBName();
	virtual bool InitDataSource(NiceData &initParam);
	virtual bool ReadyCreateDBTable(const char* szTableIndex, tBaseTable *tableInfoData, AString tableData);

	virtual int	OnceProcessOverTime() { return 50; }

	virtual void Process();

public:
	virtual bool SaveRecordData(const char *szKey, BaseRecord *pRe) override
	{ ERROR_LOG("DataSource 未实现SaveRecordData"); return false; }
	// WARN: 参数表格,不可被主线使用, 只有完成后, 才可以继续使用
	virtual bool LoadAllRecord(ABaseTable destTable, LOAD_RECORD_CALLBACK callBack);

	// NOTE: 异步时, 必须保证投递执行顺序, 当删除后再插入同样KEY的值时, 才不会进行误删新的记录
	virtual void DelectData(const char *szKeyID, UInt64 extValue, LoadDataCallBack callBack) override;

	virtual bool ReloadRecord(const char *szKey, AutoRecord destRecord) override
	{
		ERROR_LOG("未实现 ReloadRecord");
		return false;
	}

protected:
	void ReadyDBSaver(tDBSaver *saver);

protected:
	tDBSaver			*mDBSaver;
	Array<AutoNet>		mBackNet;		// 仅用于备份, 只用于保存, 不用于读取
	
	int					mErrorCount;
	CsLock				mErrorCountLock;
};
//-------------------------------------------------------------------------
// 字符串方式保存数据
// NOTE: 仅用于DB列表表格读取与保存, 由此数据源创建的表格源还是二进制(BLOB)方式
//-------------------------------------------------------------------------
class MemoryDB_Export StringDataSource : public DataSource
{
public:
	virtual bool RestoreRecord(BaseRecord *pDestRecord, const char *pData, int size)
	{
		return pDestRecord->FullFromString(pData, false, false);
	}

	virtual bool SaveRecordData(const char *szKey, BaseRecord *pRe);

};

//-------------------------------------------------------------------------
// 使用MYSQL DB 结构方式存储
//-------------------------------------------------------------------------
class DBTableSaver;

class MemoryDB_Export MySqlDataSource : public DataSource
{
public:
	virtual tDBSaver* CreateSaver();
	virtual bool SaveRecordData(const char *szKey, BaseRecord *pRe);

	virtual bool LoadAllRecord(ABaseTable destTable, LOAD_RECORD_CALLBACK callBack);

public:
	virtual bool ReloadRecord(const char *szKey, AutoRecord destRecord) override;

	virtual bool ReadyReloadTool(NiceData &initParam) override;

public:
	virtual void Process() override
	{
		DataSource::Process();
		if (mReloadRecordTool)
			mReloadRecordTool->Process();
	}

	//NOTE: 任务线程执行, 此调用一般会 6秒调用一次 event_timedwait(mWaitEvent, 6000)
	virtual void backProcess() override;

public:
	DBTableSaver* GetMySqlSaver();

public:
	// 主线用于重新调取记录的DB
	Auto<MySqlDBTool>	mReloadRecordTool;
};

class MemoryDB_Export MySqlStringDataSource : public StringDataSource
{
public:
	virtual tDBSaver* CreateSaver();

	//NOTE: 任务线程执行, 此调用一般会 6秒调用一次 event_timedwait(mWaitEvent, 6000)
	virtual void backProcess () override;
};
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// 备份及落地二合一 数据源
// 内部设定一处或两处远程落地进程
// 直接在主线程投递消息, 节省线程消耗, 即各种数据变化请求直接转移到目的地
////class BackDataSource : public tDataSource
////{
////public:
////	virtual void Process();
////
////	virtual void ReadyCreateDBTable(const char* szTableIndex, tBaseTable *tableInfoData)
////	{
////		
////	}
////
////public:
////	virtual void SaveData(Int64 keyID, void *pData, DSIZE size)
////	{
////		SaveData(STRING(keyID), pData, size);
////	}
////	virtual void SaveData(const char *szKeyID, void *pData, DSIZE size);
////
////	virtual void LoadData(Int64 keyID, LoadDataCallBack callBack)
////	{
////		LoadData(STRING(keyID), callBack);
////	}
////
////	virtual void LoadData(const char *szKeyID, LoadDataCallBack callBack);
////
////	// WARN: 参数表格,不可被主线使用, 只有完成后, 才可以继续使用
////	virtual void LoadAllRecord(ABaseTable destTable, DBCallBack callBack);
////
////	// NOTE: 异步时, 必须保证投递执行顺序, 当删除后再插入同样KEY的值时, 才不会进行误删新的记录
////	virtual void DelectData(const char *szKeyID, LoadDataCallBack callBack);
////
////
////public:
////	Array<AutoNet>		mBackNet;
////	AString				mDBName;
////};
//-------------------------------------------------------------------------
// 以本地文件落地方式
// DB文件异步保存和删除操作需要以下处理
// DB文件存储器内必须Hash索引记录KEY对应的记录位置
// 因为新建记录无法与同步到主线中保存的记录位置
//-------------------------------------------------------------------------

class LocalFileDBSaver;
class LocalDataSource : public DataSource
{
public:
	virtual bool InitDataSource(NiceData &initParam) override;

	virtual bool ReadyCreateDBTable(const char* szTableIndex, tBaseTable *tableInfoData, AString tableData) override;

	// WARN: 参数表格,不可被主线使用, 只有完成后, 才可以继续使用
	virtual bool LoadAllRecord(ABaseTable destTable, LOAD_RECORD_CALLBACK callBack) override;

	// NOTE: 异步时, 必须保证投递执行顺序, 当删除后再插入同样KEY的值时, 才不会进行误删新的记录
	virtual void DelectData(const char *szKeyID, UInt64 extValue, LoadDataCallBack callBack) override;
	virtual bool SaveRecordData(const char *szKey, BaseRecord *pRe) override;

	// 需要处理多线同步问题, 简单方法是, 只读DB文件
	// 目前全部在同一线程, 直接读取
	virtual bool ReloadRecord(const char *szKey, AutoRecord destRecord) override;

	virtual bool ReadyReloadTool(NiceData &initParam);

public:
	virtual tDBSaver* CreateSaver() override;

	LocalFileDBSaver* GetFileDBSaver();

public:
	LocalDataSource()
#if LOCAL_DB_USE_THREAD
		: mReloadTool(NULL)
#endif
	{
		CreateSaver();
	}
	~LocalDataSource();

#if LOCAL_DB_USE_THREAD
protected:
	LocalFileDBSaver	*mReloadTool;		// 为了多线同步, 创建一个重新加载记录的副本, 且切只可使用 ReloadRecord
#endif
};
//-------------------------------------------------------------------------

#endif //_INCLUDE_DATASOURCE_H_