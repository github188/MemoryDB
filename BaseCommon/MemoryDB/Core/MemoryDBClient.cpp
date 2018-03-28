
#include "MemoryDBClient.h"
#include "DBClientRequest.h"
#include "MemoryTableFieldIndex.h"
#include "DBNetPacket.h"

#include "MemoryDB.h"

//-------------------------------------------------------------------------*

DBClient::DBClient()
{
	mDBNet = MEM_NEW DBNet(this);
	mDBNet->BindEventCenter();

	RequestDBOperateEvent::InitReadyForNet(mDBNet);

	MemoryDB::RegisterDBClientEvent(mEventCenter);

	mEmptyDBRequest = mEventCenter->StartEvent("EmptyDBRequest");
	
}

AutoEvent DBClient::StartOperate( const char *szOperateName )
{
	if (mDBNet->IsOk())
		return mDBNet->GetClientConnect()->StartEvent(szOperateName);
	else
	{
		WARN_LOG("���������¼�[%s]ʱ, ���绹δ���ӳɹ�, ��ǰ����һ���յ��¼�", szOperateName);
		return mEmptyDBRequest;
	}
}

Logic::tEventCenter* DBClient::GetEventCenter()
{
	//if (mDBNet)
	//	return mDBNet->GetEventCenter();

	return mEventCenter.getPtr();
}

ABaseTable DBClient::CreateTable( const char *szIndex )
{
	ABaseTable t = MEM_NEW SkipBaseTable(eInitPoolField);
	t->SetTableName(szIndex);
	mTableList.erase(szIndex);
	mTableList.insert(szIndex, t);
	return t;
}

HandConnect DBClient::DBNet::CreateConnect()
{
	return MEM_NEW IOCPConnect(DB_NET_INIT_BUFFER_SIZE, DB_NET_INIT_BUFFER_SIZE);
}
