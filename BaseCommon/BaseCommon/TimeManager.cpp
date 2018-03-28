//#include "stdafx.h"

#include "TimeManager.h"

#include "Assertx.h"
#include <memory>

#ifdef __LINUX__
    #include <unistd.h>
#else
    #include <windows.h>
#endif
#include "TableTool.h"

#define DEBUG_TIME_BOOST	0
#define TEST_TIME_RATE		(0.01)
#define TEST_TIME_BOOST		(100)

time_t gStartTime = 0;
//TimeManager* 	g_pTimeManager=NULL ;

TimeManager::TimeManager( )
{
__ENTER_FUNCTION

	//m_CurrentTime = 0 ;

	Init();

__LEAVE_FUNCTION
}

TimeManager::~TimeManager( )
{
__ENTER_FUNCTION


__LEAVE_FUNCTION
}

BOOL TimeManager::Init( )
{
__ENTER_FUNCTION

#if defined(__WINDOWS__)
	m_StartTime = GetTickCount64() ;
	//m_CurrentTime = GetTickCount64() ;
#elif defined(__LINUX__)
	m_StartTime		= 0;
	m_CurrentTime	= 0;
	gettimeofday(&_tstart, &tz);
#endif
	SetTime( ) ;

	gStartTime = m_SetTime;

	return TRUE ;

__LEAVE_FUNCTION

	return FALSE ;
}

//UInt64 TimeManager::CurrentTime( ) 
UInt64 TimeManager::NowTick()
{
    __ENTER_FUNCTION
        UInt64 m_CurrentTime = 0;
#if defined(__WINDOWS__)
        m_CurrentTime = GetTickCount64() ;
#elif defined(__LINUX__)
        gettimeofday(&_tend,&tz);
    double t1, t2;
    t1 =  (double)_tstart.tv_sec*1000 + (double)_tstart.tv_usec/1000;
    t2 =  (double)_tend.tv_sec*1000 + (double)_tend.tv_usec/1000;
    m_CurrentTime = (UINT)(t2-t1);
#endif

#if DEBUG_TIME_BOOST
    m_CurrentTime = m_StartTime + (m_CurrentTime - m_StartTime) * TEST_TIME_BOOST;
#endif

    return m_CurrentTime ;


    __LEAVE_FUNCTION

        return 0 ;
}

UINT	TimeManager::CurrentDate()
{
	__ENTER_FUNCTION

	SetTime( ) ;
	UINT Date;
	ConvertTU(&m_TM,Date);

	return Date;

	__LEAVE_FUNCTION

		return 0;
}


VOID TimeManager::CurrentDate(int &year, int &month, int &day, int &hour, int &minute, int &second)
{
	SetTime( ) ;
	year = m_TM.tm_year;
	month = m_TM.tm_mon;
	day = m_TM.tm_mday;
	hour = m_TM.tm_hour;
	minute = m_TM.tm_min;
	second = m_TM.tm_sec;
}

VOID TimeManager::SetTime( )
{
__ENTER_FUNCTION

	time( &m_SetTime ) ;
	localtime_s( &m_TM, &m_SetTime ) ;
	//m_TM = *ptm ;

__LEAVE_FUNCTION
}

// 得到标准时间
time_t TimeManager::GetANSITime( )
{
	__ENTER_FUNCTION

		SetTime();

	__LEAVE_FUNCTION

#if DEBUG_TIME_BOOST
		m_SetTime = m_SetTime + (m_SetTime-gStartTime)*TEST_TIME_BOOST;
#endif

	return m_SetTime;
}

UINT TimeManager::Time2DWORD( )
{
__ENTER_FUNCTION

	SetTime( ) ;

	UINT uRet=0 ;

	uRet += GetYear( ) ;
	uRet -= 2000 ;
	uRet =uRet*100 ;

	uRet += GetMonth( )+1 ;
	uRet =uRet*100 ;

	uRet += GetDay( ) ;
	uRet =uRet*100 ;

	uRet += GetHour( ) ;
	uRet =uRet*100 ;

	uRet += GetMinute( ) ;

	return uRet ;

__LEAVE_FUNCTION

	return 0 ;
}

