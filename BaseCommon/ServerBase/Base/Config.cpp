//#include "stdafx.h"

#include "Config.h"
#include "Ini.h"
#include "Log.h"
#include "FileDef.h"

Config g_Config ;

Config::Config( )
{
__ENTER_FUNCTION


__LEAVE_FUNCTION
}

Config::~Config( )
{
__ENTER_FUNCTION


__LEAVE_FUNCTION
}

BOOL Config::Init( )
{
__ENTER_FUNCTION

	LoadConfigInfo( ) ;
	LoadLoginInfo();
	LoadWorldInfo( ) ;
	LoadShareMemInfo();
	LoadBillingInfo();
	LoadMachineInfo( ) ;
	LoadServerInfo( ) ;
	LoadSceneInfo( ) ;

	return TRUE ;

__LEAVE_FUNCTION

	return FALSE ;
}

VOID Config::ReLoad( )
{
	LoadConfigInfo_ReLoad( ) ;
	LoadLoginInfo_Reload();
	LoadWorldInfo_ReLoad( ) ;
	LoadShareMemInfo_ReLoad();
	LoadBillingInfo_ReLoad();
	LoadMachineInfo_ReLoad( ) ;
	LoadServerInfo_ReLoad( ) ;
	LoadSceneInfo_ReLoad( ) ;
	LoadConfigInfo_JingJie(); // add by zengwen
}

