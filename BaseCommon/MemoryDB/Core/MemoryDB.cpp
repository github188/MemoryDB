#include "MemoryDB.h"
#include <stdarg.h>

#include "IndexBaseTable.h"
#include "EventCenter.h"
#include "MemoryDBTable.h"
#include "MemoryDBIndexTable.h"
#include "DBServerOperate.h"
#include "DataSourceOperate.h"
#include "LocalFileDBSaver.h"

#include "IOCPConnect.h"
#include "ServerIPInfo.h"
#include "SqlDBTable.h"
#include "TimeManager.h"
#include "CoolDBTable.h"

#include "MySqlDBSaver.h"
#include "DBNetPacket.h"
#include "DBTable.h"
#include "DBClientRequest.h"
#include <io.h>
#include "DBFieldTable.h"
//-------------------------------------------------------------------------
using namespace Logic;


//-------------------------------------------------------------------------*
// 当未连接，或断开时，直接开启一个空的，用来报告连接断开错误
class EmptyDBRequest : public DB_Request
{
public:
	virtual bool _DoEvent() override
	{
		mErrorInfo = "DB 未连接或已经断开";
		mResultType = eDBDisconnect;
		setFinished(false);
		Finish();
		return true;
	}
};
//-------------------------------------------------------------------------*/
void MemoryDB::RegisterDBServerEvent(AutoEventCenter serverNetEventCenter)
{
	serverNetEventCenter->RegisterEvent("DB_LoadTableField", MEM_NEW EventFactory<DB_LoadTableField_S>());
	serverNetEventCenter->RegisterEvent("DB_CreateTable", MEM_NEW EventFactory<DB_CreateTable>());
	//mEventCenter->RegisterEvent("DB_SaveRecord", MEM_NEW EventFactory<DB_SaveRecord_S>());
	serverNetEventCenter->RegisterEvent("DB_FindRecord", MEM_NEW EventFactory<DB_FindRecord_S>());
	serverNetEventCenter->RegisterEvent("DB_DeleteRecord", MEM_NEW EventFactory<DB_DeleteRecord_S>());
	serverNetEventCenter->RegisterEvent("DB_ChangeTableField", MEM_NEW EventFactory<DB_ChangeTableField>());
	//mEventCenter->RegisterEvent("DB_UpdateRecord", MEM_NEW EventFactory<DB_UpdateRecord_S>());

	serverNetEventCenter->RegisterEvent("DB_LoadAllTableField", MEM_NEW EventFactory<DB_LoadAllTableField_S>());
	serverNetEventCenter->RegisterEvent("DB_RunDBOperate", MEM_NEW EventFactory<DB_RunDBOperate_S>());

	serverNetEventCenter->RegisterEvent("DB_RequestModifyData", MEM_NEW EventFactory<DB_RequestModifyData_S>());

	serverNetEventCenter->RegisterEvent("DB_RequestRecordData", MEM_NEW EventFactory<DB_RequestRecordData_S>());
	serverNetEventCenter->RegisterEvent("DB_RequestTableDistribution", MEM_NEW EventFactory<DB_RequestTableDistribution_S>());
	serverNetEventCenter->RegisterEvent("DB_NotifyDBDistribution", MEM_NEW EventFactory<DB_NotifyDBDistribution>());
}

void MemoryDB::RegisterDBClientEvent(AutoEventCenter clientDBEventCenter)
{
	clientDBEventCenter->RegisterEvent("EmptyDBRequest", MEM_NEW EventFactory<EmptyDBRequest>());

	clientDBEventCenter->RegisterEvent("DB_CreateTable", MEM_NEW EventFactory<DB_CreateTable_C>());
	clientDBEventCenter->RegisterEvent("DB_LoadTableField", MEM_NEW EventFactory<DB_LoadTableField_C>());
	//clientDBEventCenter->RegisterEvent("DB_SaveRecord", MEM_NEW EventFactory<DB_SaveRecord_C>());
	//clientDBEventCenter->RegisterEvent("DB_UpdateRecord", MEM_NEW EventFactory<DB_UpdateRecord_C>());
	clientDBEventCenter->RegisterEvent("DB_FindRecord", MEM_NEW EventFactory<DB_FindRecord_C>());
	clientDBEventCenter->RegisterEvent("DB_DeleteRecord", MEM_NEW EventFactory<DB_DeleteRecord_C>());
	clientDBEventCenter->RegisterEvent("DB_ChangeTableField", MEM_NEW EventFactory<DB_ChangeTableField_C>());

	clientDBEventCenter->RegisterEvent("DB_LoadAllTableField", MEM_NEW EventFactory<DB_LoadAllTableField_C>());
	//clientDBEventCenter->RegisterEvent("DB_RunDBOperate", MEM_NEW EventFactory<DB_RunDBOperate_C>());	

	clientDBEventCenter->RegisterEvent("DB_RequestModifyData", MEM_NEW EventFactory<DB_RequestModifyData_C>());	
	clientDBEventCenter->RegisterEvent("DB_RequestRecordData", MEM_NEW EventFactory<DB_RequestRecordData_C>());	

	clientDBEventCenter->RegisterEvent("DB_RequestTableDistribution", MEM_NEW EventFactory<DB_RequestTableDistribution_C>());

	clientDBEventCenter->RegisterEvent("DB_NotifyDBDistribution", MEM_NEW EventFactory<DB_NotifyDBDistribution_R>());
}