UINT TimeManager::DiffTime( UINT Date1, UINT Date2 )
{
__ENTER_FUNCTION

	tm S_D1, S_D2 ;
	ConvertUT( Date1, &S_D1 ) ;
	ConvertUT( Date2, &S_D2 ) ;
	time_t t1,t2 ;
	t1 = mktime(&S_D1) ;
	t2 = mktime(&S_D2) ;
	UINT dif = (UINT)(difftime(t2,t1)*1000) ;
	return dif ;

__LEAVE_FUNCTION

	return 0 ;
}

VOID TimeManager::ConvertUT( UINT Date, tm* TM )
{
__ENTER_FUNCTION

	Assert(TM) ;
	memset( TM, 0, sizeof(tm) ) ;
	TM->tm_year = (Date>>26)&0xf ;
	TM->tm_mon  = (Date>>22)&0xf ;
	TM->tm_mday = (Date>>17)&0x1f ;
	TM->tm_hour = (Date>>12)&0x1f ;
	TM->tm_min  = (Date>>6) &0x3f ;
	TM->tm_sec  = (Date)    &0x3f ;

__LEAVE_FUNCTION
}

VOID TimeManager::ConvertTU( tm* TM, UINT& Date )
{
__ENTER_FUNCTION

	Assert( TM ) ;
	Date = 0 ;
	Date += (TM->tm_yday%10) & 0xf ;
	Date = (Date<<4) ;
	Date += TM->tm_mon & 0xf ;
	Date = (Date<<4) ;
	Date += TM->tm_mday & 0x1f ;
	Date = (Date<<5) ;
	Date += TM->tm_hour & 0x1f ;
	Date = (Date<<5) ;
	Date += TM->tm_min & 0x3f ;
	Date = (Date<<6) ;
	Date += TM->tm_sec & 0x3f ;

__LEAVE_FUNCTION
}

UINT TimeManager::GetDayTime( )
{
__ENTER_FUNCTION

	time_t st ;
	time( &st ) ;
	tm* ptm = localtime( &m_SetTime ) ;

	UINT uRet=0 ;

	uRet  = (ptm->tm_year-100)*1000 ;
	uRet += ptm->tm_yday ;

	return uRet ;

__LEAVE_FUNCTION

	return 0 ;
}

WORD TimeManager::GetTodayTime()
{
__ENTER_FUNCTION
	time_t st ;
	time( &st ) ;
	tm* ptm = localtime( &m_SetTime ) ;

	WORD uRet=0 ;

	uRet  = ptm->tm_hour*100 ;
	uRet += ptm->tm_min ;

	return uRet ;

__LEAVE_FUNCTION

return 0 ;
}

BOOL TimeManager::FormatTodayTime(WORD& nTime)
{
__ENTER_FUNCTION
	BOOL ret = FALSE;

	WORD wHour = nTime / 100;
	WORD wMin = nTime % 100;
	WORD wAddHour = 0;
	if( wMin > 59 )
	{
		wAddHour = wMin / 60;
		wMin = wMin % 60;
	}
	wHour += wAddHour;
	if( wHour > 23 )
	{
		ret = TRUE;
		wHour = wHour % 60;
	}

	return ret;

__LEAVE_FUNCTION

return FALSE ;
}

TimeManager& TimeManager::GetMe()
{
	static TimeManager sTimeMgr;
	return sTimeMgr;
}

void TimeManager::Sleep( int milSecond )
{
#if __WINDOWS__
	::Sleep(milSecond);
#elif __LINUX__
	usleep(milSecond*1000);
#else
	AssertEx(0, "²»Ö§³ÖµÄÏµÍ³");
#endif
}


UInt64 TimeManager::Now()
{
	time_t t;
	time( &t ) ;
	return (UInt64)t;
}


