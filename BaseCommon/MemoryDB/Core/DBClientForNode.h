
#ifndef _INCLUDE_DBCLIENTFORNODE_H_
#define _INCLUDE_DBCLIENTFORNODE_H_

#include "MemoryDBClient.h"
#include "MeshedNetNodeData.h"
#include "DBNodeManager.h"
//-------------------------------------------------------------------------*/
// ��Node�Ϸ��������ڵ�Ĺ���Ӧ��
// ��������ڱ��ڵ��ϣ���ֱ�ӽ��в���
// NOTE: DB���������ڵ�����Ҫ����һ�ηֲ�
//-------------------------------------------------------------------------*/
class DBNodeOperateManager : public DBNodeManager
{
public:
	virtual void LoadTable(const char *szTableIndex, const char* dbTableName, DBCallBack finishCallBack)
	{
		AssertEx(0, "����Ҫʹ�ô˹��� LoadTable");
	}
	virtual void DeleteTable(const char *szTableIndex, const char* dbTableName, DBCallBack finishCallBack)
	{
		AssertEx(0, "������, ��ʹ��DB����ɾ�����");
	}

	virtual DBOperate* GetRecord(const char* szTableName, const AString &whereCondition, DBCallBack finishCallBack);

	virtual void CreateRecord(const char* szTableName, const AString &whereCondition, DBCallBack finishCallBack);
	virtual void DeleteRecord(const char* szTableName, const char* whereCondition, DBCallBack finishCallBack);

	virtual void DeleteRecord(AutoRecord deleteRecord, DBCallBack finishCallBack)
	{
		if (deleteRecord->GetTable()!=NULL)		
			DeleteRecord(deleteRecord->GetTable()->GetTableName(), deleteRecord->getIndexData().string().c_str(), finishCallBack);
		else
			ERROR_LOG("DeleteRecord ɾ���ļ�¼��ȡ���Ϊ��");
	}
	virtual void InsertRecord(AutoRecord hRecord, bool bReplace, bool bGrowth, DBCallBack finishCallBack);
	virtual void UpdateRecord(AutoRecord hRecord, DBCallBack finishCallBack);

	// �޸�ָ�����ݵ�ֵ  szDestFieldKey : Ŀ���¼��ϢKEY, �� [SCENE_DATA:1]�����ӱ��е�KEYΪ1�ļ�¼; eMode �޸ķ�ʽ,�������ٽ�Լ���� limit "10000", szModifyFieldName �޸��ֶ���
	//virtual void ModifyData(DBCallBack callBack, const char *szTable, const char *szRecordKey, eModifyDataMode eMode, const char *limit, const char *szModifyFieldName, const char *destValue, const char *szDestFieldKey, ...)
	//virtual bool LoadRecordData(DBCallBack finishCallBack, const char *szTable, const char *szRecordKey, const char *szFieldList);
	//virtual bool LoadRecordData(DBCallBack finishCallBack, const char *szTable, const char *szRecordKey, const char *szFieldList, const char *szDestFieldInfo, ...);

	//virtual void RecordOperate(DBCallBack, eRecordOperateMode mode, const char *szTable, const char *szRecordKey, AutoRecord dataRecord, const char *subTableFieldName, ...);
	//virtual void RecordOperate(DBCallBack, eRecordOperateMode mode, const char *szTable, UInt64 recordKey, AutoRecord dataRecord, const char *subTableFieldName, ...);
	//virtual void RecordOperate(DBCallBack, eRecordOperateMode mode, const char *szTable, const char *szRecordKey, AutoRecord dataRecord, int recordCountLimit, const char *subTableFieldName, ...);

	//virtual void DeleteSubRecord(DBCallBack callBack, const char *szTable, const char *szRecordKey, const char *subTableFieldName, ...);
	//virtual void DeleteSubRecord(DBCallBack callBack, const char *szTable, UInt64 recordKey, const char *subTableFieldName, ...);

	//-------------------------------------------------------------------------------------
	// ִ��DB���ݿ�Ĵ洢���̻�洢����
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
// ��Node�Ϸ��������ڵ�Ĺ���
// �滻DBClient�����粿��
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
			ERROR_LOG("��Ч��DB�ڵ���������");
		}
	}

public:
	Hand<NetNodeConnectData>	mNodeData;
};
//-------------------------------------------------------------------------*/
// 
//-------------------------------------------------------------------------*/


#endif //_INCLUDE_DBCLIENTFORNODE_H_