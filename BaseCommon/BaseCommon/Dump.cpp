#	include <Windows.h>
#include <Tlhelp32.h>
#include <time.h>
#include <dbghelp.h>
#include <stdio.h>
#include <tchar.h>

#include "Dump.h"
#include "TableTool.h"
//--------------------------------------------------------------
//自动加载dbghelp.dll
typedef	BOOL (WINAPI * MINIDUMP_WRITE_DUMP)(
	IN HANDLE			hProcess,
	IN UINT			ProcessId,
	IN HANDLE			hFile,
	IN MINIDUMP_TYPE	DumpType,
	IN CONST PMINIDUMP_EXCEPTION_INFORMATION	ExceptionParam, OPTIONAL
	IN PMINIDUMP_USER_STREAM_INFORMATION		UserStreamParam, OPTIONAL
	IN PMINIDUMP_CALLBACK_INFORMATION			CallbackParam OPTIONAL
	);

struct tAUTO_LOAD_DBGHELP
{
	tAUTO_LOAD_DBGHELP(); 
	~tAUTO_LOAD_DBGHELP();

	HMODULE					m_hDbgHelp;
	MINIDUMP_WRITE_DUMP		m_pfnMiniDumpWriteDump;
} theDbgHelper;

tAUTO_LOAD_DBGHELP::tAUTO_LOAD_DBGHELP()
{
	m_hDbgHelp = ::LoadLibrary(_T("DBGHELP.DLL"));
	if(NULL == theDbgHelper.m_hDbgHelp) 
	{
		MessageBox( NULL, "LoadLibrary dbghelp.dll failed!", NULL, MB_OK );
		return; //Load failed!
	}
	m_pfnMiniDumpWriteDump = (MINIDUMP_WRITE_DUMP)GetProcAddress(m_hDbgHelp, (LPCSTR)("MiniDumpWriteDump"));
	if(NULL == m_pfnMiniDumpWriteDump) 
	{
		::FreeLibrary(m_hDbgHelp);
		MessageBox( NULL, "GetProcAddress MiniDumpWriteDump from dbghelp.dll failed!", NULL, MB_OK );
		m_hDbgHelp = NULL;
	}
}

tAUTO_LOAD_DBGHELP::~tAUTO_LOAD_DBGHELP()
{
	if(m_hDbgHelp)
	{
		::FreeLibrary(m_hDbgHelp);
		m_hDbgHelp= NULL;
	}
}

//创建DBGHLEP所需要的dmp信息
BOOL CreateDumpHelpFile(PEXCEPTION_POINTERS pException, TCHAR* szDumpFile)
{
	szDumpFile[0] = ' ';
	szDumpFile[1] = 0;

	// If MiniDumpWriteDump() of DbgHelp.dll available.
	if (!theDbgHelper.m_pfnMiniDumpWriteDump) return FALSE;

//	TCHAR szTempDir[MAX_PATH] = {0};
////	::GetTempPath(MAX_PATH, szTempDir);
//	::GetCurrentDirectory( MAX_PATH, szTempDir );
	TCHAR szTempFile[MAX_PATH] = {0};
//	::GetTempFileName(szTempDir, _T("dmp"), MAX_PATH, szTempFile);
	
	SYSTEMTIME nowtime;
	//GetSystemTime( &nowtime );
	GetLocalTime( &nowtime );

	TCHAR		Module_Name[MAX_PATH]={0};
	GetModuleFileName(NULL, Module_Name, MAX_PATH);
#ifdef UNICODE
	wsprintf ( szTempFile, sizeof( szTempFile),
		_T("%s_dump_%4d-%2d-%2d %2d-%2d-%2d.dmp"), 
		Module_Name,
		nowtime.wYear, nowtime.wMonth, nowtime.wDay, nowtime.wHour, nowtime.wMinute, nowtime.wSecond);
#else
#pragma warning(push)
#pragma warning(disable:4996)  
	_snprintf( szTempFile, sizeof( szTempFile),
		_T("%s_dump_%4d-%2d-%2d %2d-%2d-%2d.dmp"), 
		Module_Name,
		nowtime.wYear, nowtime.wMonth, nowtime.wDay, nowtime.wHour, nowtime.wMinute, nowtime.wSecond);
#pragma warning(pop) 
#endif


	MINIDUMP_EXCEPTION_INFORMATION	M;

	M.ThreadId = GetCurrentThreadId();
	M.ExceptionPointers = pException;
	M.ClientPointers = 0;

	HANDLE hDump_File = CreateFile(szTempFile,
		GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	//[by zcx 2010.6.12]
	theDbgHelper.m_pfnMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDump_File,
		MiniDumpWithFullMemory,//|MiniDumpWithHandleData|MiniDumpFilterMemory|MiniDumpScanMemory|MiniDumpWithIndirectlyReferencedMemory,
		(pException) ? &M : NULL, NULL, NULL);

	CloseHandle(hDump_File);
	NOTE_LOG("程序异常Dump, 现在直接退出程序");
	::GetShortPathName(szTempFile, szDumpFile, MAX_PATH);
	if(szDumpFile[0] == 0) 
	{
		//exit(19);
		return FALSE;
	}
	//exit(20);
	return TRUE;
}

BOOL tProcessException(PEXCEPTION_POINTERS pException) throw()
{
	//保存最后的错误代码
/*	UINT	dwLastError = ::GetLastError();
	if(!pException) return;

	//生成完整表述文件
	TCHAR szBigInfoFile[MAX_PATH] = {0};
	if(!CreateBigInfoFile(pException, szBigInfoFile, dwLastError))
	{
		return;
	}*/
	//knowC注释以上

	//生成dump报告
	TCHAR szDumpFile[MAX_PATH] = {0};
	return CreateDumpHelpFile(pException, szDumpFile);
}