bool MemoryDB::GetDBTableComment(NiceData &dbParam, const char *szTableName, AString &resultComment)
{
	AutoTable t = tBaseTable::NewBaseTable();
	MySqlDBTool sqlTool;
	if (sqlTool.InitStart(dbParam))
	{
		AString strSql;
		strSql.Format("show table status from %s", dbParam[DBBASE].string().c_str());			
		if (sqlTool.exeSql(strSql, true) && sqlTool.InitField(t->GetField(), true) )
		{			
			t->SetMainIndex(0, true);				
			while (true)
			{
				ARecord r = t->NewRecord();
				if (sqlTool.LoadRecord(r))
				{			
					t->AppendRecord(r, true);
					r->FullAllUpdate(false);
				}
				else
					break;
			}				
			ARecord r = t->GetRecord(szTableName);
			if (r)
			{
				resultComment = r["Comment"];				
				return true;
			}
		}
	}
	return false;
}

bool MemoryDB::SetDBTableComment(NiceData &dbParam, const char *szTableName, const AString &comment)
{
	AutoTable t = tBaseTable::NewBaseTable();
	MySqlDBTool sqlTool;
	if (sqlTool.InitStart(dbParam))
	{
		AString strSql;
		strSql.Format("alter table t_account Comment = '%s'", comment.c_str());			
		return  sqlTool.exeSql(strSql, false);
	}
	return false;
}

AString MemoryDB::GetTableInfoString(ARecord tableInfoRecord)
{
	AString strInfoData;
	// NOTE: 兼容以前的DB定义
	FieldInfo info = tableInfoRecord->getFieldInfo("DATA");
	if (info->getType()==FIELD_DATA)
	{
		AutoData d = tableInfoRecord->getDataBuffer("DATA");
		char *szTemp = (char*)ALLOCT_NEW(d->dataSize()+1);
		memcpy(szTemp, d->data(), d->dataSize());
		szTemp[d->dataSize()] = '\0';
		strInfoData = szTemp;
		ALLOCT_FREE(szTemp);
	}
	else
		strInfoData = tableInfoRecord["DATA"];

	return strInfoData;
}

//-------------------------------------------------------------------------*/
//-------------------------------------------------------------------------
MemoryDB::MemoryDB()
	: mConfigCoolDBTime(DB_COOL_TIME)
	, mbLocalModeSaveDB(false)
{
	mDBServerNet = MEM_NEW DBServerNet(this);
	mEventCenter = MEM_NEW EventCenter();

	mDBServerNet->BindEventCenter();

	RequestDBOperateEvent::InitReadyForNet(mDBServerNet);

	RegisterDBServerEvent(mEventCenter);
}

void MemoryDB::Log( const char *szInfo, ... )
{
#if DEVELOP_MODE
	va_list	va;
	va_start(va, szInfo);

	LOG_TOOL(va, szInfo);
#endif
}

bool MemoryDB::Start( const char *szIp, int nPort)
{
	mDBServerNet->StartNet(szIp, nPort);
	Log("DB ACCEPT by [%s:%d]", szIp, nPort);
	return true;
}

