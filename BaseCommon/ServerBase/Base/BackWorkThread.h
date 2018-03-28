
#ifndef _INCLUDE_BACKWORKTHREAD_H_
#define _INCLUDE_BACKWORKTHREAD_H_

#include "DataBuffer.h"

class ExecuteWorker;

class BackWorkThread : public Thread
{
public:
	BackWorkThread()
	{
	}
	virtual ~BackWorkThread()
	{
	}

public:
	bool RegisterExecuteWorker(int workerType, ExecuteWorker*)
	{

	}


protected:
	CsLock						mLock;	
	Array<ExecuteWorker*>		mExecuteWorkerList;

	AutoData					mBackWorkDataList;
	AutoData					mMainWorkDataList;
}

#endif //_INCLUDE_BACKWORKTHREAD_H_