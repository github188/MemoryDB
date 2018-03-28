
#include "DBServerOperate.h"
#include "MemoryDB.h"
#include <stdarg.h>
#include "IndexBaseTable.h"
#include "MemoryDBTable.h"
#include "MemoryDBNode.h"

#include "MemoryDBNode.h"
#include "DBNodeEvent.h"

MemoryDB* DB_Responce::GetDB()
{
	return _GetDB(mNetConnect);
}

MemoryDB* DB_Responce::_GetDB(HandConnect &conn)
{
	MemoryDB::DBServerNet *pNet = dynamic_cast<MemoryDB::DBServerNet*>(conn->GetNetHandle());
	if (pNet!=NULL)
		return pNet->mpServerDB;


	NodeServerNet *pNodeServerNet = dynamic_cast<NodeServerNet*>(conn->GetNetHandle());
	if (pNodeServerNet!=NULL)
	{
		DBMeshedNodeNet *pNodeNet = dynamic_cast<DBMeshedNodeNet*>(pNodeServerNet->mOwnerMeshNet);
		if (pNodeNet!=NULL)
			return pNodeNet->mpDBNode;
	}
	AssertEx(0, "获取 MemoryDB 失败, 不应该存在这种情况");
	return NULL;
}

void DB_Responce::ReplyError( const char *szInfo, ... )
{ 
	va_list argptr;
	va_start(argptr, szInfo);

	AString error;
	error.Format(argptr, szInfo);

	GetResponseEvent()->set("ERROR", error.c_str());
}

MemoryDBNode* DB_Responce::GetDBNode()
{	
	return dynamic_cast<MemoryDBNode*>(GetDB());
}

bool DB_CreateTable::_Restore( DataStream *scrData )
{
	DB_Responce::_Restore(scrData);
	return true;
}

bool DB_CreateTable::_DoEvent()
{
	if (mTableIndex=="")
	{
		ReplyError("表格名称为空, 请提供正确的表格名称");
		Finish();
		return true;
	}

	int checkCode = get("CHECK_CODE");
	if (checkCode==0)
	{
		ReplyError("字段校验码不正确 [%d]", checkCode);
		GetResponseEvent()->set("RESULT", false);
		Finish();
		return true;
	}
	AutoNice extParam = (tNiceData*)get("EXT_PARAM");
	AString indexFields = get("INDEX").string();
	AString errorInfo;
	ABaseTable t = GetDB()->NewCreateDBTable(get("TABLE_TYPE").string().c_str(), mTableIndex.c_str(), mRecordKey.c_str(), checkCode, indexFields, extParam, errorInfo );
	if (t)		
	{
		GetResponseEvent()->set("CHECK_CODE", t->GetField()->GetCheckCode());
		GetResponseEvent()->set("RESULT", eNoneError);
	}
	else
	{		
		ReplyError(errorInfo.c_str());
		GetResponseEvent()->set("RESULT", eCreateTableFail);
	}
	Finish();
	return true;
}
//
//bool DB_SaveRecord_S::_Restore( DataStream *scrData )
//{
//	DB_Responce::_Restore(scrData);
//
//	return scrData->readData(&mRecordData);
//}
//
//bool DB_SaveRecord_S::_DoEvent()
//{	
//	// 检查字段验证
//	int checkCode = get("CHECK_CODE");
//	ABaseTable t = GetDB()->GetTable(mTableIndex.c_str());
//	if (!t)
//	{
//		GetResponseEvent()->set("RESULT", eTableNoExist);
//		Finish();
//		return false;
//	}
//	if (!t->GetField()->CheckSame(checkCode))
//	{
//		ReplyError("字段校验失败 now [%d] of [%d]", checkCode, t->GetField()->GetCheckCode());
//		GetResponseEvent()->set("RESULT", eFieldCheckFail);
//		Finish();
//		return false;
//	}
//
//	bool bReplace = get("REPLACE");
//	bool bGrowthKey = get("GROWTH_KEY");
//	if (bGrowthKey)
//	{
//		Hand<MemoryDBNode> d = GetDB();
//		GetDB()->InsertGrowthRecord(mTableIndex.c_str(), &mRecordData, GetSelf());
//	}
//	else
//	{
//		if (mRecordKey=="")
//		{
//			ReplyError("ERROR: 未提供正确的记录KEY");
//			GetResponseEvent()->set("RESULT", eRecordKeyError);
//			Finish();
//			return true;
//		}
//
//		eDBResultType errorType = GetDB()->SaveRecord(mTableIndex.c_str(), mRecordKey.c_str(), &mRecordData, bReplace, false, GetSelf());
//	}
//	GetResponseEvent()->set("ERROR", get("ERROR").string());
//
//	return true;
//}
//
//bool DB_SaveRecord_S::_OnEvent( AutoEvent &hEvent )
//{
//	eDBResultType result = (eDBResultType)(int)hEvent->get("RESULT");
//	if (result==eNoneError)
//	{
//		bool bGrowthKey = get("GROWTH_KEY");
//		if (bGrowthKey)
//		{
//			AutoData data;
//			hEvent->get("REOCRD_DATA");
//			GetResponseEvent()->set("RECORD_DATA", data);
//			GetResponseEvent()->set("RECORD_KEY", hEvent->get("RECORD_KEY").string());
//		}		
//	}
//	else
//		GetResponseEvent()->set("ERROR", hEvent->get("ERROR").string());
//
//	GetResponseEvent()->set("RESULT", result);
//
//	Finish();
//	return true;
//}
//
//bool DB_SaveRecord_S::_OnEvent( void *pData, const type_info &dataType )
//{
//	if (dataType==typeid(ARecord))
//	{
//		ARecord re = *(ARecord*)pData;
//		if (re)
//		{
//			bool bGrowthKey = get("GROWTH_KEY");
//			if (bGrowthKey)
//			{
//				AutoData data = MEM_NEW SaveRecordDataBuffer();
//				re->saveData(data.getPtr());
//
//				GetResponseEvent()->set("RECORD_DATA", data);
//				GetResponseEvent()->set("RECORD_KEY", re->getIndexData().string());
//			}
//			GetResponseEvent()->set("RESULT", eNoneError);
//		}			
//		else
//		{
//			GetResponseEvent()->set("RESULT", get("RESULT").string());
//			GetResponseEvent()->set("ERROR", get("ERROR").string());
//		}
//		return true;
//	}
//	Finish();
//	return false;
//}