VOID Config::LoadConfigInfo( )
{
__ENTER_FUNCTION

	LoadConfigInfo_Only( ) ;
	LoadConfigInfo_ReLoad( ) ;
	LoadConfigInfo_JingJie(); // add by zengwen

__LEAVE_FUNCTION
}
VOID Config::LoadConfigInfo_Only( )
{//���ܱ��ظ���ȡ������
__ENTER_FUNCTION

	Ini ini( FILE_CONFIG_INFO ) ;

	m_ConfigInfo.m_SystemModel = ini.ReadInt( "SystemModel", "SystemModel" ) ;
	m_ConfigInfo.m_ServerID = (ID_t)(ini.ReadInt( "System", "ServerId" )) ;
	m_ConfigInfo.m_ZoneSize = ini.ReadInt( "Zone", "ZoneSize" ) ;
	m_ConfigInfo.m_MaxPortal = ini.ReadInt( "Portal", "MaxCount" ) ;
	m_ConfigInfo.m_MaxSkillObj = ini.ReadInt( "SkillObj", "MaxCount" ) ;
	m_ConfigInfo.m_MaxPlatform = ini.ReadInt( "Platform", "MaxCount" );
	m_ConfigInfo.m_MaxSpecial = ini.ReadInt( "SpecialObj", "MaxCount" );
	m_ConfigInfo.m_MaxPlayerShopNum = ini.ReadInt( "PlayerShop", "MaxCount" );
	m_ConfigInfo.m_MaxTimerCount = ini.ReadInt("SceneTimer","MaxCount");
	m_ConfigInfo.m_nMaxHumanUseTimer = ini.ReadInt("HumanTimer","MaxCount");

	Log::SaveLog( CONFIG_LOGFILE, "Load ConfigInfo.ini ...Only OK! " ) ;

__LEAVE_FUNCTION
}
VOID Config::LoadConfigInfo_JingJie()
{
__ENTER_FUNCTION

	Ini ini(FILE_JINGJIE_CONFIG) ;
	//���ؾ���������� add by zengwen	
	m_JingJieConfig.m_jingMaiCnt = ini.ReadInt("JingJieSystem", "JingMaiCnt" );
	AssertEx(m_JingJieConfig.m_jingMaiCnt == JMT_COUNT, "��������ֵ�Ƿ�,�������Լ����һ��!");
	if (m_JingJieConfig.m_jingMaiCnt > 0)
	{
		m_JingJieConfig.m_AssistItemID = ini.ReadInt("JingJieSystem", "AssistItemID");
		AssertEx(m_JingJieConfig.m_AssistItemID > 0, "����ĸ�������ID�Ƿ�!");
		m_JingJieConfig.m_assistItemValidRate = ini.ReadInt("JingJieSystem", "AssistItemValidRate");
		AssertEx(m_JingJieConfig.m_assistItemValidRate >= 0 &&
			m_JingJieConfig.m_assistItemValidRate <= 100, 
			"����ĸ������ߵ���Ч����ֵ�Ƿ�!");
		m_JingJieConfig.m_assistItemRateLv1 = ini.ReadInt("JingJieSystem", "AssistItemRateLv1");
		AssertEx(m_JingJieConfig.m_assistItemRateLv1 >= 0 &&
			m_JingJieConfig.m_assistItemRateLv1 <= 100, 
			"����ĸ������ߵ�1��Ӱ�����ֵ�Ƿ�!");
		m_JingJieConfig.m_bonusAttrLv1Point = ini.ReadInt("JingJieSystem", "BonusAttrLv1Point");
		AssertEx(m_JingJieConfig.m_bonusAttrLv1Point >= 0, "��ͨ����ʱ������1������ֵ�Ƿ�!");
		m_JingJieConfig.m_bonusPointTimer = ini.ReadInt("JingJieSystem", "BonusPointTimer");
		AssertEx(m_JingJieConfig.m_bonusPointTimer >= 1000, "���߶�ʱ������������ֵ��ʱ����̫��,��������Ӱ��������������ٶ�!");
		m_JingJieConfig.m_trainingPointMultiple = ini.ReadFloat("JingJieSystem", "TrainingPointMultiple");
		AssertEx(m_JingJieConfig.m_trainingPointMultiple >= 1.0f && m_JingJieConfig.m_trainingPointMultiple <= 8.0f, "�����㱶���Ƿ�!");
		m_JingJieConfig.m_expressTrainingpointSpeed = ini.ReadInt("JingJieSystem", "ExpressTrainingpointSpeed");
		AssertEx(m_JingJieConfig.m_expressTrainingpointSpeed >= 1, "������Ĵ����ٶ�ֵ�Ƿ�!");
		m_JingJieConfig.m_onlineBonusPoint = ini.ReadInt("JingJieSystem", "OnlineBonusPoint");
		AssertEx(m_JingJieConfig.m_onlineBonusPoint >= 0, "���߶�ʱ������������ֵ�Ƿ�!");
		for (INT iIndex = 0; iIndex < m_JingJieConfig.m_jingMaiCnt; iIndex++)
		{			
			CHAR szAddJingJieLevel[100];			
			tsnprintf(szAddJingJieLevel, 99, "AddJingJieLevel_%d", iIndex);
			m_JingJieConfig.m_addJingJieLevel[iIndex] = ini.ReadInt("JingJieSystem", szAddJingJieLevel);
			AssertEx(m_JingJieConfig.m_addJingJieLevel[iIndex] > 0 &&
				m_JingJieConfig.m_addJingJieLevel[iIndex] < MAX_JINGJIE_LEVEL,
				"��ͨ��Ӧ���������ľ������ֵ�Ƿ�!");
		}
		for (INT iIndex = 0; iIndex < MAX_JINGJIE_LEVEL; iIndex++)
		{
			CHAR szLimitTrainingPoint[100];
			tsnprintf(szLimitTrainingPoint, 99, "LimitTrainingPoint_%d", iIndex);
			m_JingJieConfig.m_limitTrainingPoint[iIndex] = ini.ReadInt("JingJieSystem", szLimitTrainingPoint);
			AssertEx(m_JingJieConfig.m_limitTrainingPoint[iIndex] >= 0 &&
				m_JingJieConfig.m_limitTrainingPoint[iIndex] <= INT_MAX, "��Ӧ�����������洢����ֵ�Ƿ�!");

			AssertEx(m_JingJieConfig.m_expressTrainingpointSpeed <= m_JingJieConfig.m_limitTrainingPoint[iIndex], "�ڵ�ǰ������ֵ�洢���޵�ǰ����������Ĵ����ٶ�ֵ�ǷǷ���!");
		}
	}

	Log::SaveLog( CONFIG_LOGFILE, "Load JingJieConfig.ini ...ReLoad OK! " ) ;

__LEAVE_FUNCTION
}
VOID Config::LoadConfigInfo_ReLoad( )
{//�����ظ���ȡ������
__ENTER_FUNCTION

	Ini ini( FILE_CONFIG_INFO ) ;
	CHAR	szText[32];

	m_ConfigInfo.m_DropParam = ini.ReadInt( "Global", "DropParam" ) ;

	ini.ReadText( "Global", "ExpParam", szText, sizeof( szText ) -1 ) ;
	m_ConfigInfo.m_ExpParam = (FLOAT)(atof( szText ));

	// װ���;���� [1/10/2011 knowC]
	m_ConfigInfo.m_EquipDamagePointOnBeHit = ini.ReadInt( "Global", "EquipDamagePointOnBeHit" ) ;
	m_ConfigInfo.m_EquipDamagePointOnHitTar = ini.ReadInt( "Global", "EquipDamagePointOnHitTar" ) ;

	// ��������� [5/10/2010 knowC]
	m_ConfigInfo.m_HierogramConfig.m_nAddOnceTimeMS		=	ini.ReadInt( "Hierogram", "AddOnceTimeMS" );
	m_ConfigInfo.m_HierogramConfig.m_nMaxTotalTimeMS	=	ini.ReadInt( "Hierogram", "MaxTotalTimeMS" );
	m_ConfigInfo.m_HierogramConfig.m_nMaxScanRadius		=	ini.ReadInt( "Hierogram", "MaxScanRadius" );
	m_ConfigInfo.m_HierogramConfig.m_nEffect_SkillID	=	ini.ReadInt( "Hierogram", "Effect_SkillID" );

	//��ȡ�������� 
	m_ConfigInfo.m_MountConfig.m_nRecoverTimeInterval	=	ini.ReadInt( "Mount", "RecoverLifeTimeMS" );
	m_ConfigInfo.m_MountConfig.m_nRecoverPercent	=	ini.ReadInt( "Mount", "RecoverLifePercent" );

	m_ConfigInfo.m_RecoverTime = ini.ReadInt( "TimeSetting", "RecoverTime" ) ;
	m_ConfigInfo.m_MaxMonster = ini.ReadInt( "Monster", "MaxCount" ) ;
	m_ConfigInfo.m_MaxPet = ini.ReadInt( "Pet", "MaxCount" ) ;
	m_ConfigInfo.m_PetBodyTime = ini.ReadInt( "Pet", "PetBodyTime" ) ;
	m_ConfigInfo.m_PetHappinessInterval = ini.ReadInt( "Pet", "PetHappinessInterval" );
	m_ConfigInfo.m_PetLifeInterval = ini.ReadInt( "Pet", "PetLifeInterval" );
	m_ConfigInfo.m_PetCallUpHappiness = ini.ReadInt( "Pet", "PetCallUpHappiness" );
	m_ConfigInfo.m_PetPlacardTime = (UINT)(ini.ReadInt( "Pet", "PetPlacardTime" ));
	m_ConfigInfo.m_PetPlacardNeedLevel = ini.ReadInt( "Pet", "PetPlacardNeedLevel" );
	m_ConfigInfo.m_PetPlacardNeedHappiness = ini.ReadInt( "Pet", "PetPlacardNeedHappiness" );
	m_ConfigInfo.m_PetPlacardNeedLife = ini.ReadInt( "Pet", "PetPlacardNeedLife" );
	m_ConfigInfo.m_DefaultRespawnTime = ini.ReadInt( "Monster", "DefaultRespawnTime" ) ;
	m_ConfigInfo.m_DropBoxRecycleTime = ini.ReadInt("TimeSetting","DropBoxRecycle");
	m_ConfigInfo.m_TimeChangeInterval	= ini.ReadInt("TimeSetting","TimeChangeInterval");
	m_ConfigInfo.m_PositionRange = ini.ReadInt( "Monster", "DefaultPositionRange" ) ;
	m_ConfigInfo.m_AIType = ini.ReadInt( "Monster", "DefaultAIType" ) ;
	m_ConfigInfo.m_DisconnectTime = ini.ReadInt( "TimeSetting", "DisconnectTime" ) ;
	ini.ReadText( "Temp", "UserPath", m_ConfigInfo.m_UserPath, _MAX_PATH ) ;
	m_ConfigInfo.m_DefaultBodyTime = ini.ReadInt( "Monster", "DefaultBodyTime" ) ;
	m_ConfigInfo.m_DefaultRefuseScanTime = ini.ReadInt( "Monster", "DefaultRefuseScanTime");
	m_ConfigInfo.m_OutGhostTime = ini.ReadInt( "Human", "OutGhostTime" ) ;
	m_ConfigInfo.m_CanGetExpRange = (FLOAT)(ini.ReadInt( "Human", "CanGetExpRange" ));
	m_ConfigInfo.m_DefaultMoveSpeed = ini.ReadInt( "Human", "DefaultMoveSpeed" ) ;
	m_ConfigInfo.m_DefaultAttackSpeed = ini.ReadInt( "Human", "DefaultAttackSpeed" ) ;
	m_ConfigInfo.m_HumanVERecoverInterval = ini.ReadInt( "Human", "HumanVERecoverInterval" ) ;
	m_ConfigInfo.m_WallowAge = ini.ReadInt( "Human", "WallowAge" ) ;
	m_ConfigInfo.m_WallowStartTime = ini.ReadInt( "Human", "WallowStartTime" ) ;
	m_ConfigInfo.m_WallowStartTime2 = ini.ReadInt( "Human", "WallowStartTime2" ) ;
	m_ConfigInfo.m_WallowAwokeTime = ini.ReadInt( "Human", "WallowAwokeTime" ) ;
	m_ConfigInfo.m_nAvailableFollowDist = ini.ReadInt( "Team", "AvailableFollowDist" );
	m_ConfigInfo.m_nTimeForLoseFollow = ini.ReadInt( "Team", "TimeForLoseFollow" );
	m_ConfigInfo.m_nOutofBeDamagedTime = ini.ReadInt( "Team", "OutofBeDamagedTime" );//add by zengwen
	if (m_ConfigInfo.m_nOutofBeDamagedTime <= 0)
	{
		AssertEx(FALSE, "��һ�����ֵĸ��˻�������Ȩ������ ���÷Ƿ�!");
	}
	m_ConfigInfo.m_nDeadMonsterScanZone = ini.ReadInt( "Team", "DeadMonsterScanZone" );//add by zengwen
	if (m_ConfigInfo.m_nDeadMonsterScanZone < 0)
	{
		AssertEx(FALSE, "�������ڹ��ܱ�N�������������˻����Ϊ�˹ֹ����� ���÷Ƿ�!");
	}
	m_ConfigInfo.m_nExpIncreaseRate = ini.ReadInt( "Team", "ExpIncreaseRate" );//add by zengwen
	if (m_ConfigInfo.m_nExpIncreaseRate < 0)
	{
		AssertEx(FALSE, "ɱ����ʱÿ��һ���ܾ������ӵİٷֱ��� ���÷Ƿ�!");
	}
	m_ConfigInfo.m_nExpIncreaseRateExtra = ini.ReadInt( "Team", "ExpIncreaseRateExtra" );//add by zengwen
	if (m_ConfigInfo.m_nExpIncreaseRateExtra < 0)
	{
		AssertEx(FALSE, "���������жϸ������һ������һ���ҵĳ����ö���ٷֱȵĻ������齱�� ���÷Ƿ�!");
	}
	m_ConfigInfo.m_nDiceBetTime = ini.ReadInt( "Team", "DiceBetTime" );//add by zengwen
	if (m_ConfigInfo.m_nDiceBetTime < 1)
	{
		AssertEx(FALSE, "���㷽ʽʰȡ��ʱ�ĶĲ�ʱ�� ���÷Ƿ�!");
	}
	m_ConfigInfo.m_nFoundDurationHour = ini.ReadInt( "Guild", "FoundDuration" );
	m_ConfigInfo.m_nDefaultMaxMemberCount = ini.ReadInt( "Guild", "DefaultMaxMemberCount" );
	m_ConfigInfo.m_nResponseUserCount = ini.ReadInt( "Guild", "ResponseUserCount" );
	m_ConfigInfo.m_nPasswdPoint = ini.ReadInt( "Relation", "PasswdPoint" );
	m_ConfigInfo.m_nPromptPoint = ini.ReadInt( "Relation", "PromptPoint" );
	m_ConfigInfo.m_nDeleteDelayTime = ini.ReadInt( "MinorPassword", "DeleteDelayTime" );
	m_ConfigInfo.m_nDeleteDelayTime *= 3600;
	m_ConfigInfo.m_nExpPoint = ini.ReadInt( "Relation", "ExpPoint" );
	
	// ����������� [12/8/2010 knowC]
	m_ConfigInfo.m_nEnemyAvailabTime = ini.ReadInt( "Relation", "EnemyAvailableTime" );
	m_ConfigInfo.m_nEnemyTimerInterval	= ini.ReadInt( "Relation", "EnemyTimerInterval" );
	m_ConfigInfo.m_nFriendTimerInterval	= ini.ReadInt( "Relation", "FriendTimerInterval" );



	//m_ConfigInfo.m_DefaultRefreshRate	=	ini.ReadInt("Obj_Human","DefaultRefreshRate")	;
	m_ConfigInfo.m_nHashOnlineUserCount = ini.ReadInt( "World", "HashOnlineUserCount" );
	m_ConfigInfo.m_nHashMailUserCount = ini.ReadInt( "World", "HashMailUserCount" );
	m_ConfigInfo.m_nMaxOfflineUserCount = ini.ReadInt( "World", "MaxOfflineUserCount" );
	ini.ReadText( "Global", "RespawnParam", szText, sizeof(szText)-1 ) ;
	m_ConfigInfo.m_fRespawnParam = (FLOAT)(atof(szText)) ;
	m_ConfigInfo.m_KickUserTime = ini.ReadInt( "TimeSetting", "KickUserTime" );
	m_ConfigInfo.m_nDefaultDamageFluctuation = ini.ReadInt("Combat","DefaultDamageFluctuation");

	m_ConfigInfo.m_nMinGoodBadValue = ini.ReadInt( "GoodBad", "MinGoodBadValue" );
	m_ConfigInfo.m_nMaxGoodBadValue = ini.ReadInt( "GoodBad", "MaxGoodBadValue" );
	m_ConfigInfo.m_nLevelNeeded = ini.ReadInt( "GoodBad", "LevelNeeded" );
	m_ConfigInfo.m_nMemberLevel = ini.ReadInt( "GoodBad", "MemberLevel" );
	m_ConfigInfo.m_fGoodBadRadius = (FLOAT)ini.ReadInt( "GoodBad", "GoodBadRadius" );
	m_ConfigInfo.m_nBonusPerMember = ini.ReadInt( "GoodBad", "BonusPerMember" );
	m_ConfigInfo.m_nMaxBonus = ini.ReadInt( "GoodBad", "MaxBonus" );
	m_ConfigInfo.m_nPenaltyWhenMemberDie = ini.ReadInt( "GoodBad", "PenaltyWhenMemberDie" );
	m_ConfigInfo.m_nWorldChatItemIndex = ini.ReadInt("WorldChat","ItemIndex");

	m_ConfigInfo.m_ThisRegion.m_nRegionIndex = ini.ReadInt("ThisRegion","RegionIndex");
	ini.ReadText( "ThisRegion", "RegionName", m_ConfigInfo.m_ThisRegion.m_RegionName, _MAX_PATH ) ;

	m_ConfigInfo.m_nOldRegionCount = ini.ReadInt("OldRegion","RegionCount");
	if( m_ConfigInfo.m_nOldRegionCount > 0 )
	{
		m_ConfigInfo.m_OldRegion = new _REGION_INFO[m_ConfigInfo.m_nOldRegionCount];
		for( INT i = 0; i < m_ConfigInfo.m_nOldRegionCount; i ++ )
		{
			CHAR szRegionIndex[100];
			CHAR szRegionName[100];
			tsnprintf( szRegionIndex, 99, "RegionIndex%03d", i+1 );
			tsnprintf( szRegionName, 99, "RegionName%03d", i+1 );
			m_ConfigInfo.m_OldRegion[i].m_nRegionIndex = ini.ReadInt( "OldRegion", szRegionIndex ) ;
			ini.ReadText( "OldRegion", szRegionName, m_ConfigInfo.m_OldRegion[i].m_RegionName, _MAX_PATH ) ;
		}
	}
	m_ConfigInfo.m_DefaultCheckMisSumbitTagRate = ini.ReadInt("TimeSetting", "DefaultCheckMisSumbitTagRate"); // �����ύ����ı�ʶ�ļ��ʱ�� add by zengwen
	AssertEx(m_ConfigInfo.m_DefaultCheckMisSumbitTagRate >= 1000, "�����ύ����ı�ʶ�ļ��ʱ�����õ�̫��,�������������������������ٶ�!");

	m_ConfigInfo.m_RedValue = ini.ReadInt("PK","redValue");;					// 		redValue=100;����
	m_ConfigInfo.m_BlackValue = ini.ReadInt("PK","blackvalue");					// 		blackvalue=300;����
	m_ConfigInfo.m_KillerAddValue  = ini.ReadInt("PK","killerAddValue");		// 		killerAddValue=100;�������ӵ�ɱ��
	m_ConfigInfo.m_HelperAddValue  = ini.ReadInt("PK","helperAddValue");		// 		helperAddValue=50;�������ӵ�ɱ��
	m_ConfigInfo.m_PkProtectLevel  = ini.ReadInt("PK","pkProtectLevel");		// 		pkProtectLevel=10;PK�����ȼ�
	m_ConfigInfo.m_HelperCheckTime = ini.ReadInt("PK","helperCheckTime");
	m_ConfigInfo.m_BreakAwayPKTime = ini.ReadInt("PK","breakAwaypkTime");
	
	Log::SaveLog( CONFIG_LOGFILE, "Load ConfigInfo.ini ...ReLoad OK! " ) ;

__LEAVE_FUNCTION
}


