

#ifndef __CONFIG_H__
#define __CONFIG_H__
#include "ServerBaseHead.h"
#include "BaseType.h"

struct _REGION_INFO
{
	INT				m_nRegionIndex;			//��Ϸ���������룬ֻ�ܴ�001�ŵ�420�ţ���
	CHAR			m_RegionName[_MAX_PATH];//������������
};

// ���������� [5/10/2010 knowC]
struct HIEROGRAM_CONFIG
{
	INT		m_nAddOnceTimeMS;
	INT		m_nMaxTotalTimeMS;
	INT		m_nMaxScanRadius;
	INT		m_nEffect_SkillID;
	HIEROGRAM_CONFIG()				//Ĭ��ֵ
	{
		m_nAddOnceTimeMS	=	10 * 60 * 1000;
		m_nMaxTotalTimeMS	=	30 * 60 * 1000;
		m_nMaxScanRadius	=	2;
		m_nEffect_SkillID	=	224;
	}
};

//����ϵͳ����   [12/28/2010 knowC]
struct MOUNT_CONFIG
{
	INT		m_nRecoverTimeInterval;
	INT		m_nRecoverPercent;
	MOUNT_CONFIG()
	{
		m_nRecoverTimeInterval	= 10000;
		m_nRecoverPercent	=	1;
	}
};

////������������ add by zengwen
//struct _JINGJIE_COINFIG
//{
//	//������
//	INT				m_jingMaiCnt;
//	//��������
//	INT				m_AssistItemID;
//	//����һ�㾳��ʱ������һ�����Ե���
//	INT				m_bonusAttrLv1Point;
//	//�������������洢����
//	INT				m_limitTrainingPoint[MAX_JINGJIE_LEVEL];
//	//��ͨ��Ӧ�����������㾳��
//	INT				m_addJingJieLevel[JMT_COUNT];
//	//�������ߵ���Ч����(����Ч��������������1��Ӱ�����������Ӧ���������������2��Ӱ�����)
//	INT				m_assistItemValidRate;
//	//1���ĸ�����ָʹ�õ��ߺ�����Ӧ�ĸ�������1��Ѩλ,����ͨ��Ѩλ����һ�����Դ�ͨ2��Ѩλ
//	INT				m_assistItemRateLv1;
//	//ÿ������ٺ���ִ��һ�ν��������Ĳ���
//	INT				m_bonusPointTimer;
//	//ÿ����һ���ӿɻ�õ���������
//	INT				m_onlineBonusPoint;
//	//�����㱶��
//	FLOAT			m_trainingPointMultiple;
//	//����ʱÿ�봫����������
//	INT				m_expressTrainingpointSpeed;
//
//	_JINGJIE_COINFIG()
//	{
//		m_assistItemValidRate = 0;
//		m_assistItemRateLv1 = 0;
//		m_AssistItemID = INVALID_ID;
//		m_jingMaiCnt = JMT_COUNT;
//		m_bonusAttrLv1Point = 0;
//		m_bonusPointTimer = 1000;
//		m_onlineBonusPoint = 0;
//		m_trainingPointMultiple = 0.0F;
//		m_expressTrainingpointSpeed = 1;
//		for (INT iIndex = 0; iIndex < MAX_JINGJIE_LEVEL; iIndex++)
//		{
//			m_limitTrainingPoint[iIndex] = 0;
//		}
//		for (INT iIndex = 0; iIndex < JMT_COUNT; iIndex++)
//		{			
//			m_addJingJieLevel[iIndex] = 1;
//		}
//	}
//	INT GetTrainingPointLimitByLevel(const INT iLevel)
//	{
//		if (iLevel < 1 || iLevel > MAX_JINGJIE_LEVEL)
//		{
//			AssertEx(FALSE, "GetTrainingPointLimitByLevel�����Ľ��յĵȼ�ֵ�Ƿ�!");
//			return 0;
//		}
//		
//		INT iPoint = m_limitTrainingPoint[iLevel - 1];
//		if (iPoint > 0)
//		{
//			return iPoint;
//		}
//
//		return 0;		
//	}
//	
//
//};