UInt64 TimeManager::NextTwoCenturyYearTime()
{
    time_t now;
    time(&now);
    tm tempTm = *localtime(&now);
    tempTm.tm_year = tempTm.tm_year % 100;
    tempTm.tm_year += 200;
    tempTm.tm_yday = 0;
    tempTm.tm_hour = 0;
    tempTm.tm_min = 0;
    tempTm.tm_sec = 0;

    time_t t = mktime(&tempTm);
    return (UInt64)t;
}

AString TimeManager::GetDate()
{
	AString date;
	date.Format("%4d/%2d/%2d", GetYear(), GetMonth()+1, GetDay());
	return date;
}

AString TimeManager::GetTime()
{
	AString time;
	time.Format( "%2d:%2d:%2d", GetHour(), GetMinute(), GetSecond() );
	return time;
}

AString TimeManager::GetDateTime()
{
	AString dateTime;
	dateTime = GetDate();
	dateTime += " ";
	dateTime += GetTime();

	return dateTime;
}

AString TimeManager::ToDataTime( UInt64 dateTime )
{
	time_t t = (time_t)dateTime;
	tm *tempTm = localtime(&t);

	AString strData;
	if (tempTm!=NULL)
	{
		strData.Format("%4d-%02d-%02d %02d:%02d:%02d", tempTm->tm_year+1900, tempTm->tm_mon+1, tempTm->tm_mday, tempTm->tm_hour, tempTm->tm_min, tempTm->tm_sec);
	}
	else
		strData.Format("*Invalid data time (%llu)", dateTime);
	return strData;
}

AString TimeManager::ToTimeString( UInt64 dateTime )
{
	time_t t = (time_t)dateTime;
	tm *tempTm = localtime(&t);

	AString strData;
	if (tempTm!=NULL)
	{
		strData.Format("%2d:%2d:%2d", tempTm->tm_hour, tempTm->tm_min, tempTm->tm_sec);
	}
	else
		strData.Format("*Invalid data time (%llu)", dateTime);
	return strData;
}

int TimeManager::UnixTimeToDate( UInt64 unixTime, int timeArray[] )
{
	if(unixTime < 1)
		return 0;

	time_t t = (time_t)unixTime;
	tm *tempTm = localtime(&t);

	timeArray[0] = tempTm->tm_year + 1900;
	timeArray[1] = tempTm->tm_mon + 1;
	timeArray[2] = tempTm->tm_mday;
	timeArray[3] = tempTm->tm_hour;
	timeArray[4] = tempTm->tm_min;
	timeArray[5] = tempTm->tm_sec;

	return 1;
}

bool TimeManager::UnixTimeisToday( UInt64 unixTime )
{	
	static int oneDaySecond = 24 * 3600;
	UInt64 nowTime = (int)TimeManager::Now();
	return (nowTime/oneDaySecond == unixTime/oneDaySecond);	
}

int TimeManager::GetWeekBySecond(UInt64 sec)
{
	time_t t = (time_t)sec;
	tm *tempTm = localtime(&t);
	return tempTm->tm_wday;
}

int TimeManager::GetMonthBySecond(UInt64 sec)
{
	time_t t = (time_t)sec;
	tm *tempTm = localtime(&t);
	return tempTm->tm_mon;
}

