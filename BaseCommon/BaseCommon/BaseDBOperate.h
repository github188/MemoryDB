
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
	// DB 线程内做的事情
	virtual void Execute(TaskManager *pTaskMgr)
	{
		if (!mErrorInfo.empty())
		{
			TableTool::Log("XXX Error: DB 操作错误 : [%s]", mErrorInfo.c_str());
		}
	}

	void OnEnd(TaskManager *pTaskMgr)
	{
		mFinishCallBack.run(this, mErrorInfo.empty());

		InitClearUp();
		SetFinish(true);
		// 目前不进行重用优化
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
	AString					mTableName;			// DB表格名, 只用与DB线程操作DB表

public:
	AString					mTableIndex;
	AString					mWhereString;

	AString					mErrorInfo;
	DBErrorType             mErrorType;


public:
	// for Redis DB reply
	AString					mReplyString;

public:
	// 完成后的数据
	AutoRecord				mResultRecord;		// 查询或创建结果记录
	AutoTable				mResultTable;		// 新建的结果表格
	AutoNice				mResultNiceData;	// 执行存储过程后的结果数据
	HandDataBuffer			mResultData;		// 取多条记录时, 结果表格序列化数据, 或大记录二进制数据

	DBCallBack				mFinishCallBack;	// 完成后回调函数

protected:
	bool					mFinish;
};

typedef Hand<DBOperate>		HandDBOperate;


#endif  //_INCLUDE_BASEDBOPERATE_H_