//Config
struct _CONFIG_INFO
{
	INT				m_SystemModel;
	ID_t			m_ServerID ;
	INT				m_ZoneSize ;
	INT				m_DropParam;				//ȫ�ֵ������[0-100]
	FLOAT			m_ExpParam;					//ȫ�־�����������
	INT				m_EquipDamagePointOnBeHit;		//��Ʒ�;öȵ�΢��ֵ
	INT				m_EquipDamagePointOnHitTar;		//��Ʒ�;öȵ�΢��ֵ
	INT				m_RecoverTime ;
	INT				m_MaxMonster ;
	INT				m_MaxPet ;
	INT				m_PetBodyTime ;
	INT				m_PetHappinessInterval;		// ������ֶ��½�1��ʱ����(����)
	INT				m_PetLifeInterval;			// ���������½�1��ʱ����(����)
	INT				m_PetCallUpHappiness;		// ������ֶȴ��ڲſ����ٳ�
	UINT			m_PetPlacardTime;			// ����һ�����﹫��ĳ���ʱ��
	INT				m_PetPlacardNeedLevel;		// �������﹫��ĳ������輶��
	INT				m_PetPlacardNeedHappiness;	// �������﹫��ĳ���������ֶ�
	INT				m_PetPlacardNeedLife;		// �������﹫��ĳ�����������
	INT				m_DefaultRespawnTime ;
	INT				m_DropBoxRecycleTime;
	INT				m_TimeChangeInterval;		//ʱ��������
	INT				m_PositionRange ;
	INT				m_AIType ;
	INT				m_DisconnectTime ;
	CHAR			m_UserPath[_MAX_PATH] ;
	INT				m_MaxPortal ;
	INT				m_MaxSkillObj ;
	INT				m_DefaultBodyTime ;			//ʬ�����ʱ��
	INT				m_OutGhostTime ;			//�ͷ����ʱ��
	FLOAT			m_CanGetExpRange ;			//ɱ����ɫ����Եõ�EXP�ķ�Χ
	INT				m_DefaultMoveSpeed ;
	INT				m_DefaultAttackSpeed ;
	INT				m_HumanVERecoverInterval;	// ����������Ļָ�ʱ����(����)
	INT				m_WallowAge;
	INT				m_WallowStartTime;
	INT				m_WallowStartTime2;
	INT				m_WallowAwokeTime;
	INT				m_DefaultRefreshRate ;		//������ˢ��ʱ��
	INT				m_DefaultCheckZoneRate ;	//ˢ��������Ϣ��ʱ����
	INT				m_DefaultCheckMisSumbitTagRate ;//������ύ��ʶ�ļ��ʱ���� add by zengwen
	INT				m_DefaultRefuseScanTime;	//ȱʡ���������󲻻�ɨ����˵�ʱ��
	INT				m_MaxPlatform;
	INT				m_MaxSpecial;				// ����OBJ������
	INT				m_MaxPlayerShopNum;			// ����̵������
	INT             m_MaxTimerCount;			//������ʱ����Ŀ
	INT				m_nMaxHumanUseTimer;		//Obj_Human��ʹ�ü�ʱ����Ŀ
	INT				m_nAvailableFollowDist;		// ��Ӹ������Ч����
	INT				m_nTimeForLoseFollow;		// ��Ӹ��泬����Ч����೤ʱ��������˳���Ӹ���(��)
	INT				m_nOutofBeDamagedTime;		// ��һ�����ֵĸ��˻����ӵ��һ��N��Ĺ���Ȩ
	INT				m_nDeadMonsterScanZone;		// �������ڹ��ܱ�N�������������˻����Ϊ�˹ֹ�����
	INT				m_nExpIncreaseRate;			// ɱ����ʱÿ��һ���ܾ������ӵİٷֱ��ʣ�����ʱ�谴����ӳɹ����ܾ�����ȥ���������ٷ��䵽ÿ���� 
	INT				m_nExpIncreaseRateExtra;	// ���������жϸ������һ������һ���ҵĳ����ö���ٷֱȵĻ������齱��
	INT				m_nDiceBetTime;				// ���㷽ʽʰȡ��ʱ�ĶĲ�ʱ��(��)
	INT				m_nFoundDurationHour;		// ��������
	INT				m_nDefaultMaxMemberCount;	// Ĭ�ϰ�����ɰ�����
	INT				m_nResponseUserCount;		// ���������Ҫ����Ӧ���������������
	INT				m_nPasswdPoint;				// ���ٵ��Ѻö���Ҫ�����������
	INT				m_nPromptPoint;				// ���ٵ��Ѻö���Ҫ��ʾ������
	INT				m_nDeleteDelayTime;			// ����Сʱ�Ժ��ٴ�ǿ�ƽ����ɾ����������
	INT				m_nExpPoint;				// ��ö��پ�����������Ѻö�
	INT				m_nHashOnlineUserCount ;	//������ҵ�hash���С
	INT				m_nHashMailUserCount ;		//ӵ���ʼ������hash���С
	INT				m_nMaxOfflineUserCount ;	//��½����Ϸ������������ֵ
	FLOAT			m_fRespawnParam ;			//����ˢ������ϵ��
	INT				m_KickUserTime ;			//����������Ϸ��ҵ��߳�����ʱ��
	INT				m_nDefaultDamageFluctuation; // ȱʡ���˺�����

