
#include "TaskManager.h"
#include "TableTool.h"

#include <stdarg.h>
#if __WINDOWS__
#	include <Windows.h>
#endif

#include "TimeManager.h"
//------------------------------------------------------------------------------
TaskManager::TaskManager() 
	: mHaveWillTask(false)
	, m_bHaveFinish(false)
	, m_bBackListEmpty(true)
	, mNowTaskCount(0)
{
	mWaitEvent = event_create(false, true);
	mTaskFactoryMap.resize(INIT_TASTFACTORY_COUNT);
//#if __WINDOWS__
//	mWaitEvent = (int)::CreateEvent(NULL, FALSE, FALSE, NULL);
//#endif
	//start();
}
//------------------------------------------------------------------------------
TaskManager::~TaskManager()
{
	Close();
	event_destroy(mWaitEvent);
	
//#if __WINDOWS__
//	::CloseHandle((HANDLE)mWaitEvent);
//	mWaitEvent = 0;
//#endif
}


//------------------------------------------------------------------------------
void TaskManager::Close(void)
{
	mClose = true;
	event_set(mWaitEvent);
	//stop();		
	WorkThread::Close();

	Lock();
	mMainThreadWaitTaskList.clear();
	mBackRunTaskList.clear();
	mBackThreadWaitFinishedList.clear();
	mMainThreadRunFinishList.clear();
	mMainThreadMoveToBackList.clear();
	for (size_t i = 0; i<mTaskFactoryMap.size(); ++i)
	{
		if (mTaskFactoryMap[i])
		{
			mTaskFactoryMap[i]->OnClearUp();
			mTaskFactoryMap[i]._free();		
		}
	}
	mTaskFactoryMap.clear();
	UnLock();

//#if __WINDOWS__
//	::SetEvent((HANDLE)mWaitEvent);
//#endif


}
//------------------------------------------------------------------------------
void TaskManager::RegisterTask( AutoTaskFactory hTaskFactory )
{
	int taskType = hTaskFactory->GetType();
	if 	(taskType<(int)mTaskFactoryMap.size())
	{
		if (mTaskFactoryMap[taskType])
		{
			Log("Warn: already exist task factory>>>[%d], now replace", taskType);
		}
	}
	else
	{
		size_t count = mTaskFactoryMap.size() * 2;
		while (taskType>=(int)count)
		{
			count *= 2;
		}
		mTaskFactoryMap.resize(count);			
	}
	mTaskFactoryMap[taskType] = hTaskFactory;
	Log("Info: Succeed register task [%d]", taskType);
}
//------------------------------------------------------------------------------

// Note: run in main thread. 产生一个新的任务, 同时会加入到任务将要执行列表中.
HandTask TaskManager::StartTask(int taskType)
{
	AutoTaskFactory hF = GetTaskFactory(taskType);
	if (hF)
	{
		HandTask  hTask = hF->NewTask();
#if	DEBUG_FOR_TASK
		static __declspec(thread) size_t code = 0;
		hTask->mCode = ++code;
#endif
		// 注意, 这里加入在列表的最前面, 但在转移到后台列表时, 
		// 又从最前面开始读取, 然后再加到后台执行的列表的最前面, 所以后产生的任务,还是会被放在执行列表的最后面
		mMainThreadWaitTaskList.push(hTask);

		mHaveWillTask = true;

		++mNowTaskCount;

		return hTask;
	}
	return HandTask();
}
//------------------------------------------------------------------------------

// Note: run in main thread
AutoTaskFactory TaskManager::GetTaskFactory(int taskType)
{
	if (taskType>=0 && taskType<(int)mTaskFactoryMap.size())
		return mTaskFactoryMap[taskType];

	return AutoTaskFactory();
}
//------------------------------------------------------------------------------

// main or back
void TaskManager::Log(const char *szInfo, ... )
{
#if DEVELOP_MODE
	va_list va;
	va_start( va, szInfo);
	LOG_TOOL(va, szInfo);
#endif
}
//------------------------------------------------------------------------------


/// Note: run in main thread
void TaskManager::Process(void)
{
	bool bHave = false;
	if (mHaveWillTask && m_bBackListEmpty)
	{
		/// 加锁转移任务 1 等待处理->转移到后台列表, 2 后台等待完成的列表->主线执行完成任务列表		
		Lock();
		// step 2 将可以完成列表中的任务[先>后]转移到执行完成列表中
		mMainThreadWaitTaskList.swap(mMainThreadMoveToBackList);
		mHaveWillTask = false;
		m_bBackListEmpty = false;
		UnLock();
				
		event_set(mWaitEvent);
	}
	// step 3 先处理可完成列表中的事件

	if (m_bHaveFinish)
	{
		Lock();
		mMainThreadRunFinishList.swap(mBackThreadWaitFinishedList);
		m_bHaveFinish = false;
		UnLock();

		size_t count = mMainThreadRunFinishList.size();
		Array<HandTask> &list = mMainThreadRunFinishList._getList();
		for (size_t i=0; i<count; ++i)
		{
			HandTask hTask = list[i];
#if DEBUG_FOR_TASK
			Log(">>>>[%u]开始执行任务完成#########", hTask->mCode);
#endif
			--mNowTaskCount;
			hTask->OnFinished(this);
			hTask->mpFactory->OnTaskEnd(hTask);
		}

		mMainThreadRunFinishList.clear();
	}
}

//------------------------------------------------------------------------------


/// run in back thread.
VOID TaskManager::backWorkThread()
{
	while (IsActive())
	{				
		//#if __WINDOWS__
		//		if (mBackRunTaskList.empty())
		//			WaitForSingleObject((HANDLE)mWaitEvent, INFINITE);
		//#endif		
		// step 3 后台执行任务
		bool bHave = false;
		Lock();
		if (!mMainThreadMoveToBackList.empty())
		{
			mBackRunTaskList.swap(mMainThreadMoveToBackList);
			bHave = true;
			m_bBackListEmpty = true;
		}
		UnLock();

		if (bHave)						
		{			
			size_t count = mBackRunTaskList.size();
			Array<HandTask> &list = mBackRunTaskList._getList();
			for (size_t i=0; i<count; ++i)
			{
				HandTask &hTask = list[i];
				if (!hTask)
				{
					continue;
				}
#if DEBUG_FOR_TASK
				Log(">>>>[%u]后台执行任务############", hTask->mCode);
#endif
				try{
					hTask->Execute(this);		
				}
				catch (...)
				{
					ERROR_LOG("任务执行异常 >[%d][%s]", hTask->mpFactory->GetType(), hTask->mpFactory->GetTypeName());
				}
				Lock();
				mBackThreadWaitFinishedList.push(hTask);
				hTask.setNull();
				m_bHaveFinish = true;
				UnLock();
			}	
			mBackRunTaskList.clear();
		}
		else
		{
			//TimeManager::Sleep(100);
			/// 先停止此功能				
			event_timedwait(mWaitEvent, 6000);
		}
		backProcess();
	}
}


void TaskManager::onBeforeClose(void)
{
	event_set(mWaitEvent);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