bool DB_FindRecord_S::_DoEvent()
{
	if (mRecordKey!="")
	{
		GetDB()->FindRecord(mTableIndex.c_str(), mRecordKey.c_str(), GetSelf());
		GetResponseEvent()->set("ERROR", get("ERROR").string());
		return true;
	}
	else
	{
		Log("ERROR: 未提供 记录KEY [RECORD_KEY]");
		GetResponseEvent()->set("RESULT", eRecordKeyError);
	}

	Finish();
	return true;
}

bool DB_FindRecord_S::_OnEvent( void *pData, const type_info &dataType )
{
	if (dataType==typeid(ARecord))
	{
		ARecord re = *(ARecord*)pData;
		if (re)
		{
			AutoData data = MEM_NEW SaveRecordDataBuffer(128);
			if (re->saveData(data.getPtr()))
			{
				GetResponseEvent()->set("RECORD_DATA", data);
				GetResponseEvent()->set("RESULT", eNoneError);
			}
			Finish();
			return true;
		}
		else
			GetResponseEvent()->set("RESULT", eRecordNoExist);
	}
	else
	{
		AssertEx(0, "不可使用其他类型参数");
		GetResponseEvent()->set("RESULT", eSourceCodeError);
	}
	Finish();

	return false;
}

bool DB_FindRecord_S::_OnEvent( AutoEvent &evt )
{
	eDBResultType errorType = (eDBResultType)(int)get("RESULT");
	if (errorType==eNoneError)
	{
		AutoData recordData;
		evt->get("RECORD_DATA", recordData);

		if (recordData)
		{
			GetResponseEvent()->set("RESULT", eNoneError);
			GetResponseEvent()->set("RECORD_DATA", recordData);				
			Finish();
			return true;
		}
	}
	GetResponseEvent()->set("RESULT", errorType);
	GetResponseEvent()->set("ERROR", evt->get("ERROR").string());
	Finish();
	return true;
}
//
//bool DB_UpdateRecord_S::_DoEvent()
//{
//	setState(STATE_ALREADY_RESPONSE, true);
//	// 检查字段验证
//	//int checkCode = get("CHECK_CODE");
//	ABaseTable t = GetDB()->GetTable(mTableIndex.c_str());
//	if (!t)
//	{
//		//GetResponseEvent()->set("RESULT", eTableNoExist);
//		Finish();
//		return false;
//	}
//	if (!t->GetField()->CheckSame(mCheckCode))
//	{
//		ERROR_LOG("字段校验失败 now [%d] of [%d]", mCheckCode, t->GetField()->GetCheckCode());
//		//GetResponseEvent()->set("RESULT", eFieldCheckFail);
//		Finish();
//		return false;
//	}
//
//	if (mRecordKey=="")
//	{
//		Log("ERROR: 未提供 记录KEY [RECORD_KEY]");
//		//GetResponseEvent()->set("RESULT", eRecordKeyError);
//	}
//	else
//	{
//		GetDB()->SaveRecord(mTableIndex.c_str(), mRecordKey.c_str(), &mUpdateData, false, true, GetSelf());
//		//GetResponseEvent()->set("ERROR", get("ERROR").string());
//		return true;
//	}
//
//	Finish();
//	return true;
//}
//
//bool DB_UpdateRecord_S::_OnEvent( AutoEvent &hEvent )
//{
//	//GetResponseEvent()->set("RESULT", (bool)hEvent->get("RESULT"));
//	//GetResponseEvent()->set("ERROR", hEvent->get("ERROR").string());
//	Finish();
//	return true;
//}
//
//bool DB_UpdateRecord_S::_Restore( DataStream *scrData )
//{
//	DB_Responce::_Restore(scrData);
//	if (!scrData->read(mCheckCode))
//		return false;
//	if (scrData->readData(&mUpdateData))
//		return true;
//
//	ReplyError("恢复更新数据失败");
//	return false;
//}
enum SELECT_TYPE
{
	SELECT_NONE,
	SELECT_Equal,
	SELECT_Greater,
	SELECT_Less,
	SELECT_GreaterEqual,
	SELECT_LessEqual,	
};
bool DB_LoadTableField_S::_DoEvent()
{
	ABaseTable table = GetDB()->GetTable(mTableIndex.c_str());
	if (table && table->GetField())
	{
		mFieldData->clear(false);
		table->GetField()->saveToData(mFieldData.getPtr());

		//AString fieldData = table->GetField()->ToString();
		GetResponseEvent()->set("FIELD_CODE", table->GetField()->GetCheckCode());
		GetResponseEvent()->set("FIELD_DATA", mFieldData);
		GetResponseEvent()->set("RESULT", eNoneError);

		int loadCount = get("LOAD_COUNT");
		if (loadCount>0)
		{
			AString whereField = get("WHERE_FILED");
			AString relational = get("RELA");
			AString whereValue = get("WHERE_VALUE");

			Int64 whereInt = TOINT64(whereValue.c_str());
			float whereFloat = TOFLOAT(whereValue.c_str());
			int whereFieldCol = -1;
			SELECT_TYPE selectType = SELECT_NONE;
			FieldInfo info = NULL;
			if (whereField!="" && whereValue!="" && relational!="")
			{
				//AString selectStr;
				info = table->GetField()->getFieldInfo(whereField.c_str());
				if (info!=NULL)
				{
					whereFieldCol = table->GetField()->getFieldCol(whereField.c_str());
					if (relational=="=")
					{
						selectType = SELECT_Equal;
					}
					else if (relational==">")
					{
						selectType = SELECT_Greater;
					}
					else if (relational==">=")
					{
						selectType = SELECT_GreaterEqual;
					}
					else if (relational=="<")
					{
						selectType = SELECT_Less;
					}
					else if (relational=="<=")
					{
						selectType = SELECT_LessEqual;
					}

					//	selectStr.Format(" WHERE `%s` %s '%s'", whereField.c_str(), relational.c_str(), whereValue);
					//else
					//	selectStr.Format(" WHERE `%s` %s %s", whereField.c_str(), relational.c_str(), whereValue);
				}
			}

			AString whereField2 = get("WHERE_FILED2");
			AString relational2 = get("RELA2");
			AString whereValue2 = get("WHERE_VALUE2");

			Int64 whereInt2 = TOINT64(whereValue2.c_str());
			float whereFloat2 = TOFLOAT(whereValue2.c_str());
			int whereFieldCol2 = -1;
			SELECT_TYPE selectType2 = SELECT_NONE;
			FieldInfo info2 = NULL;
			if (whereField2!="" && whereValue2!="" && relational2!="")
			{
				//AString selectStr;
				info2 = table->GetField()->getFieldInfo(whereField2.c_str());
				if (info2!=NULL)
				{
					whereFieldCol2 = table->GetField()->getFieldCol(whereField2.c_str());
					if (relational2=="=")
					{
						selectType2 = SELECT_Equal;
					}
					else if (relational2==">")
					{
						selectType2 = SELECT_Greater;
					}
					else if (relational2==">=")
					{
						selectType2 = SELECT_GreaterEqual;
					}
					else if (relational2=="<")
					{
						selectType2 = SELECT_Less;
					}
					else if (relational2=="<=")
					{
						selectType2 = SELECT_LessEqual;
					}
				}
			}

			// 选择字段
			AString selectString = get("SELECT_FIELD");
			static EasyMap<int, FieldInfo> selectList;
			selectList.clear(false);
			if (selectString!="" && selectString!="*")
			{
				Array<AString> list;
				AString::Split(selectString.c_str(), list, " ", 1000);
				AutoField f = table->GetField();

				for (int n=0; n<list.size(); ++n)
				{
					int col = f->getFieldCol(list[n].c_str());
					if (col>=0)
						selectList.insert(col, f->getFieldInfo(col));
					else
					{
						ReplyError("字段 [%s]不存在", list[n].c_str());
						GetResponseEvent()->set("RESULT", eFieldNoExist);
						Finish();
						return true;
					}
				}	
			}
			GetResponseEvent()["SELECT_FIELD_COUNT"] = (int)selectList.size();

			bool bSelect = !selectList.empty();
			Array<int> arr(selectList.size());
			if (bSelect)
			{
				for (int i=0; i<selectList.size(); ++i)
				{
					arr[i] = selectList.getKey(i);
				}
				static AutoData temp = MEM_NEW DataBuffer(64);
				temp->clear(false);
				temp->writeArray(arr);
				GetResponseEvent()["SELECT_FILED_LIST"] = temp.getPtr();
			}

			static AutoData reData = MEM_NEW SaveRecordDataBuffer(1024);
			reData->clear(false);
			static AutoData recordData = MEM_NEW SaveRecordDataBuffer(1024);
			recordData->clear(false);

			int num  = 0;						
			for (TableIt tIt(table); tIt; ++tIt)
			{
				AutoRecord re = tIt.getCurrRecord();
				if (!re)
					continue;

				// 使用第一个条件排除
				if (whereFieldCol>=0 && info!=NULL && selectType!=SELECT_NONE)
				{
					if (info->getType()==FIELD_STRING)
					{
						//NOTE_LOG("比较值 %s,  %s", re[whereFieldCol].string().c_str(), whereValue.c_str());
						switch (selectType)
						{
						case SELECT_Equal:
							if (re[whereFieldCol].string()!=whereValue)
								continue;
							break;
						case SELECT_Greater:
							if (re[whereFieldCol].string()<=whereValue)
								continue;
							break;
						case SELECT_GreaterEqual:
							if (re[whereFieldCol].string()<whereValue)
								continue;
							break;
						case SELECT_Less:
							if (re[whereFieldCol].string()>=whereValue)
								continue;
							break;
						case SELECT_LessEqual:
							if (re[whereFieldCol].string()>whereValue)
								continue;
							break;
						}							
					}
					else if (info->getType()==FIELD_FLOAT)
					{
						switch (selectType)
						{
						case SELECT_Equal:
							if ( abs( (float)re[whereFieldCol]-whereFloat )>0.0001f )
								continue;
							break;
						case SELECT_Greater:
							if ( (float)re[whereFieldCol]-whereFloat<=0.f )
								continue;
							break;
						case SELECT_GreaterEqual:
							if ( (float)re[whereFieldCol]-whereFloat<0.f )
								continue;
							break;
						case SELECT_Less:
							if ( (float)re[whereFieldCol]-whereFloat>=0.f )
								continue;
							break;
						case SELECT_LessEqual:
							if ( (float)re[whereFieldCol]-whereFloat>0.f )
								continue;
							break;
						}							
					}
					else
					{
						//NOTE_LOG("比较值 %d,  %d", (Int64)re[whereFieldCol], whereInt);
						switch (selectType)
						{
						case SELECT_Equal:
							if ((Int64)re[whereFieldCol]!=whereInt)
								continue;
							break;
						case SELECT_Greater:
							if ((Int64)re[whereFieldCol]<=whereInt)
								continue;
							break;
						case SELECT_GreaterEqual:
							if ((Int64)re[whereFieldCol]<whereInt)
								continue;
							break;
						case SELECT_Less:
							if ((Int64)re[whereFieldCol]>=whereInt)
								continue;
							break;
						case SELECT_LessEqual:
							if ((Int64)re[whereFieldCol]>whereInt)
								continue;
							break;
						}									
					}
				}

				// 使用第二个条件排除
				if (whereFieldCol2>=0 && info2!=NULL && selectType2!=SELECT_NONE)
				{
					if (info2->getType()==FIELD_STRING)
					{
						//NOTE_LOG("比较值 %s,  %s", re[whereFieldCol].string().c_str(), whereValue.c_str());
						switch (selectType2)
						{
						case SELECT_Equal:
							if (re[whereFieldCol2].string()!=whereValue2)
								continue;
							break;
						case SELECT_Greater:
							if (re[whereFieldCol2].string()<=whereValue2)
								continue;
							break;
						case SELECT_GreaterEqual:
							if (re[whereFieldCol2].string()<whereValue2)
								continue;
							break;
						case SELECT_Less:
							if (re[whereFieldCol2].string()>=whereValue2)
								continue;
							break;
						case SELECT_LessEqual:
							if (re[whereFieldCol2].string()>whereValue2)
								continue;
							break;
						}							
					}
					else if (info2->getType()==FIELD_FLOAT)
					{
						switch (selectType2)
						{
						case SELECT_Equal:
							if ( abs( (float)re[whereFieldCol2]-whereFloat2 )>0.0001f )
								continue;
							break;
						case SELECT_Greater:
							if ( (float)re[whereFieldCol2]-whereFloat2<=0.f )
								continue;
							break;
						case SELECT_GreaterEqual:
							if ( (float)re[whereFieldCol2]-whereFloat2<0.f )
								continue;
							break;
						case SELECT_Less:
							if ( (float)re[whereFieldCol2]-whereFloat2>=0.f )
								continue;
							break;
						case SELECT_LessEqual:
							if ( (float)re[whereFieldCol2]-whereFloat2>0.f )
								continue;
							break;
						}							
					}
					else
					{
						//NOTE_LOG("比较值 %d,  %d", (Int64)re[whereFieldCol], whereInt);
						switch (selectType2)
						{
						case SELECT_Equal:
							if ((Int64)re[whereFieldCol2]!=whereInt2)
								continue;
							break;
						case SELECT_Greater:
							if ((Int64)re[whereFieldCol2]<=whereInt2)
								continue;
							break;
						case SELECT_GreaterEqual:
							if ((Int64)re[whereFieldCol2]<whereInt2)
								continue;
							break;
						case SELECT_Less:
							if ((Int64)re[whereFieldCol2]>=whereInt2)
								continue;
							break;
						case SELECT_LessEqual:
							if ((Int64)re[whereFieldCol2]>whereInt2)
								continue;
							break;
						}									
					}
				}

				if (bSelect)
				{	
					//NOTE_LOG("%d, %s, %s", (int)re[0], re[arr[0]].string().c_str(), re[arr[1]].string().c_str());
					if (!re->SaveSelectData(reData.getPtr(), selectList))
					{
						ReplyError("保存记录失败 >[%s]", re[0].string().c_str());
						GetResponseEvent()->set("RESULT", eRecordSaveToUpdateDataFail);
						Finish();
						return true;								
					}			
				}
				else if (!re->saveData(recordData.getPtr()))
				{
					ERROR_LOG("Save record data fail >[%s]", re->getIndexData().string().c_str());
					break;
				}

				if (++num>=loadCount)
					break;
			}				

			GetResponseEvent()["LOAD_COUNT"] = num;
			GetResponseEvent()["RECORD_DATA"] = recordData.getPtr();
			GetResponseEvent()["SELECL_RECORD"] = reData.getPtr();
		}
	}
	else
	{
		ReplyError("表格不存在[%s]", mTableIndex.c_str());
		GetResponseEvent()->set("RESULT", eTableNoExist);
	}

	Finish();
	return true;
}