	INT				m_nMinGoodBadValue;			// ��С�ƶ�ֵ
	INT				m_nMaxGoodBadValue;			// ����ƶ�ֵ
	INT				m_nLevelNeeded;				// ����ƶ�ֵ����͵ȼ�
	INT				m_nMemberLevel;				// �ܸ��ӳ������ƶ�ֵ�Ķ�����ߵȼ�
	FLOAT			m_fGoodBadRadius;			// ����ƶ�ֵ�İ뾶
	INT				m_nBonusPerMember;			// ÿ����Ч���������ӵ��ƶ�ֵ����
	INT				m_nMaxBonus;				// ɱ��һֻ���ܸ��ӳ�����������ƶ�ֵ����
	INT				m_nPenaltyWhenMemberDie;	// ��Ч���ѵ��������ٶӳ����ƶ�ֵ����
	INT             m_nWorldChatItemIndex;

	HIEROGRAM_CONFIG	m_HierogramConfig;		//����������

	_REGION_INFO	m_ThisRegion;				//��ǰ��Ϸ��
	_REGION_INFO*	m_OldRegion;				//����
	INT				m_nOldRegionCount;			//�����ĸ���

	INT				m_nEnemyAvailabTime;		//���˵���Чʱ�䣬������ֵ�����˱�ɾ��,����Ϊ��λ	
	INT				m_nEnemyTimerInterval;			
 	INT				m_nFriendTimerInterval;	

	INT				m_RedValue;				// 		redValue=100;����
	INT				m_BlackValue;			// 		blackvalue=300;����
	INT				m_KillerAddValue;		// 		killerAddValue=100;�������ӵ�ɱ��
	INT				m_HelperAddValue;		// 		helperAddValue=50;�������ӵ�ɱ��
	INT				m_PkProtectLevel;		// 		pkProtectLevel=10;PK�����ȼ�
	INT				m_HelperCheckTime;		//		helperCheckTime=30;�ж����׵ļ��ʱ��,��������ǰ�Ƶ�ʱ��
	INT				m_BreakAwayPKTime;		//		breakAwaypkTime=6;������ս������ս��״̬��ʱ��
	INT				m_PkChangeTime;			//		pkModeChangeTime=60;��Ҵ�ɱ¾�л��غ�ƽ��ʱ��

