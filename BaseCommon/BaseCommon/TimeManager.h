
#ifndef __TIMEMANAGER_H__
#define __TIMEMANAGER_H__
#include "BaseCommon.h"
#include "AutoString.h"

#ifdef __LINUX__
#	include <sys/utsname.h>
#	include <sys/time.h>
#	include <time.h>
#else
#	include <time.h>
#endif
//-------------------------------------------------------------------------*/
// NOTE: GetANSITime 未处理多线情况，修改为直接调用 Now()
//-------------------------------------------------------------------------*/
class BaseCommon_Export TimeManager
{
public:	
	static TimeManager& GetMe();

public :
	TimeManager( ) ;
	~TimeManager( ) ;

	BOOL			Init( ) ;

	//当前时间计数值，起始值根据系统不同有区别
	//返回的值为：微妙单位的时间值
	//UInt64			CurrentTime( ) ;
	//直接返回，不调用系统接口
	//UInt64			CurrentSavedTime( ){ return m_CurrentTime ; } ;
	//取得服务器端程序启动时的时间计数值
	UInt64			StartTime( ){ return m_StartTime ; } ;

	//将当前的系统时间格式化到时间管理器里
	VOID			SetTime( ) ;

	// 得到标准时间
	time_t			GetANSITime( );

public :
	//***注意：
	//以下接口调用没有系统调用，只针对时间管理器内的数据
	//

	//取得设置时间时候的“年、月、日、小时、分、秒、星期的值”
	INT				GetYear( ){ return m_TM.tm_year+1900 ; } ;	//[1900,????]
	INT				GetMonth( ){ return m_TM.tm_mon ; } ;		//[0,11]
	INT				GetDay( ){ return m_TM.tm_mday ; } ;		//[1,31]
	INT				GetHour( ){ return m_TM.tm_hour ; } ;		//[0,23]
	INT				GetMinute( ){ return m_TM.tm_min ; } ;		//[0,59]
	INT				GetSecond( ){ return m_TM.tm_sec ; } ;		//[0,59]
	//取得当前是星期几；0表示：星期天，1～6表示：星期一～星期六
	UINT			GetWeek( ){ return m_TM.tm_wday ; } ;
	//将当前的时间（年、月、日、小时、分）转换成一个UINT来表示
	//例如：0,507,211,233 表示 "2005.07.21 12:33"
	UINT			Time2DWORD( ) ;
	//取得当前的日期[4bit 0-9][4bit 0-11][5bit 0-30][5bit 0-23][6bit 0-59][6bit 0-59]
	UINT			CurrentDate( ) ;
	VOID			CurrentDate(int &year, int &month, int &day, int &hour, int &minute, int &second);
	//取得服务器启动后的运行时间（毫秒）
	//UInt64			RunTime( ){ 
	//	CurrentTime( ) ;
	//	return (m_CurrentTime-m_StartTime);  
	//} ;
	//UInt64			RunTick( )
	//{
	//	CurrentTime();
	//	return m_CurrentTime-m_StartTime;  
	//};
	//取得两个日期时间的时间差（单位：毫秒）, Ret = Date2-Data1
	UINT			DiffTime( UINT Date1, UINT Date2 ) ;
	//将一个UINT的日期转换成一个tm结构
	VOID			ConvertUT( UINT Date, tm* TM ) ;
	//将一个tm结构转换成一个UINT的日期
	VOID			ConvertTU( tm* TM, UINT& Date ) ;
	//取得已天为单位的时间值, 千位数代表年份，其他三位代表时间（天数）
	UINT			GetDayTime( ) ;
	//得到当前是今天的什么时间2310表示23点10分
	WORD			GetTodayTime();
	BOOL			FormatTodayTime(WORD& nTime);

	AString			GetDate();
	AString			GetTime();
	AString			GetDateTime();
	
	static AString	ToDataTime(UInt64 dateTime);
	static AString	ToTimeString(UInt64 dateTime);
	// 时间戳转换为年月日时分秒
	static int      UnixTimeToDate(UInt64 unixTime, int timeArray[]);
	// 判断一个时间戳是不是今天范围内
	static bool     UnixTimeisToday( UInt64 unixTime );
	// 获取相对1970年的秒数对应的星期值
	static int		GetWeekBySecond(UInt64 sec);
	static int		GetMonthBySecond(UInt64 sec);

	// 取得指定星期定时时间, , bNowNext, 当前已到是否取下一个定时
	static int		GetTimingSecond(int week, int hour, int minute, bool bNowNext = true, bool bLog = false);
	
	// 取得指定日期定时时间, bNowNext, 当前已到是否取下一个定时
	static int		GetMdayTimingSceond(int Mday, int hour, int minute, bool bNowNext = true);

	// 取得定时秒数 , bNowNext, 当前已到是否取下一个定时
	static int		GetTimingSecond(int hour, int minute, bool bNowNext = true);

	// 
	static	void	Sleep(int milSecond);

	// 获取当前相对于1970.1.1号的时间(秒)
	static UInt64	Now();

	// 获取当前机器开机时间(毫秒)
	static UInt64	NowTick();

    // 获取 下一个200年的相对于1970.1.1号时间(秒)
    static UInt64   NextTwoCenturyYearTime();

public :
	UInt64			m_StartTime ;
	//UInt64			m_CurrentTime ;
	time_t			m_SetTime ;
	tm				m_TM ;
#ifdef __LINUX__
	struct timeval _tstart, _tend;
	struct timezone tz;
#endif



};






#endif
