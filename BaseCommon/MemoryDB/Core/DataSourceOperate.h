
#ifndef _INCLUDE_DATASOURCEOPERATE_H_
#define _INCLUDE_DATASOURCEOPERATE_H_

#include "TaskManager.h"
#include "DataSource.h"
#include "DBSaver.h"

#include "LocalFileDBSaver.h"

enum SAVER_TASK
{
	eSaverSaveData,
	eSaverLoadData,
	eSaverLoadAllRecord,
	eSaverDeleteData,

	eSaverSaveRecord,
	eMySqlLoadAllRecord,
	eRunSqlGetData,

	eSaveDataForLocalDB,
	eDeleteRecordDataForLocalDB,
};

class DataTask : public Task
{
public:
	tDBSaver* GetDBSaver(TaskManager *pTaskMgr)
	{
		DataSource *p = dynamic_cast<DataSource*>(pTaskMgr);
		AssertEx(p!=NULL, "Muse be DataSource ");
		return p->mDBSaver;
	}

public:
	// DB 线程内做的事情
	virtual void Execute(TaskManager *pTaskMgr) = 0;

	// 使用线程内调用
	virtual void OnFinished(TaskManager *pTaskMgr){}
	virtual void InitFree() override
	{
		mKeyID.free();
		mData.clear(false);
	}

public:
	EasyString			mKeyID;
	DataBuffer			mData;
};

class DB_SaveData : public DataTask
{
public:
	virtual void Execute(TaskManager *pTaskMgr)
	{
		if (!GetDBSaver(pTaskMgr)->SaveData(mKeyID.c_str(), mData.data(), mData.dataSize()))
		{
			mData.seek(0);
			ERROR_LOG("DB [%s] Save data fail > [%s]", GetDBSaver(pTaskMgr)->GetDBName(), mKeyID.c_str());
		}
		else
			mData.seek(mData.tell());
	}

	virtual void OnFinished(TaskManager *pTaskMgr)
	{
		mCallBack.run(NULL, mData.tell()==mData.dataSize());
	}

public:
	DBCallBack	mCallBack;
};

class DB_LoadData : public DataTask
{
public:
	virtual void Execute(TaskManager *pTaskMgr)
	{
		if (!GetDBSaver(pTaskMgr)->LoadData(mKeyID.c_str(), &mData))
		{
			ERROR_LOG("Load data fail > [%s]", mKeyID.c_str());
		}
	}

	virtual void OnFinished(TaskManager *pTaskMgr)
	{
		mCallBack.run(mKeyID.c_str(), mData.data(), mData.dataSize());
	}

public:
	LoadDataCallBack	mCallBack;

};

class DB_DeleteData : public DataTask
{
public:
	DB_DeleteData()
		: mbDelectCount(0)
	{

	}
public:
	virtual void Execute(TaskManager *pTaskMgr)
	{
		mbDelectCount = 0;
		if (!GetDBSaver(pTaskMgr)->DelectData(mKeyID.c_str(), 0))
		{
			ERROR_LOG("Delete data fail > [%s]", mKeyID.c_str());
		}
		else
			mbDelectCount = 1;
	}

	virtual void OnFinished(TaskManager *pTaskMgr)
	{
		mCallBack.run(mKeyID.c_str(), NULL, mbDelectCount);
	}

public:
	LoadDataCallBack	mCallBack;
	int					mbDelectCount;
};

class DB_LoadAllRecord : public DataTask
{
public:
	DB_LoadAllRecord()
		: mpDataSource(NULL)
	{

	}

public:
	virtual void Execute(TaskManager *pTaskMgr)
	{
		mpDataSource = dynamic_cast<DataSource*>(pTaskMgr);
		if (GetDBSaver(pTaskMgr)==NULL)
		{
			mErrorInfo = "存储器当前未初始成功";			
			return;
		}
		LoadDataCallBack  callBack(&DB_LoadAllRecord::OnLoadOneData, this);
		if (!GetDBSaver(pTaskMgr)->LoadAllData(callBack, mErrorInfo))
		{
			ERROR_LOG("Save data fail > [%s]", mKeyID.c_str());
		}
	}

public:
	void OnLoadOneData(const char *szKey, const char *p, int size)
	{
		ARecord r = mTargetTable->NewRecord();
		
		StringDataSource *pSource = dynamic_cast<StringDataSource*>(mpDataSource);
		AssertNote(pSource != NULL, "此操作只用于 StringDataSource");
		if (pSource->RestoreRecord(r.getPtr(), p, size))
		{
			r->_set(mTargetTable->GetMainIndexCol(), szKey);
			bool b = mTargetTable->AppendRecord(r, true);
			AssertEx(b, "Must append record succeed");
		}
		else
		{
			//mTargetTable->RemoveRecord(szKey);
			ERROR_LOG("恢复记录失败 > [%s]", szKey);
		}
		r->FullAllUpdate(false);
	}

	virtual void OnFinished(TaskManager *p)
	{
		mFinishCall.run(NULL, NULL, 0); //(DBOperate*)this, mErrorInfo.empty());
	}

	virtual void InitFree() override
	{
		DataTask::InitFree();

		mTargetTable.setNull();
		mErrorInfo.setNull();
		mFinishCall.cleanup();
		mpDataSource = NULL;
	}


public:
	ABaseTable		mTargetTable;
	AString			mErrorInfo;
	LoadDataCallBack		mFinishCall;