	MOUNT_CONFIG	m_MountConfig;			//��������

	_CONFIG_INFO( )
	{
		m_DropParam				= 100;
		m_ServerID				= INVALID_ID ;
		m_ZoneSize				= 10 ;
		m_EquipDamagePointOnBeHit	=	1000;		//  [1/10/2011 knowC]
		m_EquipDamagePointOnHitTar	=	1000;		//  [1/10/2011 knowC]
		m_RecoverTime			= 1000 ;
		m_MaxMonster			= 1000 ;
		m_MaxPet				= 1000 ;
		m_PetBodyTime			= 2000 ;
		m_PetHappinessInterval	= 600000;
		m_PetLifeInterval		= 30 * 60 * 1000;
		m_PetCallUpHappiness	= 60;
		m_PetPlacardTime		= 1000*60*30;
		m_DefaultRespawnTime	= 300000 ;
		m_DropBoxRecycleTime	= 300000 ;
		m_TimeChangeInterval	= 300000 ;
		m_PositionRange			= 10 ;
		m_AIType				= 1 ;
		m_DisconnectTime		= 0 ;
		memset( m_UserPath, 0, _MAX_PATH ) ;
		m_MaxPortal				= 0 ;
		m_MaxPlatform			= 0;
		m_MaxSpecial			= 1024;
		m_MaxPlayerShopNum		= 512;
		m_MaxSkillObj			= 1024;
		m_DefaultBodyTime		= 0 ;
		m_OutGhostTime			= 0 ;
		m_CanGetExpRange		= 30.f;
		m_DefaultMoveSpeed      = 2800 ;
		m_DefaultAttackSpeed    = 2000 ;
		m_HumanVERecoverInterval= 300000;
		m_DefaultRefreshRate    = 60000 ;
		m_DefaultCheckZoneRate  = 1000 ;
		m_DefaultCheckMisSumbitTagRate = 1000;
		m_nAvailableFollowDist	= 15;
		m_nTimeForLoseFollow	= 30;
		m_nOutofBeDamagedTime	= 8;
		m_nDeadMonsterScanZone	= 5;
		m_nExpIncreaseRate		= 0;
		m_nExpIncreaseRateExtra = 0;
		m_nDiceBetTime			= 1;
		m_nFoundDurationHour	= 24;
		m_nDefaultMaxMemberCount= 160;
		m_nResponseUserCount	= 10;
		m_nPasswdPoint			= 0;
		m_nPromptPoint			= 0;
		m_nDeleteDelayTime		= 0;
		m_nExpPoint				= 0;
		m_nHashOnlineUserCount	= 1000 ;
		m_nHashMailUserCount	= 1000 ;
		m_nMaxOfflineUserCount	= 0 ;
		m_fRespawnParam			= 1.0f ;
		m_KickUserTime			= 300000 ;
		m_nDefaultDamageFluctuation = 10 ;

		m_nMinGoodBadValue		= 0;
		m_nMaxGoodBadValue		= 9999;
		m_nLevelNeeded			= 31;
		m_nMemberLevel			= 19;
		m_fGoodBadRadius		= 15.;
		m_nBonusPerMember		= 1;
		m_nMaxBonus				= 2;
		m_nPenaltyWhenMemberDie	= 10;

		m_nWorldChatItemIndex	= 0;

		m_nOldRegionCount		= 0;
		m_OldRegion				= NULL;

		m_nEnemyAvailabTime		= 0;
		m_nEnemyTimerInterval	=	1000 * 60;//Ĭ��һ����
		m_nFriendTimerInterval	=	1000 * 60;//Ĭ��һ����
	};
	~_CONFIG_INFO( )
	{
		SAFE_DELETE(m_OldRegion);
	};
};

