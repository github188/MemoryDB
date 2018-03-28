#ifndef _INCLUDE_COMMONDEFINE_H_
#define _INCLUDE_COMMONDEFINE_H_

#include "ObjectID.h"
#include "TicketCode.h"
//#include "DBOperate.h"
#include "BaseTable.h"
#include "TableManager.h"

//-------------------------------------------------------------------------*/
#define RELEASE_RUN_VER								0

#define SERVER_VERSION_FLAG						"2017.06.23.18"		// 此版本仅用于识别编译版本, 不与客户端登陆比对, 日期+两位数递增
//-------------------------------------------------------------------------
#define	USE_RESOURCES_SERVER					1			// 是否使用资源服务器
#define NET_HEARTBEAT_ONCE_TIME				10			// 服务器网络发送心跳包的间隔时间
#define CHECK_MEMORY_ONCE_TIME				(10)		// 隔10秒检查一次内存, 同时向LG同步一次连接数量, 为了稳定，通过检查结果可以限制过载登陆
#define RS_WAIT_CLIENT_OVERTIME				(10*60)		// 资源服务器等待客户端回复断开超时时间 10分钟
//-----------------------------------------------------------------------------------------------------
const int gsCode = MAKE_INDEX_ID("HomeGame");
#define NET_SAFT_CHECK_CODE						(gsCode)
#define	GS_NODE_NET_SAFECODE					100			// 
#define ACCOUNT_NET_SAFECODE					101			// 帐号服务器网络安全码

#define DEBUG_AI_INFO									1
#define GS_AVAIL_MEMORY_MIN						(256)		// GS最小正常工作内存限制 (M), 小于此值，会阻止玩家登陆
#define GS_PLAYER_CONNECT_WAIT_TIME		(60*60)	// GS玩家连接最大无反应时间，1小时内无接收消息，则断开连接
#define GS_UPDATE_ROLE_DB_ONCE_TIME		1				// 1秒更新一次玩家的DB数据记录

#define SCENE_THEAD_MAX_PLAYER_COUNT	(500)		//一个线程内最大允许的玩家数量, 同上面的定义, 一个玩家一个场景
#define ARENA_SCENE_CONFIG_INDEX			(20000)		// 斗兽场广场场景配置
#define AREA_ACTIVE_MAX_COUNT					(20)				// 区域允许同时存在的活动的对象(排除死亡)最大数量, 超时此数量时,LUA刷出怪物血条为0
#define PAY_ORDER_MAX_COUNT					(10000000)	// 定单表格最大允许数量
#define  PLAYER_MOVE_STEP_LENGTH				(175)			// 玩家每次移动一次的步长 NOTE: 此值需要与客户端中的 sStepLength 相同
//-------------------------------------------------------------------------*
// 多语言翻译常量字符串
//#define LANGUAGE(szString)		szString
#define LANGUAGE(szString)		_LanguageTool::_ToLanguage(szString).c_str()

// 字典翻译
struct _LanguageTool
{
	static AString _ToLanguage(const char *sz)
	{
		return AString(sz);
	}
};
//-------------------------------------------------------------------------*/
#if RELEASE_RUN_VER
#		define DEBUG_ARENA_INFO		1
#		define DEBUG_LOG(logInfo, ...) { }
#else
#		define DEBUG_ARENA_INFO		1
#		define DEBUG_LOG(logInfo, ...) { TableTool::yellow(); TableTool::Log(1, logInfo, ##__VA_ARGS__); TableTool::white(); }
#endif
//-------------------------------------------------------------------------*/
enum SERVER_NODE
{
	eUnknow = 0,
	eGameServer,
	eResourceServer,
	eLoginServer,
	eServerTypeMax,
};
#define  IS_GS ((int)node->get("SERVER_TYPE")==eGameServer)
#define  IS_RS ((int)node->get("SERVER_TYPE")==eResourceServer)
#define  IS_LG ((int)node->get("SERVER_TYPE")==eLoginServer)

