//#include "stdafx.h"
#include "stdio.h"
#include "LogEvent.h"

#include "Lock.h"
#include "AutoString.h"
#include "TimeManager.h"

#include <stdarg.h>


#ifdef __WINDOWS__
#include <Windows.h>
#include <tchar.h>
#endif


// 使用后台线程进行写日志方式
#include "WorkThread.h"
#include <list>
#include "ThreadEvent.h"

#include "NiceData.h"

#include "LogThread.h"
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
ThreadLog::~ThreadLog()
{
	//mLogThreadManager->Close();
	//delete mLogThreadManager;
	//mLogThreadManager = NULL;
	Close();
}

ThreadLog::ThreadLog()
{
	mLogThreadManager = MEM_NEW LogThreadManager();
	mLogThreadManager->InitThread();
}

ThreadLog::ThreadLog( const char *name,const char* mode,int size /*= 0 */ )
{
	mLogThreadManager = MEM_NEW LogThreadManager();
	setFile(name, mode);
	mLogThreadManager->InitThread();
}

bool ThreadLog::setFile( const char *logFileName,const char* mode )
{
	if ( mLogThreadManager->mpLogFile )
	{
		fclose((FILE*)mLogThreadManager->mpLogFile);
		mLogThreadManager->mpLogFile = NULL;
	} 
#pragma warning(push)
#pragma warning(disable:4996)
#if defined (_UNICODE) && defined(__WINDOWS__)
	AString temp = logFileName;
	mLogThreadManager->mpLogFile = _tfopen( temp.getWString().getPtr(), L"wb" );
#else
	mLogThreadManager->mpLogFile = fopen( logFileName, mode );
#endif
#pragma warning(pop)

	return mLogThreadManager->mpLogFile!=NULL;
}


void ThreadLog::log( const char* lpszFormat, ... )
{
	va_list argptr;
	va_start(argptr, lpszFormat);
#if __WINDOWS__ || __IOS__ || __SERVER__
	logVa( argptr, lpszFormat );
#endif
}


void ThreadLog::logVa( va_list &argList,const char* lpszFormat, bool bPrint )
{
	//LogTask temp;
	LogTask *logTask = mLogThreadManager->AlloctLogTask(); //new LogTask();

	char *szLogBuffer = logTask->szLogBuffer;

	int len = vsnprintf(NULL, 0, lpszFormat, argList);
	//int len = _vscprintf(lpszFormat, argptr);
	//	va_end(argptr);
	
	int year, month;
	int day;
	int hour;
	int minute;
	int second;
	TimeManager::GetMe().CurrentDate(year, month, day, hour, minute, second);

#if __LINUX__
	int nLen = sprintf( szLogBuffer,"[%2d:%2d:%2d] ",hour,minute,second );
#else
	int nLen = sprintf_s( szLogBuffer,LOG_DATA_TIME_LENGTH,"[%02d-%02d %02d:%02d:%02d] ",month+1,day,hour,minute,second );
#endif
	if (len+nLen+2>LOG_INFO_MAX_LENGTH)
		len = LOG_INFO_MAX_LENGTH-nLen-2;

#if __LINUX__
	vsnprintf( szLogBuffer+nLen, len+1, (const char*)lpszFormat, argList);
#else
	_vsnprintf_s ( szLogBuffer+nLen, len+1, len, (const char*)lpszFormat, argList);
#endif
	va_end(argList);

	szLogBuffer[nLen+len] = '\n';
	szLogBuffer[nLen+len+1] = '\0'; 
#if DEVELOP_MODE
    logTask->mbPrint = true;
    //printf(szLogBuffer);
    //logTask->mbPrint = false;
#else
    logTask->mbPrint = true;
#endif
	mLogThreadManager->AppendLog(logTask);
}