//Login
struct _LOGIN_INFO
{
	enum ASKAUTHTYPE
	{
		USE_BILLING = 0,
		USE_WEB,
	};
	ID_t			m_LoginID ;
	CHAR			m_DBIP[IP_SIZE];
	UINT			m_DBPort;
	CHAR			m_DBName[DATABASE_STR_LEN];
	CHAR			m_DBUser[DB_USE_STR_LEN];
	CHAR			m_DBPassword[DB_PASSWORD_STR_LEN];
	INT			    m_AskAuthType;
	CHAR			m_ClientMainPath[MAX_PATH];

	_LOGIN_INFO()
	{
		m_LoginID = INVALID_ID;
		m_AskAuthType = USE_BILLING;
	}
	~_LOGIN_INFO()
	{

	}
};

//Billing
struct _BILLING_INFO
{
	ID_t			m_BillingID;
	CHAR			m_OuterIP[IP_SIZE] ;
	CHAR			m_IP[IP_SIZE] ;
	UINT			m_Port ;
	CHAR			m_DBIP[IP_SIZE];
	UINT			m_DBPort;
	CHAR			m_DBName[DATABASE_STR_LEN];
	CHAR			m_DBUser[DB_USE_STR_LEN];
	CHAR			m_DBPassword[DB_PASSWORD_STR_LEN];
	CHAR			m_WebIP[IP_SIZE];
	UINT			m_WebPort;
	CHAR			m_WebLinkWebKEY[KEY_SIZE];
	UINT			m_WebServerId;

	_BILLING_INFO()
	{
		m_BillingID	 = INVALID_ID;
		memset( m_OuterIP, 0, IP_SIZE ) ;
		memset( m_IP, 0, IP_SIZE ) ;
		m_Port = 0 ;
		memset(m_DBIP,0,IP_SIZE);
		m_DBPort		= INVALID_ID;
		memset(m_DBName,0,DATABASE_STR_LEN);
		memset(m_DBUser,0,DB_USE_STR_LEN);
		memset(m_DBPassword,0,DB_PASSWORD_STR_LEN);
		memset(m_WebIP,0,IP_SIZE);
		m_WebPort = 0;
		memset(m_WebLinkWebKEY,0,KEY_SIZE);
		m_WebServerId = INVALID_ID;
	}

	~_BILLING_INFO()
	{

	}
};

//World
struct _WORLD_INFO
{
	ID_t			m_WorldID ;
	SM_KEY			m_GuildKey;
	SM_KEY			m_MailKey;
	BOOL			m_EnableShareMem;
	_WORLD_INFO( )
	{
		m_WorldID	= INVALID_ID ;
		m_GuildKey	=	0;
		m_MailKey	=	0;
		m_EnableShareMem = FALSE;
	}
	~_WORLD_INFO( )
	{
	}
};

//ShareMem

struct _SHAREMEM_DATA
{
	SM_KEY				m_Key;
	SHAREMEM_TYPE		m_Type;

	_SHAREMEM_DATA()
	{
		m_Key	=	0;
		m_Type	=	ST_INVAILD;
	}

};

// ͨ�������ڴ淽ʽʵ�ֵ���ϢͨѶ�õ��Ĺ����ڴ��KEY,һ����������������KEY(�ӷ�).
typedef std::vector<std::pair<SM_KEY,SM_KEY>>	SMU_MESSAGE_KEY;

struct _SHAREMEM_INFO 
{
	UINT				m_SMUObjCount;

	_SHAREMEM_DATA*	    m_pShareData;

	CHAR			m_DBIP[IP_SIZE];
	UINT			m_DBPort;
	CHAR			m_DBName[DATABASE_STR_LEN];
	CHAR			m_DBUser[DB_USE_STR_LEN];
	CHAR			m_DBPassword[DB_PASSWORD_STR_LEN];
	INT				SMUInterval;
	INT				DATAInterval;

	SMU_MESSAGE_KEY	mSUMMsgKey;
	

