/********************************************************************
	created:	2013/09/07
	created:	7:9:2013   11:19
	filename: 	C:\Work\BaseCommon\ServerBase\Base\BaseThread.h
	file path:	C:\Work\BaseCommon\ServerBase\Base
	file base:	BaseThread
	file ext:	h
	author:		Yang WenGe
	
	purpose:	�����߳�, һ�㴦�����߼�, Ϊ����һ��������ʵ�ֶ�������߳�

	NOTE:		1 �����ر�: NotifyThreadClose() ʱ��������
				2 �������Ҫ�ȴ�, ���ܵ��ô˶���� NotifyThreadClose �� CanClose
				3 �ȴ��¼���ɺ�, ���� CanClose ����߳������ر�
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
	// ����׼���رն��� NOTE: �˹��ܱ���ֻ���߳�ѭ���ڵ���
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

    virtual void OnDBReadySucceed(tDBManager *dbMgr) {  Log("DB �����ɹ�...\n*************************************************************"); }

	// �����ر�, ��ȷ�����ͷ�
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
	AutoEventCenter				mMainEventCenter;			// ��ǰ�̹߳��õ���Ҫ�¼�����, �����ݿ⼰ȫ�ֽ���
	AutoNodePool				mShareNodePool;				// ����ʹ�õĶ���ڵ��

	UInt64								mBeginTime;					// �ӿ�������ǰ����ʱ��
	UInt64								mRunCount;

	//TimeManager					mTimeManager;


	HANDLE						mCloseEvent;


public:
	bool						mQuestClose;
	bool						mbStop;

};
//-------------------------------------------------------------------------

#endif //_INCLUDE_BASETHREAD_H_