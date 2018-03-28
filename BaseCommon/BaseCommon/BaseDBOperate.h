
#ifndef _INCLUDE_BASEDBOPERATE_H_
#define _INCLUDE_BASEDBOPERATE_H_

#include "Task.h"

class DBOperate : public Task
{
public:
	DBOperate();

public:
	virtual bool NeedImmediatelyFinish() { return true; }
	virtual void OnBegin(TaskManager *pTaskMgr);

public:
	//FOR Redis DB
	virtual bool Run(){ return false; }
	virtual void OnDBFinish(AString &replyResult, ReplyData &dataList, AString &errorInfo){}

public:
	// DB �߳�����������
	virtual void Execute(TaskManager *pTaskMgr)
	{
		if (!mErrorInfo.empty())
		{
			TableTool::Log("XXX Error: DB �������� : [%s]", mErrorInfo.c_str());
		}
	}

	void OnEnd(TaskManager *pTaskMgr)
	{
		mFinishCallBack.run(this, mErrorInfo.empty());

		InitClearUp();
		SetFinish(true);
		// Ŀǰ�����������Ż�
		//AssertEx(mFactory, "Error: self task factory is null, why no exist.");
		//mFactory->OnTaskEnd(GetSelf());	 //!!! Need test may be bug
	}

	void InitClearUp( void )
	{
		SetFinish(false);
		mErrorType = DB_NO_ERROR;
		mResultTable.setNull();
		mResultNiceData.setNull();
		mResultRecord.setNull();
		mResultData.setNull();

		mTableName.setNull();
		mTableIndex.setNull();
		mErrorInfo.setNull();

		mWorkThreadMgr = NULL;

		mFinishCallBack.cleanup();
	}


	virtual bool IsFinish(void) { return mFinish; }

	virtual void SetFinish(bool bFinish) { mFinish = bFinish; }
	virtual bool AutoFinish(void) { return true; }	

	virtual void Lock() {  }
	virtual void UnLock(){  }
	virtual bool TryLock(){ return true; }

public:
	virtual void Finish(void){ SetFinish(true); }
	virtual void ReadyToDBRecordData(AutoRecord hRecord) {}
	virtual void InitClearUp(void);

public:
	DataTableManager* GetUseDataMgr(void);

	DBTableManager*  GetDBDataMgr(void);

	GameDBTable* GetDBTable(void);

	UseDBTable* GetMainThreadTable();

	bool HasError(void){ return !mErrorInfo.empty(); }

	void SetErrorInfo( const char *szErrorString, bool bAppend = true );

	void Log( const char* szInfo, ... );

public:
	AString					mTableName;			// DB�����, ֻ����DB�̲߳���DB��

public:
	AString					mTableIndex;
	AString					mWhereString;

	AString					mErrorInfo;
	DBErrorType             mErrorType;


public:
	// for Redis DB reply
	AString					mReplyString;

public:
	// ��ɺ������
	AutoRecord				mResultRecord;		// ��ѯ�򴴽������¼
	AutoTable				mResultTable;		// �½��Ľ�����
	AutoNice				mResultNiceData;	// ִ�д洢���̺�Ľ������
	HandDataBuffer			mResultData;		// ȡ������¼ʱ, ���������л�����, ����¼����������

	DBCallBack				mFinishCallBack;	// ��ɺ�ص�����

protected:
	bool					mFinish;
};

typedef Hand<DBOperate>		HandDBOperate;


#endif  //_INCLUDE_BASEDBOPERATE_H_