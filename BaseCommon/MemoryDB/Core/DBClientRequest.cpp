#include "DBClientRequest.h"

#include "MemoryDBClient.h"
#include "DBManager.h"

#include "MeshedNodeNet.h"
#include "MemoryDBNode.h"
#include "DBClientForNode.h"
//bool DB_UpdateRecord_C::_DoEvent()
//{
//	if (!mResultRecord)
//	{
//		mResultType = eNoSetRequestRecord;
//		mErrorInfo = "δ������Ҫ���µļ�¼";
//		Finish();
//		return false;
//	}
//
//	if (!mResultRecord->NeedUpdate())
//	{
//		mResultType = eRecordNotNeedUpdate;
//		mErrorInfo = "��¼���ݲ���Ҫ����";
//		Finish();
//		return false;
//	}
//
//	mRecordKey = mResultRecord->getIndexData().string();	
	
//	return true;
//}

//bool DB_UpdateRecord_C::_Serialize( DataStream *destData )
//{
//	DB_Request::_Serialize(destData);
//
//	UpdateDataBuffer	mBufferData(128);
//	bool b = mResultRecord->saveUpdateData(&mBufferData);
//	if (!b)
//	{
//		mErrorInfo = ("�����޸�����ʧ��");
//		mResultType = eRecordSaveToUpdateDataFail;
//		Finish();
//		return false;
//	}
//	destData->write((int)mResultRecord->GetTable()->GetField()->GetCheckCode());
//
//	destData->writeData(&mBufferData);
//	mResultRecord->FullAllUpdate(false);
//	return true;
//}

void DB_LoadTableField_C::_OnResp( AutoEvent &respEvent )
{	
	int checkCode = respEvent->get("FIELD_CODE");
	AutoData fieldData = (DataStream*)respEvent->get("FIELD_DATA");
	if (fieldData)
	{
		ABaseTable table = GetDB()->CreateTable(mTableIndex.c_str());
		fieldData->seek(0);
		if (table->GetField()->restoreFromData(fieldData.getPtr()))
		{
			if (table->GetField()->GetCheckCode()==checkCode)
			{
				table->SetMainIndex(0, false);
				mResultTable = table;		

				// ���ؼ�¼����
				int count = respEvent["LOAD_COUNT"];
				if (count>0)
				{
					int selectFieldCount = respEvent["SELECT_FIELD_COUNT"];
					if (selectFieldCount>0)
					{
						AutoData fieldIndexData = (DataStream*)respEvent["SELECT_FILED_LIST"];
						Array<int> selectList;
						if (fieldIndexData && fieldIndexData->readArray(selectList))
						{
							EasyMap<int, FieldInfo> selectField;
							for (int i=0; i<selectList.size(); ++i)
							{
								int col = selectList[i];
								FieldInfo info = table->GetField()->getFieldInfo(col);
								if (info==NULL)
								{
									selectField.clear();
									break;
								}
								else
									selectField.insert(col, info);
							}
							if (selectFieldCount!=selectField.size())
							{
								ERROR_LOG("ѡ���ֶ�����ʧ��");
							}
							else
							{							
								AutoData d = (DataStream*)respEvent["SELECL_RECORD"];
								if (d)
								{
									d->seek(0);
									for (int i=0; i<count; ++i)
									{
										AutoRecord re = table->NewRecord();
										if (re->restoreSelectData(d.getPtr(), selectField))
										{
											//NOTE_LOG("%d, %s, %s", (int)re[0], re[selectList[0]].string().c_str(), re[selectList[1]].string().c_str());
											table->AppendRecord(re, true);
										}
										else
										{							
											ERROR_LOG("KEY�ֶβ�����");
											break;
										}
									}
								}
							}
						}
						else
						{
							AutoData d = (DataStream*)respEvent["RECORD_DATA"];
							if (d)
							{
								d->seek(0);
								if (!table->LoadFromData(d.getPtr()))
									ERROR_LOG("�ָ���¼����ʧ��");
							}						
						}
					}
				}
			}
			else
			{
				ERROR_LOG("[%s]����ֶμ���ʧ��", mTableIndex.c_str());
				mResultType = eFieldCheckFail;
			}
		}
		else
		{
			ERROR_LOG("[%s]����ֶλָ�ʧ��", mTableIndex.c_str());
			mResultType = eFieldCheckFail;
		}		
	}
	else
	{		
		TABLE_LOG("[%s]����ֶ�����δ�ظ�", mTableIndex.c_str());
		mResultType = eFieldNotSet;
	}

	DB_Request::_OnResp(respEvent);
}

