#include "DBClientForNode.h"
#include "MemoryDBNode.h"

//-------------------------------------------------------------------------*/
AutoEvent DBClientForNode::StartOperate(const char *szOperateName)
{
	if (mNodeData && mNodeData->mNodeConnect)
		return mNodeData->mNodeConnect->StartEvent(szOperateName);

	ERROR_LOG("启动DB操作[%s]失败, 未注册或 DB Node 网络还未连接成功, 当前返回一个空的事件", szOperateName);
	return mEmptyDBRequest;
}

DBOperate* DBNodeOperateManager::GetRecord(const char* szTableName, const AString &whereCondition, DBCallBack finishCallBack)
{
	if (mDBNode->CheckKeyInThisNodeRange(szTableName, whereCondition.c_str()))
	{
		mTempRequest->initData();
		mTempRequest->mResultRecord = mDBNode->GetTable(szTableName)->GetRecord(whereCondition.c_str());
		mTempRequest->mCallBack = finishCallBack;
		mTempRequest->Finish();
		return mTempRequest.getPtr();
	}
	return DBNodeManager::GetRecord(szTableName, whereCondition, finishCallBack);
}

void DBNodeOperateManager::CreateRecord(const char* szTableName, const AString &whereCondition, DBCallBack finishCallBack)
{
	if (mDBNode->CheckKeyInThisNodeRange(szTableName, whereCondition.c_str()))
	{
		mTempRequest->initData();
		mTempRequest->mResultRecord = mDBNode->GetTable(szTableName)->CreateRecord(whereCondition.c_str(), true);
		mTempRequest->mCallBack = finishCallBack;
		mTempRequest->Finish();
		return;
	}
	return DBNodeManager::CreateRecord(szTableName, whereCondition, finishCallBack);
}

void DBNodeOperateManager::DeleteRecord(const char* szTableName, const char* whereCondition, DBCallBack finishCallBack)
{
	if (mDBNode->CheckKeyInThisNodeRange(szTableName, whereCondition))
	{
		mTempRequest->initData();
		AutoRecord re = mDBNode->GetTable(szTableName)->GetRecord(whereCondition);
		if (re)
			mTempRequest->mResultType = mDBNode->GetTable(szTableName)->DeleteRecord(re) ? eNoneError:eRecordNoExist;
		else
			mTempRequest->mResultType = eRecordNoExist;
		mTempRequest->mCallBack = finishCallBack;
		mTempRequest->Finish();
		return;
	}
	return DBNodeManager::DeleteRecord(szTableName, whereCondition, finishCallBack);
}

void DBNodeOperateManager::InsertRecord(AutoRecord hRecord, bool bReplace, bool bGrowth, DBCallBack finishCallBack)
{
	tBaseTable *pTable = hRecord->GetTable();
	if (pTable==NULL)
	{
		ERROR_LOG("记录表格不可为空");
		return;
	}
	AString key = hRecord->getIndexData().string();
	if (mDBNode->CheckKeyInThisNodeRange(pTable->GetTableName(), key.c_str()))
	{
		mTempRequest->initData();
		int code = hRecord->getField()->GetCheckCode();
		if (code==mDBNode->GetTable(pTable->GetTableName())->GetField()->GetCheckCode())
		{
			SaveRecordDataBuffer d(1024);
			hRecord->saveData(&d);
			AutoRecord re;
			mTempRequest->mResultType = mDBNode->_SaveRecord(pTable->GetTableName(), key.c_str(), &d, bReplace, false);
			mTempRequest->mCallBack = finishCallBack;
			mTempRequest->Finish();
			return;
		}
	}
	DBNodeManager::InsertRecord(hRecord, bReplace, bGrowth, finishCallBack);
}

void DBNodeOperateManager::UpdateRecord(AutoRecord hRecord, DBCallBack finishCallBack)
{
	tBaseTable *pTable = hRecord->GetTable();
	if (pTable==NULL)
	{
		ERROR_LOG("记录表格不可为空");
		return;
	}
	AString key = hRecord->getIndexData().string();
	if (mDBNode->CheckKeyInThisNodeRange(pTable->GetTableName(), key.c_str()))
	{
		mTempRequest->initData();
		int code = hRecord->getField()->GetCheckCode();
		if (code==mDBNode->GetTable(pTable->GetTableName())->GetField()->GetCheckCode())
		{
			SaveRecordDataBuffer d(1024);
			hRecord->saveUpdateData(&d);
			AutoRecord re;
			mTempRequest->mResultType = mDBNode->_SaveRecord(pTable->GetTableName(), key.c_str(), &d, false, true);
			mTempRequest->mCallBack = finishCallBack;
			mTempRequest->Finish();
			return;
		}
	}
	DBNodeManager::UpdateRecord(hRecord, finishCallBack);
}

DBOperate* DBNodeOperateManager::ExeSqlFunction(DBCallBack finishCallBack, const char *szTable, const char *szRecordKey, const char *szFunctionOprerate, AutoNice paramData)
{
	if (mDBNode->CheckKeyInThisNodeRange(szTable, szRecordKey))
	{
		mTempRequest->initData();
		eDBResultType result = eNoneError;
		mDBNode->RunOperate(NULL, szFunctionOprerate, szTable, szRecordKey, paramData, result);
		mTempRequest->mResultType = result;
		mTempRequest->mCallBack = finishCallBack;
		mTempRequest->Finish();
		return mTempRequest.getPtr();
	}
	return DBNodeManager::ExeSqlFunction(finishCallBack, szTable, szRecordKey, szFunctionOprerate, paramData);
}

//-------------------------------------------------------------------------*/

Hand<DBClient> DBNodeOperateManager::_CreateDBClinet(GSKEY nodeIpPort)
{
	Hand<DBClient> clientDB;

	Hand<MeshedNodeNet> net = mDBNode->GetNodeNet();
	NetNodeList &list = net->GetNetNodeList();

	for (auto it = list.begin(); it.have(); it.next())
	{
		Hand<NetNodeConnectData> node = it.get();
		if (node && (GSKEY)node["DBSERVER_IPPORT"]==nodeIpPort)
		{
			clientDB = MEM_NEW DBClientForNode(node);
			node->SetUseData(clientDB);
		}
	}

	return clientDB;
}

bool DBNodeOperateManager::InitDB(NiceData *initParam)
{
	mDBClient = MEM_NEW DBClientForNode();

	mTempRequest = mDBNode->GetNodeNet()->GetEventCenter()->StartEvent("EmptyDBRequest");

	return true;
}

DBNodeOperateManager::DBNodeOperateManager(MemoryDBNode *pNode) 
	: mDBNode(pNode)
	, DBNodeManager(false)
{
	
}
