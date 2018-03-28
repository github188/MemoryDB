
#ifndef _INCLUDE_LOGTHREAD_H_
#define _INCLUDE_LOGTHREAD_H_

#include "WorkThread.h"
#include "Lock.h"
#include <list>
#include "ThreadEvent.h"

struct LogTask
{
public:
	char szLogBuffer[LOG_INFO_MAX_LENGTH];

	bool mbPrint;
};
//-------------------------------------------------------------------------
class LogThreadManager : public WorkThread
{
	friend class ThreadLog;
public:
	LogThreadManager()
		: mpLogFile(NULL)
	{
		mWaitEvent = event_create(false, true);
	}

	virtual ~LogThreadManager()
	{
		Close();
		if (mpLogFile!=NULL)
			fclose(mpLogFile);
		mpLogFile = NULL;

		for (std::list<LogTask*>::iterator it=mLogPool.begin(); it!=mLogPool.end(); ++it)
		{
			delete *it;
		}
		mLogPool.clear();

		for (std::list<LogTask*>::iterator it=mFreeList.begin(); it!=mFreeList.end(); ++it)
		{
			delete *it;
		}
		mFreeList.clear();

	}

public:
	void AppendLog(LogTask *logTask)
	{
		mLogListLock.lock();
		mLogList.push_back(logTask);
		mLogListLock.unlock();
		event_set(mWaitEvent);
	}

	LogTask* AlloctLogTask()
	{
		CsLockTool  t(mFreeListLock);

		if (mFreeList.empty())
		{
			mPoolLock.lock();
			mFreeList.swap(mLogPool);
			mPoolLock.unlock();
		}

		if (!mFreeList.empty())
		{
			LogTask *p = *mFreeList.begin();
			mFreeList.pop_front();
			memset(p->szLogBuffer, 0, LOG_INFO_MAX_LENGTH);
			//printf(" @ last count [%u]\n", mFreeList.size());
			return p;
		}

		return new LogTask();
	}

	virtual bool needWait(){ return true; }

public:
	virtual void backWorkThread(void)
	{
		while (IsActive())
		{
			if (needWait())
				event_timedwait(mWaitEvent, 10000);

			std::list<LogTask*> tempList;
			mLogListLock.lock();
			tempList.swap(mLogList);
			mLogListLock.unlock();

			for (std::list<LogTask*>::iterator it=tempList.begin(); it!=tempList.end(); ++it)
			{
//#if __SERVER__ && __WINDOWS__ && !DEVELOP_MODE
				if ((*it)->mbPrint)
					printf((*it)->szLogBuffer);
//#endif
				__writeLog((*it)->szLogBuffer);
				//delete *it;
			}
			if (mpLogFile!=NULL)
			{
				fflush(mpLogFile);
			}
			mPoolLock.lock();

			for (std::list<LogTask*>::iterator it=tempList.begin(); it!=tempList.end(); ++it)
			{
				mLogPool.push_back(*it);
			}
			mPoolLock.unlock();
		}
	}

	virtual void onBeforeClose(void) 
	{
		event_set(mWaitEvent);
	}

	virtual void __writeLog(const char *szLog)
	{
		if (mpLogFile!=NULL)
		{
			fprintf(mpLogFile, "%s", szLog);
			//fflush(mpLogFile);
		}
	}

protected:
	FILE					*mpLogFile;
	CsLock					mLogListLock;
	std::list<LogTask*>		mLogList;	
	event_handle			mWaitEvent;

	std::list<LogTask*>		mLogPool;
	CsLock					mPoolLock;

	std::list<LogTask*>		mFreeList;

	CsLock					mFreeListLock;
};



#endif //_INCLUDE_LOGTHREAD_H_