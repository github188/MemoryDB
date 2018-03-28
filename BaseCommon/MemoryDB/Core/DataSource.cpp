
#include "DataSource.h"
#include "DBSaver.h"
#include "MemoryDB.h"
#include "DataSourceOperate.h"

#include "ServerEvent.h"
#include "IOCPServerNet.h"
#include "MySqlDBSaver.h"
#include "TableUpdateLoader.h"
#include "LocalFileDBSaver.h"
#include "TimeManager.h"

using namespace Logic;

//-------------------------------------------------------------------------
class tDB_DataBackRequest : public tBaseNetEvent
{
public:
	//virtual bool _DoEvent()
	//{
	//	WaitTime(10);
	//	return tServerEvent::_DoEvent();
	//}

	//virtual bool _OnTimeOver()
	//{
	//	ERROR_LOG("严重错误: 数据同步到备份服务器失败, 需要检查备份服务器是否工作正常");
	//	return true;
	//}

	virtual bool _Serialize(DataStream *destData)
	{
		if (tBaseNetEvent::_Serialize(destData))
		{
			destData->writeString(mDBName);
			return destData->writeString(mKey);
		}
		return false;
	}

public:
	AString		mDBName;
	AString		mKey;
	AutoData	mRecordData;
	int			mCheckCode;
};
//-------------------------------------------------------------------------
class DB_DataSaveBack : public tDB_DataBackRequest
{
public:
	virtual bool _Serialize(DataStream *destData)
	{
		if (tDB_DataBackRequest::_Serialize(destData))
		{
			destData->write(mCheckCode);
			return destData->writeData(mRecordData.getPtr());
		}
		return false;
	}
};

class DB_RequestLoadData : public tDB_DataBackRequest
{

};

class DB_RequestLoadAllData : public tDB_DataBackRequest
{

};


class DB_DeleteDataBack : public tDB_DataBackRequest
{

};

class DB_MySqlCommand : public tBaseNetEvent
{
public:
	DB_MySqlCommand()
		: mRecordData(NULL)
	{

	}

public:
	virtual bool _Serialize(DataStream *destData)
	{
		if (tBaseNetEvent::_Serialize(destData))
		{
			destData->writeString(mDBName);
			destData->writeString(mSqlString);
			//destData->writeString(mUpdateSql);

			destData->write((int)mRecordData->size());
			for (int i=0; i<mRecordData->size(); ++i)
			{
				destData->writeData((*mRecordData)[i].getPtr());
			}
			return true;
		}
		return false;
	}

public:
	AString		mDBName;
	AString		mSqlString;
	//AString		mUpdateSql;
	Array<AutoData>	*mRecordData;
};

class DB_DataSourceInfo : public tServerEvent
{

public:
	virtual bool _DoEvent()
	{
		WaitTime(30);
		return true;
	}
	
};
//-------------------------------------------------------------------------
class DBBackClientNet : public IOCPClientNet
{
public:
	DBBackClientNet()
	{
		mEventCenter = MEM_NEW EventCenter();

		mEventCenter->RegisterEvent("DB_DataSourceInfo", MEM_NEW EventFactory<DB_DataSourceInfo>());
		mEventCenter->RegisterEvent("DB_DataSaveBack", MEM_NEW EventFactory<DB_DataSaveBack>());
		mEventCenter->RegisterEvent("DB_DeleteDataBack", MEM_NEW EventFactory<DB_DeleteDataBack>());
		mEventCenter->RegisterEvent("DB_MySqlCommand", MEM_NEW EventFactory<DB_MySqlCommand>());

	}

	virtual void OnConnected()
	{
		if (!mBackDBInfo)
		{
			ERROR_LOG("备份初始错误: 未设置备份数据库信息");
			return;
		}
		Hand<DB_DataSourceInfo>	infoEvt = GetClientConnect()->StartEvent("DB_DataSourceInfo");
		mBackDBInfo->set(DBNAME, mDBName.c_str());
		infoEvt->set("BACK_INFO", mBackDBInfo);
		infoEvt->DoEvent();
	}

public:
	virtual Logic::tEventCenter* GetEventCenter(void) const
	{
		return (Logic::tEventCenter*)(mEventCenter.getPtr());
	}

