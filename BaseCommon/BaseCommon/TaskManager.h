/********************************************************************
	created:	2013/08/24
	created:	24:8:2013   13:41
	filename: 	C:\Work\BaseCommon\BaseCommon\TaskManager.h
	file path:	C:\Work\BaseCommon\BaseCommon
	file base:	TaskManager
	file ext:	h
	author:		Yang WenGe
	
	purpose:	任务线程管理
			简化任务机制，所有任务在后台执行一次后送回主线 2016.2.28
*********************************************************************/

#ifndef _INCLUDE_TASKMANAGER_H_
#define _INCLUDE_TASKMANAGER_H_

#include "Array.h"
#include "PoolList.h"
#include "EasyStack.h"
#include "EasyMap.h"
#include "WorkThread.h"
#include "Task.h"
#include "TaskFactory.h"
#include "BaseCommon.h"

#include "ThreadEvent.h"
#include "Lock.h"
//---------------------------------------------------------------------------------

#define INIT_TASTFACTORY_COUNT		16


typedef PoolList<HandTask>			PoolTaskList;
typedef Array<AutoTaskFactory>		TaskFactoryMap;
typedef EasyStack<HandTask>		TaskList;

//---------------------------------------------------------------------------------
// 后台任务线程管理
// NOTE: 1 任务数量一次处理比较多时, 任务完成等待时间会比较长, 可使用任务立即完成属性解决
//		 2 如果后台不存在执行任务, 则进入线程休眠 event_wait(mWaitEvent);
//---------------------------------------------------------------------------------
class BaseCommon_Export TaskManager : public WorkThread
{
public:
	TaskManager();
	virtual ~TaskManager();

public:
	virtual void Close(void);
	
	// 如果在一个处理周期内时间大于此值, 则立即进入下一循环
	virtual int	OnceProcessOverTime(){ return 10; }

	virtual int GetNowTaskCount(){ return mNowTaskCount; }

public:
	// Note: run in main thread
	virtual void RegisterTask( AutoTaskFactory hTaskFactory );

	// Note: run in main thread. 产生一个新的任务, 同时会加入到任务将要执行列表中.
	virtual HandTask StartTask(int taskType);

	// Note: run in main thread
	virtual AutoTaskFactory GetTaskFactory(int taskType);
	// main or back
	virtual void Log(const char *szInfo, ... );

public:
	/// Note: run in main thread
	virtual void Process(void);

public:
	/// run in back thread.
	virtual VOID backWorkThread ();

	//NOTE: 此调用一般会 6秒调用一次 event_timedwait(mWaitEvent, 6000)
	virtual void backProcess(){}

	virtual void onBeforeClose(void);;

protected:
	void Lock(void){ mLock.lock(); }
	void UnLock(void){ mLock.unlock(); }
	bool TryLock(void){ return mLock.trylock(); }

protected:
	//bool						mActive;
	bool						mHaveWillTask;
	bool						m_bNeedProcess;
	bool						m_bHaveFinish;
	bool						m_bBackListEmpty;
	CsLock						mLock;	

	
	// 1 主线加锁后, 先处理可完成列表中的事件,或转移到完成列表中 2 再将追加的任务转移到后台要处理的列表中.
	// 2 后台线程加锁后, 先从执行列表中,检查出可以完成的任务,并移动到完成列表中, 然后再把要执行的任务加进执行列表
	TaskList					mMainThreadWaitTaskList;			// 主线追加的任务临时列表,不加锁追加
	TaskList					mMainThreadMoveToBackList;			// 主线再加锁后从临时列表中移动到这个列表
	TaskList					mBackRunTaskList;					// 后台线程加锁后,先从这个列表中检查出完成的任务, 加到主线完成列表中, 然后再从主线要移动的列表中加入

	TaskList					mBackThreadWaitFinishedList;		// 后台加锁后可以完成的列表
	TaskList					mMainThreadRunFinishList;			// 主线要处理的完成的任务

	TaskFactoryMap				mTaskFactoryMap;					// 任务工厂映射表, 为了效率,当前使用数组

	event_handle				mWaitEvent;

	int							mNowTaskCount;
};


//---------------------------------------------------------------------------------


#endif //_INCLUDE_TASKMANAGER_H_