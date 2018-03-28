#include "MemoryDBNode.h"

#include "MemoryDBIndexTable.h"

#include "DBNodeEvent.h"
#include "MeshedNetNodeData.h"
#include "StringHashIndex.h"
#include "ServerIPInfo.h"
#include "TimeManager.h"
#include "DataSource.h"
#include <stdarg.h>
#include "DBClientForNode.h"
#include "DBNetPacket.h"

//-------------------------------------------------------------------------*/
//-------------------------------------------------------------------------*/
class f_getAlltableField : public tDBNodeOpreate
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData)
	{
		AutoTable mResultTable = tBaseTable::NewBaseTable();
		mResultTable->SetField("TABLE_NAME", FIELD_STRING, 0);
		mResultTable->SetField("FIELD_DATA", FIELD_DATA, 1);
		mResultTable->SetField("FIELD_CODE", FIELD_INT, 2);
		TableHashMap &list = pDBNode->GetAllTableList();

		for (auto it=list.begin(); it.have(); it.next())
		{
			AutoDBTable table = it.get();
			if (!table || table==pDBNode->GetDBTableListTable())
				continue;

			ARecord re = mResultTable->CreateRecord(table->GetTableName(), true);

			AutoData fieldData = MEM_NEW DataBuffer(512);
			if (table->GetField()->saveToData(fieldData.getPtr()))
			{
				re->set("FIELD_DATA", fieldData);
				re->set("FIELD_CODE", table->GetField()->GetCheckCode());
			}
			else
			{
				ERROR_LOG("[%s] Save field data fail", table->GetTableName());
				return eDBOperateResultFail;
			}
		}

		mResultData->set("TABLE_LIST", &mResultTable, typeid(mResultTable));
		return eNoneError;
	}
};

class f_deleteRecord : public tDBNodeOpreate
{
public:
	virtual int Execute(MemoryDBNode *pDBNode, const char *szTable, const char *recordKey, AutoNice &paramData)
	{
		AutoTable t = pDBNode->GetTable(szTable);
		if (!t)
		{
			return eTableNoExist;
		}
		AutoIndex index = t->GetMainIndex();
		ARecordIt it = index->GetRecordIt();
		AutoRecord re = *it;
		while(!re && (*it))
		{
			++(*it);
			 re = *it;
		}

		if (re)
			if (t->DeleteRecord(re))
				return eNoneError;

		return eRecordNoExist;
	}
};
//-------------------------------------------------------------------------
class TM_MemoryCheckEvent : public CEvent
{
public:
	TM_MemoryCheckEvent()
		: mpDBNode(NULL)
		, mLastReceiveCount(0)
		, mLastSendCount(0)
		, mLastReceiveSizeBySec(0)
		, mLastSendSizeBySec(0)
		, mLastFrameCount(0)
	{

	}

public:
	virtual bool _DoEvent()
	{
		setFinished(false);
		WaitTime(DB_STATE_UPDATE_TIME);
		return true;
	}

	virtual bool _OnTimeOver()
	{
		//static const size_t MinSize = 512*1024*1024;	// С��512Mֹͣ�½���¼

		size_t availSize = mpDBNode->AvailMemory();
		size_t nowUseSize = mpDBNode->NowUseMemory();

		if (availSize<DB_AVAIL_MEMORY_MIN*1024*1024)
		{
			LOG_RED;
			printf("���ؾ���: �����ڴ�[%.3f]С��[%.3f], ��ͣ�½���¼����\r\n",
				(float)availSize/1024/1024,
				(float)DB_AVAIL_MEMORY_MIN
				);
			if (mpDBNode->GetmemoryState())
			{
				mpDBNode->SetMemoryState(false);
				mpDBNode->OnNotifyMemoryWarn(availSize);
			}
		}
		else if (!mpDBNode->GetmemoryState())
		{
			mpDBNode->SetMemoryState(true);
			INFO_LOG("�ڴ�״̬�ָ�����, �ָ���������");
		}

		LOG_GREEN;
		printf("Now use memory [%.3f]M, Last free memory [%.3f]M, Connect [%d]\r\n", 
			(float)nowUseSize/1024/1024, 
			(float)availSize/1024/1024,
			mpDBNode->GetDBServerNet()->GetConnectCount()
			);
		LOG_WHITE;

		mpDBNode->NowDBState();

		// DB������Ϣ
		Hand<MemoryDB::DBServerNet> net = mpDBNode->GetDBServerNet();
		mLastReceiveCount = net->mReceiveCount;
		mLastSendCount = net->mSendCount;

		mLastReceiveSizeBySec = net->mReceiveSize/DB_STATE_UPDATE_TIME;
		mLastSendSizeBySec = net->mSendSize/DB_STATE_UPDATE_TIME;

		net->mReceiveCount = 0;
		net->mSendCount = 0;
		net->mReceiveSize = 0;
		net->mSendSize = 0;

		mLastFrameCount = (float)mpDBNode->mFrameCount/DB_STATE_UPDATE_TIME;
		mpDBNode->mFrameCount = 0;

		DoEvent();
		return true;
	}	

public:
	MemoryDBNode	*mpDBNode;

	int			mLastReceiveCount;
	int			mLastSendCount;
	size_t		mLastReceiveSizeBySec;
	size_t		mLastSendSizeBySec;

	float		mLastFrameCount;
};
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
DBMeshedNodeNet::DBMeshedNodeNet( MemoryDBNode *pDBNode, const char *szServerIp, int nServerPort, int safeCheckCode ) 
	: MeshedNodeNet(szServerIp, nServerPort, safeCheckCode)
	, mpDBNode(pDBNode)	
{
	SetEventCenter(MEM_NEW EventCenter(), MEM_NEW EventCenter());
	RegisterDBNodeNetEvent();
}

void DBMeshedNodeNet::RegisterDBNodeNetEvent()
{
	AutoEventCenter center =GetEventCenter()->GetSelf(); 
	AutoEventCenter serverCenter = GetServerCenter();

	center->RegisterEvent("DD_NodifyNodeList", MEM_NEW SR_RequestEventFactory<DD_NodifyNodeList>());
	serverCenter->RegisterEvent("DD_NodifyNodeList", MEM_NEW EventFactory<DD_NodifyNodeList_R>());
	
	//center->RegisterEvent("DD_RequestDataDistributionInfo", MEM_NEW SR_RequestEventFactory<DD_RequestDataDistributionInfo>());
	//serverCenter->RegisterEvent("DD_RequestDataDistributionInfo", MEM_NEW EventFactory<DD_RequestDataDistributionInfo_R>());

	center->RegisterEvent("DD_RequestSaveRecord", MEM_NEW SR_RequestEventFactory<DD_RequestSaveRecord>());
	serverCenter->RegisterEvent("DD_RequestSaveRecord", MEM_NEW EventFactory<DD_RequestSaveRecord_R>());

	center->RegisterEvent("DD_RequestFindRecord", MEM_NEW SR_RequestEventFactory<DD_RequestFindRecord>());
	serverCenter->RegisterEvent("DD_RequestFindRecord", MEM_NEW EventFactory<DD_RequestFindRecord_R>());

	center->RegisterEvent("DD_RequestDeleteRecord", MEM_NEW SR_RequestEventFactory<DD_RequestDeleteRecord>());
	serverCenter->RegisterEvent("DD_RequestDeleteRecord", MEM_NEW EventFactory<DD_RequestDeleteRecord_R>());

	center->RegisterEvent("DD_RequestCreateGrowthRecord", MEM_NEW SR_RequestEventFactory<DD_RequestCreateGrowthRecord>());
	serverCenter->RegisterEvent("DD_RequestCreateGrowthRecord", MEM_NEW EventFactory<DD_RequestCreateGrowthRecord_R>());

	center->RegisterEvent("DD_RunDBOpereate", MEM_NEW SR_RequestEventFactory<DD_RunDBOpereate>());
	serverCenter->RegisterEvent("DD_RunDBOpereate", MEM_NEW EventFactory<DD_RunDBOpereate_R>());

	center->RegisterEvent("DD_RequestGetRecordData", MEM_NEW SR_RequestEventFactory<DD_RequestGetRecordData>());
	serverCenter->RegisterEvent("DD_RequestGetRecordData", MEM_NEW EventFactory<DD_RequestGetRecordData_R>());

	center->RegisterEvent("DD_RequestModifyData", MEM_NEW SR_RequestEventFactory<DD_RequestModifyData>());
	serverCenter->RegisterEvent("DD_RequestModifyData", MEM_NEW EventFactory<DD_RequestModifyData_R>());

	center->RegisterEvent("DD_LoadRecordData", MEM_NEW SR_RequestEventFactory<DD_LoadRecordData>());
	serverCenter->RegisterEvent("DD_LoadRecordData", MEM_NEW EventFactory<DD_LoadRecordData_R>());

	center->RegisterEvent("DD_SaveRecordData", MEM_NEW SR_RequestEventFactory<DD_SaveRecordData>());
	serverCenter->RegisterEvent("DD_SaveRecordData", MEM_NEW EventFactory<DD_SaveRecordData_R>());

	center->RegisterEvent("DD_RequestCreateNodeTable", MEM_NEW SR_RequestEventFactory<DD_RequestCreateNodeTable>());
	serverCenter->RegisterEvent("DD_RequestCreateNodeTable", MEM_NEW EventFactory<DD_RequestCreateNodeTable_R>());
	
}

