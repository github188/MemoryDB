#include "DBNodeEvent.h"

#include "MeshedNodeNet.h"
#include "ServerIPInfo.h"
#include "TableUpdateLoader.h"
#include "DBTable.h"
#include "MySqlDBTool.h"

bool DD_NodifyNodeList::_DoEvent()
{
	WaitTime(10);

	Hand<MeshedNodeNet> hMeshNet = GetNet();
	AssertEx(hMeshNet, "Must is MeshNodeNet");

	NetNodeList &list = hMeshNet->GetNetNodeList();
	for (size_t i=0; i<list.size(); ++i)
	{
		GSKEY ssKey = list.getKey(i);
		set(STRING((int)i), ssKey);
	}
	set("COUNT", (int)list.size());
	return true;
}

bool DD_NodifyNodeList_R::_DoEvent()
{
	Hand<MeshedNodeNet> net = mNetConnect->GetNetHandle()->GetSelf();
	int count = get("COUNT");
	for (int i=0; i<count; ++i)
	{
		GSKEY ssKey = get(STRING(i));
		int p1 = 0, p2 = 0;
		AString ip = ServerIPInfo::Num2IP(ssKey, p1, p2);
		net->StartNet(ip.c_str(), p1);
	}
	Finish();

	return true;
}

bool DD_RequestSaveRecord_R::_DoEvent()
{
	HandDBNode dbNode = GetDBNode();
	AString tableIndex = get("TABLE_INDEX", "Must set TABLE_INDEX").string(); 
	AString recordKey = get("RECORD_KEY", "Must set TABLE_INDEX");
	bool bReplace = get("REPLACE");
	bool bUpdate = get("UPDATE");
	AutoData recordData;
	if (!get("RECORD_DATA", recordData) || !recordData)
	{
		GetResponseEvent()->set("RESULT", eRecordRestoreDataFail);
	}
	else
	{		
		eDBResultType result = dbNode->_SaveRecord(tableIndex.c_str(), recordKey.c_str(), recordData.getPtr(), bReplace, bUpdate);
		// 是否在本节点范围内
		if (result!=eNoneError)
		{
			GetResponseEvent()->set("ERROR", "记录KEY不在存在节点内");
			// 目前不再让目标节点检测
		}			
		GetResponseEvent()->set("RESULT", result);
	}
	Finish();
	return true;
}

bool DD_RequestFindRecord_R::_DoEvent()
{
	HandDBNode dbNode = GetDBNode();
	AString tableIndex = get("TABLE_INDEX", "Must set TABLE_INDEX").string(); 
	AString recordKey = get("RECORD_KEY", "Must set TABLE_INDEX");

	
	if (dbNode->CheckKeyInThisNodeRange(tableIndex.c_str(), recordKey.c_str()))
	{
		ARecord resultRe = dbNode->_FindRecord(tableIndex.c_str(), recordKey.c_str());
		if (resultRe)
		{
			AutoData data = MEM_NEW SaveRecordDataBuffer(128);
			if (resultRe->saveData(data.getPtr()))
			{
				GetResponseEvent()->set("RECORD_DATA", data);
				GetResponseEvent()->set("RESULT", eNoneError);
			}

		}
		else
			GetResponseEvent()->set("RESULT", eRecordNoExist);

		Finish();
		return true;

	}	
	else
	{
		// 目前不再让目标节点检测
		GetResponseEvent()->set("ERROR", "不在指定DB节点范围内");
		GetResponseEvent()->set("RESULT", eRecordNoExistInNode);
	}	
	Finish();
	return false;
}


bool DD_RequestDeleteRecord_R::_DoEvent()
{
	HandDBNode dbNode = GetDBNode();
	AString tableIndex = get("TABLE_INDEX", "Must set TABLE_INDEX").string(); 
	AString recordKey = get("RECORD_KEY", "Must set TABLE_INDEX");

	if (dbNode->CheckKeyInThisNodeRange(tableIndex.c_str(), recordKey.c_str()))
	{
		ARecord deleteRecord;
		eDBResultType result = dbNode->_DeleteRecord(tableIndex.c_str(), recordKey.c_str(), deleteRecord);
		GetResponseEvent()->set("RESULT", result);
	}	
	else
	{
		// 目前不再让目标节点再次检测发送
		GetResponseEvent()->set("RESULT", eRecordNoExist);
	}	
	Finish();
	return false;
}


