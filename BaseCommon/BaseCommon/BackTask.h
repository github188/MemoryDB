
#ifndef _BACKTASK_H_
#define _BACKTASK_H_

#include "AutoPtr.h"
#include "MemBase.h"

//-------------------------------------------------------------------------------------------
class BackThreadMgr;

// 后台执行任务结构
class BaseCommon_Export BackTask : public MemBase
{
public:
	BackTask()
		: mFinish(false)
	{

	}
	virtual ~BackTask(){}
	
	void Release(void);

	virtual bool autoFinish(void) const = 0;
	virtual bool removeOnFinish() const = 0;

	void SetFinish(bool finish){ mFinish = finish; }
	bool IsFinish(void){ return mFinish; }

	void Finish(void){ SetFinish(true); }

public:
	virtual void ExeBack(BackThreadMgr *mgr) = 0;	
	virtual void ExeFinished() = 0;

	virtual bool Lock() = 0;
	virtual bool UnLock() = 0;
	virtual bool TryLock() = 0;

protected:
	bool mFinish;
};

template<>
void AutoPtr<BackTask>::FreeClass()
{
	getPtr()->Release();
}

typedef AutoPtr<BackTask>		AutoTask;


#endif //_BACKTASK_H_