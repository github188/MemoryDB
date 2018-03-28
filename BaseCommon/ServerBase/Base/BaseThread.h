/********************************************************************
	created:	2013/09/07
	created:	7:9:2013   11:19
	filename: 	C:\Work\BaseCommon\ServerBase\Base\BaseThread.h
	file path:	C:\Work\BaseCommon\ServerBase\Base
	file base:	BaseThread
	file ext:	h
	author:		Yang WenGe
	
	purpose:	工作线程, 一般处理主逻辑, 为了在一个进程内实现多个工作线程

	NOTE:		1 正常关闭: NotifyThreadClose() 时行清理工作
				2 如果有需要等待, 则不能调用此对象的 NotifyThreadClose 或 CanClose
				3 等待事件完成后, 调用 CanClose 完成线程正常关闭
*********************************************************************/
#ifndef _INCLUDE_BASETHREAD_H_
#define _INCLUDE_BASETHREAD_H_

#include "WorkThread.h"

#include "EventCenter.h"
//#include "NetHandle.h"
#include "ServerBaseHead.h"

#include "TimeManager.h"

namespace Logic
{
	class tEventCenterManager;
}

class tSafeCheck;
class tNetHandle;
class tDBManager;
class DataTableManager;
//-------------------------------------------------------------------------
class ServerBase_Dll_Export BaseThread : public WorkThread
{
	//typedef EasyMap<int, AutoNet>	NetMap;

public:
	virtual void Process(void*) = 0;
	virtual void OnStart(void*) = 0;
	virtual void OnStop(void*) = 0;
	virtual void Log(const char* szInfo, ...);

	virtual int OnceTime(void) const;

	virtual tDBManager* GetDBWork() { return NULL; }

protected:
	// 进行准备关闭动作 NOTE: 此功能必须只由线程循环内调用
	virtual bool NotifyThreadClose(){  CanClose(); return true; }

	virtual void onExceptionExist();

	virtual void OnOnceLoopEnd();

public:
	BaseThread(const char *threadIndexName);
	virtual ~BaseThread();

	virtual void InitEventCenter(AutoEventCenter eventCenter);
	AutoEventCenter GetMainEventCenter(void) const { return mMainEventCenter; }


public:
	virtual void start() { InitThread(); }

	virtual VOID run(void);

	virtual void _Release(void);

	bool IsActive(void)
	{
		return mActive;
	}

	virtual void setActive(bool bActive){ mActive = bActive; }

    virtual void OnDBReadySucceed(tDBManager *dbMgr) {  Log("DB 启动成功...\n*************************************************************"); }

	// 正常关闭, 正确处理释放
	virtual void Close();

	virtual void CanClose();

	virtual bool IsStop() const { return mbStop; }
	virtual bool IsClosing() const { return mbClosing; }

public:
	virtual void backWorkThread(void) { run(); }

public:
	AutoNodePool GetShareNodePool(void){ return mShareNodePool; }

protected:	
	bool									mActive;
	bool									mbClosing;
	AutoEventCenter				mMainEventCenter;			// 当前线程公用的主要事件中心, 如数据库及全局交互
	AutoNodePool				mShareNodePool;				// 共享使用的对象节点池

	UInt64								mBeginTime;					// 从开机到当前的总时间
	UInt64								mRunCount;

	//TimeManager					mTimeManager;


	HANDLE						mCloseEvent;


public:
	bool						mQuestClose;
	bool						mbStop;

};
//-------------------------------------------------------------------------

#endif //_INCLUDE_BASETHREAD_H_