VOID Config::LoadLoginInfo( )
{
	__ENTER_FUNCTION

	LoadLoginInfo_Only( ) ;
	LoadLoginInfo_Reload( ) ;

	__LEAVE_FUNCTION
}
VOID Config::LoadLoginInfo_Only( )
{//���ܱ��ظ���ȡ������
	__ENTER_FUNCTION

	Ini ini( FILE_LOGIN_INFO ) ;

	m_LoginInfo.m_LoginID = (ID_t)(ini.ReadInt( "System", "LoginId" )) ;

	ini.ReadText( "System", "DBIP", m_LoginInfo.m_DBIP, IP_SIZE ) ;
	m_LoginInfo.m_DBPort	=	(UINT)(ini.ReadInt("System","DBPort"));
	ini.ReadText( "System", "DBName", m_LoginInfo.m_DBName, DATABASE_STR_LEN ) ;
	ini.ReadText( "System", "DBUser", m_LoginInfo.m_DBUser, DB_USE_STR_LEN ) ;
	ini.ReadText( "System", "DBPassword", m_LoginInfo.m_DBPassword, DB_PASSWORD_STR_LEN ) ;
	m_LoginInfo.m_AskAuthType	=	(BYTE)(ini.ReadInt("System","AskAuthType"));
//	ini.ReadText( "System", "ClientMainPath", m_LoginInfo.m_ClientMainPath, MAX_PATH ) ;

	Log::SaveLog( CONFIG_LOGFILE, "Load LoginInfo.ini ...Only OK! " ) ;

	__LEAVE_FUNCTION
}
VOID Config::LoadLoginInfo_Reload( )
{//�����ظ���ȡ������
	__ENTER_FUNCTION
		Log::SaveLog( CONFIG_LOGFILE, "Load LoginInfo.ini ...ReLoad OK! " ) ;
	__LEAVE_FUNCTION
}