bool MemoryDB::InitDB( NiceData &initParam )
{
	mConfigCoolDBTime = initParam.get(COOLTIME);

	AString tableList = initParam.get("TABLE_LIST");
	AssertNote(!tableList.empty(), "需要配置TABLE_LIST, 列表名称, 用于工作的数据表格列表,DB数据库");
	
	mbLocalModeSaveDB = initParam[ISLOCALDB];

	mDataSourceConfig = initParam;

	// 连接数据源DB
	DBTable *pDBTable =  MEM_NEW DBTable(); 
	mTableListDBTable = pDBTable;
	mTableListDBTable->SetTableName(tableList.c_str());

	mTableList.erase(tableList);

	// 表格列表使用字符串明码保存
	mDataSourceConfig.set(DBNAME, mTableListDBTable->GetTableName());
	mDataSourceConfig.set("CHECK_CODE", 0);

	if (mbLocalModeSaveDB)
	{
		NOTE_LOG("当前使用本地DB文件模式落地, DB 文件目录 [DB/]");

		AString path;
		path.Format("DB/%s.txt", tableList.c_str());
        mTableListDBTable->SetTableName(path.c_str());

		//  Load 中会把 path 设置表格名称
		if (!mTableListDBTable->LoadCSV(path.c_str(), true))
		{
			if (_access(path.c_str(), 0)!=-1)
			{
				ERROR_LOG("[%s] db list file exist, but load fail", path.c_str());
				return false;
			}
            mTableListDBTable->AppendField("ID", FIELD_STRING)->setMaxLength(64);
            mTableListDBTable->AppendField("DATA", FIELD_STRING)->setMaxLength(6*1024);
            mTableListDBTable->AppendField("SLOT", FIELD_DATA);
            mTableListDBTable->AppendField("DB_INFO", FIELD_STRING)->setMaxLength(6*1024);
            mTableListDBTable->ReadyField();
            if (!mTableListDBTable->GetField()->check())
            {
                ERROR_LOG("List field error >%s", mTableListDBTable->GetTableName());
                return false;
            }
            if (!mTableListDBTable->SaveCSV(path.c_str(), true))
            {
                ERROR_LOG("[%s] db list file create fail", path.c_str());
                return false;
            }
		}
		//mTableListDBTable->SaveCSV("d:/test_db_field.txt", true);
		LoadAllDBTable();
	}
	else
	{
		// NOTE: 修改为直接使用DBTable 进行读取 2017.9.13
		AutoDBTool t = MEM_NEW MySqlDBTool();
		if (!t->InitStart(initParam))
		{
			ERROR_LOG("DB ready fail >%s", t->getErrorMsg());
			return false;
		}
		pDBTable->Init(t, tableList.c_str());
		if (!pDBTable->LoadDB(true, true))
		{
			mTableListDBTable->AppendField("ID", FIELD_STRING)->setMaxLength(64);
			mTableListDBTable->AppendField("DATA", FIELD_STRING)->setMaxLength(6*1024);
			mTableListDBTable->AppendField("SLOT", FIELD_DATA);
			mTableListDBTable->AppendField("DB_INFO", FIELD_STRING)->setMaxLength(6*1024);
			mTableListDBTable->ReadyField();
			if (!mTableListDBTable->GetField()->check() || !pDBTable->CreateToDB())
			{
				ERROR_LOG("List field error or Create table list fail >%s", t->getErrorMsg());
				return false;
			}
		}
		
		LoadAllDBTable();
	}

	return true;
}