void DB_LoadTableField_S::SetRespData(AutoEvent &hResp)
{
	//!!! NOTE: 临时支持中转操作
	enum TARGET_NET_FLAG
	{
		eNetFlagNone = 0,
		eLOCAL,
		eDB_SERVER,			// 发送到 t_player 表格记录所在的DBNODE
		eGS_SERVER,			// GameServer服务器
		eCLIENT,			// 指定客户端
		eDB_NODE,			// DB节点
		eDB_DATANODE,		// 发送到 t_playerdata 表格记录所在的DBNODE
	};

	if (getTargetFlag()>0)
	{
		hResp->setTargetFlag((short)eCLIENT);
		hResp->setSendTarget(_getTarget());			
	}
}

bool DB_DeleteRecord_S::_DoEvent()
{
	if (mRecordKey!="")
	{
		GetDB()->DeleteRecord(mTableIndex.c_str(), mRecordKey.c_str(), GetSelf());
		GetResponseEvent()->set("ERROR", get("ERROR").string());
		return true;
	}
	else
	{
		ReplyError("未提供 RECORD_KEY");
		GetResponseEvent()->set("RESULT", eRecordKeyError);
	}

	Finish();
	return true;
}

bool DB_DeleteRecord_S::_OnEvent( AutoEvent &evt )
{
	eDBResultType errorType = (eDBResultType)(int)evt->get("RESULT");
	GetResponseEvent()->set("RESULT", errorType);
	GetResponseEvent()->set("ERROR", eRecordNoExist);
	Finish();
	return true;
}