void DBMeshedNodeNet::OnAppendNetNode( NetNodeConnectData *nodeData )
{
	//if (mpDBNode->mMainDBNode)
	//{ 
	//	AutoEvent evt = nodeData->mNodeConnect->StartEvent("DD_NodifyNodeList");
	//	evt->DoEvent();
	//}
	//AutoEvent infoRequet = nodeData->mNodeConnect->StartEvent("DD_RequestDataDistributionInfo");
	//infoRequet->DoEvent();
}

void DBMeshedNodeNet::OnResponseNode(GSKEY otherPartyGSKey, AutoEvent &requstEvt, AutoEvent &respEvt)
{
	MeshedNodeNet::OnResponseNode(otherPartyGSKey, requstEvt, respEvt);
	ABaseTable keyInfoTable = mpDBNode->GetNowDataTableKeyInfo();
	respEvt["KEY_INFO"] = keyInfoTable.getPtr();

	UInt64 serverIPKey = ServerIPInfo::IP2Num(mpDBNode->GetDBServerNet()->GetIp(), mpDBNode->GetDBServerNet()->GetPort(), 0); 
	respEvt["DBSERVER_IPPORT"] = serverIPKey;

	respEvt["DB_CONFIG"] = mpDBNode->GetDBConfig().ToJSON();
	//AutoTable infoTable = mpDBNode->GetAllDBTableDistributionData();
	//respEvt->set("INFO_TABLE", infoTable);
	//respEvt->set("DB_IPPORT", ServerIPInfo::IP2Num(mpDBNode->GetDBServerNet()->GetIp(), mpDBNode->GetDBServerNet()->GetPort(), port, 0));
}

void DBMeshedNodeNet::OnReceiveResponse(NetNodeConnectData *nodeData, AutoEvent &requestEvt, AutoEvent &respEvt)
{
	MeshedNodeNet::OnReceiveResponse(nodeData, requestEvt, respEvt);
	// �����ڲ�����DB
	nodeData->set("DBSERVER_IPPORT", (GSKEY)respEvt["DBSERVER_IPPORT"]);
	nodeData->set("DB_CONFIG", respEvt["DB_CONFIG"].string());

	AutoTable keyInfoTable = (tBaseTable*)respEvt["KEY_INFO"];
	if (keyInfoTable)
		mpDBNode->AppendNodeDistributionData(nodeData->GetSelf(), keyInfoTable);
	else
		ERROR_LOG("DB δ�ṩ KEY_INFO table");
	
	AutoTable disTable = mpDBNode->GetAllDBTableDistributionData();
	if (disTable)
	{
		Hand<DBNodeOperateManager> mgr = mpDBNode->GetDBOperateMgr();
		mgr->RefreshNodeDistributionData(disTable);

		//??? ����
		//for(int i=0; i<10; ++i)
		//{
		//	mpDBNode->Process();
		//	TimeManager::Sleep(100);
		//}
		//mgr->GetRecord("t_account", "w", DBCallBack(&DBMeshedNodeNet::OnTestFinish, this));
	}
	else
	{ 
		ERROR_LOG("δ�ṩ�ֲ���Ϣ���"); 
	}
}