void MemoryDB::InitReadyMemoryDBTable( const char *szTableIndex, AString fieldData, bool bStartDB )
{
	if (fieldData=="")
	{
		ERROR_LOG("[%s]表格信息数据为空", szTableIndex);
		return;
	}

	// 分解出表格信息
	NiceData	tableInfo;
	if (!tableInfo.FullJSON(fieldData))
	{
		ERROR_LOG("[%s] 还原JSON数据失败 >%s", szTableIndex, fieldData.c_str());
		return;
	}

	AString tableType = tableInfo["TABLE_TYPE"].string();
	AString keyType = tableInfo["KEY_TYPE"].string();
	AString indexFieldsInfo = tableInfo["INDEX_FIELD"].string();
	int fieldCheckCode = tableInfo["FIELD_CODE"];
	AString fieldInfo = tableInfo["FIELD_INFO"].string();

	if (fieldInfo.empty())
	{
		ERROR_LOG("[%s]表格字段信息数据为空", szTableIndex);
		return;
	}

	Auto<MemoryDBTable>	t = CreateNewDBTable(szTableIndex, tableType.c_str());	
	t->mLoadLimitField = tableInfo["LOAD_LIMIT_FIELD"].string();
	t->mLoadLimitRange = tableInfo["LOAD_LIMIT_RANGE"];

	bool b = t->GetField()->FullFromString(fieldInfo);
	if (b)
	{		
		if (!t->GetField()->CheckSame(fieldCheckCode))
		{
			ERROR_LOG("[%s]表格字段信息校验失败, 中止此DB表格的使用", szTableIndex);
			mTableList.erase(szTableIndex);
            return;
        }

        // 应用 DB 子表
        bool bHave = false;
        for (int i=0; i<t->GetField()->getCount(); ++i)
        {
            DBTableFieldInfo *pInfo = dynamic_cast<DBTableFieldInfo*>( t->GetField()->getFieldInfo(i) );
            if (pInfo!=NULL && pInfo->hasAttribute(_DB_FIELD_TABLE))
            {
                AString subName;
                subName.Format("db_%s_%s", szTableIndex, pInfo->getName().c_str());

                ExDBTableFieldInfo *pNewField = MEM_NEW ExDBTableFieldInfo();
                pNewField->setTypeParam(subName);

                if (!GetTable(subName))
                {
                    ARecord subInfoRecord = mTableListDBTable->GetRecord(subName);
                    if (!subInfoRecord)
                    {
                        ERROR_LOG("Sub DB %s table info no exist", subName.c_str());
                        return;
                    }
                    AString subDBInfoString = GetTableInfoString(subInfoRecord);
                    if (subDBInfoString.length()<=0)
                    {
                        ERROR_LOG("Sub DB %s info is NULL", subName.c_str());
                        return;
                    }
                    InitReadyMemoryDBTable(subName.c_str(), subDBInfoString, true);
                }
                AutoDBTable subDB = GetTable(subName);
                if (!subDB)
                {
                    ERROR_LOG("Sub DB table load fail %s", subName.c_str());
                    mTableList.erase(szTableIndex);
                    return;
                }
                if (subDB->GetField()->getFieldType(0)!=FIELD_INT)
                {
                    ERROR_LOG("Sub DB field table key must is INT, now %s", subDB->GetField()->getFieldInfo(0)->getTypeString());
                    mTableList.erase(szTableIndex);
                    return;
                }
                if (!t->GetField()->_replaceField(i, pNewField))
                {
                    ERROR_LOG("Replace DB sub table field fail %s", subName.c_str());
                    mTableList.erase(szTableIndex);
                    return;
                }
                pNewField->mDBTable = subDB;
                pNewField->mSubTableField = subDB->GetField();
                bHave = true;					
            }
        }
        if (bHave)
        {
            t->GetField()->_updateInfo();
            if (!t->GetField()->check())
            {
                ERROR_LOG("Check sub DB field fail %s", szTableIndex);
                mTableList.erase(szTableIndex);
                return;
            }
        }		

        // 优化字段索引
		t->GetField()->_optimizeNameHash(11.5f);

		//NOTE: DB 禁止使用 ID 使用数组索引, 使用HASH索引效率和数组差不多
        //	NOTE: 内存表全部使用 HASH主索引 2017.6.20
		t->SetMainIndex(0, true); 

		// 设置哈希SLOT
		ARecord infoRecord = mTableListDBTable->GetRecord(szTableIndex);
		//AssertNote(infoRecord, "DB info must exist %s", szTableIndex);
		AutoData slotInfo = (DataStream*)infoRecord["SLOT"];
		if (slotInfo)
		{
			slotInfo->seek(0);
			short slotIndex = 0;
			while (true)
			{
				if (slotInfo->read(slotIndex))
					t->mKeyHashSlotList.insert(slotIndex, true);
				else
					break;
			}
		}

		// 设置索引
		if (indexFieldsInfo!="")
		{
			Array<AString> indexFields;
			AString::Split(indexFieldsInfo.c_str(), indexFields, " ", 100);
			if (!indexFields.empty())
			{
				for (int i=0; i<indexFields.size(); ++i)
				{
                    // NOTE: 用于索引的字段, 目的一般为排行, 所以必须使用有序索引, 不可使用哈希
					if (t->SetIndexField(indexFields[i].c_str(), false))	
					{
						LOG_GREEN;
						TABLE_LOG("成功设置索引字段>[%s] at table [%s]", indexFields[i].c_str(), szTableIndex);
					}
					else
					{
						ERROR_LOG("ERROR: 设置索引字段失败>[%s] at table [%s]", indexFields[i].c_str(), szTableIndex);
					}
				}
				LOG_WHITE;
			}
		}
		if (bStartDB)
		{		
			bool bOk = ReadyTableDBDataSource(t, fieldCheckCode);
			if (!bOk)
			{
				mTableList.erase(szTableIndex);
				ERROR_LOG("[%s]表格存储器初始失败", szTableIndex);			
				return;
			}		
			OnLoadDBTableBefore(t);
			t->LoadAllRecord();
		}
				
		OnLoadDBTableFinished(t);
	}
	else
	{
		ERROR_LOG("还原字段信息失败 [%s] [%s]", szTableIndex, fieldData.c_str());
	}

}