void ThreadLog::Close()
{
	if (mLogThreadManager!=NULL)
		mLogThreadManager->Close();
	delete mLogThreadManager;
	mLogThreadManager = NULL;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

EasyLog::EasyLog( const char *name,const char* mode,int size ) 
{
#pragma warning(push)
#pragma warning(disable:4996)
#if defined (_UNICODE) && defined(__WINDOWS__)
	AString temp = name;
	m_pFile = _tfopen( temp.getWString().getPtr(), L"wb" );
#else
	m_pFile = fopen( name, mode );
#endif
#pragma warning(pop)
	mLock = new CsLock;
}

EasyLog::EasyLog()
{
	m_pFile = NULL; 
	mLock = new CsLock;
}




void EasyLog::log ( const char * lpszFormat, ... )
{	
	va_list argptr;
	va_start(argptr, lpszFormat);
#if __WINDOWS__ || __IOS__ || __SERVER__
	logVa( argptr, lpszFormat );
#endif
}

#if __SERVER__ && __WINDOWS__
BOOL SetConsoleColor(WORD wAttributes)
{

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	if (hConsole == INVALID_HANDLE_VALUE)

		return FALSE;


	BOOL bResult = SetConsoleTextAttribute(hConsole, wAttributes);



	CloseHandle(hConsole);

	return bResult;

}
#endif

#if __WINDOWS__ || __IOS__ || __SERVER__
void EasyLog::logVa( va_list &argList, const char* lpszFormat )
{

	static char szLogBuffer[LOG_INFO_MAX_LENGTH];

		int len = vsnprintf(NULL, 0, lpszFormat, argList);
		//int len = _vscprintf(lpszFormat, argptr);
		//	va_end(argptr);

		TimeManager::GetMe().CurrentDate();
		int hour = TimeManager::GetMe().GetHour();
		int minute = TimeManager::GetMe().GetMinute();
		int second = TimeManager::GetMe().GetSecond();


#if __LINUX__
		int nLen = sprintf( szLogBuffer,"[%2d:%2d:%2d] ",hour,minute,second );
#else
		mLock->lock();
		int nLen = sprintf_s( szLogBuffer,LOG_DATA_TIME_LENGTH,"[%2d:%2d:%2d] ",hour,minute,second );
#endif
		if (len+nLen+2>LOG_INFO_MAX_LENGTH)
			len = LOG_INFO_MAX_LENGTH-nLen-2;

#if __LINUX__
		vsnprintf( szLogBuffer+nLen, len+1, (const char*)lpszFormat, argList);
#else
		_vsnprintf_s ( szLogBuffer+nLen, len+1, len, (const char*)lpszFormat, argList);
#endif
		va_end(argList);

		szLogBuffer[nLen+len] = '\n';
		szLogBuffer[nLen+len+1] = '\0'; 


#if __SERVER__ && __WINDOWS__
		//SetConsoleColor(FOREGROUND_RED);
		printf(szLogBuffer);
#else 
		//printf(szLogBuffer);
#endif
		if ( m_pFile )
		{		
			fwrite( szLogBuffer, len+nLen+2, 1, (FILE*)m_pFile);
			fflush((FILE*)m_pFile);
		}
#if !__LINUX__
		mLock->unlock();
#endif

}
#endif

bool EasyLog::setFile( const char *logFileName,const char* mode )
{
	mLock->lock();
	if ( m_pFile )
	{
		fclose((FILE*)m_pFile);
		m_pFile = NULL;
	} 
#pragma warning(push)
#pragma warning(disable:4996)
#if defined (_UNICODE) && defined(__WINDOWS__)
	AString temp = logFileName;
	m_pFile = _tfopen( temp.getWString().getPtr(), L"wb" );
#else
	m_pFile = fopen( logFileName, mode );
#endif
#pragma warning(pop)
	mLock->unlock();

	return m_pFile!=NULL;
}

EasyLog::~EasyLog()
{
	if ( m_pFile ) 
	{
		fclose((FILE*)m_pFile);
		m_pFile = NULL;
	}
	delete mLock;
	mLock = NULL;

}