//bool DB_SaveRecord_C::_Serialize( DataStream *destData )
//{
//	DB_Request::_Serialize(destData);
//
//	SaveRecordDataBuffer mBufferData(128);
//
//	mResultRecord->saveData(&mBufferData);
//	mBufferData.setDataSize(mBufferData.tell());
//	mResultRecord->FullAllUpdate(false);
//	return destData->writeData(&mBufferData);
//}

DBClient* DB_Request::GetDB()
{
	DBClient::DBNet *pNet = dynamic_cast<DBClient::DBNet*>(mNetConnect->GetNetHandle());
	if (pNet!=NULL)
		return pNet->mpDBClient;
	
	NodeRequestConnect *pNodeNet = dynamic_cast<NodeRequestConnect*>(mNetConnect.getPtr());
	if (pNodeNet!=NULL)
	{
		DBClient *p = dynamic_cast<DBClient*>(pNodeNet->mNetNodeConnectData->GetUseData().getPtr());
		if (p!=NULL)
			return p;
		//GSKEY key = pNodeNet->mNetNodeConnectData->get("DBSERVER_IPPORT");
		//DBMeshedNodeNet *p =dynamic_cast<DBMeshedNodeNet*>(pNodeNet->mNetNodeConnectData->mMeshedNet.getPtr());
		//Hand<DBNodeOperateManager> mgr = p->mpDBNode->GetDBOperateMgr();
		//Hand<DBClient> pClient = mgr->mDBClientList.find(key);
		//if (pClient)
		//	return pClient.getPtr();
	}
	AssertEx(0, "�߼���һ�㲻�����������������Ƕ�̬�仯�˱����Ϣ�ֲ�");
	return NULL;
}


void DB_FindRecord_C::_OnResp( AutoEvent &respEvent )
{
	//respEvent->Dump();

	AutoData mBufferData;
	respEvent->get("RECORD_DATA", mBufferData);
	if (mBufferData)
	{
		mBufferData->seek(0);
		ABaseTable table = GetDB()->GetTable(mTableIndex.c_str());
		if (!table)
		{
			mErrorInfo.Format("��ȡ���[%s]ʧ��, ��񲻴���", mTableIndex.c_str());
			mResultType = eTableNoExist;
			Finish();
			return;
		}
		mResultRecord = table->NewRecord(); // CreateRecord(mRecordKey.c_str(), true);
		if (!mResultRecord)
		{
			mResultType = eRecordCreateFail;
			mErrorInfo = "CODE ERROR: �����¼�¼ʱʧ��, �߼�����";

#if DEVELOP_MODE
#	if !FIELD_USE_HASH_INDEX
			for (size_t i=0; i<table->GetField()->getFieldMap().size(); ++i)
			{
				const FieldIndex::FieldMap::Value &v = table->GetField()->getFieldMap().getValue(i);
				AString s = v.mKey;
				TABLE_LOG("FIELD >[%s]", s.c_str());
			}
#	endif //!FIELD_USE_HASH_INDEX
#endif //DEVELOP_MODE
			Finish();
			return;
		}
		else if (!mResultRecord->restoreData(mBufferData.getPtr()))
		{
			mResultRecord->FullAllUpdate(false);
			mResultType = eRecordRestoreDataFail;
			mErrorInfo = "�ָ��ظ��ļ�¼����ʧ��";
			Finish();
			return;
		}
		mResultRecord->FullAllUpdate(false);
	}
	DB_Request::_OnResp(respEvent);
}

void _LogFieldInfo(AutoField f, int space)
{
	const FieldIndex::FieldVec &fieldList = f->getFieldInfoList();
	for (size_t i=0; i<fieldList.size(); ++i)
	{
		FieldInfo fieldInfo = fieldList[i];

		AString info;
		for (int i=0; i<space; ++i)
		{
			info += "	";
		}

		int len = fieldInfo->getMaxLength();		
		TABLE_LOG("%s[%s]>[%s] = [%d]", info.c_str(), fieldInfo->getName().c_str(), fieldInfo->getTypeString(), len);
		if (fieldInfo->getType()==FIELD_TABLE)
		{
			DBTableFieldInfo *field = dynamic_cast<DBTableFieldInfo*>(fieldInfo);
			AssertEx(field!=NULL, "Muse is DBTableFieldInfo");
			if(field->mSubTableField)
				_LogFieldInfo(field->mSubTableField, space+1);
		}
	}
}