//-------------------------------------------------------------------------
bool DD_RequestCreateGrowthRecord_R::_DoEvent()
{
	HandDBNode dbNode = GetDBNode();
	AString tableIndex = get("TABLE_INDEX", "Must set TABLE_INDEX").string(); 

	AutoData recordScrData;
	get("RECORD_DATA", recordScrData);

	ARecord newRecord;
	eDBResultType result = GetDBNode()->_InsertGrowthRecord(tableIndex.c_str(), recordScrData.getPtr(), newRecord);

	if (newRecord && result==eNoneError)
	{
		AutoData recordData = MEM_NEW SaveRecordDataBuffer();
		newRecord->saveData(recordData.getPtr());
		GetResponseEvent()->set("RECORD_DATA", recordData);
		GetResponseEvent()->set("RESULT", result);				
		GetResponseEvent()->set("RECORD_KEY", newRecord->getIndexData().string());
	}
	else
		GetResponseEvent()->set("RESULT", eRecordCreateFail);

	Finish();
	return true;
}
//-------------------------------------------------------------------------


bool DD_RunDBOpereate_R::_DoEvent()
{
	AString tableIndex = get("TABLE_INDEX").string(); 
	AString recordKey = get("RECORD_KEY");

	//if (!GetDBNode()->CheckKeyInThisNodeRange(tableIndex, recordKey.c_str()))
	//{
	//	GetResponseEvent()->set("RESULT", eRecordNotInRange);
	//	GetResponseEvent()->set("ERROR", "操作不在节点范围内");
	//	Finish();
	//	return false;
	//}

	AString optype = get("OPERATE_TYPE", "Must set OPERATE_TYPE");
	AutoNice d;
	get("PARAM", &d, typeid(d));

	GetDBNode()->RunOperate(mNetConnect.getPtr(), DBResultCallBack(&DD_RunDBOpereate_R::OnCallFinish, this), optype.c_str(), tableIndex.c_str(), recordKey.c_str(), d);
	//Finish(); 在操作中必定会调用完成

	return true;
}

//-------------------------------------------------------------------------

bool DD_RequestGetRecordData_R::_DoEvent()
{
	AString tableIndex = get("TABLE_INDEX").string(); 
	AString recordKey = get("RECORD_KEY");

	const char *szTargetRecordInfo = NULL;

	AString targetReocrdInfo = get("TARGET_INFO");
	if (!targetReocrdInfo.empty())
		szTargetRecordInfo = targetReocrdInfo.c_str();

	AutoNice resultData = MEM_NEW NiceData();
	eDBResultType result = GetDBNode()->_GetRecordData(tableIndex.c_str(), recordKey.c_str(), szTargetRecordInfo, *mFieldList, resultData);

	if (result==eNoneError)
	{
		GetResponseEvent()->set("DATA_LIST", resultData);
	}
	GetResponseEvent()->set("RESULT", result);
	GetResponseEvent()->set("RECORD_KEY", recordKey);
	Finish();

	return true;
}

bool DD_RequestCreateNodeTable_R::_DoEvent()
{
	AString tableName = get("TABLE");
	Auto<DBTable> listTable = GetDBNode()->GetDBTableListTable();
	AutoTable t = GetDBNode()->GetTable(tableName.c_str());
	if (t || listTable->GetRecord(tableName.c_str()))
	{
		ERROR_LOG("%s already exist", tableName.c_str());
		//GetResponseEvent()->set("RESULT", (int)eCreateTableFail);
	}
	else
	{
		AString tableInfoString = get("TABLE_INFO");
		AutoData tableFieldData = (DataStream*)get("TABLE_FIELD");
		AutoData slotData = (DataStream*)get("SLOT_INFO");
		if (tableFieldData && slotData)
		{			
			AutoField f = MEM_NEW FieldIndex(NULL, 0);
			tableFieldData->seek(0);
			if (f->restoreFromData(tableFieldData.getPtr()))
			{
				Auto<MySqlDBTool> tool = listTable->GetDBTool();
				if (tool->CreateDBTable(tableName.c_str(), f, tableInfoString.c_str()) )
				{
					ARecord re = listTable->CreateRecord(tableName.c_str(), false);
					re["DATA"] = tableInfoString;
					re["SLOT"] = slotData.getPtr();
					re["DB_INFO"] = get("ALL_DB").string();
					listTable->SaveTable(true);
					GetDBNode()->InitReadyMemoryDBTable(tableName.c_str(), tableInfoString);
					GetResponseEvent()["RESULT"] = eNoneError;
					Finish();
					return true;
				}
				else
					ERROR_LOG("Create DB table fail >%s", tableName.c_str());
			}
			else
				ERROR_LOG("Field restore fail");
		}
		else
			ERROR_LOG("未提供 字段数据");
	}
	GetResponseEvent()["RESULT"] = eCreateTableFail;
	Finish();
	return true;
}
