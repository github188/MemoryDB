// LoginServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <conio.h>
#include "Login.h"
#include "LoginThread.h"
#include "Dump.h"
#include "TableManager.h"
#include "HttpClient.h"
#include "GameLog.h"
#include "Resource.h"

using namespace std;


int _tmain(int argc, _TCHAR* argv[])
{
	SetUnhandledExceptionFilter(TopLevelFilter);
	
	//HWND   hwnd=GetForegroundWindow();//直接获得前景窗口的句柄   
	//SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(NULL, "icon1.ico"));   

	AString logFileName, logErrorFileName;
	SYSTEMTIME nowtime;
	GetLocalTime( &nowtime );

	logFileName.Format("./Log/LoginServer_%4d-%2d-%2d %2d-%2d-%2d.log", 
		nowtime.wYear, nowtime.wMonth, nowtime.wDay, nowtime.wHour, nowtime.wMinute, nowtime.wSecond);

	logErrorFileName.Format("./Error/LoginServer_Error_%4d-%2d-%2d %2d-%2d-%2d.log", 
		nowtime.wYear, nowtime.wMonth, nowtime.wDay, nowtime.wHour, nowtime.wMinute, nowtime.wSecond);

	Allot::setLogFile("LoginServer");
	TableManager::SetLog( new TableLog(logFileName.c_str()) );
	TableTool::SetErrorLog( new TableLog(logErrorFileName.c_str()) );

	AutoTable configTable = tBaseTable::NewBaseTable(false);
	if (!configTable->LoadCSV("RunConfig/LoginConfig.csv"))
	{
		ERROR_LOG("加载运行配置失败 RunConfig/LoginConfig.csv, 6秒后关闭");
		Sleep(6000);
		return 0;
	}
	AutoTable infoTable = tBaseTable::NewBaseTable(false);
	if (!infoTable->LoadCSV("RunConfig/ServerLoginVersion.csv"))
	{
		ERROR_LOG("加载运行配置失败 RunConfig/ServerLoginVersion.csv, 6秒后关闭");
		Sleep(6000);
		return 0;
	}
	int serverID = configTable->GetValue("LoginIp", "SERVER_ID");
	char szDir[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH-1, szDir);
	AString tile;
	tile.Format("%s v[%s] <%d>[%s] [%s:%d] [v%s] [%s] %s",
#ifdef _DEBUG
		"LS_Debug"
#else
		"LS_Release"
#endif		
		, SERVER_VERSION_FLAG
		, (int)configTable->GetValue("LoginIp", "SERVER_ID")
		, serverID>0 ? AString::getANIS(configTable->GetValue("LoginIp", "INFO").string().c_str()).c_str() : "游戏区信息"
		, configTable->GetValue("LoginIp", "IP").string().c_str()
		, (int)configTable->GetValue("LoginIp", "PORT")
		,  infoTable->GetValue("VER", "VERSION").string().c_str()
		, SERVER_STATE_TOOL::ToStringServerState(infoTable->GetValue("VER", "STATE"))
		, szDir
		);
	SetConsoleTitle(tile.c_str());
	{	
		UDPNet	tempLog;
		tempLog.Send("127.0.0.1", 999, tile.c_str(), tile.length());
	}

	Login	*mLoginServer = new Login();
	mLoginServer->Start(MEM_NEW LoginThread(), "LoginServer", logFileName.c_str(), "");

	//HWND   hwnd=GetConsoleWindow();//直接获得前景窗口的句柄   
	if (serverID<=0)
	{	
		tBaseServer::SetIocn("LI.bmp", NULL, NULL, 0, 0);
	}
	else
		tBaseServer::SetIocn("LG.bmp", STRING(serverID), "黑体", 100, 0xFFFFFF);

	char Cmd;
	while (true)
	{
		if (mLoginServer->IsStop())
			break;
		if (kbhit())
		{	
			Cmd = getchar(); 
			if (('q' == Cmd)||('Q' == Cmd))	
			{			
				mLoginServer->Stop();
				break;
			}
			else if (('r' == Cmd)||('R' == Cmd))	
			{
				//mLoginServer->ProcessCommand("ReloadConfig");
			}
			else if (('a' == Cmd)||('A' == Cmd))
			{
				mLoginServer->ProcessCommand("a");
			}
		}
		else
			Sleep(6000);
	}

	delete mLoginServer;
	mLoginServer = NULL;

	RunLog::Close();

	//delete TableManager::getSingletonPtr();

	return 0;
}