VOID Config::LoadWorldInfo( )
{
__ENTER_FUNCTION

	LoadWorldInfo_Only( ) ;
	LoadWorldInfo_ReLoad( ) ;

__LEAVE_FUNCTION
}
VOID Config::LoadWorldInfo_Only( )
{
__ENTER_FUNCTION

	Ini ini( FILE_WORLD_INFO ) ;
	m_WorldInfo.m_WorldID	=		(ID_t)(ini.ReadInt( "System", "WorldId" )) ;
	m_WorldInfo.m_GuildKey	=		(SM_KEY)ini.ReadInt( "System","GuildSMKey");
	m_WorldInfo.m_MailKey	=		(SM_KEY)ini.ReadInt("System","MailSMKey");
	m_WorldInfo.m_EnableShareMem	= (BOOL)ini.ReadInt("System","EnableShareMem");
	Log::SaveLog( CONFIG_LOGFILE, "Load WorldInfo.ini ...Only OK! " ) ;

__LEAVE_FUNCTION
}
VOID Config::LoadWorldInfo_ReLoad( )
{
__ENTER_FUNCTION

	Log::SaveLog( CONFIG_LOGFILE, "Load WorldInfo.ini ...ReLoad OK! " ) ;

__LEAVE_FUNCTION
}

VOID	Config::LoadBillingInfo( ) 
{
	__ENTER_FUNCTION

		LoadBillingInfo_Only( ) ;
		LoadBillingInfo_ReLoad( ) ;

	__LEAVE_FUNCTION
}
VOID	Config::LoadBillingInfo_Only( ) 
{
	__ENTER_FUNCTION
		
		Ini ini( FILE_SERVER_INFO ) ;
		m_BillingInfo.m_BillingID = (UINT)(ini.ReadInt( "Billing", "BillingId" )) ;
		ini.ReadText( "Billing", "OuterIP", m_BillingInfo.m_OuterIP, IP_SIZE ) ;
		ini.ReadText( "Billing", "IP", m_BillingInfo.m_IP, IP_SIZE ) ;
		m_BillingInfo.m_Port = (UINT)(ini.ReadInt( "Billing", "Port" )) ;

		ini.ReadText( "Billing", "DBIP", m_BillingInfo.m_DBIP, IP_SIZE ) ;
		m_BillingInfo.m_DBPort = (UINT)(ini.ReadInt( "Billing", "DBPort" )) ;
		ini.ReadText( "Billing", "DBName", m_BillingInfo.m_DBName, DATABASE_STR_LEN ) ;
		ini.ReadText( "Billing", "DBUser", m_BillingInfo.m_DBUser, DB_USE_STR_LEN ) ;
		ini.ReadText( "Billing", "DBPassword", m_BillingInfo.m_DBPassword, DB_PASSWORD_STR_LEN ) ;
		ini.ReadText( "Billing", "WebIP", m_BillingInfo.m_WebIP, IP_SIZE ) ;
		m_BillingInfo.m_WebPort = (UINT)(ini.ReadInt( "Billing", "WebPort" )) ;
		m_BillingInfo.m_WebServerId = (UINT)(ini.ReadInt( "Billing", "WebServerId" )) ;
		ini.ReadText( "Billing", "WebLinkWebKEY", m_BillingInfo.m_WebLinkWebKEY, KEY_SIZE ) ;
		Log::SaveLog( CONFIG_LOGFILE, "Load BillingInfo.ini ...Only OK! " ) ;

	__LEAVE_FUNCTION
}
VOID	Config::LoadBillingInfo_ReLoad( ) 
{
	__ENTER_FUNCTION

		Log::SaveLog( CONFIG_LOGFILE, "Load BillingInfo.ini ...ReLoad OK! " ) ;

	__LEAVE_FUNCTION
}


