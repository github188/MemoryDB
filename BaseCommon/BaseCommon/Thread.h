//////////////////////////////////////////////////////////////////////
//
// Thread.h
//
//



#ifndef __THREAD_H__
#define __THREAD_H__

//#include "BaseCommon.h"
//
////////////////////////////////////////////////////
//// include files
////////////////////////////////////////////////////
////#include "BaseType.h"
//
//
//#if defined(__WINDOWS__) || defined (_WIN32)
//typedef DWORD		TID ;
////用来定义玩家各个子模型是否可见 ###
//typedef unsigned __int64 MODEL_PART;
//#elif defined(__LINUX__)
//typedef pthread_t	TID ;
//typedef unsigned long long MODEL_PART;
//typedef const char* LPCSTR;
//#endif
//
////当定义此宏时，所有线程将只执行一次后就推出。
////#define _EXEONECE 10
//
////////////////////////////////////////////////////////////////////////
////
//// class Thread
////
//// POSIX Thread Class
////
////////////////////////////////////////////////////////////////////////
//
//class BaseCommon_Export Thread 
//{
//
////////////////////////////////////////////////////
//// constants
////////////////////////////////////////////////////
//public :
//	
//	enum ThreadStatus 
//	{
//		READY ,		// 当前线程处于准备状态
//		RUNNING ,	// 处于运行状态
//		EXITING ,	// 线程正在退出
//		EXIT		// 已经退出 
//	};
//	
//
////////////////////////////////////////////////////
//// constructor and destructor
////////////////////////////////////////////////////
//
//public :
//
//	// constructor
//	Thread ( ) ;
//
//	// destructor
//	virtual ~Thread () ;
//
//
////////////////////////////////////////////////////
//// public methods
////////////////////////////////////////////////////
//
//public :
//
//	VOID start () ;
//	
//	virtual VOID stop () ;
//
//	VOID exit ( VOID * retval = NULL ) ;
//
//	virtual VOID run () = 0;
//
//	virtual void setActive(bool bActive) = 0;
//
//	virtual void onExceptionExist(){}
////////////////////////////////////////////////////
//// 
////////////////////////////////////////////////////
//public :
//	// get thread identifier
//	TID getTID () { return m_TID; }
//	
//	// get/set thread's status
//	ThreadStatus getStatus () { return m_Status; }
//	VOID setStatus ( ThreadStatus status ) { m_Status = status; }
//	
//	unsigned int GetThisThreadID(void);
//	virtual void CpuSleep(unsigned int millSecond);
//
//	static int StartApp( const char *szAppFileName, const char *szRunPath );
////////////////////////////////////////////////////
//// data members
////////////////////////////////////////////////////
//
//private :
//
//	// thread identifier variable
//	TID m_TID;
//	
//	// thread status
//	ThreadStatus m_Status;
//
//protected:
//#if defined(__WINDOWS__)
//	int m_hThread ;
//#endif
//
//};

#endif