AString MemoryDB::SaveDBTableFieldToDB( const char *szTableIndex, ABaseTable table, AString *pDBTableInfoData, AutoNice extParam )
{
	AString fieldData = table->GetField()->ToString();
	if (fieldData.empty())
	{
		AString error;
		error.Format("序列字段数据失败[%s]", szTableIndex);
		return error;
	}
	// 表格信息组成结构{节点ID分段信息}{字段校验码}{字段信息序列字符串}
	AString keyIDRangeInfo;
	EasyString indexType = "INT";
	
	FieldInfo indexField = table->GetField()->getFieldInfo(table->GetMainIndexCol());
	if (!indexField)
	{
		AString info;
		info.Format("DB %s init field error, may be has not set field", szTableIndex);
		ERROR_LOG(info.c_str());
		return info;
	}
	if (indexField->getType()==FIELD_STRING)
		indexType = "HASH";

	NiceData tableInfo;

	tableInfo["TABLE_TYPE"] = table->GetTableType();
	tableInfo["KEY_TYPE"] = indexType.c_str();
	tableInfo["INDEX_FIELD"] = table->GetIndexFieldsInfo().c_str();
	tableInfo["FIELD_CODE"] = (int)table->GetField()->GetCheckCode();
	tableInfo["FIELD_INFO"] = fieldData.c_str();

	if (extParam)
	{
		tableInfo["LOAD_LIMIT_FIELD"] = extParam["LOAD_LIMIT_FIELD"].string();
        tableInfo["LOAD_LIMIT_RANGE"] = (int)extParam["LOAD_LIMIT_RANGE"];
    }

    // 使用子DB表字段模式, 字段属性标识 _DB_FIELD_TABLE
    for (int i=0; i<table->GetField()->getCount(); ++i)
    {
        DBTableFieldInfo *info = dynamic_cast<DBTableFieldInfo*>(table->GetField()->getFieldInfo(i));
        if (info!=NULL && info->hasAttribute(_DB_FIELD_TABLE))
        {
            // 新建子表DB
            AString subName;
            subName.Format("db_%s_%s", szTableIndex, info->getName().c_str());
            AutoTable subDBTable = CreateNewDBTable(subName.c_str(), "MemoryTable");		
            subDBTable->InitField(info->mSubTableField);
            AutoNice subParam;
            if (extParam["USE_NODE"])
            {
                subParam["USE_NODE"] = true;
            }

            AString resultInfo = SaveDBTableFieldToDB(subName.c_str(), subDBTable, NULL, subParam);
            if (!resultInfo.empty())
            {
                ERROR_LOG("ERROR: Create SUB DB table %s to DB Fail >%s", subName.c_str(), resultInfo.c_str());
            }
        }
    }

	AString tableInfoData = tableInfo.ToJSON();
	
	if (tableInfoData.empty() || tableInfoData=="")
	{
		AString error;
		error.Format("[%s] 信息转换至JSON字符串失败 >%s", tableInfo.dump().c_str());
		ERROR_LOG(error.c_str());
		return error;
	}	
	if (pDBTableInfoData!=NULL)
		*pDBTableInfoData = tableInfoData;
	if (mTableListDBTable->GetRecord(szTableIndex))
	{
		AString info;
		info.Format("[%s]表格已经存在, 请先确认是否清除, 或备份后清理", szTableIndex);
		return info;
	}
	else
	{
		tableInfo["FIELD_INFO"] = "";
		bool bOk = false;
		if (mbLocalModeSaveDB)
		{		
			LocalFileDBSaver saver;
			bOk = saver.InitReadyNewDB(szTableIndex, table.getPtr(), "");
		}
		else		
		{
			// NOTE: 修改使用DBTable直接保存
			Auto<DBTable> t = mTableListDBTable;
			Auto<MySqlDBTool> tool = t->mDBTool;
			bOk = tool->CreateDBTable(szTableIndex, table->GetField(), tableInfo.ToJSON().c_str());
		}

		if (bOk)
		{
			ARecord re = mTableListDBTable->CreateRecord(szTableIndex, false);
			if (re)
			{
				OnNewCreateDBTable(szTableIndex, table, tableInfoData, extParam, re);

				if (mbLocalModeSaveDB)
				{	
					re->set("DATA", tableInfoData.c_str());
					// 本地文件DB,直接将列表表格保存到CSV文件					
					if (!mTableListDBTable->SaveCSV(mTableListDBTable->GetTableName(), true))
					{
						AString err;
						err.Format("[%s] DB list table save fail", mTableListDBTable->GetTableName());
						ERROR_LOG(err.c_str());
						return err;
					}
				}
				else
				{				
					re->set("DATA", tableInfoData.c_str());
					re->FullAllUpdate(true);
					Auto<DBTable> t = mTableListDBTable;
					if (!t->SaveTable(true))
					{
						AString err;
						err.Format("ERROR: [%s] Save db list fail >%s", szTableIndex, t->mDBTool->getErrorMsg());
						NOTE_LOG(err.c_str());
						return err;
					}
				}
			}
			else
			{
				AString info;
				info.Format("[%s]创建表格信息记录失败, 不应该出现这种错误", szTableIndex);
				return info;
			}
		}
		else
		{
			AString info;
			info.Format("[%s]创建DB表格, 请检查DB错误", szTableIndex);
			return info;
		}
	}
	return AString();
}

