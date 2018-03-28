
#include "Lock.h"

#ifdef	__LINUX__
#	include <pthread.h>
#else
#ifndef WINVER				// 允许使用特定于 Windows XP 或更高版本的功能。
#define WINVER 0x0501		// 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif

#ifndef _WIN32_WINNT		// 允许使用特定于 Windows XP 或更高版本的功能。
#define _WIN32_WINNT 0x0501	// 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif						

#ifndef _WIN32_WINDOWS		// 允许使用特定于 Windows 98 或更高版本的功能。
#define _WIN32_WINDOWS 0x0410 // 将此值更改为适当的值，以指定将 Windows Me 或更高版本作为目标。
#endif

#ifndef _WIN32_IE			// 允许使用特定于 IE 6.0 或更高版本的功能。
#define _WIN32_IE 0x0600	// 将此值更改为相应的值，以适用于 IE 的其他版本。
#endif
#	include "windows.h"

#endif

#include <exception>
#include <stdio.h>

#include "AutoKey.h"
#include "TableTool.h"

#include "TimeManager.h"

//-------------------------------------------------------------------------
#ifdef	__LINUX__

CsLock::CsLock()
{
	mLock = new pthread_mutex_t;
    pthread_mutex_init( (pthread_mutex_t*)mLock , NULL );
}

CsLock::~CsLock()
{
	pthread_mutex_destroy( (pthread_mutex_t*)mLock );
	delete ((pthread_mutex_t*)mLock);
}

void CsLock::lock()
{
	pthread_mutex_lock((pthread_mutex_t*)mLock );
}

void CsLock::unlock()
{
	pthread_mutex_unlock((pthread_mutex_t*) mLock );
}

bool CsLock::trylock()
{
	return pthread_mutex_trylock((pthread_mutex_t*)mLock )==0;	
}


//class MyLock
//{
//	pthread_mutex_t 	m_Mutex; 
//public :
//	MyLock( ){ pthread_mutex_init( &m_Mutex , NULL );} ;
//	~MyLock( ){ pthread_mutex_destroy( &m_Mutex) ; } ;
//	VOID	Lock( ){ pthread_mutex_lock(&m_Mutex); } ;
//	VOID	Unlock( ){ pthread_mutex_unlock(&m_Mutex); } ;
//};
#else
CsLock::CsLock()
{
	mLock = new CRITICAL_SECTION; InitializeCriticalSection( (CRITICAL_SECTION*)mLock );
}

CsLock::CsLock(const CsLock &other)
{
	mLock = new CRITICAL_SECTION; InitializeCriticalSection( (CRITICAL_SECTION*)mLock );
}

CsLock::~CsLock()
{
	DeleteCriticalSection( (CRITICAL_SECTION*)mLock ); 
	delete ((CRITICAL_SECTION*)mLock);
}

void CsLock::lock()
{
	EnterCriticalSection((CRITICAL_SECTION*)mLock );
}

void CsLock::unlock()
{
	LeaveCriticalSection((CRITICAL_SECTION*) mLock );
}

bool CsLock::trylock()
{
	return TryEnterCriticalSection((CRITICAL_SECTION*)mLock )==TRUE;
	return false;
}


//-------------------------------------------------------------------------
MtLock::MtLock(const CHAR*	name)
{
#ifdef __WINDOWS__
#ifdef _UNICODE
	AString temp = name;
	mLock = (void*)::CreateMutex(NULL, FALSE, temp.getWString().getPtr());
#else
	mLock = (void*)::CreateMutex(NULL, FALSE, name);
#endif
	if (mLock==0)
	{
		char szInfo[60];
		_snprintf_s(szInfo, 60, "Error code: %u", ::GetLastError());
		throw std::exception(szInfo);
	}
#endif
}

MtLock::MtLock()
{
	mLock = (void*)::CreateMutex(NULL, FALSE, NULL);
	if (mLock==0)
	{
		char szInfo[60];
		_snprintf_s(szInfo, 60, "Error code: %u", ::GetLastError());
		throw std::exception(szInfo);
	}
}

MtLock::MtLock(const MtLock &other)
{
	AssertNote(0, "互斥线程锁不支持赋值调用");
}

MtLock::~MtLock()
{
	::ReleaseMutex((HANDLE)mLock);
	::CloseHandle((HANDLE)mLock);
}
void MtLock::lock()
{
	DWORD re = ::WaitForSingleObject((HANDLE)mLock, -1);
    if (re==WAIT_FAILED)
    {
		throw std::exception("Wait for mutex error.");            
    }
	else if (re==WAIT_TIMEOUT)
	{
		throw std::exception("Wait for mutex time over."); 
	}
	//TableTool::Log("succss lock");
}
void MtLock::unlock()
{
	if (::ReleaseMutex((HANDLE)mLock)==0)
	{
		char szInfo[60];
		_snprintf_s(szInfo, 60, "Error code: %u\n", ::GetLastError());
		throw std::exception(szInfo);
	}
	//TableTool::Log("succss unlock");
}
bool MtLock::trylock()
{
	DWORD re = ::WaitForSingleObject((HANDLE)mLock, 0);
    if (re==WAIT_FAILED)
    {
		throw std::exception("Wait for mutex error.");            
    }
	else if (re==WAIT_TIMEOUT)
	{
		//throw std::exception("Wait for mutex time over."); 
	}
	else if (re==WAIT_OBJECT_0)
	{
		return true;
	}
	return false;
}
#endif
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
AutoKey::AutoKey()
	: mTimePeriod(0)
	, mCode(0)
{
	mNowSecond = TimeManager::NowTick(); 
}

KEY_TYPE AutoKey::AlloctKey()
{	
	UInt64 milSec = TimeManager::NowTick();
	
	if (milSec<mNowSecond)	
		++mTimePeriod;

	mNowSecond = milSec;

	++mCode;

	KEY_TYPE key = milSec;
	key = (key<<32) + (mTimePeriod<<16) + mCode;

	return key;
}
//-------------------------------------------------------------------------

void AutoKey::ReleaseKey(KEY_TYPE freeKey)
{

}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
