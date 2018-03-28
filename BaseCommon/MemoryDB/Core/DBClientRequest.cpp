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
//		mErrorInfo = "未设置需要更新的记录";
//		Finish();
//		return false;
//	}
//
//	if (!mResultRecord->NeedUpdate())
//	{
//		mResultType = eRecordNotNeedUpdate;
//		mErrorInfo = "记录数据不需要更新";
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
//		mErrorInfo = ("序列修改数据失败");
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

				// 加载记录数据
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
								ERROR_LOG("选择字段整理失败");
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
											ERROR_LOG("KEY字段不存在");
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
									ERROR_LOG("恢复记录数据失败");
							}						
						}
					}
				}
			}
			else
			{
				ERROR_LOG("[%s]表格字段检验失败", mTableIndex.c_str());
				mResultType = eFieldCheckFail;
			}
		}
		else
		{
			ERROR_LOG("[%s]表格字段恢复失败", mTableIndex.c_str());
			mResultType = eFieldCheckFail;
		}		
	}
	else
	{		
		TABLE_LOG("[%s]表格字段数据未回复", mTableIndex.c_str());
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
	AssertEx(0, "逻辑上一般不会出现这种情况，除非动态变化了表格信息分布");
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
			mErrorInfo.Format("获取表格[%s]失败, 表格不存在", mTableIndex.c_str());
			mResultType = eTableNoExist;
			Finish();
			return;
		}
		mResultRecord = table->NewRecord(); // CreateRecord(mRecordKey.c_str(), true);
		if (!mResultRecord)
		{
			mResultType = eRecordCreateFail;
			mErrorInfo = "CODE ERROR: 创建新记录时失败, 逻辑错误";

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
			mErrorInfo = "恢复回复的记录数据失败";
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
				TABLE_LOG("成功调取表格 [%s]", r->get("TABLE_NAME").string().c_str());

				_LogFieldInfo(t->GetField(), 0);

				//FieldInfo info = t->GetField()->getFieldInfo("ID");
				bool b = t->GetField()->check() && t->GetField()->CheckSame((int)r["FIELD_CODE"]);

				AString szString = r->get("TABLE_FIELD").string();

				AString s = t->GetField()->ToString();

				//!!ERROR_LOG("***\r\n %s [%d] : string code[%d] filed string %d>%s", r->get("TABLE_NAME").c_str(), t->GetField()->GetCheckCode(), BaseFieldIndex::_generateCode(szString), strlen(szString), szString);

				TABLE_LOG("---------------------------------- %s [%d]------", b?"字段检查成功":"字段检查失败", t->GetField()->GetCheckCode());
			}
			else
			{
				TABLE_LOG("调取表格 [%s], 恢复字段信息失败", r->get("TABLE_NAME").string().c_str());
			}
		}
	}
	else
	{
		TABLE_LOG("ERROR: 列表表格获取失败");
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
			ERROR_LOG("[DB_NotifyDBDistribution]应该由DB节点连接发送");
	}
	return true;
}

bool DB_CreateTable_C::_DoEvent()
{
	if (!mResultTable )
	{
		mResultType = eNoSetNewTable;
		mErrorInfo = "需要设置新建表格 mResultTable, 且表格字段检查正确 ";
		Finish();
		return false;
	}

	if (!mResultTable->GetField()->check())
	{
		mResultType = eFieldCheckFail;
		mErrorInfo = "mResultTable 表格字段检查失败 ";
		Finish();
		return false;
	}

	mResultTable->GetField()->_updateInfo();
	mRecordKey = mResultTable->GetField()->ToString();
	if (mRecordKey.empty())
	{
		mErrorInfo.Format("[%s]字段序列为字符串失败", mTableIndex.c_str());
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
		mErrorInfo.Format("[%s]创建的表格检验码与本地表格的检验码不相同>now[%d], new[%d]", mResultTable->GetTableName(), mResultTable->GetField()->GetCheckCode(), checkCode);
		mResultType = eFieldCheckFail;
	}
	DB_Request::_OnResp(respEvent);
}
