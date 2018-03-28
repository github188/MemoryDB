/********************************************************************
created:	2011/02/18
created:	18:2:2011   15:01
filename: 	d:\Work\server_trunk\Server\SMU\SMU_LogicDataMsg.cpp
file path:	d:\Work\server_trunk\Server\SMU
file base:	SMU_LogicDataMsg
file ext:	cpp
author:		杨文鸽

purpose:	
*********************************************************************/

#include "SMU_LogicDataMsg.h"

#include "DBManager.h"
#include "ODBCInterface.h"

#include "ShareMemory.h"

#include "DataBase/NiceTable.h"
#include "FieldTypeDefine.h"
#include "EventCenter.h"
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

#define LOGIC_RECORD_DATA_SIZE	512			// 数据库数据字段的最大保存数据的长度
//-------------------------------------------------------------------------------
// 保存动态长度的数据, 原理把连续数据分成N多固定长度的数据,然后按编号,保存到N个记录中
// 表:t_logicdata,  key:key_code, 类型:bigint, [__int64]低两位分包编号, 高位48位保存key
//-------------------------------------------------------------------------------
class DBLogicData 
{

public:
	DBLogicData()		
	{

	}

	BOOL	Load(const char* tableName, __int64 key, DataBuffer &destData, size_t maxSize)
	{
		ODBCInterface *pDBInterface = g_pDBManager->GetInterface(CHARACTER_DATABASE);
		DB_QUERY& query = pDBInterface->GetQuery();
		BOOL re = FALSE;
		int i = 0;
		while (true)
		{
			__int64 id = key;
			id = (id<<16) + i++;
			query.Clear();
			query.Parse("select key_code,data from %s \
						where key_code = %I64d", tableName, id);
			pDBInterface->Execute();
			if (pDBInterface->GetAffectedRowCount()==1 && pDBInterface->Fetch())
			{
				INT errorCode;
				destData.resize(i*LOGIC_RECORD_DATA_SIZE);
				char *loadData = destData.data() + (i-1) * LOGIC_RECORD_DATA_SIZE;
				pDBInterface->GetField(2,loadData,LOGIC_RECORD_DATA_SIZE,errorCode);
				if (errorCode!=0)
				{
					destData.resize(0);
					return FALSE;
				}
				re = TRUE;
				loadData += LOGIC_RECORD_DATA_SIZE;
			}
			else
			{
				pDBInterface->Clear();
				break;
				//AssertEx(0,"查询数据库中逻辑数据错误!");
				//return FALSE;
			}
			pDBInterface->Clear();
		}
		return re; 
	}

	BOOL	Save(const char* tableName, __int64 key, CHAR* pData, size_t size)
	{
		ODBCInterface *pDBInterface = g_pDBManager->GetInterface(CHARACTER_DATABASE);
		DB_QUERY& query = pDBInterface->GetQuery();

		Delete(tableName,key);

		int i = 0;
		while (size>0)
		{
			__int64 id = key;
			id = (id<<16) + i++;
			DataBuffer temp(LOGIC_RECORD_DATA_SIZE*2+1);	
			size_t saveSize = (size<LOGIC_RECORD_DATA_SIZE ? size:LOGIC_RECORD_DATA_SIZE);
			Binary2String(pData,saveSize,temp.data());					
			size -= saveSize;
			pData += saveSize;
			query.Clear();
			query.Parse("insert into %s (key_code,data) \
						values(%I64d,\'%s\')", tableName, id, temp.data());
			bool bRe = (pDBInterface->Execute() &&
				pDBInterface->GetAffectedRowCount()==1);
			//AssertEx(bRe,"保存逻辑数据有误!"); 				
			if (!bRe)
				return FALSE;
		}
		return TRUE;
	}
	//-------------------------------------------------------------------------------

