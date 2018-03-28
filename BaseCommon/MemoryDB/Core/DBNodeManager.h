/********************************************************************
	created:	2014/12/02
	created:	2:12:2014   18:23
	filename: 	G:\RemoteGame\BaseCommon\MemoryDB\Core\DBNodeManager.h
	file path:	G:\RemoteGame\BaseCommon\MemoryDB\Core
	file base:	DBNodeManager
	file ext:	h
	author:		Yang Wenge
	
	purpose:	优化的DB客户端
				理论上可以只通过一次网络事件完成DB操作，避免DBNode间进行中转
*********************************************************************/
#ifndef _INCLUDE_DBNODEMANAGER_H_
#define _INCLUDE_DBNODEMANAGER_H_

#include "DBManager.h"
#include "LogEvent.h"
#include "DBClientRequest.h"
#include "MemoryDBClient.h"
#include "MemoryDBHead.h"

//-------------------------------------------------------------------------
// DB表格分布信息
// 每个DB数据表对应一个DBClient
//-------------------------------------------------------------------------
class DBTableDistribution : public AutoBase
{
public:
	DBTableDistribution()
		: mbStringKey(false)
		, mDBSlotIndexList(DB_HASH_SLOT_COUNT)
	{

	}

public:
	Hand<DBClient> FindNode(Int64 key);

	Hand<DBClient> FindNode(const char *szKey);

	Hand<DBClient> _FindNode(UInt64 key);

	bool AppendNodeDistributionData( Hand<DBClient> node, AutoData slotData, bool bIsMainDB  );

	bool CheckAllHashSlotDB()
	{
		for (int i=0; i<mDBSlotIndexList.size(); ++i)
		{
			if (!mDBSlotIndexList[i])
				return false;
		}
		return true;
	}

public:
	struct KeyRange
	{
	public:
		// DBNode连接数据 使用KEY>"DBSERVER_IPPORT"保存该节点开放的DB服务IP及端口
		Hand<DBClient>	mNode;	
	};

public:
	Array<Hand<DBClient> >		mDBSlotIndexList;

	Array<KeyRange>		mDistributionList;	
	bool				mbStringKey;
};

typedef Auto<DBTableDistribution>	AutoDBTableDistribution;
//-------------------------------------------------------------------------
// DB应用管理
//-------------------------------------------------------------------------
class MemoryDB_Export DBNodeManager : public tDBManager
{
public:
	DBNodeManager();
	DBNodeManager(bool bCreateClient);

public:
	virtual void InitNodeConnectNet(tNetHandle *pDBConnectNet){}
	virtual bool IsOk(){ return mDBClient->IsConnected(); }

	virtual bool NeedSaveUpdateResponse() const { return true; }

	virtual int GetDBNetSaftCode() const { return DB_SERVER_NET_SAFE_CODE; }

public:
	virtual void Close();
	virtual void Process();

	virtual Hand<DBOperate> StartDBOperate(const char *operateType) { return mDBClient->StartOperate(operateType); }

	virtual void RegisterDBOperate(const char *szOperateType, Logic::tEventFactory *pTaskFactory);

	virtual bool InitDB( NiceData  *initParam );
	virtual bool InitConfigDB( NiceData  *initParam ) { return false; }
	virtual void StartLoadDBTableList(const char* listTableName, DBCallBack finishCallBack);

	virtual AutoTable CreateTable(const char *szTableIndex){ AssertEx(0, "不开放, 请使用DB工具创建表格"); return AutoTable(); }
	virtual AutoTable GetTable(const char *szTableIndex)
	{
		return mDBClient->GetTable(szTableIndex);
	}

public:
	virtual void SaveTable(AutoTable scrTable, DBCallBack finishCallBack) {  AssertEx(0, "不开放, 请使用DB工具创建表格"); }
	virtual void LoadTable(const char *szTableIndex, const char* dbTableName, DBCallBack finishCallBack);
	virtual void DeleteTable(const char *szTableIndex, const char* dbTableName, DBCallBack finishCallBack)
	{
		AssertEx(0, "不开放, 请使用DB工具删除表格");
	}

	virtual DBOperate* GetRecord(const char* szTableName, const AString &whereCondition, DBCallBack finishCallBack);
	virtual void GetRecordBySql(const char* szTableName, const AString &selectSqlString, DBCallBack finishCallBack)
	{
		AssertEx(0, "还未实现");
	}
	virtual void GetMultitermRecord(const AString &selectSqlString, int nNeedCount, DBCallBack finishCallBack)
	{
		AssertEx(0, "还未实现");
	}

	virtual void CreateRecord(const char* tableName, const AString &whereCondition, DBCallBack finishCallBack);
	virtual void DeleteRecord(const char* tableIndex, const char* szWhere, DBCallBack finishCallBack);