VOID Config::LoadShareMemInfo()
{
	__ENTER_FUNCTION
	
		LoadShareMemInfo_Only();
		LoadShareMemInfo_ReLoad();

	__LEAVE_FUNCTION
}

VOID	Config::LoadShareMemInfo_Only()
{
	__ENTER_FUNCTION

		Ini ini( FILE_SHARE_MEM_INFO );
		m_ShareMemInfo.m_SMUObjCount=(UINT)ini.ReadInt( "ShareMem", "KeyCount" );
		//m_ShareMemInfo.m_SMUObjCount++; //���ItemSerial�̶�Key
		m_ShareMemInfo.m_pShareData	= new _SHAREMEM_DATA[m_ShareMemInfo.m_SMUObjCount];
		

		UINT i;
		for(i=0;i<m_ShareMemInfo.m_SMUObjCount;i++)
		{
			CHAR szKeyID[256] ;
			CHAR szTypeID[256];
			memset( szKeyID,	0,	256 );
			memset( szTypeID,	0,	256);
			sprintf( szKeyID, "Key%d" ,	i) ;
			sprintf(szTypeID, "Type%d",	i);
			m_ShareMemInfo.m_pShareData[i].m_Key	=	(SM_KEY)ini.ReadInt( "ShareMem", szKeyID );
			m_ShareMemInfo.m_pShareData[i].m_Type	=	(SHAREMEM_TYPE)ini.ReadInt( "ShareMem", szTypeID );
		}

		ini.ReadText( "System", "DBIP", m_ShareMemInfo.m_DBIP, IP_SIZE ) ;
		m_ShareMemInfo.m_DBPort	=	(UINT)(ini.ReadInt("System","DBPort"));
		ini.ReadText( "System", "DBName", m_ShareMemInfo.m_DBName, DATABASE_STR_LEN ) ;
		ini.ReadText( "System", "DBUser", m_ShareMemInfo.m_DBUser, DB_USE_STR_LEN ) ;
		ini.ReadText( "System", "DBPassword", m_ShareMemInfo.m_DBPassword, DB_PASSWORD_STR_LEN ) ;
		
		m_ShareMemInfo.SMUInterval	=	ini.ReadInt("System","SMUInterval");
		m_ShareMemInfo.DATAInterval	=	ini.ReadInt("System","DATAInterval");

		// ��ȡ��Ϣ�����õ��Ĺ����ڴ��KEY [���ĸ� 2011/2/20]
		INT nServerCount=(INT)ini.ReadInt( "MsgKey", "ServerCount" );
		AssertEx(nServerCount>0,"���ô���:ͨ�������ڴ洫����Ϣ���ڵ�KEY�ķ����������������0(����SVN����һ�¹����ڴ������ļ�)");
		m_ShareMemInfo.mSUMMsgKey.resize(nServerCount);
		for( int i=0; i<nServerCount; ++i )
		{
			CHAR szInID[256] ;
			CHAR szOutID[256];
			memset( szInID,	0,	256 );
			memset( szOutID,	0,	256);
			sprintf( szInID, "ServerIn%d" ,i) ;
			sprintf(szOutID, "ServerOut%d",	i);
			m_ShareMemInfo.mSUMMsgKey[i].first	=	(SM_KEY)ini.ReadInt( "MsgKey", szInID );
			m_ShareMemInfo.mSUMMsgKey[i].second	=	(SM_KEY)ini.ReadInt( "MsgKey", szOutID );
		}
		
		Log::SaveLog( CONFIG_LOGFILE, "Load ShareMemInfo.ini ...Only OK! " ) ;

	__LEAVE_FUNCTION
}