void MemoryDB::Process()
{
	UInt64 tick = TimeManager::NowTick();
	BaseMemoryDB::Process();

#if MEMORY_DB_DEBUG
	/// for test
	static __declspec(thread)  UInt64 beginTime = TimeManager::NowTick();
	static __declspec(thread) UInt64 t = 0;
	static __declspec(thread) UInt64 netTime = 0;

	if (TimeManager::NowTick()-beginTime>10000)
	{
		INFO_LOG("Test loop [%u] at 10 sec, net use [%u]", t, netTime);
		beginTime = TimeManager::NowTick();
		netTime = 0;
		t = 0;
	}

	++t;

	UInt64 useTime = TimeManager::NowTick();
#endif
	mDBServerNet->Process();
#if MEMORY_DB_DEBUG
	netTime += TimeManager::NowTick()-useTime;
#endif
	for (auto it=mTableList.begin(); it.have(); it.next())
	{
		AutoDBTable t = it.get();
		if (t)
			t->Process(1000);
	}

	int tickDiff = (int)(TimeManager::NowTick()-tick);
	if (tickDiff > 50)
	{
		NOTE_LOG("Processs LogicDB tick = %d\n", tickDiff);
	}
}



bool MemoryDB::NowDBState()
{
	bool bFree = true;
	for (auto it=mTableList.begin(); it.have(); it.next())
	{
		AutoDBTable table = it.get();
		if (!table || table->mDataSource==NULL)
			continue;
		int taskCount = table->mDataSource->GetOperateCount()+table->mUpdateList.size();
#if DEVELOP_MODE
		{
			NOTE_LOG("[%s] 数据操作数量 [%d]", table->GetTableName(), taskCount);
			if (taskCount>0)
				bFree = false;
		}
#else
		if (taskCount>0)
		{
			printf("[%s] 数据操作数量 [%d]\r\n", table->GetTableName(), taskCount);
			bFree = false;
		}
#endif
	}
	return bFree;
}


void MemoryDB::OnSucceedStart()
{
	NOTE_LOG("------------------------------------------------------------");
	NOTE_LOG("DB 表格全部加载完成, 启动成功");
	NOTE_LOG("------------------------------------------------------------");
}