int TimeManager::GetTimingSecond(int mWeek, int mHour, int mMinute, bool bNowNext, bool bLog )
{
	if (mWeek<0 || mWeek>6 || mHour<0 || mHour>23 || mMinute<0 || mMinute>59)
	{
		ERROR_LOG("提供的时间不正确 >%d - %d - %d, 格式为 星期(0~6)-小时(0~23)-分钟(0~59)", mWeek, mHour, mMinute);
		return -1;
	}
	// 下下周一, 用当前时间+ 7 天后的, 即在下下周内, 然后取周一的时间
	time_t now;
	time(&now);

	time_t startTime = now;

	tm tempTm = *localtime(&now);
	if (mWeek > tempTm.tm_wday)
		startTime = now + (mWeek -tempTm.tm_wday)*24*3600; // 如果取得相同星期必须加上当前星期与指定星期之间的差, 不可以直接设置星期, 转换后无效
	else if (mWeek<tempTm.tm_wday)
		startTime = now + (mWeek+7-tempTm.tm_wday) * 24 *3600;

	tempTm = *localtime(&startTime);

	tempTm.tm_hour = mHour;		
	tempTm.tm_min = mMinute;

	time_t waitTime = mktime(&tempTm);

	if (!bNowNext && waitTime==now)
		return 0;

	if (waitTime<=now)
	{
		time_t tempSecond = startTime + (7*24*3600);
		tempTm = *localtime(&tempSecond);

		tempTm.tm_hour = mHour;		
		tempTm.tm_min = mMinute;

		waitTime = mktime(&tempTm);
	}

	int t = (int)(waitTime-now);
	if (bLog)
		NOTE_LOG("定时时间 [星期小时分钟]>[%d], 日期 [%d-%d %d:%d], Config [%d-%d-%d],  now %s", t, tempTm.tm_mon+1, tempTm.tm_mday, tempTm.tm_hour, tempTm.tm_min, mWeek, mHour, mMinute, TimeManager::GetMe().GetDateTime().c_str());
	return t;
}


int TimeManager::GetTimingSecond(int mHour, int mMinute, bool bNowNext /*= true*/)
{
	if (mHour<0 || mHour>23 || mMinute<0 || mMinute>59)
	{
		ERROR_LOG("提供的时间不正确 >%d - %d, 格式为 小时(0~23)-分钟(0~59)",  mHour, mMinute);
		return -1;
	}

	time_t now;
	time(&now);

	time_t startTime = now;

	tm tempTm = *localtime(&now);

	tempTm.tm_hour = mHour;		
	tempTm.tm_min = mMinute;

	time_t waitTime = mktime(&tempTm);

	if (!bNowNext && waitTime==now)
		return 0;

	if (waitTime<=now)
	{
		time_t tempSecond = startTime + (24*3600);
		tempTm = *localtime(&tempSecond);

		tempTm.tm_hour = mHour;		
		tempTm.tm_min = mMinute;

		waitTime = mktime(&tempTm);
	}

	int t = (int)(waitTime-now);
	TABLE_LOG("定时时间 >[%d], 日期 [%d-%d %d:%d], Config [%d-%d],  now %s", t, tempTm.tm_mon+1, tempTm.tm_mday, tempTm.tm_hour, tempTm.tm_min, mHour, mMinute, TimeManager::GetMe().GetDateTime().c_str());
	return t;
}

int TimeManager::GetMdayTimingSceond(int Mday, int mHour, int mMinute, bool bNowNext /*= true*/)
{
	if (Mday<1 || Mday>31 || mHour<0 || mHour>23 || mMinute<0 || mMinute>59)
	{
		ERROR_LOG("提供的时间不正确 >%d号 - %d - %d, 格式为 (1~31)号-小时(0~23)-分钟(0~59)", Mday, mHour, mMinute);
		return -1;
	}

	time_t now;
	time(&now);

	time_t startTime = now;

	tm tempTm = *localtime(&now);
	tempTm.tm_mday = Mday;

	tempTm.tm_hour = mHour;		
	tempTm.tm_min = mMinute;

	time_t waitTime = mktime(&tempTm);

	if (!bNowNext && waitTime==now)
		return 0;

	if (waitTime<=now)
	{
		tempTm.tm_mon += 1;
		if (tempTm.tm_mon>11)
		{
			tempTm.tm_year += 1;
			tempTm.tm_mon = 0;
		}

		tempTm.tm_hour = mHour;		
		tempTm.tm_min = mMinute;

		waitTime = mktime(&tempTm);
	}

	int t = (int)(waitTime-now);
	TABLE_LOG("定时时间 [号小时分钟] >[%d], 日期 [%d-%d %d:%d], Config [%d-%d-%d],  now %s", t, tempTm.tm_mon+1, tempTm.tm_mday, tempTm.tm_hour, tempTm.tm_min, Mday, mHour, mMinute, TimeManager::GetMe().GetDateTime().c_str());
	return t;
}
