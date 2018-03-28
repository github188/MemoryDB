/********************************************************************
created:	2011/02/18
created:	18:2:2011   15:01
filename: 	d:\Work\server_trunk\Server\SMU\SMU_LogicDataMsg.cpp
file path:	d:\Work\server_trunk\Server\SMU
file base:	SMU_LogicDataMsg
file ext:	cpp
author:		���ĸ�

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

#define LOGIC_RECORD_DATA_SIZE	512			// ���ݿ������ֶε���󱣴����ݵĳ���
//-------------------------------------------------------------------------------
// ���涯̬���ȵ�����, ԭ����������ݷֳ�N��̶����ȵ�����,Ȼ�󰴱��,���浽N����¼��
// ��:t_logicdata,  key:key_code, ����:bigint, [__int64]����λ�ְ����, ��λ48λ����key
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
				//AssertEx(0,"��ѯ���ݿ����߼����ݴ���!");
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
			//AssertEx(bRe,"�����߼���������!"); 				
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
// ���ձ���
BOOL SMU_SaveLogicMsg::Run( VOID *userData )
{
	return TRUE;
}
//-------------------------------------------------------------------------------

BOOL SMU_SaveLogicMsg::ReadData( CHAR *data, size_t msgDataSize,VOID *userData )
{
	// Ҳ���������ֱ�ӽ��б��浽���ݿ�
	g_dbLogic.Save(LOGIC_DATA_TABLE_NAME,GetGuid(),data,msgDataSize);
	return TRUE;
}
//-------------------------------------------------------------------------------
// ���յ���ȡ����

BOOL SMU_LoadLogicMsg_Request::Run( VOID *userData )
{
	// �����ݿ��е�ȡ
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
// �ظ�, SMU�в���ʲô����,�����������Ϣ����,ֻ�ᷢ��
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
// �߼��б����ݴ���
// �б���������,��������keyΪ��ļ�¼��,��Ϊ��KEY���߼����岻�������,Ҳ��Ӧ�ô���
BOOL _SaveLogicListTable(NiceTable &listTable)
{
	DataBuffer	tempData(512);
	size_t size = listTable.GetDataStream(tempData);
	if (size>0)
		return g_dbLogic.Save(LOGIC_DEFINE_TABLE_NAME,0,tempData.data(),size);
	else
	{
		AssertEx(0,"�����߼��б����");
	}
	return FALSE;
}
BOOL SaveLogicKeyToListTable(__int64 key)
{
	DataBuffer	tempData(512);
	if (g_dbLogic.Load(LOGIC_DEFINE_TABLE_NAME,0,tempData,SUM_DATA_MSG_MAX_SIZE))
	{
		// ˵��KEY�б��б���
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
	// ������, �����
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
		// ˵��KEY�б��б���
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
		// ˵��KEY�б��б���
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
// �����ȡ�߼���������, ����Ĭ��Ϊȫ����ȡ
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
					// �����ݿ��е�ȡ
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
			// ȫ������		
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
	// �����ݿ��е�ȡ
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