LONG __stdcall TopLevelFilter( struct _EXCEPTION_POINTERS *pExceptionInfo )
{
	if(tProcessException(pExceptionInfo) == TRUE)
		return EXCEPTION_EXECUTE_HANDLER;
	else
		return EXCEPTION_CONTINUE_SEARCH;
}

BaseCommon_Export void throw_exception(unsigned int code, EXCEPTION_POINTERS* ptr)
{
	tProcessException(ptr);
	throw MyException(code, ptr);
}

BaseCommon_Export size_t StartRun(const char *szAppFileName, const char *szRunPath)
{
	STARTUPINFO   StartupInfo;//创建进程所需的信息结构变量   
	memset(&StartupInfo,0,sizeof(StartupInfo));
	GetStartupInfo(&StartupInfo);    
	StartupInfo.lpReserved=NULL;    
	StartupInfo.lpDesktop=NULL;    
	StartupInfo.lpTitle=NULL;    
	StartupInfo.dwX=0;    
	StartupInfo.dwY=0;    
	StartupInfo.dwXSize=0;    
	StartupInfo.dwYSize=0;    
	StartupInfo.dwXCountChars=500;    
	StartupInfo.dwYCountChars=500;    
	StartupInfo.dwFlags=STARTF_USESHOWWINDOW;    
	StartupInfo.wShowWindow=SW_SHOW;    
	//说明进程将以隐藏的方式在后台执行    
	StartupInfo.cbReserved2=0;    
	StartupInfo.lpReserved2=NULL;    
	StartupInfo.hStdInput=stdin;    
	StartupInfo.hStdOutput=stdout;    
	StartupInfo.hStdError=stderr;    

	PROCESS_INFORMATION pi;

	bool fRet=CreateProcess(NULL, (LPSTR)szAppFileName, NULL, NULL,FALSE,CREATE_NEW_CONSOLE |HIGH_PRIORITY_CLASS ,NULL,szRunPath,&StartupInfo,&pi)==TRUE;
	if (fRet)
		return (size_t)pi.hProcess;

	return (size_t)(HANDLE)INVALID_HANDLE;
}

BaseCommon_Export int GenerateMiniDump(HANDLE hFile, PEXCEPTION_POINTERS pExceptionPointers, const CHAR *szAppName, bool bNeedMemoryData)
{

	BOOL bOwnDumpFile = FALSE;  
	HANDLE hDumpFile = hFile;  
	MINIDUMP_EXCEPTION_INFORMATION ExpParam;  

	typedef BOOL(WINAPI * MiniDumpWriteDumpT)(  
		HANDLE,  
		DWORD,  
		HANDLE,  
		MINIDUMP_TYPE,  
		PMINIDUMP_EXCEPTION_INFORMATION,  
		PMINIDUMP_USER_STREAM_INFORMATION,  
		PMINIDUMP_CALLBACK_INFORMATION  
		);  

	MiniDumpWriteDumpT pfnMiniDumpWriteDump = NULL;  
	HMODULE hDbgHelp = LoadLibrary("DbgHelp.dll");  
	if (hDbgHelp)  
		pfnMiniDumpWriteDump = (MiniDumpWriteDumpT)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");  

	if (pfnMiniDumpWriteDump)  
	{  
		if (hDumpFile == NULL || hDumpFile == INVALID_HANDLE_VALUE)  
		{  
			//CHAR szPath[MAX_PATH] = { 0 };  
			//CHAR szFileName[MAX_PATH] = { 0 };  
			//const CHAR* szAppName = pwAppName;  
			//const CHAR* szVersion = "v1.0";  
			//CHAR dwBufferSize = MAX_PATH;  
			SYSTEMTIME nowtime;  

			GetLocalTime(&nowtime);  
			//GetTempPath(dwBufferSize, szPath);  

			//sprintf_s(szFileName, MAX_PATH, "%s%s", szPath, szAppName);  
			//CreateDirectory(szFileName, NULL);  

			AString x;
			x.Format("%s_dump_%4d-%2d-%2d %2d-%2d-%2d.dmp", szAppName,
				nowtime.wYear, nowtime.wMonth, nowtime.wDay, nowtime.wHour, nowtime.wMinute, nowtime.wSecond
				);
			//sprintf_s(szFileName, MAX_PATH, "%s-%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp",  
			//	//szPath, szAppName, szVersion, 
			//	szAppName,
			//	stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,  
			//	stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond,  
			//	GetCurrentProcessId(), GetCurrentThreadId());  
			hDumpFile = CreateFile(x.c_str(), GENERIC_READ | GENERIC_WRITE,  
				FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);  

			bOwnDumpFile = TRUE;  
			//OutputDebugString(szFileName);  
		}  

		if (hDumpFile != INVALID_HANDLE_VALUE)  
		{  
			ExpParam.ThreadId = GetCurrentThreadId();  
			ExpParam.ExceptionPointers = pExceptionPointers;  
			ExpParam.ClientPointers = FALSE;  

			pfnMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),  
				hDumpFile, bNeedMemoryData ? MiniDumpWithFullMemory:MiniDumpWithDataSegs, (pExceptionPointers ? &ExpParam : NULL), NULL, NULL);  

			if (bOwnDumpFile)  
				CloseHandle(hDumpFile);  
		}  
	}  

	if (hDbgHelp != NULL)  
		FreeLibrary(hDbgHelp);  

	return EXCEPTION_EXECUTE_HANDLER;
}
