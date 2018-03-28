// WebDBServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include <tchar.h>
#include "MemoryDB.h"

#include "AccountServer.h"
#include "Dump.h"
#include "CommonDefine.h"

bool g_bExit = false;

int _tmain(int argc, _TCHAR* argv[])
{
	SetUnhandledExceptionFilter(TopLevelFilter);

	//HWND   hwnd=GetForegroundWindow();//ֱ�ӻ��ǰ�����ڵľ��   
	HWND   hwnd=GetConsoleWindow();//GetForegroundWindow();//ֱ�ӻ��ǰ�����ڵľ��   
	EnableMenuItem(::GetSystemMenu(hwnd,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_GRAYED);	//���ùرհ�ť
	SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(NULL, "AccountIcon.ico"));   

	AString logFileName, logErrorFileName;
	SYSTEMTIME nowtime;
	GetLocalTime( &nowtime );

	logFileName.Format("./Log/AccountServer_%4d-%2d-%2d %2d-%2d-%2d.log", 
		nowtime.wYear, nowtime.wMonth, nowtime.wDay, nowtime.wHour, nowtime.wMinute, nowtime.wSecond);

	logErrorFileName.Format("./Error/AccountServer_Error_%4d-%2d-%2d %2d-%2d-%2d.log", 
		nowtime.wYear, nowtime.wMonth, nowtime.wDay, nowtime.wHour, nowtime.wMinute, nowtime.wSecond);

	Allot::setLogFile("MemAccountServer");
	TableTool::SetErrorLog( new TableLog(logErrorFileName.c_str()) );

	tBaseTable::msCommonConfig.mRecordMemPoolBlockRecordCount = 1000;

	AccountServer	*mDBServer = new AccountServer();
	mDBServer->Start(MEM_NEW AccountThread("AccountDB"), "AccountDB", logFileName.c_str(), "RunConfig/AccountConfigList.csv");

	char szDir[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH-1, szDir);
	AString tile;
	tile.Format("AG_%s�� v[%s] %s"
		, RELEASE_RUN_VER!=0 ? "����":"����"
		, SERVER_VERSION_FLAG
		, szDir
		);
	SetConsoleTitle(tile.c_str());

	char Cmd;
	while (!g_bExit)
	{
		if (mDBServer->IsStop())
			break;
		Cmd = getchar(); 
		if (('q' == Cmd)||('Q' == Cmd))	
		{
			mDBServer->RequestClose();
		}
		else if (('r' == Cmd)||('R' == Cmd))	
		{
			mDBServer->ProcessCommand("ReloadConfig");
		}
	}
	//Sleep(6000)
	mDBServer->Stop();
	delete mDBServer;
	mDBServer = NULL;

	return 0;
}
