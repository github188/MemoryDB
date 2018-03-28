
#include "BackThreadMgr.h"

#ifndef __LINUX__
#	include <Windows.h>
#endif

#include "TableTool.h"

#include "TimeManager.h"


BackThreadMgr::BackThreadMgr()
	: mCurrentTime(0)
	, mFrameTime(0)
{
	mClose = false;
	mNeedUpdateTaskList = false;
	mBeginTime = TimeManager::NowTick();
	mRunTime = 0;
	mOnceTime = 100;
	//InitThread();
}
BackThreadMgr::~BackThreadMgr()
{
	WaitTheadFinish();
}


void BackThreadMgr::backWorkTread(void)
{	
	while ( !mClose )
	{
		waitTime();
		if (mNeedUpdateTaskList)
		{
			mAssistLock.lock();
			mNeedUpdateTaskList = false;
			mTaskList.append(mAppendList);
			mAppendList.clear();
			if (!mRemoveList.empty())
			{
				for (TaskList::iterator it=mRemoveList.begin(); it!=mRemoveList.end(); ++it)
				{
					mTaskList.remove(*it);
				}
				mRemoveList.clear();
			}
			mAssistLock.unlock();
		}
		if (!mTaskList.empty())
		{			
			for (TaskList::iterator it=mTaskList.begin(); it!=mTaskList.end(); )
			{
				AutoTask  hTask = *it;
				hTask->Lock();
				if (!hTask->IsFinish())
				{
					hTask->ExeBack(this);

					if (hTask->autoFinish())
						hTask->Finish();
				}
				else if (hTask->removeOnFinish()) 
				{
					it = mTaskList.erase(it); 
					hTask->UnLock(); 
					continue;
				}

				hTask->UnLock(); 
				++it;
			}			
		}	
	}
}


void BackThreadMgr::AppendTask( AutoTask hTask )
{
	mAssistList.push_back(hTask);
	mAssistLock.lock();
	mAppendList.push_back(hTask);
	mNeedUpdateTaskList = true;
	mAssistLock.unlock();
}
void BackThreadMgr::RemoveTask( AutoTask hTask )
{
	mAssistList.remove(hTask);
	mAssistLock.lock();
	mRemoveList.push_back(hTask);
	mNeedUpdateTaskList = true;
	mAssistLock.unlock();
}


void BackThreadMgr::Process( void )
{
	if (!mAssistList.empty())
	{			
		for (TaskList::iterator it=mAssistList.begin(); it!=mAssistList.end(); )
		{
			if ((*it)->IsFinish())
			{
				if ((*it)->TryLock())
				{
					(*it)->ExeFinished();
					if ((*it)->removeOnFinish())
					{
						(*it)->UnLock();
						it = mAssistList.erase(it);
						continue;
					}
					(*it)->SetFinish(false);
					(*it)->UnLock();
				}
			}
			++it;
		}
	}
}

void BackThreadMgr::waitTime()
{
//#ifndef __LINUX__
	size_t temp = TimeManager::NowTick();
	if (temp<mBeginTime)
	{
		mBeginTime = temp;
		mRunTime = 0;
		mCurrentTime = temp;
		mFrameTime = 1;
		return;
	}
	size_t numTime = mOnceTime * mRunTime;
	size_t alreadyTime = temp-mBeginTime;
	if (alreadyTime<numTime)
	{
#if __WINDOWS__
		Sleep(numTime-alreadyTime);
#elif __LINUX__
		delay(numTime-alreadyTime);
#endif

		temp += (numTime-alreadyTime);
	}
	++mRunTime;
	mFrameTime = temp - mCurrentTime;
	mCurrentTime = temp;
//#endif
}

void BackTask::Release(void)
{
	delete this; 
	//this->~BackTask();
	//Allot::freePtr(this);
}