VOID	Config::LoadShareMemInfo_ReLoad()
{
	__ENTER_FUNCTION

	__LEAVE_FUNCTION
}


VOID Config::LoadMachineInfo( )
{
__ENTER_FUNCTION

	LoadMachineInfo_Only( ) ;
	LoadMachineInfo_ReLoad( ) ;

__LEAVE_FUNCTION
}
VOID Config::LoadMachineInfo_Only( )
{
__ENTER_FUNCTION

	Ini ini( FILE_MACHINE_INFO ) ;
	m_MachineInfo.m_MachineCount = ini.ReadInt( "System", "MachineNumber" ) ;

	m_MachineInfo.m_pMachine = new _MACHINE_DATA[m_MachineInfo.m_MachineCount] ;
	memset( m_MachineInfo.m_pMachine, 0, sizeof(_MACHINE_DATA)*m_MachineInfo.m_MachineCount ) ;

	for( UINT i=0; i<m_MachineInfo.m_MachineCount; i++ )
	{
		CHAR szSection[256] ;
		memset( szSection, 0, 256 ) ;

		//��ʼ������
		m_MachineInfo.m_pMachine[i].Init( ) ;

		//��ȡ������i��
		sprintf( szSection, "Machine%d", i ) ;
		m_MachineInfo.m_pMachine[i].m_MachineID = (ID_t)(ini.ReadInt( szSection, "MachineId" )) ;
	}

	Log::SaveLog( CONFIG_LOGFILE, "Load MachineInfo.ini ...Only OK! " ) ;

__LEAVE_FUNCTION
}
VOID Config::LoadMachineInfo_ReLoad( )
{
__ENTER_FUNCTION

	Log::SaveLog( CONFIG_LOGFILE, "Load MachineInfo.ini ...ReLoad OK! " ) ;

__LEAVE_FUNCTION
}

