#ifndef _BACKTREADMGR_H_
#define _BACKTREADMGR_H_

#include "BackTask.h"
#include "PoolList.h"
#include "WorkThread.h"

#include "Lock.h"

typedef PoolList<AutoTask>		TaskList;
// 后台执行任务管理对象
class BaseCommon_Export BackThreadMgr : public WorkThread 
{
public:
	BackThreadMgr();
	virtual ~BackThreadMgr();

	void SetOnceTime(size_t spaceTime){ mOnceTime = spaceTime; }

public:
	// 加入任务
	void AppendTask( AutoTask  hTask );
	void RemoveTask( AutoTask hTask );
	void Process(void);

protected:

	virtual void backWorkTread(void);

	virtual void waitTime();

protected:
	TaskList      mTaskList;
	// 对于主线增减任务,更新操作, 在线程处理开始时检查并合并到mTaskList
	TaskList		mAssistList;	
	TaskList		mAppendList;
	TaskList		mRemoveList;

	CsLock	mTaskListLock;
	CsLock	mAssistLock;


	bool	mNeedUpdateTaskList;

	size_t	mBeginTime;
	size_t	mRunTime;
	size_t	mOnceTime;

public:
	size_t	mFrameTime;
	size_t  mCurrentTime;

};

#endif   //_BACKTREADMGR_H_