	DataSource		*mpDataSource;
};

//template<typename T, int type>
//class DB_SaverFactory : public TaskFactory
//{
//public:
//	virtual ~DB_SaverFactory(){}
//
//	virtual int GetType(void) { return type; }
//	virtual const char* GetTypeName(void) { return "DEFAULT"; }
//
//public:
//	virtual HandTask NewTask(void) 
//	{
//		if (!mTaskList.empty())
//		{
//			HandTask t = mTaskList.back();
//			t->SetFinish(false);
//			t->SetFree(false);
//			mTaskList.pop_back();
//			return t;
//		}
//		
//		//for (int i=0; i<mTaskList.size(); ++i)
//		//{
//		//	if (mTaskList[i] && mTaskList[i]->IsFree())
//		//	{
//  //              mTaskList[i]->SetFree(false);
//  //              mTaskList[i]->SetFinish(false);
//		//		return mTaskList[i];
//		//	}
//		//}
//		
//		HandTask t = MEM_NEW T();
//		Hand<DataTask> task = t;
//		task->mpFactory = this;
//		//mTaskList.push_back(t);
//
//		return t;
//	}
//
//public:
//	virtual void OnTaskEnd(HandTask	hTask) 
//	{
//		mTaskList.push_back(hTask);
//	}
//
//public:
//	virtual void OnClearUp(void) {}
//
//public:
//	Array<HandTask>		mTaskList;
//};

//-------------------------------------------------------------------------
// 用于DB结构记录保存
// 
//-------------------------------------------------------------------------
class MemoryDB_Export DB_SaveRecordData : public DataTask
{
public:
	virtual void Execute(TaskManager *pTaskMgr);

	virtual void InitFree()
	{
		DataTask::InitFree();
		mRecordData.clear(false);
		mSqlString = "";
		mInsertSqlString = "";
	}

public:
	AString				mSqlString;
	AString				mInsertSqlString;
	Array<AutoData>		mRecordData;
};
//-------------------------------------------------------------------------
class DB_LoadAllRecord_SQL : public DataTask
{
public:
	DB_LoadAllRecord_SQL()
	{

	}
	~DB_LoadAllRecord_SQL()
	{
		InitFree();
	}

public:
	virtual void Execute(TaskManager *pTaskMgr);

public:
	virtual void OnFinished(TaskManager *p)
	{
		mFinishCall.run(NULL, NULL, 0); // (DBOperate*)this, mErrorInfo.empty());
	}

	virtual void InitFree()
	{
		DataTask::InitFree();
		mErrorInfo = "";
		mTargetTable.setNull();
		mFinishCall.cleanup();
	}

public:
	ABaseTable		mTargetTable;
	AString			mErrorInfo;
	LOAD_RECORD_CALLBACK		mFinishCall;
};
//-------------------------------------------------------------------------
class MemoryDB_Export DB_RunSqlGetData_SQL : public DataTask
{
public:
	virtual void Execute(TaskManager *pTaskMgr); 

public:
	virtual void OnFinished(TaskManager *p)
	{
		if (mErrorInfo!="")
			ERROR_LOG("DB SQL [%s]>%s", mSqlString.c_str(), mErrorInfo.c_str());
		mFinishCall.run(mResultData.getPtr(), mErrorInfo=="");
	}

	virtual void InitFree()
	{
		DataTask::InitFree();
		mErrorInfo = "";
		mFinishCall.cleanup();
	}

public:
	AutoNice		mResultData;
	AString			mErrorInfo;
	AString			mSqlString;
	DBResultCallBack		mFinishCall;
};

//-------------------------------------------------------------------------
// 本地DB文件落地保存和删除
// DB文件存储器内必须Hash索引记录KEY对应的记录位置
// 因为新建记录无法与同步到主线中保存的记录位置
//-------------------------------------------------------------------------*/
class DB_SaveDataForLocalDB : public DataTask
{
public:
	virtual void Execute(TaskManager *pTaskMgr)
	{
		LocalDataSource *mpDataSource = dynamic_cast<LocalDataSource*>(pTaskMgr);
		if (mpDataSource==NULL)
		{
			ERROR_LOG("存储器当前未初始成功");			
			return;
		}
		UInt64 x = 0;
		if (!mpDataSource->GetFileDBSaver()->SaveRecordData(x, mRecordKey.c_str(), &mRecordData, mbNewInsert))
			ERROR_LOG("同步到DB文件失败");
	}

	virtual void InitFree()
	{
		DataTask::InitFree();
		mRecordKey.setNull();

		mbNewInsert = false;
	}

public:
	bool				mbNewInsert;
	AString				mRecordKey;
	RecordData			mRecordData;
};

class DB_DeleteRecordForLocalDB : public DataTask
{
public:
	virtual void Execute(TaskManager *pTaskMgr)
	{
		LocalDataSource *mpDataSource = dynamic_cast<LocalDataSource*>(pTaskMgr);
		if (mpDataSource==NULL)
		{
			ERROR_LOG("存储器当前未初始成功");			
			return;
		}
		mpDataSource->GetFileDBSaver()->DelectData(mRecordKey.c_str(), 0);
	}

public:
	AString				mRecordKey;
};

#endif //_INCLUDE_DATASOURCEOPERATE_H_