void MemoryDB::LoadAllDBTable()
{
    ARecordIt it = mTableListDBTable->GetRecordIt();
    if (!it)
        return;
	for (; *it; ++(*it))
	{
		ARecord r = *it;
		AString szTableIndex = r->getIndexData().string();
		// 不加载 *开头的备份表格
		if (szTableIndex.length()>0 && *szTableIndex.c_str()!='*')
		{
			AString fieldData	 = GetTableInfoString(r); 	

			InitReadyMemoryDBTable(szTableIndex.c_str(), fieldData);
		}
	}
	// 警告分布DB
	LOG_YELLOW
	for (auto it=mTableList.begin(); it; ++it)
	{
		AutoDBTable t = it.get();
		if (t && !t->mKeyHashSlotList.empty() && t->mKeyHashSlotList.size()!=DB_HASH_SLOT_COUNT)
		{
			ARecord r = mTableListDBTable->GetRecord(it.key());
			NiceData info;
			info.FullJSON(r["DB_INFO"].string());
			NOTE_LOG("WARN: [%s] is node db, total DB count [%d], need other node connect >", it.key().c_str(), (int)info["COUNT"]);			
			for (int nCode = 1; nCode<=(int)info["COUNT"]; ++nCode)
			{
				AutoNice d = (tNiceData*)info[STRING(nCode)];
				if (d)
				{
					NOTE_LOG("<%d> DB info:%s", nCode, d->dump().c_str());
				}
				else
					ERROR_LOG("缺少 DB %d info", nCode);
			}
		}
	}
	LOG_WHITE
	//所有表格全部Load OK 才提供网络服务
	AString szIp = mDataSourceConfig.get("DBSERVER_IP").string();
	int nPort = mDataSourceConfig.get("DBSERVER_PORT");
	Start(szIp.c_str(), nPort);

	OnSucceedStart();
}

// 目前使用MySQl 进行测试, 正式使用需要 提供本地数据落地
bool MemoryDB::ReadyTableDBDataSource(AutoDBTable t, int checkCode)
{
	if (t)
	{ 
		mDataSourceConfig.set(DBNAME, t->GetTableName());
		mDataSourceConfig.set("CHECK_CODE", checkCode);

		bool bRe = false;
		if (mbLocalModeSaveDB)
		{
			DataSource *pSource = MEM_NEW LocalDataSource(); 
			bRe = pSource->InitDataSource(mDataSourceConfig);
			if (bRe)
			{
				t->ReadyDataSource(pSource, mDataSourceConfig);
			}
			else
				ERROR_LOG("Ready db table fail >[%s]", t->GetTableName());
		}
		else
		{		
			//!!! 使用DB表结构方式
			MySqlDataSource *pSource = MEM_NEW MySqlDataSource();
			bRe = pSource->InitDataSource(mDataSourceConfig);
			if (bRe)
			{
				t->ReadyDataSource(pSource, mDataSourceConfig);
			}
			else
				ERROR_LOG("Ready db table fail >[%s]", t->GetTableName());
		}

		return bRe;
	}
	return true;
}

void MemoryDB::OnNotifyFinishLoadAllRecord( AutoDBTable t )
{
	if (t==mTableListDBTable)
	{
		LoadAllDBTable();
	}
}

eDBResultType MemoryDB::SaveRecord( const char *szIndex, const char *recordkey, DataStream *scrRecordData, bool bReplace, bool bUpdate, AutoEvent waitEvt )
{
	const char *szKey = recordkey;

	ABaseTable t = GetTable(szIndex);
	if (t)
	{
		ARecord now;

		now = t->GetRecord(szKey);

		if (!bReplace && now)
			return eRecordAlreadyExist;

		ARecord r = t->NewRecord();
		if (r)
		{
			scrRecordData->seek(0);
			if (r->restoreData(scrRecordData))
			{
				if (t->AppendRecord(r, bReplace))
				{
					r->FullAllUpdate(true);
					waitEvt->set("RESULT", true);
					waitEvt->Finish();
					return eNoneError;
				}
			}
		}
	}
	waitEvt->set("RESULT", eRecordCreateFail);
	waitEvt->Finish();
	return eRecordCreateFail;
}

ARecord MemoryDB::FindRecord( const char *szTable, const char *szKey )
{
	ABaseTable t = GetTable(szTable);
	if (t)
		return t->GetRecord(szKey);

	return ARecord();
}

Logic::tEventCenter* MemoryDB::GetEventCenter()
{
	if (mDBServerNet)
		return mDBServerNet->GetEventCenter();

	return NULL;
}

bool MemoryDB::RegisterDBOperate( const char *type, AutoDBNodeOpreateFactory factory )
{	
	GetEventCenter()->RegisterEvent(type, factory);
	Log("成功加入操作 [%d]", type);

	return true;
}

AutoOpereate MemoryDB::StartOperate(const char *type )
{
	return GetEventCenter()->StartEvent(type);
}

