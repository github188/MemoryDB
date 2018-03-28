

#ifndef __WINDOWS__
    #define __LINUX__ 1

    #define MAX_PATH 256

    #define _UNICODE

    #define STATE_BASECOMMON_LIB
    #define STATE_NET_LIB
    #define STATE_EVENTCORE_LIB
    #include <stdlib.h>
#endif

//-------------------------------------------------------------------------
#ifndef BaseCommon_Export
#ifdef STATE_BASECOMMON_LIB
#	define BaseCommon_Export
#	define BaseCommon_Export_H
#else
#		ifdef BASECOMMON_EXPORTS
#			define BaseCommon_Export __declspec(dllexport)
#			define BaseCommon_Export_H	__declspec(dllexport)
#		else
#			define BaseCommon_Export __declspec(dllimport)
#			define BaseCommon_Export_H
#		endif
#endif
#endif

//-------------------------------------------------------------------------
#ifndef _INCLUDE_BASECOMMON_H_
#define _INCLUDE_BASECOMMON_H_

#ifndef _CRT_RAND_S 
# define _CRT_RAND_S 
#endif
//#ifndef __LINUX__
//#	define __WINDOWS__  1
//#endif
// 字段信息支持线程安全,各线程使用独立线程对象 >FieldInfoManager::GetMe
#define	FIELD_INFO_THREAD_SAFE				0

#pragma warning(push)
#pragma warning(disable:4005)

typedef int				DSIZE;

typedef unsigned char	BYTE;
typedef unsigned char	UCHAR;			//标准无符号CHAR
typedef char			CHAR;			//标准CHAR
typedef unsigned int	UINT;			//标准无符号INT
typedef int				INT;			//标准INT
typedef unsigned short	USHORT;			//标准无符号short
typedef short			SHORT;			//标准short
typedef unsigned long	ULONG;			//标准无符号LONG(不推荐使用)
typedef long			LONG;			//标准LONG(不推荐使用)
typedef float			FLOAT;			//标准float
typedef unsigned long	DWORD;
typedef USHORT			WORD;

typedef UCHAR			uchar;
typedef USHORT			ushort;
typedef UINT			uint;
typedef ULONG			ulong;

typedef UINT			ID_t;
typedef UINT			ObjID_t;			//场景中固定的所有OBJ拥有不同的ObjID_t
typedef UINT			GUID_t;				//32位唯一编号。
typedef INT				Time_t;				//时间类型

typedef ID_t			TeamID_t;			//队伍ID
typedef ID_t			SceneID_t;			//场景ID
typedef ID_t			PlayerID_t;			//连接玩家
typedef ID_t			SkillID_t;			//技能
typedef ID_t			ActionID_t;			//动作的ID
typedef ID_t			ImpactID_t;			//效果ID
typedef ID_t			ImpactClassID_t;	//效果分组ID
typedef ID_t			Coord_t;			//网格坐标点
typedef ID_t			ScriptID_t;			//脚本
typedef ID_t			MissionID_t;		//任务
typedef ID_t			GroupID_t;			//团ID
typedef ID_t			ChannelID_t;		//频道ID
typedef ID_t			CampID_t;			//阵营ID
typedef ID_t			MenPaiID_t;			//门派ID
typedef ID_t			GuildID_t;			//帮派ID
typedef ID_t			CityID_t;			//城市ID
typedef ID_t			ZoneID_t;			//区域ID

#ifdef __LINUX__
    typedef signed char             BOOL;

#else
    typedef int						BOOL;
#endif
    typedef	unsigned char			byte;



#ifdef __LINUX__
    typedef long long               BIGINT;
    typedef long long               Int64;
    typedef unsigned long long		UInt64;
#else
    typedef __int64                 BIGINT;
    typedef __int64                 Int64;
    typedef unsigned __int64		UInt64;
#endif

typedef int							KEY;
typedef UInt64						GSKEY;
typedef	int							DBCODE;

#define TRUE	(1)
#define FALSE	(0)
#define VOID	void			//标准空
#define NULL	0

#define INVALID_HANDLE	(-1)		//无效的句柄
#define INVALID_ID		(-1)		//无效的ID值

#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable:4275)
#pragma warning(disable:4251)
#pragma warning(disable:4996)
#pragma warning(disable:4267)


#if defined(__WINDOWS__) || defined(WIN32)
#	define		tvsnprintf		_vsnprintf
#	define		tstricmp		_stricmp
#	define		tsnprintf		_snprintf
#elif defined(__LINUX__)
#	define		tvsnprintf		vsnprintf
#	define		tstricmp		strcasecmp
#	define		tsnprintf		snprintf
#endif

#define MIN(a, b)	(a<b ? a:b)
#define MAX(a, b)	(a>b ? a:b)

//-------------------------------------------------------------------------
#define DBIP		"DBIP"
#define DBPORT		"DBPORT"
#define DBUSER		"DBUSER"
#define DBPASSWORD	"DBPASSWORD"
#define DBBASE		"DBBASE"
#define DBNAME		"DBNAME"
#define COOLTIME	"COOLTIME"
#define ISLOCALDB	"LOCAL_DB"
//-------------------------------------------------------------------------
#define TABLE_USE_STL_INDEX		1
#define DB_BLOB_FIELD_DEFAULT_LENGTH				(16*1024)			// DB blob 字段默认长度

// 日志相关
#define LOG_INFO_MAX_LENGTH		1024
#define LOG_DATA_TIME_LENGTH	21
#define CONFIG_LIMIT_MAX_ID		1000000

#endif //_INCLUDE_BASECOMMON_H_

//-------------------------------------------------------------------------