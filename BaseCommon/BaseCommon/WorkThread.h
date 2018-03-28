#ifndef _WORKTHREAD_H_
#define _WORKTHREAD_H_

#include "BaseCommon.h"
#include "MemBase.h"
#include <string>

class BaseCommon_Export WorkThread : public MemBase
{
public:
	WorkThread();
	virtual ~WorkThread()
	{
		Close();	
	}

	virtual void InitThread(void);
	 
	virtual void Close(void);

	virtual void Suspend();

	virtual bool Resume();
    
    virtual bool IsThreadQuit();
    
#if __WINDOWS__
	bool _ForceClose(size_t reVal);
#else
    bool _ForceClose(VOID* reVal);
#endif

	bool IsActive(){ return !mClose; }
	void WaitTheadFinish();
	UINT GetThreadID(){ return mThreadID; }

public:
	virtual void backWorkThread(void) = 0;
	virtual void onBeforeClose(void) {};
	virtual void onExceptionExist(){}

protected:
#if _WIN64
	unsigned __int64	mBackTread;
#else
	unsigned int		mBackTread;             // 后台线程句柄
#endif
	bool				mClose;                 // 是否终止

#if __WINDOWS__
	UINT				mThreadID;
#else if __LINUX__
	UInt64				mThreadID;
#endif
	std::string				mDebugName;
};


#endif //_WORKTHREAD_H_