void DBMeshedNodeNet::OnTestFinish(DBOperate *op, bool bSu)
{
	if (bSu && op->mResultRecord)
	{
		AString testRecordData;
		op->mResultRecord->ToString(testRecordData, true, false);
		NOTE_LOG("Test result\r\n%s", testRecordData.c_str());
	}
	else
		NOTE_LOG("Test fail [%d]>[%s]", op->mResultType, op->mErrorInfo.c_str());
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
MemoryDBNode::MemoryDBNode( const char *szNodeIp, int nodePort ) 
	: MemoryDB()
	, mMainDBNode(false)
	, mMemoryOk(true)
	, mFrameCount(0)
{
	DBMeshedNodeNet *pNet = MEM_NEW DBMeshedNodeNet(this, szNodeIp, nodePort, DB_NODE_NET_CODE);
	mDBNodeNet = pNet;
	// Ϊ��֧��ֱ�����ݲ���,ע�����в���
	MemoryDB::RegisterDBServerEvent(pNet->GetServerCenter()->GetSelf());
	MemoryDB::RegisterDBClientEvent(pNet->GetEventCenter()->GetSelf());
	RequestDBOperateEvent::InitReadyForNet(mDBNodeNet);
	RequestDBOperateEvent::InitReadyForNet(pNet->GetServerCenter()->_GetNetTool(0));

	// NOTE: ֧����תDB������Ϣ��
	DBNetProtocol::RegisterPacket(pNet->GetNetProtocol());

	mDBOperateManager = MEM_NEW DBNodeOperateManager(this);

	GetEventCenter()->RegisterEvent("TM_MemoryCheckEvent", MEM_NEW EventFactory<TM_MemoryCheckEvent>());
	Hand<TM_MemoryCheckEvent> checkEvt = GetEventCenter()->StartEvent("TM_MemoryCheckEvent");
	checkEvt->mpDBNode = this;
	mMemoryCheckEvent = checkEvt;
	mMemoryCheckEvent->DoEvent();

	RegisterDBOperate("f_getAlltableField", MEM_NEW EventFactory<f_getAlltableField>);
	RegisterDBOperate("f_deleteRecord", MEM_NEW EventFactory<f_deleteRecord>);
	
}

void MemoryDBNode::Process()
{
	++mFrameCount;
	MemoryDB::Process();
	mDBNodeNet->Process();
}

void MemoryDBNode::OnNewCreateDBTable(const char *szTableName, AutoTable table, const AString &tableInfoData, AutoNice extParam, AutoRecord &tableInfoRecord)
{
	if (extParam && extParam["USE_NODE"])
	{
		// ʹ�þ��ȷֲ�
		Hand<MeshedNodeNet> net = mDBNodeNet;
		ConnectList &list = net->GetConnectList();
		int nCount = 1;
		//AString allDBInfo = "<";				
		//allDBInfo += STRING(nCount);
		//allDBInfo += "> ";
		//allDBInfo += GetDBConfig().ToJSON();
		NiceData allDBInfoData;
		for (int i=0; i<list.size(); ++i)
		{
			Hand<NodeRequestConnect> conn = list[i];
			if (conn && !conn->IsDisconnect())
			{
				++nCount;
				AutoNice d = MEM_NEW NiceData();
				d->FullJSON(conn->mNetNodeConnectData->get("DB_CONFIG").string());		
				allDBInfoData[STRING(nCount)] = d;
				//allDBInfo += "; <";
				//allDBInfo += STRING(nCount);
				//allDBInfo += "> ";
				//allDBInfo += conn->mNetNodeConnectData->get("DB_CONFIG").string();
			}
		}

		if (nCount>1)
		{			
			AutoNice d = MEM_NEW NiceData();
			d->append(GetDBConfig(), true);
			allDBInfoData[STRING(1)] = d;
			allDBInfoData["COUNT"] = nCount;
			AString allDBInfo = allDBInfoData.ToJSON();
			// ƽ��
			int x = DB_HASH_SLOT_COUNT / nCount;
			AutoData slotData = MEM_NEW DataBuffer(128);

			AutoData fieldData = MEM_NEW DataBuffer(256);
			table->GetField()->saveToData(fieldData.getPtr());
			short slotIndex = 0;
			for (int i=0; i<list.size(); ++i)
			{
				Hand<NodeRequestConnect> conn = list[i];
				if (conn && !conn->IsDisconnect())
				{
					// ����һ�������ֲ���
					slotData->clear(false);
					for (int i=0; i<x; ++i)
					{
						slotData->write(slotIndex++);
					}
					AutoEvent request = conn->StartEvent("DD_RequestCreateNodeTable");
					request["TABLE"] = szTableName;
					request["TABLE_INFO"] = tableInfoData;
					request["SLOT_INFO"] = slotData.getPtr();
					request["TABLE_FIELD"] = fieldData.getPtr();
					request["ALL_DB"] = allDBInfo;
					request->Start();
				}
			}
			slotData->clear(false);
			// ��ʣ��Ķ����䵽��ǰ�ڵ�
			for (; slotIndex<DB_HASH_SLOT_COUNT; ++slotIndex)
			{
				slotData->write(slotIndex);
			}
			tableInfoRecord["SLOT"] = slotData.getPtr();
			tableInfoRecord["DB_INFO"] = allDBInfo;
		}

	}
}

bool MemoryDBNode::InitDB( NiceData &initParam )
{	
	mDBOperateManager->InitDB(&initParam);

	AString mainIp = initParam.get("MAIN_IP");
	Data mainPort = initParam.get("MAIN_PORT");

	AssertEx(!mainIp.empty() && !mainPort.empty(), "��������DB���ڵ�IP���˿�");

	if (MemoryDB::InitDB(initParam))
	{
		Hand<MeshedNodeNet> net = mDBNodeNet;
		net->SetMainNode(mainIp.c_str(), mainPort);
		// ������ڵ�������Ϣ��ڵ������Ϣ��ͬ, ����Ϊ��DB���ڵ�, �߱����ڵ��, �ڽ��յ�����ʱ��ѵ�ǰ�Ľڵ��б��͸��Է�
		if (mainIp!= mDBNodeNet->GetIp() || (int)mainPort!=mDBNodeNet->GetPort())
			mDBNodeNet->StartNet(mainIp.c_str(), mainPort);
		else
			mMainDBNode = true; 		

		return true;
	}
	return false;
}

ABaseTable MemoryDBNode::GetNowDataTableKeyInfo()
{
	ABaseTable infoTable = tBaseTable::NewBaseTable();
	int col = 0;
	infoTable->SetField("TABLE_NAME", FIELD_STRING, col++);
	infoTable->SetField("IS_STRING_KEY", FIELD_BOOL, col++);
	//infoTable->SetField("MIN_KEY", FIELD_UINT64, col++);
	//infoTable->SetField("MAX_KEY", FIELD_UINT64, col++);
	infoTable->SetField("FIELD_DATA", FIELD_STRING, col++);
	infoTable->SetField("FIELD_CODE", FIELD_INT, col++);
	infoTable->SetField("SLOT_INFO", FIELD_DATA, col++);

	for (auto it=mTableList.begin(); it.have(); it.next())
	{
		AutoDBTable t = it.get();
		
		ARecord infoRecord = mTableListDBTable->GetRecord(it.key());
		if (!infoRecord || !t || t==mTableListDBTable)	//�ų��б���Ϣ���
			continue;

		AutoIndex index = t->GetMainIndex();
		ARecord re = infoTable->CreateRecord(t->GetTableName(), true);
		//Int64 m1, m2;
		//t->GetKeyIDRange(m1, m2);
		//re->set("MIN_KEY", m1);
		//re->set("MAX_KEY", m2);
		bool bStringKey = dynamic_cast<StringHashIndex*>(index.getPtr())!=NULL;
		re->set("IS_STRING_KEY", bStringKey);
		AString fieldData = t->GetField()->ToString();
		re->set("FIELD_DATA", fieldData.c_str());
		re->set("FIELD_CODE", t->GetField()->GetCheckCode());
		re["SLOT_INFO"] = (DataStream*)infoRecord["SLOT"];
	}

	return infoTable;
}

bool MemoryDBNode::AppendNodeDistributionData( Hand<NetNodeConnectData> node, ABaseTable keyInfoTable )
{
	for (ARecordIt tIt= keyInfoTable->GetRecordIt(); *tIt; ++(*tIt))
	{
		ARecord re = *tIt;

		// �뱾���������
		//ABaseTable localTable = GetTable(re->getIndexData().string().c_str());
		//if (localTable)
		//{
		//	Int64 nowMin=0, nowMax=0;
		//	localTable->GetKeyIDRange(nowMin, nowMax);

		//	Int64 minRange = re->get("MIN_KEY"); 
		//	Int64 maxRange = re->get("MAX_KEY");

		//	if ( (minRange>=nowMin && minRange<=nowMax)	// ��Сֵ��������
		//		|| (maxRange>=nowMin && maxRange<=nowMax) // ���ֵ��������
		//		)
		//	{
		//		ERROR_LOG("���ش���: %s>[%s]ָ������[%s]~[%s], �뵱ǰ�ڵ�[%s~%s]���ڽ���",
		//			ServerIPInfo::GetAddrInfo(node->mServerNetKey).c_str(),
		//			re->getIndexData().string(),
		//			STRING(minRange),
		//			STRING(maxRange), 
		//			STRING(nowMin),
		//			STRING(nowMax)
		//			);
		//		return false;
		//	}
		//}


		AutoDistributionData dist = mTableDistributionDataList.find(re->getIndexData().string());
		if (!dist)
		{
			dist = MEM_NEW TableKeyDistribution();
			dist->mbStringKey = re->get("IS_STRING_KEY");
			mTableDistributionDataList.insert(re->getIndexData().string(), dist);
		}
		else
		{
			if (dist->mbStringKey!=(bool)re->get("IS_STRING_KEY"))
			{
				ERROR_LOG("���ش���, ���KEY���Ͳ�һ��, ��ǰΪ [%s] ����", dist->mbStringKey ? "��ϣ�ַ���":"����" );
				return false;
			}
		}
		AutoData slotData = (DataStream*)re["SLOT_INFO"];
		bool bRe = dist->AppendNodeDistributionData(node, slotData);
		if (bRe)
		{
			bool bAllOK = false;
			AutoDBTable localTable = GetTable(re->getIndexData().string());
			if (localTable)
			{
				if (localTable->mKeyHashSlotList.empty())
					bAllOK = true;
				else
					bAllOK = dist->CheckAllHashSlot(localTable->mKeyHashSlotList);
			}
			else
			{
				EasyMap<short, bool> temp;
				bAllOK = dist->CheckAllHashSlot(temp);
			}
			NOTE_LOG("Succeed append note db [%s], Now hash slot %s", re->getIndexData().string().c_str(), bAllOK ? "������": "��ȱ");
			//LOG_YELLOW;
			//TABLE_LOG("�ɹ��趨��������Ϣ: NET: %s for table [%s] > [%s] ID Range: [%s]~[%s]", 
			//	ServerIPInfo::GetAddrInfo(node->mServerNetKey).c_str(),
			//	re->getIndexData().string().c_str(), 
			//	(dist->mbStringKey ? "�ַ���ϣKEY":"����KEY"),
			//	re->get("MIN_KEY").string().c_str(),
			//	re->get("MAX_KEY").string().c_str()
			//	);
			//LOG_WHITE;
		}
		else
		{
			ERROR_LOG("����:�趨��������Ϣʧ��: NET: %s for table [%s] > [%s] ", 
				ServerIPInfo::GetAddrInfo(node->mServerNetKey).c_str(),
				re->getIndexData().string().c_str(), 
				(dist->mbStringKey ? "�ַ���ϣKEY":"����KEY")
				)
		}

		// Ŀ��ͬ������ֶ���Ϣ
		AString szTable = re->getIndexData().string();
		ABaseTable t = GetTable(szTable.c_str());
		if (t)
		{
			int checkCode = re["FIELD_CODE"];//BaseFieldIndex::_generateCode(re->get("FIELD_DATA").string().c_str());
			if (!t->GetField()->CheckSame(checkCode))
				Log("����: [%s]����ַ���֤ʧ�� Now [%d] exist[%d]", szTable, checkCode, t->GetField()->GetCheckCode());
		}
		else
		{
			// ��������в�����, ����DB�в�����
			AString fieldData = re->get("FIELD_DATA").string();

			t = CreateTable(szTable.c_str());
			if (t->GetField()->FullFromString(fieldData))
				t->SetMainIndex(0, dist->mbStringKey);
			else
			{
				Log("����: �ָ����[%s]�ֶ���Ϣʧ��", szTable);
				mTableList.erase(szTable);
			}
		}
	}

	// ֪ͨ����Ӧ���ն�
	Hand<IOCPServerNet> net = GetDBServerNet();
	ConnectList &connectList = net->GetConnectList();
	if (!connectList.empty())
	{
		AutoEvent hNotifyEvt = net->GetEventCenter()->StartEvent("DB_NotifyDBDistribution");
		AutoTable t = GetAllDBTableDistributionData();
		hNotifyEvt->set("INFO_TABLE", t);

		for (size_t i=0; i<connectList.size(); ++i)
		{
			if (connectList[i])
			{
				connectList[i]->SendEvent(hNotifyEvt.getPtr());
			}
		}
	}
	return true;
}

bool MemoryDBNode::CheckKeyInThisNodeRange( const char *szTalbe, const char *recordKey )
{
	if (recordKey==NULL || strlen(recordKey)<=0)
		return true;

	AutoDBTable t = GetTable(szTalbe);
	if (t && t->mDataSource!=NULL)
		return t->InKeyRange(recordKey);
	return false;
}

ARecord MemoryDBNode::_CreateRecord( const char *szTable, const char *recordKey, DataStream *recordData, bool bReplace, eDBResultType &result )
{
	result = eNoneError;
	ARecord newRecord;
	// ������ڷֱ�Χ��, ����ʧ��
	ABaseTable t = GetTable(szTable);
	if (t)
	{
		newRecord = t->CreateRecord(recordKey, bReplace);

		if (newRecord)
		{
			recordData->seek(0);
			if (newRecord->restoreData(recordData))	
			{
				// not allow to modify main index field
				newRecord->_set(t->GetMainIndexCol(), recordKey);
				newRecord->FullAllUpdate(true);
				return newRecord;
			}
			else
			{
				result = eRecordRestoreDataFail;
				t->RemoveRecord(newRecord);
				newRecord.setNull();
				ERROR_LOG("�ָ���¼����ʧ��");
			}
		}
		else
			result = eRecordAlreadyExist;
	}
	else
	{
		result = eTableNoExist;
		ERROR_LOG("�ڵ��ڲ����ڷֱ�[%s]", szTable);
	}
	return newRecord;
}

eDBResultType MemoryDBNode::SaveRecord( const char *szIndex, const char *recordKey, DataStream *scrRecordData, bool bReplace, bool bUpdate, AutoEvent waitEvt )
{
	eDBResultType resultError = eNoneError;
	//-------------------------------------------------------------------------
	//NOTE: ��������ָ��KEY��һ���ļ�¼��Ӧ��KEY���и��»��滻, ������ƻ���������Ӧ��ϵ

	if (CheckKeyInThisNodeRange(szIndex, recordKey))
	{
		resultError = _SaveRecord(szIndex, recordKey, scrRecordData, bReplace, bUpdate);
	} 
	else
	{
		HandConnect conn = FindNodeByKey(szIndex, recordKey);
		if (conn) 
		{ 
			Hand<DD_RequestSaveRecord> evt = conn->StartEvent("DD_RequestSaveRecord");
			evt->mWaitEvent = waitEvt;
			evt->set("TABLE_INDEX", szIndex);
			evt->set("RECORD_KEY", recordKey);
			evt->set("RECORD_DATA", AutoData(scrRecordData));
			evt->set("REPLACE", bReplace);
			evt->set("UPDATE", bUpdate);
			evt->DoEvent();				
			return resultError;
		}
		else
		{
			waitEvt->set("ERROR", "��ӦKEY��Χ��DB�ڵ㲻����");
			resultError = eDBNodeNoExist;
		}
	}
	waitEvt->set("RESULT", resultError);
	waitEvt->OnEvent(waitEvt);
	waitEvt->Finish();
	return resultError;
}

eDBResultType MemoryDBNode::_SaveRecord( const char *szIndex, const char *recordKey, DataStream *scrRecordData, bool bReplace, bool bUpdate)
{
	eDBResultType resultError = eNoneError;

	ABaseTable baseTable = GetTable(szIndex);
	if (baseTable)
	{
		ARecord existRecord = baseTable->GetRecord(recordKey);
		if (existRecord)
		{
			if (bReplace || bUpdate)
			{							
				scrRecordData->seek(0);
				bool b = false;
				if (bUpdate)
				{
					b = existRecord->updateFromData(scrRecordData);
					// not allow to modify main index field
					existRecord->_set(baseTable->GetMainIndexCol(), recordKey);
				}
				else
				{
					b = existRecord->restoreData(scrRecordData);
					// not allow to modify main index field
					existRecord->_set(baseTable->GetMainIndexCol(), recordKey);
					if (b)
						existRecord->FullAllUpdate(true);
				}
				if (b)
				{
					
				}
				else
					resultError = eRecordRestoreDataFail;
			}
			else
				resultError = eRecordAlreadyExist;
		}
		else
		{
			if (bUpdate)
			{
				resultError = eRecordNoExist;
			}
			else
			{		
				Auto<MemoryDBRecord> resultRecord = _CreateRecord(szIndex, recordKey, scrRecordData, bReplace, resultError);
				if (!resultRecord)
				{
					resultError = eRecordCreateFail;
				}					
				else
				{
					// ֻ������������Ϊ�����¼�¼״̬
					resultRecord->SetNewInsert(true);
				}
			}
		}
	}
	else
		resultError = eTableNoExist;

	return resultError;
}

eDBResultType MemoryDBNode::_SaveTargetRecord( const char *szIndex, const char *recordKey, const char *subFieldTargetKey, DataStream *scrRecordData, eRecordOperateMode saveMode, ARecord &resultRecord, AString &errorInfo, int recordCountLimit)
{
	ABaseTable targetTable;
	ARecord mainRecord;
	ARecord targetRecord;
	AString mainRecordField;
	eDBResultType resultError = _FindTargetRecord(szIndex, recordKey, subFieldTargetKey, mainRecord, targetTable, targetRecord, mainRecordField, errorInfo);
	if (!targetTable
		|| (resultError!=eNoneError 
			&& (saveMode==eRecordSaveNoReplace && resultError!=eRecordNoExist) )
			)
	{
		TABLE_LOG(errorInfo.c_str());
		return resultError;
	}

    resultError = eNoneError;

	if (saveMode==eRecordDelete)
	{
		if (targetRecord && targetTable)
		{
			if (!targetTable->DeleteRecord(targetRecord))
				resultError = eRecordDeleteFail;

			if (mainRecord && mainRecord!=targetRecord)
				mainRecord->Update();
			resultRecord = targetRecord;
			return eNoneError;
		}
		
		return eRecordNoExist;		
	}
	
	if (scrRecordData!=NULL)
		scrRecordData->seek(0);

	if (eRecordGrowthCreate==saveMode)
	{
		if (recordCountLimit>0 && targetTable->GetRecordCount()>=recordCountLimit)
		{
			return eRecordCountOverLimit;
		}

		ARecord re = targetTable->GrowthNewRecord(scrRecordData);
		if (!re)
		{
			errorInfo.Format("��������¼ʧ�� [%s] at table [%s]", recordKey, szIndex);
			return eRecordGrowthCreateFail;
		}		
		resultRecord = re;
		
		return eNoneError;
	}
	else if (targetRecord)
	{
		AString key = targetRecord->getIndexData().string();
		if (eRecordReplaceSave==saveMode)
		{
			if (!targetRecord->restoreData(scrRecordData))
			{
				errorInfo.Format("�ָ���¼����ʧ�� [%s]", targetRecord->getIndexData().string().c_str());
				return eRecordRestoreDataFail;
			}
			//targetRecord->_set(targetRecord->GetTable()->GetMainIndexCol(), key.c_str());
			resultRecord = targetRecord;
			
		}
		else if (eRecordUpdate==saveMode || saveMode==eRecordUpdateOrInsert)
		{
			if (!targetRecord->updateFromData(scrRecordData))
			{
				errorInfo.Format("�ָ���¼����ʧ�� [%s]", targetRecord->getIndexData().string().c_str());
				return eRecordRestoreDataFail;
			}
			//targetRecord->_set(targetRecord->GetTable()->GetMainIndexCol(), key.c_str());
			resultRecord = targetRecord;
			
		}
		else
		{
			errorInfo.Format("��¼�Ѿ�����[%s]", key.c_str());
			resultError = eRecordAlreadyExist;
		}
	}
	else if (targetTable)
	{
		if (eRecordUpdate==saveMode)
		{
			errorInfo.Format("��Ҫ���µļ�¼������ [%s]", subFieldTargetKey);
			return eRecordNoExist;
		}

		if (recordCountLimit>0 && targetTable->GetRecordCount()>=recordCountLimit)
		{
			return eRecordCountOverLimit;
		}

		ARecord re = targetTable->NewRecord();
		bool bRestore = false;
		if (saveMode==eRecordUpdateOrInsert)
		{
			bRestore = re->updateFromData(scrRecordData);			
		}
		else
		{
			bRestore = re->restoreData(scrRecordData);
		}
		if (!bRestore)
		{
			errorInfo.Format("�ָ���¼����ʧ�� [%s]", subFieldTargetKey);
			return eRecordRestoreDataFail;
		}
		targetTable->AppendRecord(re, true);			
		resultRecord = re;
		
		Auto<MemoryDBRecord> r = re;
		if (r)
			r->SetNewInsert(true);
	}
	else
		resultError = eTableNoExist;

	if (resultError==eNoneError)
	{
		if (mainRecord)
			mainRecord->Update();
		else if (resultRecord)
			resultRecord->Update();
	}

	return resultError;
}

HandConnect MemoryDBNode::FindNodeByKey( const char *szTable, Int64 key )
{
	AutoDistributionData keyInfo = mTableDistributionDataList.find(szTable);
	if (keyInfo)
	{
		Hand<NetNodeConnectData> nodeData = keyInfo->FindNode(key);
		if (nodeData)
			return nodeData->mNodeConnect;
	}
	TABLE_LOG("WARN: [%s] ���ֲ����ڵ�ǰ�趨��Χ��, table [%s]", STRING(key), szTable);
	return HandConnect();
}

HandConnect MemoryDBNode::FindNodeByKey( const char *szTable, const char *szKey )
{
	AutoDistributionData keyInfo = mTableDistributionDataList.find(szTable);
	if (keyInfo)
	{
		Hand<NetNodeConnectData> nodeData = keyInfo->FindNode(szKey);
		if (nodeData)
			return nodeData->mNodeConnect;
	}
	TABLE_LOG("WARN: [%s] ���ֲ����ڵ�ǰ�趨��Χ��, table [%s]", szKey, szTable);
	return HandConnect();
}

eDBResultType MemoryDBNode::FindRecord( const char *szTable, const char *recordKey, AutoEvent waitEvt )
{
	eDBResultType resultError = eNoneError;
	ARecord resultRe;
	//static const type_info reTypeInfo = typeid(ARecord);
	if (CheckKeyInThisNodeRange(szTable, recordKey))
	{
		ABaseTable t = GetTable(szTable);
		if (t)
		{
			resultRe = t->GetRecord(recordKey);
			if (!resultRe)
				resultError = eRecordNoExist;
		}
		else
			resultError = eTableNoExist;
	}
	else
	{
		HandConnect nodeConnect = FindNodeByKey(szTable, recordKey );
		if (nodeConnect)
		{
			Hand<DD_RequestFindRecord> evt = nodeConnect->StartEvent("DD_RequestFindRecord");
			evt->set("TABLE_INDEX", szTable);
			evt->set("RECORD_KEY", recordKey);
			evt->mWaitEvent = waitEvt;
			evt->DoEvent();
			return eNoneError;
		}
		else
			resultError = eDBNodeNoExist;
	}
	waitEvt->set("RESULT", resultError);
	waitEvt->OnEvent(&resultRe, typeid(ARecord));	
	waitEvt->Finish();
	return resultError;
}

eDBResultType MemoryDBNode::DeleteRecord( const char *szTable, const char *recordKey, AutoEvent waitEvt )
{
	eDBResultType resultError = eNoneError;
	//static const type_info reTypeInfo = typeid(ARecord);
	ARecord resultRe;
	if (CheckKeyInThisNodeRange(szTable, recordKey))
	{
		ABaseTable t = GetTable(szTable);
		if (t)
		{
			resultRe = t->GetRecord(recordKey);
			if (resultRe)
			{
				if (!t->DeleteRecord(resultRe))
					resultError = eRecordDeleteFail;
			}
			else
				resultError = eRecordNoExist;
		}
		else
			resultError = eTableNoExist;
	}
	else
	{
		HandConnect nodeConnect = FindNodeByKey(szTable, recordKey );
		if (nodeConnect)
		{
			Hand<DD_RequestDeleteRecord> evt = nodeConnect->StartEvent("DD_RequestDeleteRecord");
			evt->set("TABLE_INDEX", szTable);
			evt->set("RECORD_KEY", recordKey);
			evt->mWaitEvent = waitEvt;
			evt->DoEvent();
			return eNoneError;
		}
		else
			resultError = eDBNodeNoExist;
	}
	waitEvt->OnEvent(&resultRe, typeid(ARecord));
	waitEvt->Finish();
	return resultError;
}

eDBResultType MemoryDBNode::InsertGrowthRecord( const char *szIndex, DataStream *scrRecordData, AutoEvent waitEvt )
{
	eDBResultType resultError = eNoneError;
	ARecord re;

	AutoDistributionData distData = mTableDistributionDataList.find(szIndex);
	if (distData)
	{
		int num = distData->mDistributionList.size();
		if (GetTable(szIndex))
			++num;
		int randNode = rand() % num;

		if (randNode>=distData->mDistributionList.size())
		{
			resultError = _InsertGrowthRecord(szIndex, scrRecordData, re);
		}
		else
		{
			HandConnect conn = distData->mDistributionList[randNode].mNode->mNodeConnect;
			if (conn)
			{
				Hand<DD_RequestCreateGrowthRecord> recordEvt = conn->StartEvent("DD_RequestCreateGrowthRecord");
				recordEvt->set("TABLE_INDEX", szIndex);
				if (scrRecordData!=NULL) 
				{
					AutoData reocordData = scrRecordData;
					recordEvt->set("RECORD_DATA", reocordData);
				}
				recordEvt->mWaitEvent = waitEvt;
				recordEvt->DoEvent();
				return resultError;
			}
			else
			{
				waitEvt->set("ERROR", "��ӦKEY��Χ��DB�ڵ㲻����");
				resultError = eDBNodeNoExist;
			}
		}			  
	}
	else
	{
		resultError = _InsertGrowthRecord(szIndex, scrRecordData, re);
	}
	waitEvt->set("RESULT", resultError);
	waitEvt->OnEvent(&re, typeid(re));	
	waitEvt->Finish();
	return resultError;
}

ABaseTable MemoryDBNode::NewCreateDBTable(const char *szTableType, const char *szTable, const char *fieldData, int checkCode, AString indexFieldsInfo, AutoNice extParam, AString &errorInfo )
{
	if (GetTable(szTable))
	{
		errorInfo.Format("Already Exist DB table [%s]", szTable);
		return ABaseTable();
	}
	AutoField field = MEM_NEW BaseFieldIndex();
	bool b =  field->FullFromString(fieldData);
	if (b)
	{		
		AutoTable resultTable = CreateNewDBTable(szTable, szTableType);		
		resultTable->InitField(field);

		// ������С�����KEY
		//if (extParam)
		//{
		//	Int64 minKey = extParam["MIN_KEY"];
		//	Int64 maxKey = extParam["MAX_KEY"];
		//	if (maxKey>0)
		//	{
		//		if (maxKey>minKey)
		//			resultTable->SetKeyIDRange(minKey, maxKey);
		//		else
		//			ERROR_LOG("[%s] �½������СID KEY[%s] ���ڻ���� ��� ID KEY [%s]", szTable, STRING(minKey), STRING(maxKey));
		//	}
		//}

		// ��������
		if (indexFieldsInfo!="")
		{
			Array<AString> indexFields;
			AString::Split(indexFieldsInfo.c_str(), indexFields, " ", 100);
			if (!indexFields.empty())
			{
				for (int i=0; i<indexFields.size(); ++i)
				{
					if (resultTable->SetIndexField(indexFields[i].c_str(), false))	// NOTE: �����������ֶ�, Ŀ��һ��Ϊ����, ���Ա���ʹ����������, ����ʹ�ù�ϣ
					{
						LOG_GREEN;
						TABLE_LOG("�ɹ����������ֶ�>[%s] at table [%s]", indexFields[i].c_str(), szTable);
					}
					else
					{
						ERROR_LOG("ERROR: ���������ֶ�ʧ��>[%s] at table [%s]", indexFields[i].c_str(), szTable);
					}
				}
				LOG_WHITE;
			}
		}

		if (resultTable->GetField()->CheckSame(checkCode))
		{				
			// ͬ��������Դ
			AString resultInfo = SaveDBTableFieldToDB(szTable, resultTable, NULL, extParam);

			if (!resultInfo.empty())
			{
				errorInfo = resultInfo;
				NOTE_LOG("ERROR: Create table to DB Fail >%s", errorInfo.c_str());
				return ABaseTable();
			}

			ARecord infoRe = mTableListDBTable->GetRecord(szTable);
			if (!infoRe)
			{
				errorInfo = "DB�б���Ϣ����";
				return ABaseTable();
			}

			AString fieldData = GetTableInfoString(infoRe); 
			InitReadyMemoryDBTable(szTable, fieldData);

			//??? NOTE: ����Ҫ֪ͨ�������ڵ�, Ŀǰδʵ��
			return GetTable(szTable);
		}
		else
		{
			errorInfo.Format("�ֶ�У��ʧ�� now [%d] of [%d]", resultTable->GetField()->GetCheckCode(), checkCode );
			ERROR_LOG(errorInfo.c_str());
		}
	}
	else
	{
		errorInfo.Format("�ֶ����ݻָ�ʧ�� [%s]", fieldData);
	}
	return ABaseTable();
}

//void MemoryDBNode::RunOperate( const char *operateType, const char *szTable, const char *recordKey, AutoNice &paramData, Hand<tClientEvent> requestEvent )
//{
//	AutoOpereate op = StartOperate(operateType);
//	if (!op)
//	{
//		requestEvent->GetResponseEvent()->set("ERROR", "����������");
//		requestEvent->GetResponseEvent()->set("RESULT", eDBOperateNoExist);	
//		requestEvent->Finish();
//		return;
//	}
//	op->OnStartRun(requestEvent->mNetConnect.getPtr());
//	eDBResultType result = (eDBResultType)op->Execute(this, szTable, recordKey, paramData);
//	if (result!=eWaitFinish)
//		op->Finish(requestEvent, result);
//}

AutoOpereate MemoryDBNode::RunOperate(tNetConnect *pRequestConnect, const char *operateType, const char *szTable, const char *recordKey, AutoNice &paramData, eDBResultType &result )
{
	AutoOpereate op = StartOperate(operateType);
	if (op)
	{	
		op->OnStartRun(pRequestConnect);
		result = (eDBResultType)op->Execute(this, szTable, recordKey, paramData);
		if (result!=eNoneError && result!=eWaitFinish)
			return AutoOpereate();
	}
	else
		result = eDBOperateNoExist;
	return op;
}

bool MemoryDBNode::RunOperate(tNetConnect *pRequestConnect, DBResultCallBack callBack, const char *operateType, const char *szTable, const char *recordKey, AutoNice &paramData )
{
	if (CheckKeyInThisNodeRange(szTable, recordKey))
	{
		AutoOpereate op = StartOperate(operateType);
		if (op)
		{					
			op->OnStartRun(pRequestConnect);
			eDBResultType result = (eDBResultType)op->Execute(this, szTable, recordKey, paramData, callBack);		
						
			return true;			
		}
		else
			ERROR_LOG("No exist operate [%s]", operateType);
	}
	else
	{
		HandConnect conn = FindNodeByKey(szTable, recordKey);
		if (conn)
		{
			AssertEx(0, "δʵ�ֿ�Node ִ�в���");
			NOTE_LOG("MemoryDBNode::RunOperate [%s] δʵ�ֿ�Node ִ�в���", operateType);
			callBack(NULL, false);
			//Hand<DD_RunDBOpereate> opEvent = conn->StartEvent("DD_RunDBOpereate");
			//opEvent->mResponseCallBack = callBack;
			//opEvent->set("TABLE_INDEX", szTable);
			//opEvent->set("RECORD_KEY", recordKey); 
			//opEvent->set("OPERATE_TYPE", operateType);
			//opEvent->set("PARAM", paramData);
			//opEvent->DoEvent();
			return false;
		}	
		else
		{
			if (recordKey!=NULL && strlen(recordKey)>0 && strcmp(recordKey, "0")!=0)
				WARN_LOG("No find node of table [%s], record [%s], then try run in current node", szTable, recordKey);

			AutoOpereate op = StartOperate(operateType);
			if (op)
			{					
				op->OnStartRun(pRequestConnect);
				eDBResultType result = (eDBResultType)op->Execute(this, szTable, recordKey, paramData);
				if (result!=eNoneError)
					op->mResultData->set("ERROR", (int)result);
				callBack(op->mResultData.getPtr(), result==eNoneError);
				return true;
			}
			else
				ERROR_LOG("No exist operate [%s]", operateType);
		}
	}		
	ERROR_LOG("Run DB operate [%s] fail", operateType);
	callBack(NULL, false);
	return false;
	return false;
}

ARecord MemoryDBNode::_FindRecord( const char *szTable, const char *recordKey )
{
	ARecord resultRe;
	ABaseTable t = GetTable(szTable);
	if (t)
	{
		resultRe = t->GetRecord(recordKey);	
	}
	
	return resultRe;
}

eDBResultType MemoryDBNode::_DeleteRecord( const char *szTable, const char *recordKey, ARecord &resultRe )
{
	eDBResultType resultError = eNoneError;

	ABaseTable t = GetTable(szTable);
	if (t)
	{
		resultRe = t->GetRecord(recordKey);
		if (resultRe)
		{
			if (!t->DeleteRecord(resultRe))
				resultError = eRecordDeleteFail;
		}
		else
			resultError = eRecordNoExist;
	}
	else
		resultError = eTableNoExist;

	return resultError;
}

eDBResultType MemoryDBNode::_DeleteTargetRecord( const char *szTable, const char *recordKey, const char *subFieldTargetKey, ARecord &resultRe, AString &errorInfo)
{
	ABaseTable targetTable;
	ARecord mainRecord;
	ARecord targetRecord;
	AString mainRecordField;
	eDBResultType resultError = _FindTargetRecord(szTable, recordKey, subFieldTargetKey, mainRecord, targetTable, targetRecord, mainRecordField, errorInfo);
	if (resultError!=eNoneError)
	{
		TABLE_LOG(errorInfo.c_str());
		return resultError;
	}

	if (targetRecord && targetTable)
	{
		if (!targetTable->DeleteRecord(targetRecord))
			resultError = eRecordDeleteFail;
		if (mainRecord!=targetRecord)
			mainRecord->Update();
	}

	return resultError;
}

eDBResultType MemoryDBNode::_InsertGrowthRecord( const char *szIndex, DataStream *scrRecordData, ARecord &re )
{
	eDBResultType resultError = eNoneError;

	ABaseTable t = GetTable(szIndex);
	if (t)
	{
		re = t->GrowthNewRecord(scrRecordData);
		if (!re)
			resultError = eRecordCreateFail;
	}
	else
		resultError = eTableNoExist;

	return resultError;
}

eDBResultType MemoryDBNode::_GetRecordData( const char *szIndex, const char *recordKey, const char *szTargetFieldInfo, Array<EasyString> &fieldList, AutoNice &resultData )
{
	ABaseTable table = GetTable(szIndex);
	if (table)
	{
		ARecord resultRecord;
		if (szTargetFieldInfo!=NULL)
		{
			ABaseTable targetTable;
			ARecord mainRecord;
			AString mainRecordField;
			AString errorInfo;
			eDBResultType result = _FindTargetRecord(szIndex, recordKey, szTargetFieldInfo, mainRecord, targetTable, resultRecord, mainRecordField, errorInfo );
			if (result!=eNoneError || !resultRecord)
			{
				if (result==eNoneError)
					result = eRecordNoExist;
				return result;
			}
		}
		else
			resultRecord = table->GetRecord(recordKey);

		if (resultRecord)
		{				
			if (!resultData)
				resultData = MEM_NEW NiceData();
			for (size_t i=0; i<fieldList.size(); ++i)
			{
				const char *szKey = fieldList[i].c_str();
				Data val = resultRecord->get(szKey);
				if (val.empty())
				{
					ERROR_LOG("ERROR no exist [%s] in table [%s]", szKey, szIndex);
					return eFieldNoExist;														
				}
				resultData->getOrCreate(szKey).set(val);
			}
			return eNoneError;				
		}
		else
			return eRecordNoExist;
	}
	else
		return eTableNoExist;


	return eRecordNoExist;
}

// bIncreaseOrDecrease ��ʾ�ڵ�ǰ��ֵ�������� limitValueInfo "* 0" "10000", ���ǰ���*�ո�, ���ʾֱ�ӱ��浽�ٽ�ֵ
eDBResultType MemoryDBNode::ModifyData( const char *szTable, const char *szRecordKey, const char *targetFieldInfo, const char *destField, const Data &limitValueInfo, eModifyDataMode modifyDataMode, const char *szDestValue, AString &resultValue, AString &originalValue, AString &errorInfo )
{
	AutoRecord mainRecord, targetRecord;
	AutoTable targetTable;
	AString mainRecordField;

	eDBResultType result = _FindTargetRecord(szTable, szRecordKey, targetFieldInfo, mainRecord, targetTable, targetRecord, mainRecordField, errorInfo);
	if (result!=eNoneError)
		return result;

	AString fieldString = destField;

	if (!targetRecord || !mainRecord)
	{
		errorInfo = "Ŀ���¼δ�ҵ�";
		return eRecordNoExist;
	}

	// ȡ��ǰֵ
	Data nowVal = targetRecord->get(fieldString.c_str());
	if (nowVal.empty())
	{
		errorInfo.Format("[%s]�ֶβ�����, at main table [%s]", fieldString.c_str(), szTable);
		return eFieldNoExist;
	}
	originalValue = nowVal.string();

	// ֱ���޸�
	if (modifyDataMode==eAlwaySetMode)
	{
		targetRecord->set(fieldString.c_str(), szDestValue);
		resultValue = szDestValue;

		mainRecord->Update();			
		return eNoneError;
	}

	FieldInfo info = targetRecord->getFieldInfo(fieldString.c_str());
	if (info==NULL)
	{
		errorInfo.Format("[%s]�����[%s]�ֶβ�����", szTable, destField);
		return eFieldNoExist;
	}

	if (info->getType()!=FIELD_FLOAT && info->getType()!=FIELD_INT)
	{
		errorInfo.Format("[%s]�����[%s]�ֶ����Ͳ�����ֵ����", szTable, destField);
		return eFieldTypeNotNumber;
	}

	bool bFloat = info->getType()==FIELD_FLOAT;

	switch (modifyDataMode)
	{
	case eIncreaseOrDecreaseMode:
		{
			// ����
			if (bFloat)
			{
				float now = nowVal;
				float fVal = StringTool::Float(szDestValue) + now;

				targetRecord->set(fieldString.c_str(), fVal);
				resultValue = STRING(fVal);
			}
			else
			{
				Int64 now = nowVal;
				Int64 val = TOINT64(szDestValue) + now;
				targetRecord->set(fieldString.c_str(), val);
				resultValue = STRING(val);
			}
			mainRecord->Update();			
			return eNoneError;
		}
		break;
	case eIncreaseOrDecreaseByLimitMode:
	case eIncreaseOrDecreaseToLimitMode:
		{
			// �����ٽ�����
			bool bAlwayToLimit = modifyDataMode==eIncreaseOrDecreaseToLimitMode;

			if (limitValueInfo.empty())
			{
				errorInfo = "�ٽ���ֵδ�趨";
				return eDBOperateParamNoSet;
			}

			if (bFloat)
			{
				WARN_LOG("[%s]�����[%s]�ֶ�������FLOAT����, ����Լ��ʱ������ֵֻ��Ϊ����,С��������", szTable, destField);
			}

			Int64 limit = limitValueInfo;
			Int64 destVal = TOINT64(szDestValue);
			Int64 now = nowVal;

			if (now==limit)
			{
				resultValue = nowVal.string();
				return eRecordDataNotEligible;
			}
			if (destVal==0)
			{
				resultValue = nowVal.string();
				errorInfo = "����ֵΪ��, ��������";
				return eDBOperateParamError;
			}

			now += destVal;

			if (destVal>0)
			{
				if (now<=limit)
				{						
					targetRecord->set(fieldString.c_str(), now);
					resultValue = STRING(now);
					mainRecord->Update();
					return eNoneError;
				}
			}
			else 
			{
				if (now>=limit)
				{
					targetRecord->set(fieldString.c_str(), now);
					resultValue = STRING(now);
					mainRecord->Update();
					return eNoneError;
				}
			}

			if (bAlwayToLimit)
			{
				targetRecord->set(fieldString.c_str(), limit);
				resultValue = STRING(limit);
				mainRecord->Update();
				return eNoneError;
			}
			else
			{
				resultValue = nowVal.string();
				errorInfo = "����ֵԽ�磬�޸�ʧ��";
				return eRecordDataNotEligible;
			}

		}
		break;
	default:
		errorInfo = "��֧�ֵ������޸ķ�ʽ>";
		errorInfo += (int)modifyDataMode;
	}

	return eUnknowError;
}

bool MemoryDBNode::ModifyData( DBResultCallBack callBack, const char *szTable, const char *szRecordKey, const char *targetFieldInfo, const char *destField, const Data &limitValue, eModifyDataMode modifyDataMode, const char *szDestValue, AString &resultValue, AString &originalValue, AString &errorInfo )
{
	if (CheckKeyInThisNodeRange(szTable, szRecordKey))
	{
		eDBResultType re = ModifyData(szTable, szRecordKey, targetFieldInfo, destField, limitValue, modifyDataMode, szDestValue, resultValue, originalValue, errorInfo);
		AutoNice resultData = MEM_NEW NiceData();
		if (re==eNoneError)
		{
			resultData["RESULT_VALUE"] = resultValue.c_str();
			resultData["ORIGINAL_VALUE"] = originalValue.c_str();
			callBack(resultData, true);
			return true;
		}
		else
		{
			resultData["ERROR"] = errorInfo.c_str();
			callBack(resultData, false);
		}

		return false;
	}
	else
	{
		AutoNice param = MEM_NEW NiceData();
		param->set("FIELD_KEY", targetFieldInfo);

		param->set("DEST_FIELD", destField);
		param->set("VALUE", szDestValue);

		param->set("MODE", (int)modifyDataMode);
		if (!limitValue.empty())
			param->set("LIMIT", limitValue.string());

		return RunOperate(NULL, callBack, "ModifyRecordData", szTable, szRecordKey, param);
	}

	return false;
}

eDBResultType MemoryDBNode::_FindTargetRecord( const char *szTable, const char *mainRecordKey, const char *subFieldKey, ARecord &mainRecord, ABaseTable &targetTable, ARecord &targetRecord, AString &mainRecordField, AString &errorInfo )
{
	eDBResultType resultType = eNoneError;

	mainRecordField.setNull();

	ABaseTable t = GetTable(szTable);
	if (t)
	{
		targetTable = t;

		ARecord resultRe = t->GetRecord(mainRecordKey);
		if (resultRe)
		{
			mainRecord = resultRe;

			AString fieldString = subFieldKey;
			while (true)
			{
				AString key1 = fieldString.SplitBlock('[', ']');
				if (key1!="")
				{					
					AString key = key1.SplitLeft(":");

					if (mainRecordField.empty())
						mainRecordField = key;

					ABaseTable tempTable = resultRe->GetFieldTable(key.c_str());
					if (tempTable)
					{
                        targetTable = tempTable;
						if (key1=="")
						{
                     	    resultRe.setNull();
							break;
						}

						resultRe = tempTable->GetRecord(key1.c_str());
						if (!resultRe)
						{
							errorInfo.Format("��¼[%s]�������ӱ��[%s]��", key1.c_str(), key.c_str());
							return eRecordNoExist;
						}						
					}
					else
					{
						errorInfo.Format("�ӱ��[%s]������", key.c_str());
						return eTableNoExist;
					}
				}
				else
					break;
			}

			if (mainRecordField.empty())
				mainRecordField = fieldString;

			targetRecord = resultRe;			

			return eNoneError;
		}
		else
			resultType = eRecordNoExist;
	}
	else
		resultType = eTableNoExist;

	return resultType;
}

void MemoryDBNode::Close()
{
	mDBNodeNet->StopNet();
	mTableDistributionDataList.clear(true);
	MemoryDB::Close();
}

int MemoryDBNode::TotalDataTaskCount()
{
	int count = 0;
	for (auto it=mTableList.begin(); it.have(); it.next())
	{
		AutoDBTable t = it.get();
		if (t && t->mDataSource!=NULL)
		{
			int taskCount = t->mDataSource->GetOperateCount()+t->mUpdateList.size();
			count += taskCount;
		}
	}
	return count;
}

void MemoryDBNode::TotalNetInfo(float &frames, int &revCount, int &sendCount, UInt64 &revSizeBySec, UInt64 &sendSizeBySec)
{
	Hand<TM_MemoryCheckEvent> evt = mMemoryCheckEvent;
	revCount = evt->mLastReceiveCount;
	sendCount = evt->mLastSendCount;
	revSizeBySec = evt->mLastReceiveSizeBySec;
	sendSizeBySec = evt->mLastSendSizeBySec;

	frames = evt->mLastFrameCount;
}

MemoryDBNode::~MemoryDBNode()
{
	mDBNodeNet->StopNet();
	mDBNodeNet._free();

	if (mMemoryCheckEvent)
		mMemoryCheckEvent._free();
}

ABaseTable MemoryDBNode::GetAllDBTableDistributionData()
{
	ABaseTable dataTable = tBaseTable::NewBaseTable();
	int i = 0;
	dataTable->SetField("INDEX", FIELD_STRING, i++);
	dataTable->SetField("IS_STRING_KEY", FIELD_BOOL, i++);	
	dataTable->SetField("INFO_TABLE", FIELD_TABLE, i++);
	dataTable->SetField("FIELD_DATA", FIELD_STRING, i++);

	ABaseTable info = tBaseTable::NewBaseTable();
    i=0;
	info->SetField("DBSERVER_IPPORT", FIELD_UINT64, i++);
	//info->SetField("MIN_KEY", FIELD_UINT64, i++);
	//info->SetField("MAX_KEY", FIELD_UINT64, i++);	
	info->SetField("IS_MAIN_NODE", FIELD_BOOL, i++);
	info->SetField("SLOT_INFO", FIELD_DATA, i++);

	dataTable->SetFieldTable("INFO_TABLE", info.getPtr());

	dataTable->SetMainIndex(0, false);

	//��ǰ�ڵ�ı��, ��ҪĿ����������ӵı����Ϣ
	//��Ϊ��ǰ�Ѿ������ű��أ����Բ���Ҫ���ṩ���صı����Ϣ��Ĭ���Ҳ�����Χ�ڵ�����ʱ�����ᵽ���ز���
	for (auto it=mTableList.begin(); it.have(); it.next())
	{
		AutoDBTable t = it.get();
		if (!t || t==mTableListDBTable)	//�ų��б���Ϣ���
			continue;
		
		ARecord r = dataTable->CreateRecord(t->GetTableName(), true);

		AutoIndex index = t->GetMainIndex();
		bool bStringKey = dynamic_cast<StringHashIndex*>(index.getPtr())!=NULL;
		r->set("IS_STRING_KEY", bStringKey);		
		r->set("FIELD_DATA", t->GetField()->ToString().c_str());

		AutoTable infoTable = r->GetFieldTable("INFO_TABLE");
		ARecord re = infoTable->CreateRecord( (Int64)ServerIPInfo::IP2Num(GetDBServerNet()->GetIp(), GetDBServerNet()->GetPort(), 0), true );

		//Int64 m1, m2;
		//t->GetKeyIDRange(m1, m2);
		//re->set("MIN_KEY", m1);
		//re->set("MAX_KEY", m2);
		re->set("IS_MAIN_NODE", true);
		AutoRecord infoRecord = mTableListDBTable->GetRecord(t->GetTableName());
		if (infoRecord)
		{
			AutoData slotData = (DataStream*)infoRecord["SLOT"];
			re["SLOT_INFO"] = slotData.getPtr();
		}
	}

	// ��������ڵ�ı����Ϣ
	for (auto it = mTableDistributionDataList.begin(); it.have(); it.next())
	{
		auto &kV = it.Value();
		const char *szTableIndex = kV.mKey.c_str();
		AutoDistributionData data = kV.mVal;

		AutoTable t = GetTable(szTableIndex);
		if (!t)
		{
			ERROR_LOG("��ǰ�ڵ�δ�ɹ����ر��>[%s]", szTableIndex);
			continue;
		}

		ARecord r = dataTable->GetRecord(szTableIndex);
		if (!r)
			r = dataTable->CreateRecord(szTableIndex, true);

		r->set("IS_STRING_KEY", data->mbStringKey);		
		r->set("FIELD_DATA", t->GetField()->ToString().c_str());

		AutoTable infoTable = r->GetFieldTable("INFO_TABLE");
		for (size_t n =0; n<data->mDistributionList.size(); ++n)
		{
			TableKeyDistribution::KeyRange &keyRange = data->mDistributionList[n];
			ARecord re = infoTable->CreateRecord( (Int64)(UInt64) keyRange.mNode->get("DBSERVER_IPPORT"), true );
			//re->set("MIN_KEY", keyRange.mMin);
			//re->set("MAX_KEY", keyRange.mMax);	
			re->set("IS_MAIN_NODE", false);
			re["SLOT_INFO"] = keyRange.mSlotData.getPtr();
		}		
	}

	return dataTable;
}

bool MemoryDBNode::RecordOperate( DBResultCallBack callBack, eRecordOperateMode mode, const char *szTable, const char *szRecordKey, AutoRecord dataRecord, const char *subTableFieldName, ... )
{
	AString info;
	va_list va;
	va_start(va, subTableFieldName);
	info.Format(va, subTableFieldName);

	AutoNice paramData = MEM_NEW NiceData();
	paramData->set("SAVE_MODE", (int)mode);
	paramData->set("TARGET_INFO", info.c_str());

	if (mode!=eRecordDelete && dataRecord)
	{
		AutoData recordData = MEM_NEW SaveRecordDataBuffer();
		bool b = false;
		if (mode==eRecordUpdate || mode==eRecordUpdateOrInsert)
		{
			dataRecord->Update();
			b = dataRecord->saveUpdateData(recordData.getPtr());
		}
		else
			b = dataRecord->saveData(recordData.getPtr());

		AssertEx(b, "��¼���浽����ʧ��");

		paramData->set("RECORD_DATA", recordData);
	}
	return RunOperate(NULL, callBack, "ModifySubRecord", szTable, szRecordKey, paramData);
}

bool MemoryDBNode::RecordOperate( DBResultCallBack callBack, eRecordOperateMode mode, const char *szTable, const char *szRecordKey, AutoRecord dataRecord, int recordCountLimit, const char *subTableFieldName, ... )
{
	AString info;
	va_list va;
	va_start(va, subTableFieldName);
	info.Format(va, subTableFieldName);


	AutoNice paramData = MEM_NEW NiceData();

	paramData->set("SAVE_MODE", (int)mode);
	paramData->set("TARGET_INFO", info.c_str());
	paramData->set("RECORD_COUNT_LIMIT", recordCountLimit);

	if (mode!=eRecordDelete && dataRecord)
	{
		AutoData recordData = MEM_NEW SaveRecordDataBuffer();
		bool b = false;
		if (mode==eRecordUpdate || mode==eRecordUpdateOrInsert)
		{
			dataRecord->Update();
			b = dataRecord->saveUpdateData(recordData.getPtr());
		}
		else
			b = dataRecord->saveData(recordData.getPtr());

		AssertEx(b, "��¼���浽����ʧ��");

		paramData->set("RECORD_DATA", recordData);
	}
	return RunOperate(NULL, callBack, "ModifySubRecord", szTable, szRecordKey, paramData);
}

eDBResultType MemoryDBNode::LoadRecordData( const char *szIndex, const char *szKey, AString fieldInfo, DBResultCallBack callBack )
{
	if (CheckKeyInThisNodeRange(szIndex, szKey))
	{
		AutoTable t = GetTable(szIndex);
		if (!t)
		{
			callBack(NULL, false);
			return eTableNoExist;
		}
		t->LoadRecordData(szKey, fieldInfo, callBack);
	}
	else
	{
		HandConnect conn = FindNodeByKey(szIndex, szKey);
		if (conn)
		{
			Hand<DD_LoadRecordData> evt = conn->StartEvent("DD_LoadRecordData");
			evt["TABLE"] = szIndex;
			evt["RECORD_KEY"] = szKey;
			evt["FIELD_LIST"] = fieldInfo.c_str();
			evt->mWaitCallBack = callBack;
			evt->DoEvent();			
		}
		else
		{
			ERROR_LOG("δ�ҵ� table [%s] record> [%s] DBnode δ�ҵ�", szIndex, szKey);
			callBack(NULL, false);
			return eDBNodeNoExist;
		}
	}
	return eNoneError;
}

eDBResultType MemoryDBNode::SaveRecordData( const char *szIndex, const char *szKey, AutoNice recordData, DBResultCallBack callBack )
{
	if (szKey==NULL || CheckKeyInThisNodeRange(szIndex, szKey))
	{
		AutoTable t = GetTable(szIndex);
		if (!t)
		{
			callBack(NULL, false);
			return eTableNoExist;
		}
		t->SaveRecordData(szKey, recordData, false, callBack);
	}
	else
	{
		HandConnect conn = FindNodeByKey(szIndex, szKey);
		if (conn)
		{
			Hand<DD_SaveRecordData> evt = conn->StartEvent("DD_SaveRecordData");
			evt["TABLE"] = szIndex;
			evt["RECORD_KEY"] = szKey;
			evt->set("RECORD_DATA", recordData);
			evt->mWaitCallBack = callBack;
			evt->DoEvent();			
		}
		else
		{
			ERROR_LOG("δ�ҵ� table [%s] record> [%s] DBnode δ�ҵ�", szIndex, szKey);
			callBack(NULL, false);
			return eDBNodeNoExist;
		}
	}
	return eNoneError;
}

void MemoryDBNode::_OnUserConnected( tNetConnect *pConnect )
{
	for (int i=0; i<mUserConnectList.size(); ++i)
	{
		if (!mUserConnectList[i])
		{
			pConnect->SetNetID(i);
			mUserConnectList[i] = pConnect;
			return;
		}
	}
	pConnect->SetNetID((int)mUserConnectList.size());
	mUserConnectList.push_back(pConnect);
}

HandConnect MemoryDBNode::GetUserConnect(int userNetID)
{
    if (userNetID>=0 && userNetID<mUserConnectList.size())
    {
        HandConnect conn = mUserConnectList[userNetID];
        if (conn && conn->GetNetID()==userNetID)
            return conn;
    }
    return HandConnect();
}