#define SERVER_TYPE_KEY		"SERVER_TYPE"
#define LUA_LOG_DEFINE		ThreadLog
//-------------------------------------------------------------------------
enum TARGET_NET_FLAG
{
	eNetFlagNone = 0,
	eLOCAL,
	eDB_SERVER,			// 发送到 t_player 表格记录所在的DBNODE
	eGS_SERVER,			// GameServer服务器
	eCLIENT,			// 指定客户端
	eDB_NODE,			// DB节点
	eDB_DATANODE,		// 发送到 t_playerdata 表格记录所在的DBNODE
};
//-------------------------------------------------------------------------

enum PLAYER_DB_BASE
{
    PLAYER_DB = 0,
	MAIL_DB = 1,	
	FRIEND_DB = 2,
	BOSS_DB = 3,
	DB_MAX,
};

enum WORLD_DB_BASE
{
	WORLD_ACCOUNT_DB = 0,
	WORLD_NAME_DB = 1,
	WORLD_SORT_DB = 2,
};

#define PLAYER_NAME_TABLE	"t_player_NAME"

//-----------------------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------------------
// 中转事件地址
enum EEventAddressType
{
	eAddress_GameServer,
	eAddress_WorldServer,
	eAddress_Player,
	eAddress_Client,
};
//-------------------------------------------------------------------------

//本地配置表格列表
#define RUNCONFIG						"RunConfig"							//配置文件名称
#define TABLE_CONFIG_FILE				"RunConfig/GServerConfigList.csv"
#define SERVER_GAME_CONFIG_FILE			"Config/ServerConfigList.csv"

//World服务器配置表格列表
#define TABLE_CONFIG_FILE_WORLD			"Config/TableConfigWorld.csv"


// 玩家Account表
#define DB_PLAYER_ACCOUNT_TABLE			"PlayerAccount"

//-------------------------------------------------------------------------
// 数据库中的数据表格
//
#define DB_ACCOUNT_TABLE					"t_account"
#define DB_USER_TABLE							"t_userdata"
#define DB_NEWS_FEED_TALBE				"db_t_userdata_NEWS_FEED"

#define GLOBAL_AUCCONT_TABLE			"t_globalaccount"
#define ANNOUNCEMENT_TABLE				"t_announcement"
#define PAY_SUCCEED_ORDER_TABLE		"t_payorder"

#define GAME_NAME_TABLE                 "t_name"
// DB 玩家属性表
#define GAME_PLAYER_PRIVATE_TABLE		"t_player_private"
#define GAME_PLAYER_PUBLIC_TABLE		"t_player_public"

#define GAME_MAIL_TABLE					"t_mail"

#define GAME_RANKING_TABLE				"t_ranking"

#define GAME_SDKACCOUNT_TABLE			"t_sdkaccount"

#define GAME_GUILD_TABLE				"t_guild"
#define GAME_BATTLE_TABLE				"t_battle"
#define GAME_COUPLE_TABLE				"t_couple"
#define GAME_PLAYER_DATA_TABLE			"t_playerdata"


#define GAME_TEAM_TABLE					"t_team"

#define BOSS_DB_TABLE					"t_worldboss"

#define DAMAGE_DB_TABLE					"t_damage"

#define GAME_GUILD_TREASURE             "t_guild_treasure"
#define GUILD_BATTLE_TABLE              "t_guild_battle"
//#define GUILD_BATTLE_WIHTEBOARD_TABLE   "t_guild_battle_whiteboard"		// 修改到内存缓存
#define INTER_SERVICE_BATTLE_TABLE					"t_interservice_battle"
#define WORLD_HOME_SCENE_TABLE						"t_world_homescene"

#define CD_KEY_TABLE					"t_cdkey"

//-----------------------------------------------------------------------------------------------------
// 配置表格资源包索引名称
#define CONFIG_TABLE_PACK_INDEX			"t_configtable.pak"

// 安卓更新资源列表DB table 名
#define RESOURCES_LIST_DB_ANDROID		"and_resourceslist"

// IOS更新资源列表DB table 名
#define RESOURCES_LIST_DB_IOS			"ios_resourceslist"

