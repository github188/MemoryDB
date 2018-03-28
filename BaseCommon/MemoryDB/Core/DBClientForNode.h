
#ifndef _INCLUDE_DBCLIENTFORNODE_H_
#define _INCLUDE_DBCLIENTFORNODE_H_

#include "MemoryDBClient.h"
#include "MeshedNetNodeData.h"
#include "DBNodeManager.h"
//-------------------------------------------------------------------------*/
// 在Node上访问其他节点的功能应用
// 如果操作在本节点上，会直接进行操作
// NOTE: DB连接其它节点后就需要更新一次分布
//-------------------------------------------------------------------------*/
class DBNodeOperateManager : public DBNodeManager
{
public:
	virtual void LoadTable(const char *szTableIndex, const char* dbTableName, DBCallBack finishCallBack)
	{
		AssertEx(0, "不需要使用此功能 LoadTable");
	}
	virtual void DeleteTable(const char *szTableIndex, const char* dbTableName, DBCallBack finishCallBack)
	{
		AssertEx(0, "不开放, 请使用DB工具删除表格");
	}

	virtual DBOperate* GetRecord(const char* szTableName, const AString &whereCondition, DBCallBack finishCallBack);

	virtual void CreateRecord(const char* szTableName, const AString &whereCondition, DBCallBack finishCallBack);
	virtual void DeleteRecord(const char* szTableName, const char* whereCondition, DBCallBack finishCallBack);

	virtual void DeleteRecord(AutoRecord deleteRecord, DBCallBack finishCallBack)
	{
		if (deleteRecord->GetTable()!=NULL)		
			DeleteRecord(deleteRecord->GetTable()->GetTableName(), deleteRecord->getIndexData().string().c_str(), finishCallBack);
		else
			ERROR_LOG("DeleteRecord 删除的记录获取表格为空");
	}
	virtual void InsertRecord(AutoRecord hRecord, bool bReplace, bool bGrowth, DBCallBack finishCallBack);
	virtual void UpdateRecord(AutoRecord hRecord, DBCallBack finishCallBack);

	// 修改指定数据的值  szDestFieldKey : 目标记录信息KEY, 如 [SCENE_DATA:1]场景子表中的KEY为1的记录; eMode 修改方式,增减及临界约束， limit "10000", szModifyFieldName 修改字段名
	//virtual void ModifyData(DBCallBack callBack, const char *szTable, const char *szRecordKey, eModifyDataMode eMode, const char *limit, const char *szModifyFieldName, const char *destValue, const char *szDestFieldKey, ...)
	//virtual bool LoadRecordData(DBCallBack finishCallBack, const char *szTable, const char *szRecordKey, const char *szFieldList);
	//virtual bool LoadRecordData(DBCallBack finishCallBack, const char *szTable, const char *szRecordKey, const char *szFieldList, const char *szDestFieldInfo, ...);

	//virtual void RecordOperate(DBCallBack, eRecordOperateMode mode, const char *szTable, const char *szRecordKey, AutoRecord dataRecord, const char *subTableFieldName, ...);
	//virtual void RecordOperate(DBCallBack, eRecordOperateMode mode, const char *szTable, UInt64 recordKey, AutoRecord dataRecord, const char *subTableFieldName, ...);
	//virtual void RecordOperate(DBCallBack, eRecordOperateMode mode, const char *szTable, const char *szRecordKey, AutoRecord dataRecord, int recordCountLimit, const char *subTableFieldName, ...);

	//virtual void DeleteSubRecord(DBCallBack callBack, const char *szTable, const char *szRecordKey, const char *subTableFieldName, ...);
	//virtual void DeleteSubRecord(DBCallBack callBack, const char *szTable, UInt64 recordKey, const char *subTableFieldName, ...);

	//-------------------------------------------------------------------------------------
	// 执行DB数据库的存储过程或存储函数
	virtual DBOperate* ExeSqlFunction(DBCallBack finishCallBack, const char *szTable, const char *szRecordKey, const char *szFunctionOprerate, AutoNice paramData);


public:
	Hand<DBClient> _CreateDBClinet(GSKEY nodeIpPort);

	virtual bool InitDB( NiceData *initParam ) override;

public:
	DBNodeOperateManager(MemoryDBNode *pNode);

public:
	MemoryDBNode		*mDBNode;
	Hand<DB_Request>	mTempRequest;
};
//-------------------------------------------------------------------------*/
// 在Node上访问其他节点的功能
// 替换DBClient的网络部分
//-------------------------------------------------------------------------*/
class DBClientForNode : public DBClient
{
public:
	virtual AutoEvent StartOperate(const char *szOperateName) override;

	virtual bool Start(const char *szDBIp, int port)
	{
		return false;
	}

	virtual void Reconnect()
	{		
	}

	virtual void Close()
	{
		mNetSucceedCallBack.cleanup();
		mNetDisconnectCallBack.cleanup();
		BaseMemoryDB::Close();
	}

	virtual void Process()
	{
		BaseMemoryDB::Process();
	}

	virtual bool IsConnected(){ return mNodeData->IsConnected(); }

public:
	DBClientForNode(){}
	DBClientForNode(Hand<NetNodeConnectData> dbNode)
	{
		mNodeData = dbNode;
		if (!mNodeData || !mNodeData->IsConnected())
		{
			ERROR_LOG("无效的DB节点网络数据");
		}
	}

public:
	Hand<NetNodeConnectData>	mNodeData;
};
//-------------------------------------------------------------------------*/
// 
//-------------------------------------------------------------------------*/


#endif //_INCLUDE_DBCLIENTFORNODE_H_