bool DB_LoadAllTableField_C::_OnEvent( AutoEvent &hEvent )
{
	ABaseTable tableList;
	hEvent->get("TABLE_LIST", &tableList, typeid(tableList));
	mResultTable = tableList;
	if (mResultTable)
	{
		for (ARecordIt it = mResultTable->GetRecordIt(); *it; ++(*it))
		{
			ARecord r = *it;
			ABaseTable t = GetDB()->CreateTable( r->get("TABLE_NAME").string().c_str() );
			if (t->GetField()->FullFromString(r->get("TABLE_FIELD").string().c_str()))
			{
				t->SetMainIndex(0, false);
				TABLE_LOG("�ɹ���ȡ��� [%s]", r->get("TABLE_NAME").string().c_str());

				_LogFieldInfo(t->GetField(), 0);

				//FieldInfo info = t->GetField()->getFieldInfo("ID");
				bool b = t->GetField()->check() && t->GetField()->CheckSame((int)r["FIELD_CODE"]);

				AString szString = r->get("TABLE_FIELD").string();

				AString s = t->GetField()->ToString();

				//!!ERROR_LOG("***\r\n %s [%d] : string code[%d] filed string %d>%s", r->get("TABLE_NAME").c_str(), t->GetField()->GetCheckCode(), BaseFieldIndex::_generateCode(szString), strlen(szString), szString);

				TABLE_LOG("---------------------------------- %s [%d]------", b?"�ֶμ��ɹ�":"�ֶμ��ʧ��", t->GetField()->GetCheckCode());
			}
			else
			{
				TABLE_LOG("��ȡ��� [%s], �ָ��ֶ���Ϣʧ��", r->get("TABLE_NAME").string().c_str());
			}
		}
	}
	else
	{
		TABLE_LOG("ERROR: �б����ȡʧ��");
	}

	DB_Request::_OnResp(hEvent);

	return true;
}

bool DB_NotifyDBDistribution_R::_DoEvent()
{
	//Dump();
	AutoTable distInfoTable;
	get("INFO_TABLE", distInfoTable);
	if (distInfoTable)
	{
		DBClient::DBNet *pNet = dynamic_cast<DBClient::DBNet*>(mNetConnect->GetNetHandle());
		if (pNet!=NULL)
		{
			tDBManager *pMgr = pNet->mpDBClient->GetDBManager();
			if (pMgr!=NULL)
				pMgr->OnNotifyDistribution(distInfoTable);
		}
		else
			ERROR_LOG("[DB_NotifyDBDistribution]Ӧ����DB�ڵ����ӷ���");
	}
	return true;
}

bool DB_CreateTable_C::_DoEvent()
{
	if (!mResultTable )
	{
		mResultType = eNoSetNewTable;
		mErrorInfo = "��Ҫ�����½���� mResultTable, �ұ���ֶμ����ȷ ";
		Finish();
		return false;
	}

	if (!mResultTable->GetField()->check())
	{
		mResultType = eFieldCheckFail;
		mErrorInfo = "mResultTable ����ֶμ��ʧ�� ";
		Finish();
		return false;
	}

	mResultTable->GetField()->_updateInfo();
	mRecordKey = mResultTable->GetField()->ToString();
	if (mRecordKey.empty())
	{
		mErrorInfo.Format("[%s]�ֶ�����Ϊ�ַ���ʧ��", mTableIndex.c_str());
		Finish();
		return false;
	}

	int checkCode = mResultTable->GetField()->GetCheckCode();
	set("CHECK_CODE", checkCode);

	return DB_Request::_DoEvent();
}

void DB_CreateTable_C::_OnResp(AutoEvent &respEvent)
{
	int checkCode = respEvent->get("CHECK_CODE");
	if (!mResultTable->GetField()->CheckSame(checkCode))
	{
		mErrorInfo.Format("[%s]�����ı��������뱾�ر��ļ����벻��ͬ>now[%d], new[%d]", mResultTable->GetTableName(), mResultTable->GetField()->GetCheckCode(), checkCode);
		mResultType = eFieldCheckFail;
	}
	DB_Request::_OnResp(respEvent);
}