	virtual void Process()
	{
		IOCPClientNet::Process();
		mEventCenter->ProcessEvent();
	}

public:
	AutoEventCenter		mEventCenter;

	AutoNice			mBackDBInfo;
	AString				mDBName;
};

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
bool DataSource::InitDataSource( NiceData &initParam )
{
	// 初始存储器
	tDBSaver *pSaver = CreateSaver(); // new LocalFileDBSaver(); // 

	if (!pSaver->Start(initParam))
	{			
		ERROR_LOG("数据源初始失败 > [%s]", initParam.dump().c_str());
		delete pSaver;
		return NULL;
	}
	ReadyDBSaver(pSaver);

	// 初始备份网络, 备份网络会根据参数配置, 进行写入数据源
	Data backIp = initParam.get("BACK_IP");
	Data backPort = initParam.get("BACK_PORT");

	if (!backIp.empty() && !backIp.empty())
	{
		DBBackClientNet *pNet = MEM_NEW DBBackClientNet();
		initParam.get("BACK_INFO", pNet->mBackDBInfo);
		if (! pNet->mBackDBInfo)
		{
			ERROR_LOG("备份初始错误: 未设置备份数据库信息");
			return true;
		}
		pNet->mDBName = pSaver->GetDBName();
		AutoNet backNet = pNet;
		backNet->StartNet(backIp.string().c_str(), (int)backPort);
		mBackNet.push_back(backNet);
	}

	return true;
}
//-------------------------------------------------------------------------
DataSource::DataSource() 
	: mDBSaver(NULL)
	, mErrorCount(0)
{	
	RegisterTask(MEM_NEW DefineTaskFactory<DB_SaveData, eSaverSaveData>());
	RegisterTask(MEM_NEW DefineTaskFactory<DB_LoadData, eSaverLoadData>());
	RegisterTask(MEM_NEW DefineTaskFactory<DB_LoadAllRecord, eSaverLoadAllRecord>());
	RegisterTask(MEM_NEW DefineTaskFactory<DB_DeleteData, eSaverDeleteData>());

	RegisterTask(MEM_NEW DefineTaskFactory<DB_SaveRecordData, eSaverSaveRecord>());
	RegisterTask(MEM_NEW DefineTaskFactory<DB_LoadAllRecord_SQL, eMySqlLoadAllRecord>());
	RegisterTask(MEM_NEW DefineTaskFactory<DB_RunSqlGetData_SQL, eRunSqlGetData>());
	
	RegisterTask(MEM_NEW DefineTaskFactory<DB_SaveDataForLocalDB, eSaveDataForLocalDB>());
	RegisterTask(MEM_NEW DefineTaskFactory<DB_DeleteRecordForLocalDB, eDeleteRecordDataForLocalDB>());
}

DataSource::~DataSource()
{
	if (mDBSaver)
		delete mDBSaver;

	mDBSaver = NULL;
}

void DataSource::ReadyDBSaver( tDBSaver *saver )
{
	if (mDBSaver)
		delete mDBSaver;
	mDBSaver = saver;

	if (mDBSaver!=NULL)
	{		
		InitThread();
	}
}




bool DataSource::LoadAllRecord( ABaseTable destTable, LOAD_RECORD_CALLBACK callBack )
{
	Auto<MemoryDBTable> t = destTable;
	t->SetStartUpdatePool(false);
	bool bOk = false;
	AutoTaskFactory hF = GetTaskFactory(eSaverLoadAllRecord);
	if (hF)
	{
		Auto<DB_LoadAllRecord> h = hF->NewTask();
		h->mTargetTable = destTable;
		h->mFinishCall = callBack;

		h->Execute(this);
		h->OnFinished(this);

		bOk = h->mErrorInfo.empty();
	}
	else
		ERROR_LOG("[%d] db task factory is not register", eSaverLoadAllRecord);

	t->SetStartUpdatePool(true);

	return bOk;
}

