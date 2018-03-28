#ifndef _INCLUDE_GAMEDEFINE_H_
#define _INCLUDE_GAMEDEFINE_H_

#define  SERVER_VERSION_FLAG        "2018.01.21"
#define  RELEASE_RUN_VER                0
#define  SERVER_EXE_NAME                "DdzServer.exe"

//本地配置表格列表
#define RUNCONFIG						"RunConfig"							//配置文件名称
#define TABLE_CONFIG_FILE				"RunConfig/GServerConfigList.csv"
#define SERVER_GAME_CONFIG_FILE			"Config/ServerConfigList.csv"

#define 	LGS_GATE_PACKET_ID  (PACKET_MAX+11)

#define DDZ_ROOM_PLAYER_COUNT       3
#define PAI_TYPE_COUNT                            4
#define PAI_BEGIN_CODE                            3
#define POKER_A                                         14
#define PAI_END_CODE                               15
#define PLAYER_PAI_NUM                          17

#define Pai2Num(t, code)  (code * 10 + t)
#define PaiCode(v) ((int)(v/10))
#define PaiType(v) (v%10)

enum WEB_GAME_MSG
{
    WEB_MSG_NULL = 0,

    WEB_MSG_BEGIN = 10,
    WEB_MSG_REQUEST_CREATE_ROOM = 11,
    WEB_CL_RequestLoginGame = 12,
    WEB_CL_RequestStartGame = 13,
    LG_NotifyAllGateServerInfo = 14,
    GL_NotifyPlayerLeaveGS = 15,
	CL_REGISTER_ACCOUNT = 16,
	//CL_REQUEST_UPLOAD_PHOTO = 17,
	CL_REQUEST_LOAD_PHOTO = 18,
	//CL_REQUEST_LOAD_SELF_PHOTO = 19,
	//CL_REQUEST_LOAD_HEAD_PHOTO = 20,
	CL_REQUEST_DB_DATA = 21,
	CL_REQUEST_UPDATE_DB_DATA = 22,

	CL_REQEUST_SAVE_NEWSFEED = 23,
	CL_LOAD_NEWSFEED = 24,
	CL_LOAD_BIG_PHOTO = 25,

	WEB_MSG_GATE_BEGIN = 1000,     
    
     GC_StartPaiData = 1003,
     CG_RequestChuPai = 1004,
     
     WEB_MSG_GATE_END = 8000,
};

enum GAME_OBJECT_TYPE
{
    eGameDdzRoom,
    eGameDdzPlayer,
};

enum GAME_ERROR
{
    eRROR_NONE = 0,
    eRROR_PAI_NULL = 1,
    eRROR_NO_RIGHT_CHUPAI = 2,         // 未轮到玩家出牌
    eRROR_PAI_RULE_ERROR = 3,            //  出牌不正确, 不符合规则
    eRROR_PAI_DATA_ERROR = 4,
    eRROR_PAI_NO_EXIST = 5,
    eRROR_PLAYER_NO_EXIST = 6,
    eRROR_PAI_TYPE_ERROR = 7,
    eRROR_PAI_COUNT_ERROR = 8,
    eRROR_PAI_VALUE_LOW = 9,
};

enum LOGIN_RESULT
{
	eOk = 0,
	eUnknowLoginResult = 1,
	eCreateAccountSucceed,
	eLoginSucceed,
	eAccountIsNull,
	ePasswordIsNull,
	eAccountExist,
	eAccountNoExist,
	ePasswordError,
	eDBQueryError,
	eNoLoginState,
	eNullData,

	eCheckStateSuceed,
	eCheckLoginTimeFail,
	eLoginStateOverTime,
};

enum WEB_BASE_MSG
{
	WEB_SEND_BIG_MSG = 5,   // NOTE: 对方的接收ID必须设置为此ID
	WEB_REV_BIG_MSG = 4,
};

#endif //_INCLUDE_GAMEDEFINE_H_