// 资源文件列表 DB table 名
#define RESOURCES_LIST_DB_TABLE			"r_resourceslist"

// 公告资源DB表名
#define ANNO_DB_RESOURCE				"anno_table.csv"

// 充值定单
#define  PAY_ORDERFORM_DB_TABLE		"t_orderform"

// 玩家金矿副本数据表
#define  PLAYER_HOME_DB_TABLE			"t_playerhome"

//-----------------------------------------------------------------------------------------------------
#define PLAYER_LOG(logInfo, ...) { TableTool::LogPlayer(logInfo, ##__VA_ARGS__); }



//  [5/6/2015 Administrator]

enum RolePart
{
	All = -1,
	One = 0,
	Hair,
	Coat,
	Pant,
	Shoes,
	Face,
	HandPart,//
	PartEnd,
};


enum ItemType
{
	eOrdinaryItem = 2,
	ePacketItem = 3,
	eGemItem = 4,
	eFuWenItem = 5,
	eEquioFragmentItem = 10,
    ePetFragmentItem = 11
};

enum PlayerSex
{
	eSexNone = 0,
	eSexBoy = 1,
	eSexGirl = 2,
};

enum ResourceType
{
	eIsGold = 0,
	eIsGem = 1,
};


enum ChatChannel
{
	eChat_World,
	eChat_Hall,
	eChat_Room,
	eChat_Private,
	eChat_Guild,
};

//装备穿戴部位
// 对应客户端 EQUIP_LOCATION
enum SuitParts
{
	eWeapon = 1,
	eHead,
    eHand,
	eCoat,
	eLeg,
	eFoot,

    eJewelryNecklace,
    eJewelryRing,
    eJewelryBracelet,
    eJewelryEarring,

    eFashionWeapon = 13,
    eFashionHead,
    eFashionFace,
    eFashionCoat,
    eFashionLeg,
    eFashionEffect,
};
//-------------------------------------------------------------------------

// 饰品类型
enum Accessories
{
	eHeadwear = 0,
	eNecklace,
	eBracelets,
	eRing,
	eEarring,
	eTattoo,
};

//-------------------------------------------------------------------------

enum eCAMP
{
    eNpcCamp = 0,
    ePetCamp = 0,
	ePropCamp = 0,
    eChestCamp = 2,
    ePlayerCamp = 1000,
    eMonsterCamp = 2000,
	ePVPPlayerCamp = 300,
};

//-------------------------------------------------------------------------
enum eGoDirection
{
	eGoBegin,
	eNone,
	eGoLeft,
	eGoRight,
	eLeftGoUp,
	eLeftGoDown,
	eRightGoUp,
	eRightGoDown,
};
//-------------------------------------------------------------------------*/

enum SKILL_TYPE
{
    ePROFESSION_SKILL   = 0,        //专家技能
    eORDINARY_SKILL     = 1,        //普通技能
    eULTRA_SKILL        = 2,        //觉醒技能
    ePASSIVE_SKILL      = 10,       //被动技能
    eGENIUS_SKILL       = 20,       //天赋
};


enum TEAM_NOTIFY_TYPE
{
    TEAM_NOTIFY_NONE        = 0,
    TEAM_CREATE             = 1,    //组队创建
    TEAM_MEMBER_ONLINE,             //组员上线 
    TEAM_MEMBER_OFFLINE,            //组员下线 
    TEAM_MEMBER_LEAVE,              //组员退队
    TEAM_MEMBER_JOIN,               //新组员加入
    TEAM_MEMBER_KICKED,             //组员被踢
    TEAM_LEADER_APPOINT,            //队长委派
    TEAM_ACTIVITY_START,            //组队活动开启
    TEAM_ACTIVITY_CHANGE,           //组队活动变更
    TEAM_ACTIVITY_STOP,             //组队活动关闭 
};


enum ACTIVITY_STATE
{
    PENDING         = -1,
    PREPARING       = 0,
    RUNNING         = 1,
    WAIT_TO_ABORT,
    INTERUPTED,
    COMPLETED,
    FINAL_SETTLED,
    EXITED,
};


