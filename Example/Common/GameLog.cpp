
#include "GameLog.h"
#include "NiceData.h"
#include "TableManager.h"
#include "BaseRecord.h"
#include "SqlLog.h"
#include <stdarg.h>
#include "Assertx.h"
#include "CommonDefine.h"

void GameLog::InitGameLog(TableManager* tm)
{
	//建立所有的日志对象，每一张表对应一个
	ARecord gameLogRec = tm->GetRecord(RUNCONFIG, GAMELOGCONFIGREC);
	if (!gameLogRec)
	{
		ERROR_LOG("游戏日志初始配置不存在, 当前日志不可用");
		return;
	}
	//AssertEx(gameLogRec,"GameLog:配置文件读取失败 :<%s> <%s>", GAMELOGCONFIG, GAMELOGCONFIGREC);

	AString szGameLogIp = gameLogRec->get("STRING").string();
	int szGameLogPort = gameLogRec->get("VALUE");
	AString szDBAccount = gameLogRec->get("STRING2").string();
	AString szDBPassword = gameLogRec->get("STRING3").string();
	AString szLogDBName = gameLogRec->get("STRING4").string();

	NiceData sourceParam;
	sourceParam.set(DBIP, szGameLogIp);
	sourceParam.set(DBPORT, szGameLogPort);

	sourceParam.set(DBBASE, szLogDBName);

	sourceParam.set(DBUSER, szDBAccount);
	sourceParam.set(DBPASSWORD, szDBPassword); 


	// 注意： 必须保证数据库表格已经准备好
	sourceParam.set(DBNAME, _ACTIVATE_LOG_TABLE);
	mpActivateLog = new SQLLog(&sourceParam);

	sourceParam.set(DBNAME, _LAUNCH_LOG_TABLE);
	mpLaunchLog = new SQLLog(&sourceParam);

	sourceParam.set(DBNAME, _ACCOUNTCREATE_LOG_TABLE);
	mpAccountCreateLog = new SQLLog(&sourceParam);

	sourceParam.set(DBNAME, _LOGIN_LOG_TABLE);
	mpLoginLog = new SQLLog(&sourceParam);

	sourceParam.set(DBNAME, _ROLECREATE_LOG_TABLE);
	mpRoleCreateLog = new SQLLog(&sourceParam);

	sourceParam.set(DBNAME, _ONLINE_LOG_TABLE);
	mpOnLineSampleLog = new SQLLog(&sourceParam);
	//...
}

void GameLog::CloseGameLog()
{
	SAFE_DELETE( mpActivateLog );
	SAFE_DELETE( mpLaunchLog );
	SAFE_DELETE( mpAccountCreateLog );
	SAFE_DELETE( mpLoginLog );
	SAFE_DELETE( mpRoleCreateLog );
	SAFE_DELETE( mpOnLineSampleLog );
}

//------------------------------------------------------------------
SQLLog *GameLog::mpActivateLog = NULL;
SQLLog *GameLog::mpLaunchLog = NULL;
SQLLog *GameLog::mpAccountCreateLog = NULL;
SQLLog *GameLog::mpLoginLog = NULL;
SQLLog *GameLog::mpRoleCreateLog = NULL;
SQLLog *GameLog::mpOnLineSampleLog = NULL;

//...

void UDPLog::Log(const char *logInfo, ...)
{
	va_list argptr;
	va_start(argptr, logInfo);

	int len = vsnprintf(NULL, 0, logInfo, argptr);

	if (len>=LOG_INFO_MAX_LENGTH)
		len = LOG_INFO_MAX_LENGTH-1;

#if __LINUX__
	vsnprintf( mBuffer, len+1, (const char*)logInfo, argptr);
#else
	_vsnprintf_s ( mBuffer, len+1, len, (const char*)logInfo, argptr);
#endif
	va_end(argptr);

	//mBuffer[len] = '\0'; 

	Send(mIPInfo, mBuffer, len);
}
//-------------------------------------------------------------------------*/
RunLog* RunLog::msLog = NULL; // new RunLog("127.0.0.1", 1234);
bool RunLog::mbLog = false;
CsLock	RunLog::msLock;

void RunLog::Init(const char *sz, int port)
{
	SAFE_DELETE(msLog);
	msLog = MEM_NEW RunLog(sz, port);
	msLog->Log("Start run log >[%s:%d]", sz, port);
	NOTE_LOG("Start run log >[%s:%d]", sz, port);
	if (sz!=NULL && port>0)
	{	
		mbLog = true;
	}
	else
		mbLog = false;
}

void RunLog::Close()
{
	SAFE_DELETE(msLog);
	mbLog = false;
}
//-------------------------------------------------------------------------*/