	_SHAREMEM_INFO()
	{
		m_pShareData	=	NULL;
		m_SMUObjCount	=	0;
		memset(m_DBIP,0,IP_SIZE);
		m_DBPort		= -1;
		memset(m_DBName,0,DATABASE_STR_LEN);
		memset(m_DBUser,0,DB_USE_STR_LEN);
		memset(m_DBPassword,0,DB_PASSWORD_STR_LEN);
		SMUInterval		= 60;
		DATAInterval	= 300000;
	}
	~_SHAREMEM_INFO()
	{
		SAFE_DELETE_ARRAY(m_pShareData);
	};
};

//Machine
struct _MACHINE_DATA
{
	ID_t			m_MachineID ;

	_MACHINE_DATA( )
	{
		Init( ) ;
	};
	VOID Init( )
	{
		m_MachineID = INVALID_ID ;
	}
};

struct _MACHINE_INFO
{
	_MACHINE_DATA*	m_pMachine ;
	UINT			m_MachineCount ;

	_MACHINE_INFO( )
	{
		m_pMachine = NULL ;
		m_MachineCount = 0 ;
	};
	~_MACHINE_INFO( )
	{
		SAFE_DELETE_ARRAY( m_pMachine ) ;
		m_MachineCount = 0 ;
	};
};

//Server
struct _SERVER_DATA
{
	ID_t			m_ServerID ;
	ID_t			m_MachineID ;
	CHAR			m_IP0[IP_SIZE] ;
	UINT			m_Port0 ;
	CHAR			m_IP1[IP_SIZE] ;
	UINT			m_Port1 ;
	INT				m_Type ;

	SM_KEY			m_HumanSMKey;
	SM_KEY			m_PlayShopSMKey;
	SM_KEY			m_ItemSerialKey;
	BOOL			m_EnableShareMem;


	_SERVER_DATA( )
	{
		Init( ) ;
	};
	VOID Init( )
	{
		m_ServerID = INVALID_ID ;
		m_MachineID = INVALID_ID ;
		memset( m_IP0, 0, IP_SIZE ) ;
		m_Port0 = 0 ;
		memset( m_IP1, 0, IP_SIZE ) ;
		m_Port1 = 0 ;
		m_Type = -1 ;
		m_EnableShareMem = FALSE;
	}
};
struct _SERVER_WORLD
{
	CHAR			m_IP[IP_SIZE] ;
	UINT			m_Port ;

	_SERVER_WORLD( )
	{
		Init( ) ;
	}
	VOID Init( )
	{
		memset( m_IP, 0, IP_SIZE ) ;
		m_Port = 0 ;
	}
};



#define OVER_MAX_SERVER 256
struct _SERVER_INFO
{
	_SERVER_DATA*	m_pServer ;	
	UINT			m_ServerCount ;
	INT				m_HashServer[OVER_MAX_SERVER] ;
	_SERVER_WORLD	m_World ;

	_SERVER_INFO( )
	{
		m_pServer = NULL ;
		m_ServerCount = 0 ;
		for( INT i=0; i<OVER_MAX_SERVER; i++ )
		{
			m_HashServer[i] = -1 ;
		}
	}
	~_SERVER_INFO( )
	{
		SAFE_DELETE_ARRAY( m_pServer ) ;
		m_ServerCount = 0 ; 
	}
};


//Scene
struct _SCENE_DATA
{
	SceneID_t		m_SceneID ;					//����ID
	INT				m_ResID;					//�ͻ���ʹ����ԴID add by zcx 2010.2.8
	INT				m_IsActive ;				//�ǲ��ǿ���ʹ�õĳ���
	CHAR			m_szName[_MAX_PATH] ;		//��������
	CHAR			m_szFileName[_MAX_PATH] ;	//������Դ�ļ����ļ���
	ID_t			m_ServerID ;				//���д˳�����Server�˳���ID
	ID_t			m_Type ;					//��ǰ��������ĳ������
												//���ֵΪINVALID_ID�����ڸ���
	ID_t			m_ThreadIndex ;				//�����̵߳�����
	