//-------------------------------------------------------------------------*/
struct TableEnumTool
{
public:
	bool mbOk;

public:
	TableEnumTool(const char *tableIndex, tBaseTable *t, const char *fieldName, DataTableManager &configMgr )
		: mbOk(false)
	{

		if (t==NULL)
		{
			t = configMgr.GetTable(tableIndex).getPtr();
			if (t==NULL)
			{
				WARN_LOG("获取[%s]表格失败, 检查未完成", tableIndex);
				return;
			}
		}
		AutoField f = t->GetField();

		AString infoString;
		for (int i=0; i<f->getCount(); ++i)
		{
			if (i>0)
				infoString += ",";
			infoString += f->getFieldInfo(i)->getName();
		}

		Array<AString> nameList;
		AString::Split(fieldName, nameList, ",", 100);
		for (int i=0; i<nameList.size(); ++i)
		{
			FieldInfo info = f->getFieldInfo(i);
			if (info==NULL)
			{
				ERROR_LOG("检查枚举表格[%s]失败, 不存在[%d]>[%s]字段\r\nTABLE [%s]\r\nENUM [%s]", 
					tableIndex, i, nameList[i].c_str(), infoString.c_str(), fieldName);
				return;
			}
			if (nameList[i]!=info->getName())
			{
				ERROR_LOG("检查枚举表格[%s]失败, 列[%d]>[%s]字段与枚举名称[%s]不相符\r\nTABLE [%s]\r\nENUM [%s]", 
					tableIndex, i, info->getName().c_str(), nameList[i].c_str(), infoString.c_str(), fieldName);
				return;
			}
		}
		GREEN_LOG("Succeed check table enum >[%s]", tableIndex);
		mbOk = true;
	}
};