void DataSource::DelectData( const char *szKeyID, UInt64 extValue, LoadDataCallBack callBack )
{
	Auto<DB_DeleteData> h = StartTask(eSaverDeleteData);
	h->mKeyID = szKeyID;
	h->mCallBack = callBack;

	for (size_t i=0; i<mBackNet.size(); ++i)
	{			
		HandConnect conn = mBackNet[i]->GetClientConnect();
		Hand<DB_DeleteDataBack> evt = conn->StartEvent("DB_DeleteDataBack");
		evt->mDBName = GetDBName();
		evt->mKey = szKeyID;
		evt->DoEvent();
	}
}

bool DataSource::ReadyCreateDBTable( const char* szTableIndex, tBaseTable *tableInfoData, AString tableData )
{
	if (mDBSaver!=NULL)
		return mDBSaver->InitReadyNewDB(szTableIndex, tableInfoData, tableData);
	return false;
}

const char* DataSource::GetDBName()
{
	if (mDBSaver!=NULL)
		return mDBSaver->GetDBName();

	return "UNDEFENE";
}



void DataSource::Process()
{
	TaskManager::Process(); 

	if (!mBackNet.empty())
	{
		for (size_t i=0; i< mBackNet.size(); ++i)
		{
			mBackNet[i]->Process();
		}
	}
}


