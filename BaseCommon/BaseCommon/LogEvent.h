#ifndef _INCLUDE_LOGEVENT_H_
#define _INCLUDE_LOGEVENT_H_

#include <stdio.h>
#include "BaseCommon.h"

//-------------------------------------------------------------------------*/
class LogThreadManager;

class BaseCommon_Export ThreadLog 
{
protected:
	LogThreadManager	*mLogThreadManager;

public:
	virtual ~ThreadLog();
	ThreadLog();
	ThreadLog( const char *name,const char* mode,int size = 0  );
	ThreadLog(LogThreadManager *pThread)
	{
		mLogThreadManager = pThread;
	}

	void Close();

public:
	bool setFile( const char *logFileName,const char* mode );

	// ¼ÇÂ¼
	void log( const char* lpszFormat, ... );
#if __WINDOWS__ || __IOS__ || __SERVER__
	virtual void logVa( va_list &argList,const char* lpszFormat, bool bPrint = false );
#endif


};

//-------------------------------------------------------------------------
class CsLock;

class BaseCommon_Export EasyLog 
{
protected:
	void		*m_pFile;
	CsLock		*mLock;

public:
	virtual ~EasyLog();
	EasyLog();
	EasyLog( const char *name,const char* mode,int size = 0  );

public:
	bool setFile( const char *logFileName,const char* mode );

	// ¼ÇÂ¼
	void log( const char* lpszFormat, ... );            
#if __WINDOWS__ || __IOS__ || __SERVER__
	void logVa( va_list &argList,const char* lpszFormat );
#endif

};

//-------------------------------------------------------------------------

#endif //_INCLUDE_LOGEVENT_H_