bool DB_DeleteRecord_S::_OnEvent( void *pData, const type_info &dataType )
{
	if (dataType==typeid(ARecord))
	{
		ARecord re = *(ARecord*)pData;
		if (re)
		{			
			GetResponseEvent()->set("RESULT", eNoneError);			
			Finish();
			return true;
		}
		else
			GetResponseEvent()->set("RESULT", eRecordNoExist);
	}
	else
	{
		AssertEx(0, "不可使用其他类型参数");
		GetResponseEvent()->set("RESULT", eSourceCodeError);
	}
	Finish();
	return true;
}

bool DB_LoadAllTableField_S::_DoEvent()
{
	//??? NOTE_LOG("[%s] 开始处理...", GetEventName());
	mResultTable = tBaseTable::NewBaseTable();
	mResultTable->SetField("TABLE_NAME", FIELD_STRING, 0);
	mResultTable->SetField("TABLE_FIELD", FIELD_STRING, 1);
	mResultTable->SetField("FIELD_CODE", FIELD_INT, 2);
	TableHashMap &list = GetDB()->GetAllTableList();

	for (auto it=list.begin(); it.have(); it.next())
	{
		AutoDBTable table = it.get();
		if (!table || table==GetDB()->GetDBTableListTable())
			continue;

		ARecord re = mResultTable->CreateRecord(table->GetTableName(), true);

		AString fieldData = table->GetField()->ToString();

		re->set("TABLE_FIELD", fieldData.c_str());
		re->set("FIELD_CODE", table->GetField()->GetCheckCode());
	}

	GetResponseEvent()->set("TABLE_LIST", &mResultTable, typeid(mResultTable));
	Finish();
	return true;
}