#define DefineTableEnum(tableIndex, table, fieldName)	\
	TableEnumTool	sTableEnumTool(#tableIndex, table, fieldName, configMgr);

#define CheckEnum(tableIndex, table, fieldName)	\
	TableEnumTool	sTableEnumTool(#tableIndex, table, fieldName, configMgr);

#define IS_CHECK_OK	sTableEnumTool.mbOk
//-------------------------------------------------------------------------*/
// 控制台GS信息数据
enum GS_STATE_INFO
{
	eGSKEY,
	ePLAYER_COUNT,
	eCONN_COUNT,
	eSCENE_COUNT, 
	eOBJECT_COUNT,
	eFRAME,
	eUSE_MEM,
	eFREE_MEM,
	eRECEIVE_COUNT,
	eSEND_COUNT,
	eRECEIVE_SIZE,
	eSEND_SIZE,
	eRESTART_COUNT,
	eDB_CONNECT_STATE,
	eRS_COUNT,
	eCONFIG_CHECK,
	eEXE_INFO,
};
// 控制台DB信息数据
enum DB_STATE_INFO
{
	eDB_INFO,
	eDB_USE_MEM,
	eDB_FREE_MEM,
	eDB_TASK_COUNT,
	eDB_CONNECT_COUNT,
	eDB_RECEIVE_COUNT,
	eDB_SEND_COUNT,
	eDB_RECEIVE_SIZE,
	eDB_SEND_SIZE,
	eDB_FRAME_COUNT,
	eDB_CONFIG_INFO,
	eDB_PAY_INFO,
};
//-------------------------------------------------------------------------*/
enum MOVE_RESULT
{
	eMoveFail,
	eMoveArrive,
	eMoveSucceed,
};
//-------------------------------------------------------------------------*/
enum SDK_TYPE
{
	SDK_NONE,
	UC,
	SDK360,
	ANYSDK,
	QUICK,
};
//-------------------------------------------------------------------------*/
enum ARENA_REQUEST_RESUTL
{
	ARENA_ERROR,		// 错误
	CREATE_ARENA,		// 新建
	ENTER_ARENA,		// 在同一GS上，直接进入挑战
	CHANGE_GS_ENTER_ARENA,	// 需要跳转到擂主GS，然后挑战
	WAIT_ARENA,				// 进入等待队列中
	ERROR_HAS_TEAM,			// 当前在队伍里
	ARENA_NO_START,
	ARENA_ALREADY_SUCCEED,	// 已经成功占领过
};
enum ARENA_BATTLE_RESULT
{
	eARENA_RESULT_UNKWON = 0,
	eARENA_RESULT_FAIL = 1,		// 被怪物或挑战者打死
	eARENA_RESULT_SUCCEED = 2,	// 守擂成功
	eARENA_ATTACK_FAIL = 3,		// 挑战失败
	eARENA_ATTACK_SUCCEED = 4,	// 挑战成功
};

enum GUILD_TREASURE_EVENT_TYPE
{
    eRandTrriggeredEvent = 1,
    eRobEvent = 2,
    eRobedEvent = 3,
    ePickupEvent = 4,
    eRobFailedEvent = 5,
    eRobedFailedEvent = 6,
};
//-------------------------------------------------------------------------*/
#define SERVER_RUN_UPDATE_PACK "s_serverrun.pak"					// Server运行文件, 用于部署工具更新或新建游戏区
#define SERVER_LUA_DATA_PACK	"s_serverlua.pak"

#define SERVER_EXE_UPDATE_PACK "s_serverexe.pak"
#define SERVER_EXE_NAME				"GameServer.exe"
#define SERVER_PDB_NAME				"GameServer.pdb"
//-------------------------------------------------------------------------*/
#define DB_EXE_UDPATE_PACK	"s_dbexe.pak"
#define DB_EXE_NAME				"LogicDBServer.exe"
#define DB_PDB_NAME				"LogicDBServer.pdb"
//-------------------------------------------------------------------------*/
enum
{
	SERVER_UPDATE_OK,
	SERVER_UPDATE_PACK_NOEXIST,
	SERVER_UPDATE_PACK_NOEXIST_EXE,
	SERVER_UPDATE_PACK_NOEXIST_PDB,
	SERVER_UPDATE_REPLACE_PDB_FAIL,
	SERVER_UPDATE_REPLACE_EXE_FAIL,
	SERVER_THREADID_NOT_ZREO,
	SERVER_ALREADY_NEWEST,
};
//-------------------------------------------------------------------------*/
// 服务区显示状态信息
enum SERVER_SHOW_STATE
{
	SHOW_STATE_NONE,
	SHOW_STATE_NEW = 1,
	SHOW_STATE_TUIJIAN = 2,
	SHOW_STATE_FULL = 3,
	SHOW_STATE_HOT = 4,
};

#define TRANS_TEXT(x) x

class SERVER_STATE_TOOL
{
public:
	static const char* ToStringServerState(int state)
	{
		switch (state)
		{
		case SHOW_STATE_NEW:
			return TRANS_TEXT("新区");

		case SHOW_STATE_TUIJIAN:
			return TRANS_TEXT("推荐");

		case SHOW_STATE_FULL:
			return TRANS_TEXT("已满");

		case SHOW_STATE_HOT:
			return TRANS_TEXT("火爆");

		default:
			return TRANS_TEXT("正常");
		}
	}
};
//-------------------------------------------------------------------------*/
enum PAY_RESULT
{
	ePayFail = 0,
	ePaySucceed = 1,
	ePayAlreadyDone,
	ePayNoSetData,
	ePayNoSetServerOlder,
	ePayNoExistServerOlder,
	ePaySDKOlderError,
	ePayPlayerDBNoexist,
	ePayZhongZhiFail,
};
//-------------------------------------------------------------------------*/
enum FORE_REQUEST_LG_TYPE
{
	LG_INFO,
	TOTAL_PAY_AMOUNT,
	RES_LOGIN_KEY,
	REQUEST_LOGIN_AND_GS_IP,
};
enum LOGIN_IP_TYPE
{
	LOGIN_IP_GS,
	LOGIN_IP_LG,
	LOGIN_IP_RS,
};

#define 	GWG_EVENT_PACKET_ID  (PACKET_MAX+10)

//-------------------------------------------------------------------------*/
#endif //_INCLUDE_COMMONDEFINE_H_