	BOOL	Delete(const char* tableName, __int64 key)
	{
		ODBCInterface *pDBInterface = g_pDBManager->GetInterface(CHARACTER_DATABASE);
		DB_QUERY& query = pDBInterface->GetQuery();
		int i = 0;
		while (true)
		{
			__int64 id = key;
			id = (id<<16) + i++;
			query.Clear();
			query.Parse("delete \
						from %s where key_code = %I64d", tableName, id);
			pDBInterface->Execute();
			if (pDBInterface->GetAffectedRowCount()!=1)
				break;
		}
		return TRUE;
	}
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
DBLogicData		g_dbLogic;

typedef std::map<__int64,DataBuffer>	DefineTableMap;
DefineTableMap g_logicDefineMap;

#define LOGIC_DATA_TABLE_NAME		"t_logicdata"
#define LOGIC_DEFINE_TABLE_NAME		"t_logicdefine"
//-------------------------------------------------------------------------------
// 接收保存
BOOL SMU_SaveLogicMsg::Run( VOID *userData )
{
	return TRUE;
}
//-------------------------------------------------------------------------------

BOOL SMU_SaveLogicMsg::ReadData( CHAR *data, size_t msgDataSize,VOID *userData )
{
	// 也可以在这儿直接进行保存到数据库
	g_dbLogic.Save(LOGIC_DATA_TABLE_NAME,GetGuid(),data,msgDataSize);
	return TRUE;
}
//-------------------------------------------------------------------------------
// 接收到调取请求

BOOL SMU_LoadLogicMsg_Request::Run( VOID *userData )
{
	// 从数据库中调取
	SMU_LoadLogicMsg_Result *loadResult = new SMU_LoadLogicMsg_Result();
	size_t loadSize = 0;

	g_dbLogic.Load(LOGIC_DATA_TABLE_NAME,GetGuid(),loadResult->mLogicData,SUM_DATA_MSG_MAX_SIZE);

	loadResult->SetGuid(GetGuid());
	loadResult->SetSceneID(GetSceneID());
	loadResult->SetObjID(GetObjID());
	((SMU_DataMsgManager*)userData)->AddSendMsg(loadResult);

	return TRUE;
}


//-------------------------------------------------------------------------------
// 回复, SMU中不做什么处理,不会有这个消息接收,只会发送
BOOL SMU_LoadLogicMsg_Result::Run( VOID *userData )
{
	return TRUE;
}

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

BOOL SMU_Sync_LogicData_Request::Run( VOID *userData )
{
	return TRUE;
}

BOOL SMU_LogicCommonData_Result::Run( VOID *userData )
{
	return TRUE;
}
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// 逻辑列表数据处理
// 列表表格数据体,保存在以key为零的记录中,因为零KEY的逻辑定义不允许存在,也不应该存在
BOOL _SaveLogicListTable(NiceTable &listTable)
{
	DataBuffer	tempData(512);
	size_t size = listTable.GetDataStream(tempData);
	if (size>0)
		return g_dbLogic.Save(LOGIC_DEFINE_TABLE_NAME,0,tempData.data(),size);
	else
	{
		AssertEx(0,"保存逻辑列表出错");
	}
	return FALSE;
}
BOOL SaveLogicKeyToListTable(__int64 key)
{
	DataBuffer	tempData(512);
	if (g_dbLogic.Load(LOGIC_DEFINE_TABLE_NAME,0,tempData,SUM_DATA_MSG_MAX_SIZE))
	{
		// 说明KEY列表有保存
		NiceTable listTabe;
		if (listTabe.LoadFromDataStream(&tempData))
		{
			if (!listTabe.ExistRecord((int)key))
			{
				listTabe.CreateRecord((int)key);
				return _SaveLogicListTable(listTabe);				
			}
			return TRUE;
		}
	}
	// 不存在, 或出错
	NiceTable	listTable;
	listTable.SetField("Index",FIELD_INT,0);
	listTable.CreateRecord((int)key);
	return _SaveLogicListTable(listTable);
}
BOOL LoadLogicKeyList( TableIt &listIt )
{
	DataBuffer	tempData(512);
	if (g_dbLogic.Load(LOGIC_DEFINE_TABLE_NAME,0,tempData,SUM_DATA_MSG_MAX_SIZE))
	{
		// 说明KEY列表有保存
		listIt.SetTable(new NiceTable());
		if (listIt.GetTable()->LoadFromDataStream(&tempData))
		{
			return TRUE;
		}
	}
	return FALSE;
}
BOOL DeleteLogicKey( __int64 key )
{
	DataBuffer	tempData(512);
	if (g_dbLogic.Load(LOGIC_DEFINE_TABLE_NAME,0,tempData,SUM_DATA_MSG_MAX_SIZE))
	{
		// 说明KEY列表有保存
		NiceTable listTabe;
		if (listTabe.LoadFromDataStream(&tempData))
		{
			listTabe.DeleRecord((int)key);
			return _SaveLogicListTable(listTabe);
		}
		return FALSE;
	}
	return TRUE;
}
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// 请求调取逻辑定义数据, 现在默认为全部调取
BOOL SMU_Load_LogicDefine_Request::Run( VOID *userData )
{
	if (g_logicDefineMap.empty())
	{
		TableIt	listIt;
		if (LoadLogicKeyList(listIt))
		{
			for (listIt.InitBegin(); listIt._Have(); listIt._Next())
			{
				__int64 key = (unsigned int)listIt.getIntKey();
				if (key!=0)
				{				
					// 从数据库中调取
					SMU_Load_LogicDefine_Result *loadResult = _LoadDataDefineFromDB(key);
					if ( loadResult )
					{
						g_logicDefineMap[key] = DataBuffer();
						g_logicDefineMap[key].resize(loadResult->mLogicData->size());
						g_logicDefineMap[key].write(loadResult->mLogicData->data(),loadResult->mLogicData->size());
						if (GetGuid()==0 || key==(__int64)GetGuid())
							((SMU_DataMsgManager*)userData)->AddSendMsg(loadResult);
						else
							delete loadResult;
					}
				}
			}
		}
	}
	else
	{
		if (GetGuid()==0)
		{
			// 全部发送		
			for (DefineTableMap::iterator it=g_logicDefineMap.begin(); it!=g_logicDefineMap.end(); ++it)
			{
				_SendDataDefine(it->first,it->second,userData);
			}
		}
		else
		{
			DefineTableMap::iterator it = g_logicDefineMap.find(GetGuid());
			if (it!=g_logicDefineMap.end())
			{
				_SendDataDefine(it->first,it->second,userData);
			}
		}
	}

	return TRUE;
}

SMU_Load_LogicDefine_Result* SMU_Load_LogicDefine_Request::_LoadDataDefineFromDB( __int64 key )
{
	// 从数据库中调取
	SMU_Load_LogicDefine_Result *loadResult = new SMU_Load_LogicDefine_Result();
	if ( g_dbLogic.Load(LOGIC_DEFINE_TABLE_NAME,key,loadResult->mLogicData,SUM_DATA_MSG_MAX_SIZE) )
	{
		loadResult->SetGuid((unsigned int)key);
		loadResult->SetSceneID(GetSceneID());
		loadResult->SetObjID(GetObjID());
	}
	else
	{
		delete loadResult;
		loadResult = NULL;
	}
	return loadResult;
}

void SMU_Load_LogicDefine_Request::_SendDataDefine( __int64 key, DataBuffer &data,VOID *useData )
{
	SMU_Load_LogicDefine_Result *loadResult = new SMU_Load_LogicDefine_Result();
	loadResult->mLogicData->resize(data.size());
	loadResult->mLogicData->write(data.data(),data.size());
	loadResult->SetGuid((unsigned int)key);
	loadResult->SetSceneID(GetSceneID());
	loadResult->SetObjID(GetObjID());
	((SMU_DataMsgManager*)useData)->AddSendMsg(loadResult);
}
//-----------------------------------------------------------------------------------

BOOL SMU_Load_LogicDefine_Result::Run( VOID *userData )
{
	return TRUE;
}
//-----------------------------------------------------------------------------------

BOOL SMU_Save_LogicDefine_Request::ReadData( CHAR *data, size_t msgDataSize,VOID *userData )
{
	if (GetGuid()==0)
		return FALSE;

	if (g_dbLogic.Save(LOGIC_DEFINE_TABLE_NAME,GetGuid(),data,msgDataSize))
	{
		SaveLogicKeyToListTable(GetGuid());
		g_logicDefineMap[GetGuid()] = DataBuffer();
		g_logicDefineMap[GetGuid()].resize(msgDataSize);
		g_logicDefineMap[GetGuid()].write(data,msgDataSize);
	}
	return TRUE;
}
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------

BOOL SMU_Close_Server_Request::Run( VOID *userData )
{
	return TRUE;
}

bool SMU_SendLogicEvent::SetSendEvent( Logic::tEvent *event )
{
	mLogicData->_free();
	mLogicData->writeStringBYTE(event->GetEventName().c_str());
	return event->_Serialize(mLogicData);
}