bool DB_RunDBOperate_S::_DoEvent()
{
	AString szOp = get("OPERATE_TYPE");
	if (szOp=="")
	{
		GetResponseEvent()->set("ERROR", "未指定操作");
		GetResponseEvent()->set("RESULT", eDBOperateNoSet);
		Finish();
		return false;
	}

	AutoNice paramData;
	get("PARAM", &paramData, typeid(paramData));

	MemoryDBNode *node = GetDBNode();
	if (
		mTableIndex==""
		|| (node->CheckKeyInThisNodeRange(mTableIndex.c_str(), mRecordKey.c_str()))
		)
	{		
		node->RunOperate(mNetConnect.getPtr(), DBResultCallBack(&DB_RunDBOperate_S::OnCallFinish, this), szOp.c_str(), mTableIndex.c_str(), mRecordKey.c_str(), paramData);
		//Finish();
		return true;
	}
	else
	{
		HandConnect conn = node->FindNodeByKey(mTableIndex.c_str(), mRecordKey.c_str());
		if (conn)
		{
			Hand<DD_RunDBOpereate> opEvent = conn->StartEvent("DD_RunDBOpereate");
			opEvent->mWaitEvent = GetSelf();
			opEvent->set("TABLE_INDEX", mTableIndex.c_str());
			opEvent->set("RECORD_KEY", mRecordKey.c_str()); 
			opEvent->set("OPERATE_TYPE", szOp);
			opEvent->set("PARAM", paramData);
			opEvent->DoEvent();
		}
		else
		{
			node->RunOperate(mNetConnect.getPtr(), DBResultCallBack(&DB_RunDBOperate_S::OnCallFinish, this), szOp.c_str(), mTableIndex.c_str(), mRecordKey.c_str(), paramData);
			//Finish();
			return true;
			//GetResponseEvent()->set("ERROR", "对应KEY范围的DB节点不存在");
			//GetResponseEvent()->set("RESULT", eDDNodeEventNoExist);
		}
	}

	return true;
}