	virtual void DeleteRecord(AutoRecord deleteRecord, DBCallBack finishCallBack);
	virtual void InsertRecord(AutoRecord hRecord, bool bReplace, bool bGrowth, DBCallBack finishCallBack);
	virtual void UpdateRecord(AutoRecord hRecord, DBCallBack finishCallBack);
	virtual void AutoSaveRecord(AutoRecord hRecord, DBCallBack finishCallBack)
	{
		AssertEx(0, "不再使用");
	}

	// 修改指定数据的值  szDestFieldKey : 目标记录信息KEY, 如 [SCENE_DATA:1]场景子表中的KEY为1的记录; eMode 修改方式,增减及临界约束， limit "10000", szModifyFieldName 修改字段名
	virtual void ModifyData(DBCallBack callBack, const char *szTable, const char *szRecordKey, eModifyDataMode eMode, const char *limit, const char *szModifyFieldName, const char *destValue, const char *szDestFieldKey, ...);
	virtual bool LoadRecordData(DBCallBack finishCallBack, const char *szTable, const char *szRecordKey, const char *szFieldList);
	virtual bool LoadRecordData(DBCallBack finishCallBack, const char *szTable, const char *szRecordKey, const char *szFieldList, const char *szDestFieldInfo, ...);

	virtual void RecordOperate(DBCallBack, eRecordOperateMode mode, const char *szTable, const char *szRecordKey, AutoRecord dataRecord, const char *subTableFieldName, ...);
	virtual void RecordOperate(DBCallBack, eRecordOperateMode mode, const char *szTable, UInt64 recordKey, AutoRecord dataRecord, const char *subTableFieldName, ...);
	virtual void RecordOperate(DBCallBack, eRecordOperateMode mode, const char *szTable, const char *szRecordKey, AutoRecord dataRecord, int recordCountLimit, const char *subTableFieldName, ...);

	virtual void DeleteSubRecord(DBCallBack callBack, const char *szTable, const char *szRecordKey, const char *subTableFieldName, ...);
	virtual void DeleteSubRecord(DBCallBack callBack, const char *szTable, UInt64 recordKey, const char *subTableFieldName, ...);

	//-------------------------------------------------------------------------------------
	// 执行DB数据库的存储过程或存储函数
	virtual DBOperate* ExeSqlFunction(DBCallBack finishCallBack, const char *szTable, const char *szRecordKey, const char *szFunctionOprerate, AutoNice paramData);

public:
	virtual void Log(const char* szInfo, ...);

	virtual const char* GetDBName(){ return mDBName.c_str(); }
	virtual void SetDBEvent(Logic::tEventCenter *eventCenter, DBCallBack startCallBack, DBCallBack disconnectCallBack);
	virtual void LoadAllTable(DBCallBack finishCallBack);

	virtual void Reconnect(){ mDBClient->Reconnect(); }
	HandConnect GetDBConnect(const char *szDBTable, int dbKey);
	HandConnect GetRandDBConnect(const char *szDBTable);

	void OnConnectedMainDB(DBOperate *op, bool bSu);

	void OnResponseDistribution(DBOperate *op, bool bSu);

	virtual void OnNotifyDistribution(AutoTable distTable);


public:
	//DB表格分布部分
	bool RefreshNodeDistributionData(ABaseTable keyInfoTable );
	void ClearDistribution();

	// 根据表格节点分布信息，获取到与所在节点连接，启动DB请求
	AutoEvent StartDBOperate( const char *szOperateName, const char *szTable, Int64 key );

	AutoEvent StartDBOperate( const char *szOperateName, const char *szTable, const char *szKey );

	virtual Hand<DBClient> _CreateDBClinet(GSKEY nodeIpPort);

public:
	Hand<DBClient>				mDBClient;

	ThreadLog					mLog;
	EasyString					mDBName;

	DBCallBack					mConnectedCall;

	EasyHash<GSKEY, Hand<DBClient>>	mDBClientList;

protected:
	EasyHash<EasyString, AutoDBTableDistribution>		mTableDistributionDataList;

};

//-------------------------------------------------------------------------
class MainNodeClient : public DBClient
{
public:
	MainNodeClient(tDBManager *pMgr)
		: mpDBManager(pMgr)
	{

	}
	~MainNodeClient()
	{

	}

public:
	virtual tDBManager* GetDBManager(){ return mpDBManager; }
	virtual int GetDBNetSaftCode() const override { return dynamic_cast<DBNodeManager*>(mpDBManager)->GetDBNetSaftCode(); }

public:
	tDBManager *mpDBManager;
};
//-------------------------------------------------------------------------*/

#endif //_INCLUDE_DBNODEMANAGER_H_