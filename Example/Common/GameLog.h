
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
// ��Ӫ��־
//-----------------------------------------------------------------------
#define GAMELOGCONFIGREC			"GameLogData"		//�����ļ��еļ�¼��

//���ݿ�����־����
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
//�ӿ�  XXX_LOG(format, ...)
// ˵����
//     ��ʽ��������Ϊ: "%d %d"��ʹ��c ��ʽ������ ����ÿһ�����ݶ�Ӧ����־���е�һ���У��м��ÿո����ָ���
//
//     ����ʱ��(datetime) ֵ �ַ�����ʽ������"2014-01-01 13:01:01" �� "2014\01\01 13:01:01" 
//       ����ֱ��ʹ�� TimeManager::GetDateTime().c_str() ʵ���������ַ���
//
//      ʱ��(time) �Լ� ʱ����time��  ֵ �ַ�����ʽ������"13:01:01" 
//
//ʾ����
//      ��ṹΪ ��
//            | Field1 | Field2 | Field3 | Field4 |
//        ���� Field1 : datetime
//             Field2 : time
//             Field3 : char        (��Ӧ����c���ַ�������)
//             Field4 : int
//
//     ��Ӧ����־��ʽ��Ϊ��
//            XXX_LOG("%s %s %s %d", "2014-01-01 13:01:01", "13:01:01", "fie3d1Value", field4Value);
//            
//            ��� fieldxValue �������ַ��ܣ�������������������� ` �� ��/˫���� ' " ��
//
//������ṹ�������  Gamelog ��־��
//-----------------------------------------------------------------------
#if SAVE_MYSQL_LOG
// ������¼��־ gameActivateLog
#	define GAMEACTIVATE_LOG(logInfo, ...)		GameLog::mpActivateLog->log(logInfo, ##__VA_ARGS__)

// ����������־ gameLaunchLog
#	define GAMELAUNCH_LOG(logInfo, ...)		GameLog::mpLaunchLog->log(logInfo, ##__VA_ARGS__)

// �û�������־
#	define ACCOUNTCREATE_LOG(logInfo, ...)		GameLog::mpAccountCreateLog->log(logInfo, ##__VA_ARGS__)

// �û�����־	gameLoginLog
#	define LOGIN_LOG(logInfo, ...)				GameLog::mpLoginLog->log(logInfo, ##__VA_ARGS__)

// ��ɫ������־	roleCreateLog
#	define ROLECREATE_LOG(logInfo, ...)		GameLog::mpRoleCreateLog->log(logInfo, ##__VA_ARGS__)

// ����������־	onlineSampleLog
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