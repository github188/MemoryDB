//--------------------------------------------------------------------------------
//
// Filename   : Assert.h
//
//--------------------------------------------------------------------------------

#ifndef __ASSERT__H__
#define __ASSERT__H__

#include "BaseCommon.h"

#if !RELEASE_MODE && !DEVELOP_MODE
#	define RELEASE_MODE
#endif

#ifndef uint
	typedef unsigned int uint;
#endif

BaseCommon_Export extern int g_Command_Assert ;//控制参数，不提示Assert的对话框，直接忽略
BaseCommon_Export extern int g_Command_IgnoreMessageBox ;//控制参数，跳过MyMessageBox的中断

//--------------------------------------------------------------------------------
//
// 
// 
//
//--------------------------------------------------------------------------------
BaseCommon_Export  void __assert__ (const char* file, uint line, const char* func, const char* expr) ;
BaseCommon_Export  void __assertex__ (const char* file, uint line, const char* func, const char* expr, const char* msg, ...) ;
BaseCommon_Export  void __assertspecial__ (const char* file, uint line, const char* func, const char* expr, const char* msg) ;
BaseCommon_Export  void __messagebox__(const char*msg ) ;
//--------------------------------------------------------------------------------
//
// 
// 
//
//--------------------------------------------------------------------------------
extern void __protocol_assert__ (const char* file, uint line, const char* func, const char* expr) ;

#ifdef __WINDOWS__
#define AssertNote(expr, msg, ...) ((void)((expr)?0:(__assertex__(__FILE__,__LINE__,__FUNCTION__,#expr,msg, ##__VA_ARGS__),0)))
#endif
//#if defined(NDEBUG)
//#define Assert(expr) ((void)0)
//#define AssertEx(expr,msg) ((void)0)
//#define AssertSpecial(expr,msg) ((void)0)
//#define MyMessageBox(msg) ((void)0)
//#elif defined(__LINUX__)
#if defined RELEASE_MODE
	#define Assert(expr) ((void)0)
	#define AssertEx(expr, msg, ...) ((void)0)
	#define AssertSpecial(expr,msg) ((void)0)
	#define MyMessageBox(msg) ((void)0)
#elif defined(__LINUX__)
	#define Assert(expr) {if(!(expr)){__assert__(__FILE__,__LINE__,__PRETTY_FUNCTION__,#expr);}}
	#define ProtocolAssert(expr) ((void)((expr)?0:(__protocol_assert__(__FILE__,__LINE__,__PRETTY_FUNCTION__,#expr),0)))
	#define AssertEx(expr, msg, ...) {if(!(expr)){__assertex__(__FILE__,__LINE__,__PRETTY_FUNCTION__,#expr, msg, ##__VA_ARGS__);}}
	#define AssertSpecial(expr,msg) {if(!(expr)){__assertspecial__(__FILE__,__LINE__,__PRETTY_FUNCTION__,#expr,msg);}}
	#define AssertExPass(expr,msg) {if(!(expr)){__assertex__(__FILE__,__LINE__,__PRETTY_FUNCTION__,#expr,msg);}}
	#define MyMessageBox(msg) ((void)0)
#elif defined(__WIN_CONSOLE__) || defined(__WIN32__) || defined(__WINDOWS__) || defined(_WINDOWS) || defined (WIN32)
	#define Assert(expr) ((void)((expr)?0:(__assert__(__FILE__,__LINE__,__FUNCTION__,#expr),0)))
	#define AssertEx(expr, msg, ...) ((void)((expr)?0:(__assertex__(__FILE__,__LINE__,__FUNCTION__,#expr,msg, ##__VA_ARGS__),0)))
	#define AssertSpecial(expr,msg) ((void)((expr)?0:(__assertspecial__(__FILE__,__LINE__,__FUNCTION__,#expr,msg),0)))
	#define MyMessageBox(msg) __messagebox__(msg)
#elif defined(__MFC__)
	#define Assert(expr) ASSERT(expr)
	#define AssertEx(expr, msg, ...) ((void)0)
	#define AssertSpecial(expr,msg) ((void)0)
	#define MyMessageBox(msg) ((void)0)
#endif


#if defined(GAME_CLIENT)
#define __ENTER_FUNCTION 
#define __LEAVE_FUNCTION
#elif 1

#if defined(__WINDOWS__) || defined(WIN32)
#	if defined(NDEBUG)
#		define __ENTER_FUNCTION {try{
#		define __LEAVE_FUNCTION }catch(...){AssertSpecial(false,__FUNCTION__);}}
#	else
#		define __ENTER_FUNCTION 
#		define __LEAVE_FUNCTION 
#	endif
#else	//linux
#	define __ENTER_FUNCTION {try{
#	define __LEAVE_FUNCTION }catch(...){AssertSpecial(false,__PRETTY_FUNCTION__);}}
#endif 
#endif


///////////////////////////////////////////////////////////////////////
//调试预定义宏定义
///////////////////////////////////////////////////////////////////////

#define __ENTER_FUNCTION_FOXNET if(1){
#define __LEAVE_FUNCTION_FOXNET }


//根据指针值删除内存
#ifndef SAFE_DELETE
#define SAFE_DELETE(x)	if( (x)!=NULL ) { delete (x); (x)=NULL; }
#endif
//根据指针值删除数组类型内存
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(x)	if( (x)!=NULL ) { delete[] (x); (x)=NULL; }
#endif
//根据指针调用free接口
#ifndef SAFE_FREE
#define SAFE_FREE(x)	if( (x)!=NULL ) { free(x); (x)=NULL; }
#endif
//根据指针调用Release接口
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x)	if( (x)!=NULL ) { (x)->Release(); (x)=NULL; }
#endif



#endif
