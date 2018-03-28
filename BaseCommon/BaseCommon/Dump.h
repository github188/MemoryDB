
#ifndef _INCLUDE_DUMP_H_
#define _INCLUDE_DUMP_H_

#include "BaseCommon.h"
#include <Windows.h>
#include <stdio.h>
#include <eh.h>

BaseCommon_Export LONG __stdcall TopLevelFilter( struct _EXCEPTION_POINTERS *pExceptionInfo );
BaseCommon_Export BOOL tProcessException(PEXCEPTION_POINTERS pException) throw();

class MyException
{
public:
	MyException(unsigned int _code, EXCEPTION_POINTERS* _ptr)
		: code(_code), ptrException(_ptr) {}
	~MyException() {}
	unsigned int code;
	EXCEPTION_POINTERS* ptrException;
};

BaseCommon_Export void throw_exception( unsigned int code, EXCEPTION_POINTERS* ptr);

BaseCommon_Export size_t StartRun(const char *szAppFileName, const char *szRunPath);

//NOTE: 可 DUMP 详细内存数据及堆栈
BaseCommon_Export int GenerateMiniDump(HANDLE hFile, PEXCEPTION_POINTERS pExceptionPointers, const CHAR *szAppName, bool bNeedMemoryData);  

#endif //_INCLUDE_DUMP_H_