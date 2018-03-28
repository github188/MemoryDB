

#include "BaseThread.h"

#include "TableManager.h"

#include "SocketInputStream.h"
#include "SocketOutputStream.h"

#include "BaseNetManager.h"
#include "Assertx.h"

#include "NetConnect.h"

#include "EventFactory.h"

#include <Windows.h>


#define  NEED_SAFT_CHECK		0
//--------------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------------
BaseThread::BaseThread(const char *threadIndexName)
: mActive(true)	
, mShareNodePool(MEM_NEW NodePool())
, mRunCount(0)
, mQuestClose(false)
, mCloseEvent(NULL)
, mbStop(false)
, mbClosing(false)
{	
	//mTimeManager.Init();
	mBeginTime =TimeManager::NowTick();

	mShareNodePool->setBlockSize(sizeof(PoolList<AObject>::ListNode));
	
	//CRand::SetSpeed (TimeManager::Now()&0xFFFFFFFF, TimeManager::NowTick());
    CRand::SetSeed((uint)TimeManager::NowTick());
}
//--------------------------------------------------------------------------------------------------------

BaseThread::~BaseThread()
{
	_Release();

	mShareNodePool._free();
}
//--------------------------------------------------------------------------------------------------------

VOID BaseThread::run( void )
{
	//_set_se_translator(throw_exception);
	mbClosing = false;
	Log("Thread *%d* begin ready", mThreadID);
	mCloseEvent = ::CreateEvent(NULL, FALSE, FALSE, "EXIT_SERVER");

	OnStart(NULL);
	if (mQuestClose)
	{
		mbStop = true;
		Log("逻辑线程初始开始失败，直接退出");
		return;
	}
	Log("Thread *%d* start ", mThreadID );


	mBeginTime = TimeManager::NowTick();

	while (IsActive())
	{
		if (mQuestClose)
		{
			mQuestClose = false;
			mbClosing = true;
			if (NotifyThreadClose())
			{
				setActive(false);
				break;
			}
			mbClosing = false;
		}

		try{
			if (mMainEventCenter)
				mMainEventCenter->ProcessEvent();
			Process(NULL);
		}
		catch (std::exception &e)
		{
			ERROR_LOG("Thread [%d] process error >%s", mThreadID, e.what());
		}
		catch (...)
		{
			ERROR_LOG("Thread [%d] process error, Please check dump file", mThreadID);
		}

		++mRunCount;
		OnOnceLoopEnd();
	}
	Log("Thread *%d* stoping ", mThreadID);
	OnStop(NULL);
	_Release();
	Log("Thread *%d* close ok", mThreadID);

	mbStop = true;
}

// 初始网络,可重复调用此功能,为当前线程准备多个网络连接或服务
//bool BaseThread::AppendNet(tNetHandle *pServerTool)
//{
//	AssertEx(pServerTool, "网络注册工具为空"); 
//	tNetHandle *hNetTool = pServerTool;
//	if ( hNetTool )
//	{
//		// 注意, 这里是允许插入多个相同主键的元素
//		mNetMap.insert(MAKE_INDEX_ID(pServerTool->GetIndexName()), pServerTool);
//	}
//	return NULL!=hNetTool;
//}

void BaseThread::Log(const char* szInfo, ...)
{
#if DEVELOP_MODE
	va_list va;
	va_start(va, szInfo);
	LOG_TOOL(va, szInfo);
#endif
}



void BaseThread::_Release( void )
{
	mMainEventCenter.setNull();
	mShareNodePool._free();
}


int BaseThread::OnceTime( void ) const
{
	return 10;
}

void BaseThread::InitEventCenter( AutoEventCenter eventCenter )
{
	//AssertEx (eventCenter, "EventCenter is null.");

	mMainEventCenter = eventCenter;
}

void BaseThread::Close()
{
	mQuestClose = true;
	WorkThread::Close();
}

void BaseThread::CanClose()
{
	SetEvent(mCloseEvent);
}


void BaseThread::onExceptionExist()
{
	NotifyThreadClose();
	TABLE_LOG("ERROR: [%d]线程异常退出, 进程退出处理, 请检查DUMP信息", mThreadID);
	TableTool::SetLog(NULL);
	::exit(9999);
}

void BaseThread::OnOnceLoopEnd()
{
	if (OnceTime()>0)
	{	
		UInt64 n = TimeManager::NowTick();
		if (n>=mBeginTime)
		{
			int lastTime = (int)(mRunCount * OnceTime() - (n-mBeginTime));
			if (lastTime>0)
				TimeManager::Sleep(lastTime);
		}
		else
		{
			mBeginTime = n;
			mRunCount = 0;
		}
	}
}

