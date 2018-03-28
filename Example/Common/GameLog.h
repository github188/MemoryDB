
#ifndef _GAMELOG_H_
#define _GAMELOG_H_

#include <SqlLog.h>
#include "UDPNet.h"
#include "ServerIPInfo.h"

#define RUN_LOG(logInfo, ...)	{ if (RunLog::mbLog) { RunLog::msLock.lock(); RunLog::GetMe().Log(logInfo, ##__VA_ARGS__); RunLog::msLock.unlock();} }
//#define RUN_LOG {}
//-------------------------------------------------------------------------*/
class UDPLog : public UDPNet
{
public:
	UDPLog(const char *sz, int port)
	{
		mIPInfo = ServerIPInfo::IP2Num(sz, 0, port);
	}

	void Log(const char *logInfo, ...);

public:
	UInt64	mIPInfo;
	char	mBuffer[LOG_INFO_MAX_LENGTH];
};


class RunLog : public UDPLog
{
public:
	static RunLog& GetMe(){ return *msLog; }
	
	static void Init(const char *sz, int port);
	static void Close();
	static void OpenLog(){  mbLog = true; }
	static void CloseLog(){ mbLog = false; }

protected:
	RunLog(const char *sz, int port): UDPLog(sz, port) {}

protected:

	static RunLog	*msLog;

public:
	static bool		mbLog;
	static CsLock	msLock;
};

//-----------------------------------------------------------------------
// 运营日志
//-----------------------------------------------------------------------
#define GAMELOGCONFIGREC			"GameLogData"		//配置文件中的记录名

//数据库中日志表名
#define _GAMELOG_MAIN_TABLE			"gameDefaultLog"

#define _ACTIVATE_LOG_TABLE			"gameActivateLog"
#define _LAUNCH_LOG_TABLE			"gameLaunchLog"
#define _LOGIN_LOG_TABLE			"gameLoginLog"
#define _ACCOUNTCREATE_LOG_TABLE	"accountCreateLog"
#define _ROLECREATE_LOG_TABLE		"roleCreateLog"
#define _CHARGE_LOG_TABLE			"chargeLog"
#define _CONSUME_LOG_TABLE			"consumptionLog"
#define _ONLINE_LOG_TABLE			"onlineSampleLog"
//...


//-----------------------------------------------------------------------
//接口  XXX_LOG(format, ...)
// 说明：
//     格式化串样例为: "%d %d"，使用c 格式化符号 并且每一个数据对应于日志表中的一个列，中间用空格做分隔符
//
//     日期时间(datetime) 值 字符串格式样例："2014-01-01 13:01:01" 或 "2014\01\01 13:01:01" 
//       可以直接使用 TimeManager::GetDateTime().c_str() 实例方法的字符串
//
//      时间(time) 以及 时长（time）  值 字符串格式样例："13:01:01" 
//
//示例：
//      表结构为 ：
//            | Field1 | Field2 | Field3 | Field4 |
//        其中 Field1 : datetime
//             Field2 : time
//             Field3 : char        (对应的是c的字符串类型)
//             Field4 : int
//
//     对应的日志格式化为：
//            XXX_LOG("%s %s %s %d", "2014-01-01 13:01:01", "13:01:01", "fie3d1Value", field4Value);
//            
//            如果 fieldxValue 本身是字符窜，不能在里面有特殊符号 ` 和 单/双引号 ' " 等
//
//各个表结构定义详见  Gamelog 日志表
//-----------------------------------------------------------------------
#if SAVE_MYSQL_LOG
// 启动记录日志 gameActivateLog
#	define GAMEACTIVATE_LOG(logInfo, ...)		GameLog::mpActivateLog->log(logInfo, ##__VA_ARGS__)

// 启动性能日志 gameLaunchLog
#	define GAMELAUNCH_LOG(logInfo, ...)		GameLog::mpLaunchLog->log(logInfo, ##__VA_ARGS__)

// 用户创建日志
#	define ACCOUNTCREATE_LOG(logInfo, ...)		GameLog::mpAccountCreateLog->log(logInfo, ##__VA_ARGS__)

// 用户登日志	gameLoginLog
#	define LOGIN_LOG(logInfo, ...)				GameLog::mpLoginLog->log(logInfo, ##__VA_ARGS__)

// 角色创建日志	roleCreateLog
#	define ROLECREATE_LOG(logInfo, ...)		GameLog::mpRoleCreateLog->log(logInfo, ##__VA_ARGS__)

// 在线样本日志	onlineSampleLog
#define ONLINE_LOG(logInfo, ...)			GameLog::mpOnLineSampleLog->log(logInfo, ##__VA_ARGS__)
#else
#	define GAMEACTIVATE_LOG(logInfo, ...)	
#	define GAMELAUNCH_LOG(logInfo, ...)	
#	define ACCOUNTCREATE_LOG(logInfo, ...)	
#	define LOGIN_LOG(logInfo, ...)		
#	define ROLECREATE_LOG(logInfo, ...)	
#	define ONLINE_LOG(logInfo, ...)	
#endif
//...

//-----------------------------------------------------------------------
//class SQLLog;
class TableManager;
class GameLog
{

private:
	GameLog();
public:
	~GameLog();

public:
	static void InitGameLog(TableManager *tm);
	static void CloseGameLog();

public:
	static SQLLog		*mpActivateLog;
	static SQLLog		*mpLaunchLog;
	static SQLLog		*mpAccountCreateLog;
	static SQLLog		*mpLoginLog;
	static SQLLog		*mpRoleCreateLog;
	static SQLLog		*mpOnLineSampleLog;
	//...
};

#endif