tDBSaver* DataSource::CreateSaver()
{
	return MEM_NEW MySqlDBSaver();
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

//void BackDataSource::SaveData( const char *szKeyID, void *pData, DSIZE size )
//{
//	for (size_t i=0; i<mBackNet.size(); ++i)
//	{			
//		HandConnect conn = mBackNet[i]->GetConnect(0);
//		Hand<DB_DataSaveBack> evt = conn->StartEvent("DB_DataSaveBack");
//		evt->mDBName = mDBName;
//		if (!evt->mRecordData)
//		{
//			evt->mRecordData = MEM_NEW DataBuffer(size);
//		}
//		else if (evt->mRecordData->size()<size)
//			evt->mRecordData->resize(size);
//		else
//			evt->mRecordData->clear(false);
//
//		evt->mRecordData->seek(0);
//		evt->mRecordData->_write(pData, size);
//		evt->mKey = szKeyID;
//		evt->DoEvent();
//	}
//}
//
//void BackDataSource::LoadData( const char *szKeyID, LoadDataCallBack callBack )
//{
//	int randSource = rand()%mBackNet.size();
//	HandConnect conn = mBackNet[randSource]->GetConnect(0);
//	if (conn)
//	{
//		Hand<DB_RequestLoadData> evt = conn->StartEvent("DB_RequestLoadData");
//		evt->mDBName = mDBName;
//		evt->mKey = szKeyID;
//		evt->DoEvent();
//	}
//}
//
//void BackDataSource::LoadAllRecord( ABaseTable destTable, DBCallBack callBack )
//{
//	int randSource = rand()%mBackNet.size();
//	HandConnect conn = mBackNet[randSource]->GetConnect(0);
//	if (conn)
//	{
//		Hand<DB_RequestLoadAllData> evt = conn->StartEvent("DB_RequestLoadAllData");
//		evt->mDBName = mDBName;
//		evt->DoEvent();
//	}
//}
//
//void BackDataSource::DelectData( const char *szKeyID, LoadDataCallBack callBack )
//{
//	for (size_t i=0; i<mBackNet.size(); ++i)
//	{			
//		HandConnect conn = mBackNet[i]->GetConnect(0);
//		Hand<DB_DeleteDataBack> evt = conn->StartEvent("DB_DeleteDataBack");
//		evt->mDBName = mDBName;
//		evt->mKey = szKeyID;
//		evt->DoEvent();
//	}
//}
//
//void BackDataSource::Process()
//{
//	for (size_t i=0; i<mBackNet.size(); ++i)
//	{						
//		if (mBackNet[i]->IsOk())
//			mBackNet[i]->Process();
//	}
//}


//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
bool MySqlDataSource::SaveRecordData( const char *szKey, BaseRecord *pRe )
{
	Auto<DB_SaveRecordData> dbOp = StartTask(eSaverSaveRecord);
	MemoryDBRecord *pMemoryRecord = dynamic_cast<MemoryDBRecord*>(pRe);
	AssertEx(pMemoryRecord!=NULL, "Must is MemoryDBRecord");

	if ( GetMySqlSaver()->mMySqlDB._MakeSaveSqlData(dbOp->mSqlString, dbOp->mRecordData, pRe->GetSelf(), pMemoryRecord->IsNewInsert()) )
	{
		if (!pMemoryRecord->IsNewInsert() && pMemoryRecord->GetTable()->NeedTrySaveRecord())
			GetMySqlSaver()->mMySqlDB._MakeSaveSqlData(dbOp->mInsertSqlString, dbOp->mRecordData, pRe->GetSelf(), true);

		pMemoryRecord->SetNewInsert(false);
		Data indexValue = pRe->getIndexData();
		dbOp->mKeyID = indexValue.string();
		if (!mBackNet.empty())
		{
			Hand<DB_MySqlCommand> commEvt = mBackNet[0]->GetEventCenter()->StartEvent("DB_MySqlCommand");
			commEvt->mDBName = GetDBName();
			commEvt->mSqlString = dbOp->mSqlString;
			//commEvt->mUpdateSql = dbOp->mUpdateSql;
			commEvt->mRecordData = &(dbOp->mRecordData);

			for (size_t i=0; i<mBackNet.size(); ++i)
			{			
				HandConnect conn = mBackNet[i]->GetClientConnect();
				conn->SendEvent(commEvt.getPtr());
			}
		}

		return true;
	}
	else
		ERROR_LOG("生成SQL字符串失败>[MySqlDataSource]");


	return false;
}

tDBSaver* MySqlDataSource::CreateSaver()
{
	return MEM_NEW DBTableSaver();
}

void MySqlDataSource::backProcess()
{
	DataSource::backProcess();
	if (mDBSaver!=NULL)
		mDBSaver->Process();
}

DBTableSaver* MySqlDataSource::GetMySqlSaver()
{
	return dynamic_cast<DBTableSaver*>(mDBSaver);
}

bool MySqlDataSource::LoadAllRecord( ABaseTable destTable, LOAD_RECORD_CALLBACK callBack )
{
	Auto<MemoryDBTable> t = destTable;
	t->SetStartUpdatePool(false);

#if !COOL_START_LOAD_RECORD
	if (strcmp(destTable->GetTableType(), "CoolTable")==0)
	{
		AString limitField = t->mLoadLimitField;
		if (limitField!="")
		{
			MySqlDBTool &dbTool = GetMySqlSaver()->mMySqlDB;
			AString sql;
			int range = t->mLoadLimitRange;
			if (range>0)
			{				
				UInt64 timeLimit = TimeManager::Now()-range;
				printf("[%s] 开始完全加载[%.1f] 小时 [%d] 秒 之前的记录\r\n", GetDBName(), (float)range/3600, range);
				// 先调取时间限制之后的记录
				sql.Format("SELECT * from `%s` WHERE `%s`>=%llu"
					, GetDBName()
					, limitField.c_str()
					, timeLimit
					);
				if (dbTool.exeSql(sql, true))
				{
					int count = 0;
					while (true)
					{					
						ARecord r = destTable->NewRecord();
						if (dbTool.LoadRecord(r))
						{			
							destTable->AppendRecord(r, true);
							r->FullAllUpdate(false);							
							++count;
						}
						else
							break;
					}
					TableTool::green();
					printf("[%s] 完全加载[%d]记录\r\n", GetDBName(), count);
					TableTool::white();
				}
				else
				{
					ERROR_LOG("Load all record fail >[%s]", dbTool.getErrorMsg());
					return false;
				}

				// 再加载所有时间之间的所有记录
				sql.Format("SELECT `%s` from `%s` WHERE `%s`<%llu "
					, destTable->GetMainIndex()->IndexFieldName()
					, GetDBName()
					, limitField.c_str()
					, timeLimit
					);
			}
			else
				sql.Format("SELECT `%s` from `%s`"
				, destTable->GetMainIndex()->IndexFieldName()
				, GetDBName()
				);

			if (dbTool.exeSql(sql, true))
			{
				int count = 0;
				while (true)
				{
					ARecord r = destTable->NewRecord();	
					if (dbTool.LoadRecord(r))
					{			
						destTable->AppendRecord(r, true);
						//!!! 在初始加载后，将空间进行了释放, 可根据配置，是否在加载时，加载全部数据，如果加载还需要修改[DataSource.cpp 589 line]
						r->InitData();
						dynamic_cast<IndexDBRecord*>(r.getPtr())->_freeData();
						++count;
					}
					else
						break;
				}
				TableTool::green();
				printf("[%s]加载冷却[%d]记录\r\n", GetDBName(), count);
				TableTool::white();
			}
			else
			{
				ERROR_LOG("Load all record fail >[%s]", dbTool.getErrorMsg());
				return false;
			}	
			t->SetStartUpdatePool(true);
			return true;
		}				
	}
#endif

	bool bOk = false;
	AutoTaskFactory hF = GetTaskFactory(eMySqlLoadAllRecord);
	if (hF)
	{
		Auto<DB_LoadAllRecord_SQL> h = hF->NewTask();
		h->mTargetTable = destTable;
		h->mFinishCall = callBack;

		h->Execute(this);
		h->OnFinished(this);
		bOk = h->mErrorInfo.empty();
	}
	t->SetStartUpdatePool(true);
	return bOk;
}

bool MySqlDataSource::ReloadRecord(const char *szKey, AutoRecord destRecord)
{
	AString sql;
	AutoIndex mainIndex = destRecord->GetTable()->GetMainIndex();
	if (mainIndex->IsStringHashKey())
		sql.Format("Select * from `%s` where %s = '%s'", GetDBName(), mainIndex->IndexFieldName(), szKey);
	else
		sql.Format("Select * from `%s` where %s = %s", GetDBName(), mainIndex->IndexFieldName(), szKey);

	if (!mReloadRecordTool->exeSqlNeedLoad(sql))
	{
		ERROR_LOG("执行重新加载语句错误>[%s]", sql.c_str());
		return ARecord();
	}
	if (!mReloadRecordTool->LoadRecord(destRecord))
	{
		ERROR_LOG("重新加载记录失败[%s]", szKey);
		return ARecord();
	}	
	return true;
}

bool MySqlDataSource::ReadyReloadTool(NiceData &initParam)
{
	if (!mReloadRecordTool)
		mReloadRecordTool = MEM_NEW MySqlDBTool();

	if (mReloadRecordTool->InitStart(initParam))
	{
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
void DB_SaveRecordData::Execute(TaskManager *pTaskMgr)
{
	MySqlDataSource *pDataSource = dynamic_cast<MySqlDataSource*>(pTaskMgr);

	DBTableSaver *pSaver = pDataSource->GetMySqlSaver();
	int affectCount = 0;
	
	bool bError = false;
	if (mSqlString!="")
	{
		bError = !pSaver->mMySqlDB.exeSql(mSqlString, mRecordData, &affectCount);
		if (bError && !mRecordData.empty())
		{
			for (int i=0; i<mRecordData.size(); ++i)
			{
				if (mRecordData[i]->dataSize()>16*1024*1024) // 64000)
				{
					ERROR_LOG("严重错误: Data %d size %d >64000\r\n >%s", i, mRecordData[i]->dataSize(), mSqlString.c_str());
				}				
			}
		}
	}
	if (affectCount<=0)
	{
		if (mInsertSqlString!="")
			bError = !pSaver->mMySqlDB.exeSql(mInsertSqlString, mRecordData, &affectCount);

		if (affectCount<=0 && bError)
		{
			WARN_LOG("更新或保存记录失败 [%s] at table [%s], SQL>%s", mKeyID.c_str(), pSaver->GetDBName(), mSqlString.c_str());
			NOTE_LOG("MYSQL ERROR: %s", pSaver->mMySqlDB.getErrorMsg());
			pDataSource->AddErrorCount(1);
		}
	}
}

void DB_LoadAllRecord_SQL::Execute(TaskManager *pTaskMgr)
{
	MySqlDataSource *mpDataSource = dynamic_cast<MySqlDataSource*>(pTaskMgr);
	if (mpDataSource==NULL)
	{
		mErrorInfo = "存储器当前未初始成功";			
		return;
	}

	AString sql;
	if (strcmp(mTargetTable->GetTableType(), "SQLTable")==0
#if !COOL_START_LOAD_RECORD
		|| strcmp(mTargetTable->GetTableType(), "CoolTable")==0
#endif
		)
		sql.Format("SELECT `%s` from `%s`", mTargetTable->GetMainIndex()->IndexFieldName(), mpDataSource->GetDBName());
	else
		sql.Format("SELECT * from `%s`", mpDataSource->GetDBName());
	

	mpDataSource->GetMySqlSaver()->mMySqlDB.exeSql(sql, true);

	MemoryDBTable *pTable = dynamic_cast<MemoryDBTable*>(mTargetTable.getPtr());
	while (true)
	{
		if (!pTable->LoadFromDB(&(mpDataSource->GetMySqlSaver()->mMySqlDB)))		
			break;
	}
}
//-------------------------------------------------------------------------
void DB_RunSqlGetData_SQL::Execute(TaskManager *pTaskMgr)
{
	if (mSqlString!="")
	{
		MySqlDataSource *mpDataSource = dynamic_cast<MySqlDataSource*>(pTaskMgr);
		if (mpDataSource==NULL)
		{
			mErrorInfo = "存储器当前未初始成功";			
			return;
		}
		mResultData = mpDataSource->GetMySqlSaver()->mMySqlDB.ExeSqlFunction(mSqlString, true);
	}
	else
		mErrorInfo = "SQL语句为空";
}
//-------------------------------------------------------------------------

tDBSaver* MySqlStringDataSource::CreateSaver()
{
	return MEM_NEW DBTableSaver();
}

void MySqlStringDataSource::backProcess()
{
	DataSource::backProcess();
	if (mDBSaver!=NULL)
		mDBSaver->Process();
}

//-------------------------------------------------------------------------


//-------------------------------------------------------------------------*/

bool StringDataSource::SaveRecordData(const char *szKey, BaseRecord *pRe)
{
	AString recordData;
	if (pRe->ToString(recordData, false, false))
	{
		//SaveData(szKey, (void*)recordData.c_str(), recordData.length()+1);
		Auto<DB_SaveData> h = StartTask(eSaverSaveData);
		h->mKeyID = szKey;
		h->mData._write((void*)recordData.c_str(), recordData.length() + 1);
		//h->mCallBack = callBack;
		return true;
	}

	return false;
}

bool LocalDataSource::InitDataSource(NiceData &initParam)
{
#if LOCAL_DB_USE_THREAD
	InitThread();
#endif
	return mDBSaver->Start(initParam);
}

bool LocalDataSource::ReadyCreateDBTable(const char* szTableIndex, tBaseTable *tableInfoData, AString tableData)
{
	return GetFileDBSaver()->InitReadyNewDB(szTableIndex, tableInfoData, tableData);
}

bool LocalDataSource::LoadAllRecord(ABaseTable destTable, LOAD_RECORD_CALLBACK callBack)
{
	// 同步加载
	bool b = GetFileDBSaver()->ReadAllRecord(destTable.getPtr());
	callBack(NULL, NULL, b);
	return b;
}

void LocalDataSource::DelectData(const char *szKeyID, UInt64 extValue, LoadDataCallBack callBack)
{
#if LOCAL_DB_USE_THREAD
	Hand<DB_DeleteRecordForLocalDB> op = StartTask(eDeleteRecordDataForLocalDB);
	op->mRecordKey = szKeyID;
#else
	bool re = GetFileDBSaver()->DelectData(szKeyID, extValue);
	callBack(NULL, NULL, re?1:0);
#endif
}

//-------------------------------------------------------------------------*/

bool LocalDataSource::SaveRecordData(const char *szKey, BaseRecord *pRe)
{
	MemoryDBRecord *p = dynamic_cast<MemoryDBRecord*>(pRe);
	if (p->mExtValue<=0 && !p->mbNewInsert)
	{
		ERROR_LOG("[%s]记录[%s]不为插入方式, 未设置DB位置", GetFileDBSaver()->mDBName.c_str(), pRe->getIndexData().string().c_str());
		return false;
	}

#if LOCAL_DB_USE_THREAD
	Hand<DB_SaveDataForLocalDB> op = StartTask(eSaveDataForLocalDB);
	RecordData &mData = op->mRecordData;
	op->mRecordKey = szKey;
	op->mbNewInsert = p->mbNewInsert;
#else
	RecordData mData;
#endif
	// db 文件的位置
	UInt64 pos = p->mExtValue;
	// 转到re
	AutoField f = pRe->getField();
	int count = f->getCount();
	mData.resize(count);
	void *d = p->_getRecordData();
	for (int i=0; i<count; ++i)
	{
		if (pRe->HadChanged(i))
		{
			DataBuffer &data = mData[i].mData;
			data.clear(false);
			FieldInfo info = f->getFieldInfo(i);
			info->saveData(d, &data);
			if (data.dataSize()>info->getDBDataLength())
			{
				ERROR_LOG("[%s] 记录[%s]数据大小[%d]大于最大长度[%d]", GetFileDBSaver()->mDBName.c_str(), pRe->get(0).string().c_str(), data.dataSize(), info->getMaxLength()+2);
				return false;
			}
			mData[i].mbUpdate = true;
		} 
	}

#if LOCAL_DB_USE_THREAD
	return true;
#else
	bool bSave = GetFileDBSaver()->SaveRecordData(p->mExtValue, szKey, &mData, p->mbNewInsert);
	return bSave;
#endif
}

bool LocalDataSource::ReloadRecord(const char *szKey, AutoRecord destRecord)
{
	MemoryDBRecord *p = dynamic_cast<MemoryDBRecord*>(destRecord.getPtr());
#if LOCAL_DB_USE_THREAD
	return mReloadTool->ReadRecordData((int)p->mExtValue, destRecord);
#else
	return GetFileDBSaver()->ReadRecordData((int)p->mExtValue, destRecord);
#endif
}

class OnlyReloadRecordLocalDBSaver : public LocalFileDBSaver
{
public:
	virtual bool SaveRecordData(UInt64 &recordPos, const char *szKey, RecordData *data, bool) override
	{
		AssertNote(0, "Can not use OnlyReloadRecordLocalDBSaver::SaveRecordData");
		return false;
	}

	virtual bool DelectData(const char *szKeyID, UInt64 extValue) override
	{
		AssertNote(0, "Can not use OnlyReloadRecordLocalDBSaver::DelectData");
		return false;
	}

	virtual bool ReadAllRecord(tBaseTable *destTable) override
	{
		AssertNote(0, "Can not use OnlyReloadRecordLocalDBSaver::ReadAllRecord");
		return false;
	}	

	void ResaveRecordCount()
	{
		AssertNote(0, "Can not use OnlyReloadRecordLocalDBSaver::ResaveRecordCount");
	}

};

bool LocalDataSource::ReadyReloadTool(NiceData &initParam)
{
#if LOCAL_DB_USE_THREAD
	if (mReloadTool==NULL)
		mReloadTool = MEM_NEW OnlyReloadRecordLocalDBSaver();
	return mReloadTool->Start(initParam);
#else
	return true;
#endif
}

tDBSaver* LocalDataSource::CreateSaver()
{
	if (mDBSaver==NULL)
	{
#if LOCAL_DB_USE_THREAD
		mDBSaver = MEM_NEW ThreadLocalFileDBSaver();
#else
		mDBSaver = MEM_NEW LocalFileDBSaver();
#endif
	}
	return mDBSaver;
}

LocalFileDBSaver* LocalDataSource::GetFileDBSaver()
{
	return dynamic_cast<LocalFileDBSaver*>(mDBSaver);
}

LocalDataSource::~LocalDataSource()
{
#if LOCAL_DB_USE_THREAD
	SAFE_DELETE(mReloadTool);
#endif
}