	_SCENE_DATA()
	{
		Init( ) ;
	};
	VOID Init( )
	{
		m_SceneID = INVALID_ID ;
		m_IsActive = 1 ;
		memset( m_szName, 0, _MAX_PATH ) ;
		memset( m_szFileName, 0, _MAX_PATH ) ;
		m_ServerID = INVALID_ID ;
		m_Type = INVALID_ID ;
		m_ThreadIndex = INVALID_INDEX ;
	}
};

struct _SCENE_INFO
{
	_SCENE_DATA*	m_pScene ;
	SceneID_t		m_SceneCount ;
	INT				m_HashScene[MAX_SCENE] ;

	_SCENE_INFO(){
		m_pScene = NULL ;
		m_SceneCount = 0 ;
		for( INT i=0; i<MAX_SCENE; i++ )
		{
			m_HashScene[i] = INVALID_ID ;
		}
	};
	~_SCENE_INFO(){
		SAFE_DELETE_ARRAY( m_pScene ) ;
		m_SceneCount = 0 ;
	};
};



class ServerBase_Dll_Export Config
{
public :
	Config( ) ;
	~Config( ) ;

	BOOL					Init( ) ;
	VOID					ReLoad( ) ;


	VOID					LoadConfigInfo( ) ;
	VOID					LoadConfigInfo_Only( ) ;
	VOID					LoadConfigInfo_ReLoad( ) ;

	VOID					LoadLoginInfo( );
	VOID					LoadLoginInfo_Only( );
	VOID					LoadLoginInfo_Reload( );

	VOID					LoadWorldInfo( ) ;
	VOID					LoadWorldInfo_Only( ) ;
	VOID					LoadWorldInfo_ReLoad( ) ;

	VOID					LoadBillingInfo( ) ;
	VOID					LoadBillingInfo_Only( ) ;
	VOID					LoadBillingInfo_ReLoad( ) ;

	VOID					LoadShareMemInfo();
	VOID					LoadShareMemInfo_Only();
	VOID					LoadShareMemInfo_ReLoad();
	
	VOID					LoadMachineInfo( ) ;
	VOID					LoadMachineInfo_Only( ) ;
	VOID					LoadMachineInfo_ReLoad( ) ;
	
	VOID					LoadServerInfo( ) ;
	VOID					LoadServerInfo_Only( ) ;
	VOID					LoadServerInfo_ReLoad( ) ;
	
	VOID					LoadSceneInfo( ) ;
	VOID					LoadSceneInfo_Only( ) ;
	VOID					LoadSceneInfo_ReLoad( );

	ID_t					SceneID2ServerID(SceneID_t sID) const;
	ID_t					SceneResID2ServerID(INT sID) const;
	SceneID_t				SceneResID2SceneID(INT sID) const;
	INT						SceneID2SceneResID(SceneID_t sID) const;

	ID_t					Key2ServerID(SM_KEY key)	const;

	VOID					LoadCopySceneInfo( ) ;
	VOID					LoadCopySceneInfo_Only( ) ;
	VOID					LoadCopySceneInfo_ReLoad( ) ;

	VOID					LoadConfigInfo_JingJie(); // add by zengwen


public :
	_CONFIG_INFO			m_ConfigInfo ;
	_LOGIN_INFO				m_LoginInfo;
	_WORLD_INFO				m_WorldInfo ;
	_MACHINE_INFO			m_MachineInfo ;
	_SERVER_INFO			m_ServerInfo ;
	_SCENE_INFO				m_SceneInfo ;
	_SHAREMEM_INFO			m_ShareMemInfo;
	_BILLING_INFO			m_BillingInfo;
	_JINGJIE_COINFIG		m_JingJieConfig;		//����������� add by zengwen

};


ServerBase_Dll_Export extern Config g_Config ;

#endif
