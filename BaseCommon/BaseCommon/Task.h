
#ifndef _INCLUDE_TASK_H_
#define _INCLUDE_TASK_H_

#include "Auto.h"
#include "EasyStack.h"

#define  DEBUG_FOR_TASK		0

class TaskManager;
class TaskFactory;

class Task : public AutoBase
{
	friend class TaskManager;
	friend class TaskFactory;
public:
	virtual void InitFree() = 0;
	// back thread run
	virtual void Execute(TaskManager *pTaskMgr) = 0;

	// main thread run
	virtual void OnFinished(TaskManager *pTaskMgr) = 0;

public:
	Task()
		:mpFactory(NULL)
	{	}

	Auto<Task> GetSelf(){ return Auto<Task>(this); }

protected:
#if DEBUG_FOR_TASK
	size_t		mCode;
#endif
	TaskFactory	*mpFactory;
};

typedef Auto<Task>		HandTask;


#endif //_INCLUDE_TASK_H_