VOID Config::LoadServerInfo( )
{
__ENTER_FUNCTION

	LoadServerInfo_Only( ) ;
	LoadServerInfo_ReLoad( ) ;

__LEAVE_FUNCTION
}
VOID Config::LoadServerInfo_Only( )
{
__ENTER_FUNCTION

	Ini ini( FILE_SERVER_INFO ) ;
	m_ServerInfo.m_ServerCount = ini.ReadInt( "System", "ServerNumber" ) ;

	m_ServerInfo.m_pServer = new _SERVER_DATA[m_ServerInfo.m_ServerCount] ;
	memset( m_ServerInfo.m_pServer, 0, sizeof(_SERVER_DATA)*m_ServerInfo.m_ServerCount ) ;

	for( UINT i=0; i<m_ServerInfo.m_ServerCount; i++ )
	{
		CHAR szSection[256] ;
		memset( szSection, 0, 256 ) ;

		//��ʼ������
		m_ServerInfo.m_pServer[i].Init( ) ;

		//��ȡ������i��
		sprintf( szSection, "Server%d", i ) ;
		m_ServerInfo.m_pServer[i].m_ServerID = (ID_t)(ini.ReadInt( szSection, "ServerId" )) ;
		m_ServerInfo.m_pServer[i].m_MachineID = (ID_t)(ini.ReadInt( szSection, "MachineId" )) ;
		ini.ReadText( szSection, "IP0", m_ServerInfo.m_pServer[i].m_IP0, IP_SIZE ) ;
		m_ServerInfo.m_pServer[i].m_Port0 = (UINT)(ini.ReadInt( szSection, "Port0" )) ;
		ini.ReadText( szSection, "IP1", m_ServerInfo.m_pServer[i].m_IP1, IP_SIZE ) ;
		m_ServerInfo.m_pServer[i].m_Port1 = (UINT)(ini.ReadInt( szSection, "Port1" )) ;
		//enum SERVER_TYPE
		m_ServerInfo.m_pServer[i].m_Type = (UINT)(ini.ReadInt( szSection, "Type" )) ;
		m_ServerInfo.m_pServer[i].m_HumanSMKey	=	(SM_KEY)(ini.ReadInt(szSection,"HumanSMKey"));
		m_ServerInfo.m_pServer[i].m_PlayShopSMKey = (SM_KEY)(ini.ReadInt(szSection,"PlayShopSMKey"));
		m_ServerInfo.m_pServer[i].m_ItemSerialKey = (SM_KEY)(ini.ReadInt(szSection,"ItemSerialKey"));
		m_ServerInfo.m_pServer[i].m_EnableShareMem = (BOOL)(ini.ReadInt(szSection,"EnableShareMem"));


	}

	ini.ReadText( "World", "IP", m_ServerInfo.m_World.m_IP, IP_SIZE ) ;
	m_ServerInfo.m_World.m_Port = (UINT)(ini.ReadInt( "World", "Port" )) ;


	//
	for(UINT i=0; i<m_ServerInfo.m_ServerCount; i++ )
	{
		ID_t ServerID = m_ServerInfo.m_pServer[i].m_ServerID ;

		Assert( ServerID != INVALID_ID && ServerID < OVER_MAX_SERVER ) ;

		Assert( m_ServerInfo.m_HashServer[ServerID] == -1 ) ;

		m_ServerInfo.m_HashServer[ServerID] = i ;
	}

	Log::SaveLog( CONFIG_LOGFILE, "Load ServerInfo.ini ...Only OK! " ) ;

__LEAVE_FUNCTION
}
VOID Config::LoadServerInfo_ReLoad( )
{
__ENTER_FUNCTION

	Log::SaveLog( CONFIG_LOGFILE, "Load ServerInfo.ini ...ReLoad OK! " ) ;

__LEAVE_FUNCTION
}