void MemoryDB::Close()
{
	if (!mDBServerNet)
		return;

	Process();
	TimeManager::Sleep(1000);
	Process();

	for (auto it=mTableList.begin(); it.have(); it.next())
	{
		AutoDBTable t = it.get();
		if (t)
		{
			t->Process(0);
			t->ClearAll();		
			t._free();
		}		
	}

	BaseMemoryDB::Close();
	mTableList.clear(true);
	mTableListDBTable._free();

	for (int i=0; i<10; ++i)
	{
		mDBServerNet->Process();
		TimeManager::Sleep(10);
	}
	
	mDBServerNet->StopNet();
	mDBServerNet._free();
}

ABaseTable MemoryDB::CreateIndexTable( const char *szIndex )
{
	ABaseTable t = MEM_NEW MemoryDBIndexTable((MemoryDB*)this); 
	t->SetTableName(szIndex);
	mTableList.erase(szIndex);
	mTableList.insert(szIndex, t);
	return t;
}


HandConnect MemoryDB::CreateNewUserConnect()
{
	return MEM_NEW DBUserConnect();
}

HandConnect MemoryDB::FindGSConnect( GSKEY gskey )
{
	Hand<IOCPServerNet> net = GetDBServerNet();
	ConnectList &connectList = net->GetConnectList();
	for (size_t i=0; i<connectList.size(); ++i)
	{
		HandConnect conn = connectList[i];
		if (conn && !conn->IsDisconnect() && conn->GetIPKey()==gskey)
			return conn;
	}
	return HandConnect();
}

AutoTable MemoryDB::CreateNewDBTable( const char *szTableIndex, const char *szType )
{
	ABaseTable t;
	if (strcmp(szType, "MemoryTable")==0)
		t = MEM_NEW MemoryDBTable(this);
	else if (strcmp(szType, "CoolTable")==0)
	{
		if (mbLocalModeSaveDB)
		{
			NOTE_LOG("NOTE: 当前使用DB文件模式落地, 测试 CoolTable >[%s]", szTableIndex);
		}
		// 如果配置冷处理时间为0，则直接使用内存表代替，不进行冷处理
		if (mConfigCoolDBTime>0)
		{
			CoolDBTable *p = NULL;
			t = p = MEM_NEW CoolDBTable(this);
			p->mCoolTime = mConfigCoolDBTime;
		}
		else
		{
			t = MEM_NEW MemoryDBTable(this);
			NOTE_LOG("NOTE: [%s] CoolTable now use MemoryDBTable because config cool time is zero", szTableIndex);
		}
	}
	else if (strcmp(szType, "IndexTable")==0)
		t = MEM_NEW MemoryDBIndexTable((MemoryDB*)this);
	else if (strcmp(szType, "SQLTable")==0)
	{
		if (mbLocalModeSaveDB)
		{
			ERROR_LOG("当前使用DB文件模式落地, 不支持 SQLTable, DB Table [%s] 使用默认 MemoryDBTable", szTableIndex);
            t = MEM_NEW MemoryDBTable(this);
		}
		else
			t = MEM_NEW SqlDBTable((MemoryDB*)this);
	}
	else if (strcmp(szType, "TempTable")==0)
		t = MEM_NEW TempMemoryTable((MemoryDB*)this);
	else
	{
		ERROR_LOG("未能识别的表格类型>[%s], 默认为 MemoryDBTable", szType);
		t = MEM_NEW MemoryDBTable(this);
	}

	if (t)
	{
		t->SetTableName(szTableIndex);
		AutoTable existTable = mTableList.find(szTableIndex);
		if (existTable)
		{
			existTable._free();
			mTableList.erase(szTableIndex); 
		}
		mTableList.insert(szTableIndex, t);
		//NOTE: 将存储数组数量提高到总数量的两倍大小, 加速查找速度
		mTableList.OptimizeSpeed(12.5f);
#if DEVELOP_MODE
		//if (mTableList.OptimizeSpeed(2))
		//	printf("Table Hash list optimize once\r\n");
		//mTableList.Dump();
#endif
	}
	return t;
}

HandConnect MemoryDB::DBServerNet::CreateConnect()
{
	return mpServerDB->CreateNewUserConnect();
}

bool MemoryDB::DBServerNet::OnAddConnect( tNetConnect *pConnect )
{
	mpServerDB->_OnUserConnected(pConnect); 
	mpServerDB->OnUserConnected(pConnect); 
	return true;
}
