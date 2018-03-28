
#include "WorkThread.h"
#include "TableTool.h"

#if __WINDOWS__
#include <process.h>
#include <Windows.h>
#include <exception>
#include "Dump.h"

WorkThread::WorkThread()
{
	mClose = false;
	mBackTread = NULL;

	mDebugName = "Defaul";
}

UINT WINAPI RunWorkThread( LPVOID lpParameter )
{
	try{
		_set_se_translator(throw_exception);
		((WorkThread*)lpParameter)->backWorkThread();
	}
	catch (std::exception &e)
	{
		ERROR_LOG("[%u]线程异常> [%s]", ((WorkThread*)lpParameter)->GetThreadID(), e.what());
		throw e;
		//((WorkThread*)lpParameter)->onExceptionExist();
	}
	//catch (...)
	//{
	//	ERROR_LOG("线程异常退出 >[%u]", ((WorkThread*)lpParameter)->GetThreadID());
	//	((WorkThread*)lpParameter)->onExceptionExist();
	//}
	return 0;
}

void WorkThread::InitThread( void )
{
	if (mBackTread!=NULL)
		Close();

	mClose = false;	
	mBackTread = _beginthreadex( NULL, 0, RunWorkThread, this, 0, &mThreadID );
	GREEN_LOG("[%s] 开启线程 >[%u]", mDebugName.c_str(), mThreadID);
}

void WorkThread::Close( void )
{
	mClose = true;
	if ( mBackTread )
	{
		onBeforeClose();
		if (WaitForSingleObject( (HANDLE)mBackTread, 6000 )!=WAIT_OBJECT_0)
		{
			ERROR_LOG("[%s]正常关闭线程失败>[%u] (关闭超时6秒), 当前强制关闭", mDebugName.c_str(), mThreadID);
			_ForceClose(10);
		}
		CloseHandle((HANDLE)mBackTread);
		mBackTread = NULL;
	}
}


void WorkThread::Suspend()
{
#if __WINDOWS__
	::SuspendThread((HANDLE)mBackTread);
#else
	AssertNote(0, "未实现");
#endif
}


bool WorkThread::Resume()
{
#if __WINDOWS__
	while (true)
	{
		UINT re = ::ResumeThread((HANDLE)mBackTread);
		if (re==0xFFFFFFFF)
		{
			ERROR_LOG("线程恢复失败");
			return false;
		}
		else if (re<=0)
			return true;
	}
#else
	AssertNote(0, "未实现");
#endif
	return false;
}

bool WorkThread::IsThreadQuit()
{
	return mBackTread==NULL;
}

bool WorkThread::_ForceClose( size_t reVal )
{
	if ( mBackTread)
    {
		TerminateThread( (HANDLE)mBackTread, reVal );
        CloseHandle((HANDLE)mBackTread);
		mBackTread = NULL;
    }
    return true;
}
void WorkThread::WaitTheadFinish()
{
	Close();
}
#else

#	if __LINUX__

#include <pthread.h>

WorkThread::WorkThread()
{
	mClose = false;
	mBackTread = NULL;
}

VOID* RunWorkThread( VOID* lpParameter )
{
	((WorkThread*)lpParameter)->backWorkThread();
	return 0;
}

void WorkThread::InitThread( void )
{
    if (mBackTread!=NULL)
        Close();
    
	mClose = false;

	pthread_create( (pthread_t*)&mThreadID, NULL , RunWorkThread , (VOID*)this );
	GREEN_LOG("[%s] 开启线程 >[%u]", mDebugName.c_str(), mThreadID);
}

void WorkThread::Close( void )
{
	mClose = true;
    
    _ForceClose(NULL);
    //if (mThreadID!=0)
    //    pthread_kill((pthread_t)mThreadID, SIGKILL);
    //mThreadID = 0;
	
}

bool WorkThread::IsThreadQuit()
{
    if ( mThreadID==0 )
        return true;
    
    return  pthread_kill((pthread_t)mThreadID, 0)==3;
}


bool WorkThread::_ForceClose(VOID *reVal )
{
    if (!mClose)
    {
        //mClose = true;
        //pthread_cancel((pthread_t)mThreadID);
        pthread_kill((pthread_t)mThreadID, SIGKILL);
        mThreadID = 0;
    }    
    return mThreadID==0;
}

void WorkThread::WaitTheadFinish()
{
	Close();
}

#	endif

#endif