VOID Config::LoadSceneInfo( )
{
__ENTER_FUNCTION

	LoadSceneInfo_Only( ) ;
	LoadSceneInfo_ReLoad( ) ;

__LEAVE_FUNCTION
}
VOID Config::LoadSceneInfo_Only( )
{
__ENTER_FUNCTION

	Ini ini( FILE_SCENE_INFO ) ;

	//��ȡ��������
	m_SceneInfo.m_SceneCount = (UINT)(ini.ReadInt( "system", "scenenumber" )) ;

	m_SceneInfo.m_pScene = new _SCENE_DATA[m_SceneInfo.m_SceneCount] ;
	memset( m_SceneInfo.m_pScene, 0, sizeof(_SCENE_DATA)*m_SceneInfo.m_SceneCount ) ;


	for( SceneID_t i=0; i<m_SceneInfo.m_SceneCount; i++ )
	{
		CHAR szSection[256] ;
		memset( szSection, 0, 256 ) ;

		//��ȡ������i��
		sprintf( szSection, "scene%d", i ) ;
		m_SceneInfo.m_pScene[i].m_SceneID = i ;
		m_SceneInfo.m_pScene[i].m_IsActive = ini.ReadInt( szSection, "active" ) ;
		m_SceneInfo.m_pScene[i].m_ResID = ini.ReadInt( szSection, "clientres" ) ;//�ͻ���ʹ����ԴID add by zcx 2010.2.8
		ini.ReadText( szSection, "name", m_SceneInfo.m_pScene[i].m_szName, _MAX_PATH ) ;
		ini.ReadText( szSection, "file", m_SceneInfo.m_pScene[i].m_szFileName, _MAX_PATH ) ;
		m_SceneInfo.m_pScene[i].m_ServerID = (ID_t)(ini.ReadInt( szSection, "serverId" )) ;
		m_SceneInfo.m_pScene[i].m_Type = (ID_t)(ini.ReadInt( szSection, "type" )) ;
		m_SceneInfo.m_pScene[i].m_ThreadIndex = (ID_t)(ini.ReadInt( szSection, "threadindex" )) ;
		
	}

	//
	for(SceneID_t i=0; i<m_SceneInfo.m_SceneCount; i++ )
	{
		SceneID_t SceneID = m_SceneInfo.m_pScene[i].m_SceneID ;

		Assert( SceneID!=INVALID_ID && SceneID<MAX_SCENE ) ;

		Assert( m_SceneInfo.m_HashScene[SceneID]==-1 ) ;

		m_SceneInfo.m_HashScene[SceneID] = i ;
	}

	Log::SaveLog( CONFIG_LOGFILE, "Load SceneInfo.ini ...Only OK! " ) ;

__LEAVE_FUNCTION
}
VOID Config::LoadSceneInfo_ReLoad( )
{
__ENTER_FUNCTION

	Log::SaveLog( CONFIG_LOGFILE, "Load SceneInfo.ini ...ReLoad OK! " ) ;

__LEAVE_FUNCTION
}

ID_t Config::SceneID2ServerID(SceneID_t sID) const
{
	__ENTER_FUNCTION

	Assert(sID>=0);
	Assert(sID<(INT)m_SceneInfo.m_SceneCount);
	return	m_SceneInfo.m_pScene[m_SceneInfo.m_HashScene[sID]].m_ServerID;

	__LEAVE_FUNCTION

	return -1;
}

ID_t Config::SceneResID2ServerID(INT sID) const
{//ͨ����ԴID��ȡ������ID add by zcx 2010.2.8
	__ENTER_FUNCTION

		Assert(sID>=0);
	//Assert(sID<(INT)m_SceneInfo.m_SceneCount);

	for (int i =0; i< m_SceneInfo.m_SceneCount; i++)
	{
		if(m_SceneInfo.m_pScene[i].m_ResID == sID)
			return m_SceneInfo.m_pScene[i].m_ServerID;
	}	

	__LEAVE_FUNCTION

		return -1;
}

SceneID_t	Config::SceneResID2SceneID(INT sID) const
{//ͨ����ԴID��ȡ����ID add by zcx 2010.2.8
	__ENTER_FUNCTION

	Assert(sID>=0);
	//Assert(sID<(INT)m_SceneInfo.m_SceneCount);

	for (int i =0; i< m_SceneInfo.m_SceneCount; i++)
	{
		if(m_SceneInfo.m_pScene[i].m_ResID == sID)
			return m_SceneInfo.m_pScene[i].m_SceneID;
	}	

	__LEAVE_FUNCTION

		return -1;
}

INT	Config::SceneID2SceneResID(SceneID_t sID) const
{//ͨ��������ȡ��ԴID add by zcx 2010.2.8
	__ENTER_FUNCTION

		Assert(sID>=0);
	Assert(sID<(INT)m_SceneInfo.m_SceneCount);

	for (int i =0; i< m_SceneInfo.m_SceneCount; i++)
	{
		if(m_SceneInfo.m_pScene[i].m_SceneID == sID)
			return m_SceneInfo.m_pScene[i].m_ResID;
	}	

	__LEAVE_FUNCTION

	return -1;
}

ID_t Config::Key2ServerID(SM_KEY key) const
{
	__ENTER_FUNCTION
	
		Assert(key>0);
		
			for( UINT i=0; i<m_ServerInfo.m_ServerCount; i++ )
			{

				if(m_ServerInfo.m_pServer[i].m_EnableShareMem)
				{
					if(m_ServerInfo.m_pServer[i].m_ItemSerialKey == key)
					{
						return	m_ServerInfo.m_pServer[i].m_ServerID;
					}
				}
				
			}

	return -1;
		
	__LEAVE_FUNCTION

	return -1;	
}

