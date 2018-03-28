

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
// �ֶ���Ϣ֧���̰߳�ȫ,���߳�ʹ�ö����̶߳��� >FieldInfoManager::GetMe
#define	FIELD_INFO_THREAD_SAFE				0

#pragma warning(push)
#pragma warning(disable:4005)

typedef int				DSIZE;

typedef unsigned char	BYTE;
typedef unsigned char	UCHAR;			//��׼�޷���CHAR
typedef char			CHAR;			//��׼CHAR
typedef unsigned int	UINT;			//��׼�޷���INT
typedef int				INT;			//��׼INT
typedef unsigned short	USHORT;			//��׼�޷���short
typedef short			SHORT;			//��׼short
typedef unsigned long	ULONG;			//��׼�޷���LONG(���Ƽ�ʹ��)
typedef long			LONG;			//��׼LONG(���Ƽ�ʹ��)
typedef float			FLOAT;			//��׼float
typedef unsigned long	DWORD;
typedef USHORT			WORD;

typedef UCHAR			uchar;
typedef USHORT			ushort;
typedef UINT			uint;
typedef ULONG			ulong;

typedef UINT			ID_t;
typedef UINT			ObjID_t;			//�����й̶�������OBJӵ�в�ͬ��ObjID_t
typedef UINT			GUID_t;				//32λΨһ��š�
typedef INT				Time_t;				//ʱ������

typedef ID_t			TeamID_t;			//����ID
typedef ID_t			SceneID_t;			//����ID
typedef ID_t			PlayerID_t;			//�������
typedef ID_t			SkillID_t;			//����
typedef ID_t			ActionID_t;			//������ID
typedef ID_t			ImpactID_t;			//Ч��ID
typedef ID_t			ImpactClassID_t;	//Ч������ID
typedef ID_t			Coord_t;			//���������
typedef ID_t			ScriptID_t;			//�ű�
typedef ID_t			MissionID_t;		//����
typedef ID_t			GroupID_t;			//��ID
typedef ID_t			ChannelID_t;		//Ƶ��ID
typedef ID_t			CampID_t;			//��ӪID
typedef ID_t			MenPaiID_t;			//����ID
typedef ID_t			GuildID_t;			//����ID
typedef ID_t			CityID_t;			//����ID
typedef ID_t			ZoneID_t;			//����ID

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
#define VOID	void			//��׼��
#define NULL	0

#define INVALID_HANDLE	(-1)		//��Ч�ľ��
#define INVALID_ID		(-1)		//��Ч��IDֵ

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
#define DB_BLOB_FIELD_DEFAULT_LENGTH				(16*1024)			// DB blob �ֶ�Ĭ�ϳ���

// ��־���
#define LOG_INFO_MAX_LENGTH		1024
#define LOG_DATA_TIME_LENGTH	21
#define CONFIG_LIMIT_MAX_ID		1000000

#endif //_INCLUDE_BASECOMMON_H_

//-------------------------------------------------------------------------