void DB_RunDBOperate_S::OnCallFinish(AutoNice resultData, bool bSu)
{
	GetResponseEvent()->set("RESULT_DATA", resultData);
	GetResponseEvent()->set("RESULT",  (int)(bSu? eNoneError: eDBOperateResultFail));
	Finish();
}

void DB_RunDBOperate_S::SetRespData(AutoEvent &hResp)
{
	//!!! NOTE: 临时支持中转操作
	enum TARGET_NET_FLAG
	{
		eNetFlagNone = 0,
		eLOCAL,
		eDB_SERVER,			// 发送到 t_player 表格记录所在的DBNODE
		eGS_SERVER,			// GameServer服务器
		eCLIENT,			// 指定客户端
		eDB_NODE,			// DB节点
		eDB_DATANODE,		// 发送到 t_playerdata 表格记录所在的DBNODE
	};

	if (getTargetFlag()>0)
	{
		hResp->setTargetFlag((short)eCLIENT);
		hResp->setSendTarget(_getTarget());			
	}
}

bool DB_RequestModifyData_S::_DoEvent()
{
	AString szFieldInfo = get("FIELD_KEY").string();
	
	AString destValue = get("VALUE");	

	AString destField = get("DEST_FIELD");

	Data limit = get("LIMIT");
	eModifyDataMode eMode = (eModifyDataMode)(int)get("MODE");

	Hand<MemoryDBNode> dbNode = GetDB();
	if (dbNode->CheckKeyInThisNodeRange(mTableIndex.c_str(), mRecordKey.c_str()))
	{
		AString resultValue;
		AString originalValue;
		eDBResultType result = dbNode->ModifyData(mTableIndex.c_str(), mRecordKey.c_str(), szFieldInfo.c_str(), destField.c_str(), limit, eMode, destValue.c_str(), resultValue, originalValue, mErrorInfo);
		GetResponseEvent()->set("RESULT", result);
		GetResponseEvent()->set("RESULT_VALUE", resultValue.c_str());
		GetResponseEvent()["ORIGINAL_VALUE"] = originalValue.c_str();
	}
	else
	{
		HandConnect conn = dbNode->FindNodeByKey(mTableIndex.c_str(), mRecordKey.c_str());
		if (conn)
		{
			Hand<DD_RequestModifyData> questEvt = conn->StartEvent("DD_RequestModifyData");
			questEvt->set("TABLE_INDEX", mTableIndex.c_str());
			questEvt->set("RECORD_KEY", mRecordKey.c_str()); 
			
			questEvt->set("MODE", (int)eMode);
			questEvt->set("FIELD_KEY", szFieldInfo);

			questEvt->set("DEST_FIELD", destField);
			questEvt->set("VALUE", destValue);
			
			if (!limit.empty())
				questEvt->set("LIMIT", limit.string());

			questEvt->mWaitEvent = GetSelf();
			questEvt->DoEvent();
			return true;
		}
		else
		{
			GetResponseEvent()->set("ERROR", "没有指定KEY的DB节点");
			GetResponseEvent()->set("RESULT", eDBNodeNoExist);								
		}
	}
	Finish();
	return true;
}
//-------------------------------------------------------------------------
bool DB_RequestRecordData_S::_DoEvent()
{
	const char *szTargetFieldInfo = NULL;

	AString targetFieldInfo = get("TARGET_INFO");
	if (!targetFieldInfo.empty())
		szTargetFieldInfo = targetFieldInfo.c_str();

	if (GetDBNode()->CheckKeyInThisNodeRange(mTableIndex.c_str(), mRecordKey.c_str()))
	{
		AutoNice resultData = MEM_NEW NiceData();
		eDBResultType result = GetDBNode()->_GetRecordData(mTableIndex.c_str(), mRecordKey.c_str(), szTargetFieldInfo, *mFieldList, resultData);
		if (result==eNoneError)
		{
			GetResponseEvent()->set("DATA_LIST", resultData);
		}
		GetResponseEvent()->set("RESULT", result);
	}
	else
	{
		HandConnect conn = GetDBNode()->FindNodeByKey(mTableIndex.c_str(), mRecordKey.c_str());
		if (conn)
		{
			Hand<DD_RequestGetRecordData> questEvt = conn->StartEvent("DD_RequestGetRecordData");
			questEvt->mFieldList = mFieldList;
			questEvt->set("TABLE_INDEX", mTableIndex.c_str());
			questEvt->set("RECORD_KEY", mRecordKey.c_str());
			if (szTargetFieldInfo!=NULL)
				questEvt->set("TARGET_INFO", szTargetFieldInfo);

			questEvt->mWaitEvent = GetSelf();
			questEvt->DoEvent();
			return true;
		}
		else
		{
			GetResponseEvent()->set("ERROR", "没有指定KEY的DB节点");
			GetResponseEvent()->set("RESULT", eDBNodeNoExist);								
		}
	}
	Finish();
	return true;
}

bool DB_RequestRecordData_S::_Restore( DataStream *scrData )
{
	if (DB_Responce::_Restore(scrData))
	{
		DSIZE count = 0;
		if (!scrData->read(count))
			return false;

		mFieldList = MEM_NEW StringArray();
		mFieldList->resize(count);

		for (DSIZE i=0; i<count; ++i)
		{
			if (!scrData->readString((*mFieldList)[i]))
				return false;
		}
		return true;
	}
	return false;
}

bool DB_RequestRecordData_S::_OnEvent( AutoEvent &hEvent )
{
	AutoNice resultData;
	hEvent->get("DATA_LIST", resultData);
	if (resultData)
		GetResponseEvent()->set("DATA_LIST", resultData);

	GetResponseEvent()->set("RESULT",  (int)hEvent->get("RESULT"));
	GetResponseEvent()->set("ERROR", hEvent->get("ERROR").string());
	Finish();

	return true;
}
//-------------------------------------------------------------------------

bool DB_RequestTableDistribution_S::_DoEvent()
{
	AutoTable infoTable = GetDBNode()->GetAllDBTableDistributionData();
	GetResponseEvent()->set("INFO_TABLE", infoTable);
	Finish();
	return true;
}
