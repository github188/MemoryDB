#ifndef __GAMESTRUCT_H__
#define __GAMESTRUCT_H__

#include "BaseCommon.h"
#include <cmath>
#include <math.h>
#include "EasyMap.h"
#include "Array.h"

using namespace std;
//-------------------------------------------------------------------------
// 设定网络协议,设置1 使用TCP协议; 设置0 使用UDP协议
#define Net_Connect_Server		(10)		// 异步连接服务器, 任务ID 定义为零时同步阻塞连接, NOTE:不能和已有的任务ID相同
#define	USE_THREAD_FINDPATH		1			// 后台线程寻路
#define NET_HEART_CHECK			1			// 网络心跳包检查
#define USE_DYNAMIC_IMAGE_LAOD  0           //是否动态加载图片
// 网络检查等待时间, 超过此时间后, 则进行下线处理, (10分钟)
#define PLAYER_NET_CHECK_OVERTIME	(600)
// 客户端每隔30秒发送一次网络检查心跳消息
#define NET_CHECK_ONCE_TIME			(10)
#define	WAIT_RESUME_OVER_TIME		(480)		// 客户端最小化允许8分钟, 服务器等待10分钟, 网络检查每30秒一次
//-------------------------------------------------------------------------

#define GET_SECOND_TIME(day, hour, minu, second)( (day * 24 + hour) * 3600 + minu * 60 +second)

#define STR_ID_TO_NUM(x)                        ((x)+'0'>'9' ? (x)-10+'a' : (x)+'0')
#define STR_NUM_TO_ID(x)                        ((x)>'9' ? (x)-'a'+10 : (x)-'0')
#define BOUNDOF(x)								((int)((x)+0.5))
#define IS_NUMBER(x)							((x) >= '0' && (x) <= '9')
#define IS_LETTER(x)							(((x) >= 'a' && (x) <= 'z') || ((x) >= 'A' && (x) <= 'Z'))

#define ArrayLen(ar)       (sizeof(ar) / sizeof(ar[0]))
#define ZeroArray(ar)      { for (int i = 0; i < ArrayLen(ar); i++) ar[i] = 0; }

typedef unsigned int							TMVAL;

// 金矿副被匹配后查看时间
#define WORLD_HOME_WAIT_LOOK		(20)
#define WORLD_HOME_BATTLE_TIME		(3*60)
#define WORLD_HOME_LOCK_TIME			(WORLD_HOME_WAIT_LOOK+WORLD_HOME_BATTLE_TIME+10)

// 战斗加血道具ID
#define BATTLE_USE_ITEM_ID		(43)

// 最大好友数量
#define MAX_FRIEND_NUM      50

// 最大体力值
#define MAX_POINT           10

#define MAX_DAILY_POINT     50

// 每恢复1点体力值所需要的时间
#define POINT_GENERATE_TIME 600

#define MAX_TEAM_MEMBER     4

#define MAX_GUILD_MEMBER	50

#define MAX_FORGE_LEVEL		20

#define MAX_PET_EVOLVE		5

#define MAX_PET_STAR		5

#define MAX_PLAYER_LEVEL	100

#define MAX_PET_LEVEL		80

#define PET_SKILL_ITEM_ID	1000

#define MAX_BAG_SLOT		260

#define MAX_BOTTING_SCENE_PLAYER 10

#define MAX_NPC_NUM			10

#define MAX_VIP_LEVEL		15

#define MAX_GOLD_LEVEL		15

#define MAX_MAIL_NUM		200

#define IS_TREASURE_NPC_INDEX(x) ((x)>=1 && (x)<=10)
#define IS_WABAO_NPC_INDEX(x) ((x)>=11 && (x)<=20)
#define IS_CITY_WAR_NPC_INDEX(x) ((x)>=21 && (x)<=30)
#define IS_WORLD_BOSS_NPC_INDEX(x) ((x)>=31 && (x)<=40)
#define IS_WABAO_TEAM_NPC_INDEX(x) ((x)>=41 && (x)<=50)

#define RAND(x)				((int)((float)rand() / RAND_MAX * (x)))
template <UINT nSize>
class BitFlagSet_T
{
public:
	enum
	{
		BIT_SIZE = nSize, //位标记的长度，单位是二进制位
		BYTE_SIZE = 1+BIT_SIZE/8, //信息区占用的字节数
	};
	BitFlagSet_T(VOID)
	{
		memset((VOID*)m_aBitFlags, '\0', sizeof(m_aBitFlags));
	};
	//复制构造器
	BitFlagSet_T(BitFlagSet_T const& rhs)
	{
		memcpy((VOID*)m_aBitFlags, (VOID*)(rhs.GetFlags()), sizeof(m_aBitFlags));
	};
	~BitFlagSet_T() {};
	//复制操作符
	BitFlagSet_T& operator=(BitFlagSet_T const& rhs)
	{
		memcpy((VOID*)m_aBitFlags, (VOID*)(rhs.GetFlags()), sizeof(m_aBitFlags));
		return *this;
	};
	//设置所有标记位
	VOID MarkAllFlags(VOID)
	{
		memset((VOID*)m_aBitFlags, 0xFF, sizeof(m_aBitFlags));
	};
	//清除所有标记位
	VOID ClearAllFlags(VOID)
	{
		memset((VOID*)m_aBitFlags, 0x00, sizeof(m_aBitFlags));
	};
	//取指定的标记位
	BOOL GetFlagByIndex(INT const nIdx) const
	{
		if(0>nIdx||BIT_SIZE<=nIdx)
		{
			AssertEx(FALSE,"[BitFlagSet_T::GetFlagByIndex]: Index out of range!");
			return FALSE;
		}
		unsigned int nIndex = nIdx;
		return 0!=(m_aBitFlags[nIdx>>3]&(char)(1<<nIdx%8));
	}
	//清除指定的标记位
	void ClearFlagByIndex(INT const nIdx)
	{
		if(0>nIdx||BIT_SIZE<=nIdx)
		{
			AssertEx(FALSE,"[BitFlagSet_T::ClearFlagByIndex]: Index out of range!");
			return;
		}
		m_aBitFlags[nIdx>>3] &= ~(0x01<<(nIdx%8));
	}
	//设定指定的标记位
	VOID MarkFlagByIndex(INT const nIdx)
	{
		if(0>nIdx||BIT_SIZE<=nIdx)
		{
			AssertEx(FALSE,"[BitFlagSet_T::MarkFlagByIndex]: Index out of range!");
			return;
		}
		m_aBitFlags[nIdx>>3] |=	0x01<<(nIdx%8);
	}
	//所占用的字节数
	UINT GetByteSize(VOID) const {return BYTE_SIZE;}
	//所支持的标记数
	UINT GetBitSize(VOID) const {return BIT_SIZE;}
	//取数据区的指针
	CHAR const* const GetFlags(VOID) const {return m_aBitFlags;}
protected:
private:
	CHAR m_aBitFlags[BYTE_SIZE]; //数据存储区
};

struct WORLD_POS : public MemBase
{
	FLOAT	 x ;
	FLOAT	 y ;

	WORLD_POS(VOID)					: x(0.0f), y(0.0f)	{}
	WORLD_POS(FLOAT fX, FLOAT fZ)	: x(fX)	, y(fZ)		{}
	VOID	CleanUp( ){
		x = 0.0f ;
		y = 0.0f ;
	};

	WORLD_POS& operator=(WORLD_POS const& rhs)
	{
		x = rhs.x;
		y = rhs.y;
		return *this;
	}

	WORLD_POS operator+(const WORLD_POS& point) const
	{
		return WORLD_POS(x + point.x,
			y + point.y);
	}

	WORLD_POS operator-(const WORLD_POS& point) const
	{
		return WORLD_POS(x - point.x,
			y - point.y);
	}

	WORLD_POS operator*(float len) const
	{
		return WORLD_POS(x * len,
			y * len);
	}

	BOOL	operator==(WORLD_POS& Ref)
	{
		return (fabs(x-Ref.x)+fabs(y-Ref.y))<0.0001f;
	}
	BOOL	operator==(const WORLD_POS& Ref)
	{
		return (fabs(x-Ref.x)+fabs(y-Ref.y))<0.0001f;
	}

	FLOAT   DistanceSquareFrom(const WORLD_POS& otherPoint) const
	{
		if(this == &otherPoint)
			return 0.0;
		FLOAT disX = fabs(x - otherPoint.x);
		FLOAT disY = fabs(y - otherPoint.y);
		if(disX + disY < 0.0001f)
			return 0.0;

		return disX * disX + disY * disY;
	}

	FLOAT Distance(const WORLD_POS &otherPoint) const
	{
		FLOAT s = DistanceSquareFrom(otherPoint);
		return sqrtf(s);
	}

	VOID Normalize()
	{
		FLOAT l = sqrtf(x*x+y*y);
		if (l>0)
		{
			x /= l;
			y /= l;
		}
	}

	FLOAT Dot(const WORLD_POS &otherVec)
	{
		return x * otherVec.x + y * otherVec.y;
	}
};

typedef WORLD_POS		POS;
typedef WORLD_POS		Vector2;

//-------------------------------------------------------------------------*/
struct BoundBox
{
	POS mMin;
	POS mMax;

	int mLayer;
	//int mData1;
	//int mData2;
	//int mData3;

	POS GetCenter() const
	{
		return POS(mMin.x + (mMax.x-mMin.x)*0.5f, mMin.y+(mMax.y-mMin.y)*.5f);
	}

	float UpY() const
	{
		return mMax.y;
	}

	float BottomY() const
	{
		return mMin.y;
	}

	float LeftX() const
	{
		return mMin.x;
	}

	float RightX() const
	{
		return mMax.x;
	}

	bool InXBound(const POS &testPoint) const
	{
		if (testPoint.x > RightX() || testPoint.x < LeftX())
			return false;

		return true;
	}

	bool InYBound(const POS &testPoint) const
	{
		if (testPoint.y > UpY() || testPoint.y < BottomY())
			return false;

		return true;
	}

	bool OnGround(const POS &testPoint) const
	{
		return testPoint.y >= UpY() && testPoint.y - UpY() < 0.001f;
	}

	// 求得一个点垂直相交与此范围盒上的位置
	bool Intersect(POS &intersectPoint, const POS &startPoint, bool bDown = true) const
	{
		if (startPoint.x>mMax.x || startPoint.x<mMin.x)
			return false;

		if (startPoint.y<mMax.y)
			return false;

		intersectPoint.x = startPoint.x;
		intersectPoint.y = mMax.y;

		return true;
	}

	bool InBox(const POS &checkPoint) const
	{
		//if (bCenterDown && checkPoint.m_fZ>(mMin.m_fZ+(mMax.m_fZ-mMin.m_fZ)*0.5f))
		//	return false;

		return checkPoint.x>=mMin.x
			&& checkPoint.x<=mMax.x
			&& checkPoint.y>=mMin.y
			&& checkPoint.y<=mMax.y;
	}

	bool InBoxFuzzy(const POS &checkPoint, float fuzzy = 50.0f) const
	{
		return checkPoint.x>=(mMin.x - fuzzy)
			&& checkPoint.x<=(mMax.x + fuzzy)
			&& checkPoint.y>=(mMin.y - fuzzy)
			&& checkPoint.y<=(mMax.y + fuzzy);
	}

	void Merger(const BoundBox &box)
	{
		mMin.x = min(mMin.x, box.mMin.x);
		mMin.y = min(mMin.y, box.mMin.y);

		mMax.x = max(mMax.x, box.mMax.x);
		mMax.y = max(mMax.y, box.mMax.y);
	}
};
//-------------------------------------------------------------------------*/

enum ABNORMAL_STATE
{
    eABS_None,
    eABS_Bleed,
    eABS_Frozen,
    eABS_Dizziness,
    eABS_Burn,
    eABS_Electrified,
    eABS_Weakness,
    eABS_Poison,

    ABS_MAX_COUNT
};

struct DamageResult
{
    int    mTotalDamage;
    float   mTotalDamageF;
    int    mBaseDamage;
    int    mOriginTotalDamage;
    int    mActualOriginTotalDamage;
    bool	mIsMiss;
    bool	mIsCritical;
    bool    mIsBackAttack;
    bool    mIsAsymmetricAttack;
    bool    mIsHeadKill;        // 斩杀

    int     mBlocked;           // 格挡
    int     mCompensate;        // 护盾抵消

    int     mAdditionDamages[ABS_MAX_COUNT];     // 附加伤害值(冰冻、灼烧、感电...）
    float   mAdditionDamagesF[ABS_MAX_COUNT];     // 附加伤害值(冰冻、灼烧、感电...）

    int     mSelfHpAlter;        // 自身(回血)
    int     mSelfDamage;         // 伤害反弹

    int     mAttackerId;

    int     mHitNum;            //客户端数值，客户端自己解释

    DamageResult()
        : mTotalDamage(0)
        , mTotalDamageF(0)
        , mBaseDamage(0)
        , mActualOriginTotalDamage(0)
        , mIsMiss(false)
        , mIsCritical(false)
        , mIsBackAttack(false)
        , mIsAsymmetricAttack(false)
        , mIsHeadKill(false)
        , mBlocked(0)
        , mCompensate(0)
        , mSelfHpAlter(0)
        , mSelfDamage(0)
        , mAttackerId(0)
        , mHitNum(0)
    {
        ZeroArray(mAdditionDamages);
        ZeroArray(mAdditionDamagesF);
    }
};

inline INT Float2Int(float TValue)
{
    INT iValue = (INT)TValue;

    if (TValue > 0.0f)
    {
        if (iValue < 0)
            iValue = INT_MAX;
    }
    else if (TValue < 0.0f)
    {
        if (iValue > 0)
            iValue = INT_MIN;
    }

    //if (TValue > (float)INT_MAX)
    //    return INT_MAX;
    //else if (TValue < (float)INT_MIN)
    //    return INT_MIN;

	if(TValue - iValue < 0.500000f)
	{
		return iValue;
	}
	else
	{
        if (iValue < INT_MAX)
            iValue = iValue + 1;
		return iValue;
	}
};

float CaculateValue(const char* str, int level);

class CMyTimer  
{
private:
	UInt64 m_uTickTerm;
	UInt64 m_uTickOld;

public:
	BOOL m_bOper;

public:	
	CMyTimer()
	{
		CleanUp() ;
	}

	BOOL IsSetTimer( ){ return m_bOper ; }

	VOID SetTermTime( UInt64 uTerm ){ m_uTickTerm =uTerm; }
	UInt64 GetTermTime( ){ return m_uTickTerm ; }

	UInt64 GetTickOldTime( ){ return m_uTickOld; }

	VOID CleanUp( )
	{ 
		m_uTickTerm = 0 ;
		m_bOper = FALSE ;
		m_uTickOld = 0 ;
	}

	VOID BeginTimer(UInt64 uTerm, UInt64 uNow)
	{
		m_bOper = TRUE;
		m_uTickTerm =uTerm;	
		m_uTickOld =uNow;
	}

	BOOL CountingTimer(UInt64 uNow)
	{
		if(!m_bOper)
			return FALSE;

		UInt64 uNew =uNow;

		if(uNew<m_uTickOld+m_uTickTerm )
			return FALSE;

		m_uTickOld =uNew;

		return TRUE;
	}
	UInt64 GetLeaveTime(UInt64 uNow)//剩余时间;
	{
		if(!CountingTimer(uNow))
		{
			return m_uTickTerm+m_uTickOld-uNow;
		}
		return 0;
	}
};

class DropGrouItems
{
public:
    DropGrouItems()
        : mDropItemType(0)
        , mDropItemId(0)
        , mDropItemNum(0)
        , mDropRate(0)
    {
    }

public:
    int mDropItemType;
    int mDropItemId;
    int mDropItemNum;
    int mDropRate;
};

enum CHAR_GENDER
{
    GENDER_MALE = 1,
    GENDER_FEMAIL = 2,
    GENDER_OTHER = 3
};

enum CHAR_ATTR_INDEX
{
	ATTR_BASE_BEGIN = 1,
	ATTR_ATT = ATTR_BASE_BEGIN,						// 攻击
	ATTR_DEF,										// 防御
	ATTR_MAX_HP,									// 最大生命
	ATTR_CRIT,										// 暴击率
	ATTR_HIT,										// 命中率
	ATTR_MISS,										// 闪避率
	ATTR_IGNORE_DEF_ATT,							// 无视防御攻击
	ATTR_CRIT_PERCENT,								// 暴击伤害百分比
	ATTR_PENETRATE,									// 攻击穿透
	ATTR_ATT_DISTANCE,								// 攻击距离
	ATTR_MOVE_SPEED,								// 移动速度
	ATTR_ATT_SPEED,									// 攻击速度
	ATTR_IMMUNITY,									// 伤害减免
	ATTR_IMMUNITY_PERCENT,							// 伤害减免百分比
	ATTR_HURT,										// 伤害增强
	ATTR_HURT_PERCENT,								// 伤害增强百分比
	ATTR_JUMP,										// 跳跃力
	ATTR_CRIT_IMMUNITY,								// 暴击抵抗
	ATTR_HARD,										// 硬直
	ATTR_ATT_FIRE,									// 火属性攻击
	ATTR_ATT_COLD,									// 冰属性攻击
	ATTR_ATT_LIGHT,									// 电属性攻击
	ATTR_ATT_DARK,									// 暗属性攻击
	ATTR_FIRE_IMMUNITY,								// 火属性抗性
	ATTR_COLD_IMMUNITY,								// 冰属性抗性
	ATTR_LIGHT_IMMUNITY,							// 电属性抗性
	ATTR_DARK_IMMUNITY,								// 暗属性抗性
    ATTR_CONTROL_IMMUNITY,                          // 对控制效果（冰冻、眩晕、减速等）是否免疫
	ATTR_BASE_END,

    // 特殊属性
	ATTR_EXTRA_BEGIN = ATTR_BASE_END + 9,			// 预留一些空间以备以后增加基础属性类型
	ATTR_BUFF_ATTR	= ATTR_EXTRA_BEGIN,			    // Buffer类属性;包括技能触发、Buffer触发等
	ATTR_ALL_ELEMENT_ATTACK,						// 提升火,冰,电,暗属性技能的攻击
    ATTR_ALL_ELEMENT_IMMUNITY,					    // 减免受到火,冰,电,暗属性技能攻击时的伤害

	ATTR_GENERATE_HP,								// 周期性生命回复// 即将删除不用
	ATTR_CLEAR_ALL_BUFF,							// 清除所有状态

    ATTR_DROP_GOLD_MULTIPLE,                        // 掉落金币翻倍倍数
    ATTR_HUNT_EXP_MULTIPLE,                         // 狩猎经验翻倍倍数
    ATTR_STAGE_EXP_MULTIPLE,                        // 通关经验翻倍倍数
    
    ATTR_EXTRA_END,
	ATTR_MAX_NUMBER,
};

enum CONTROL_IMMUNITY_ATTR_FLAG
{
    CONTROL_IMMUNITY_ATTR_NON = 0,
    CONTROL_IMMUNITY_ATTR_FROZEN = 1,
    CONTROL_IMMUNITY_ATTR_DIZZI = 2,
    CONTROL_IMMUNITY_ATTR_SLOWDOWN = 3,

    CONTROL_ATTR_FLAG_MAX_NUM,
};

// AffectSlot: 计算各个属性类型的影响槽位，槽位用于装备属性、宠物属性、天赋属性等的配置
// attrType: CHAR_ATTR_INDEX
#define AffectSlot(attrType, isByRate) ((isByRate) ? ((attrType) * 2) : ((attrType) * 2 - 1))

// IsRateSlot: 是百分比槽位还是固定值槽位
#define IsRateSlot(attrSlot) ((attrSlot) % 2 == 0)

// AttrTypeFromSlot: 计算槽位所影响的属性类型
#define AttrTypeFromSlot(attrSlot)  ((attrSlot + 1) / 2)

// RateSlotFromAttrType: 计算指定属性值的百分比槽位
#define RateSlotFromAttrType(attrType)  ((attrType) * 2)

// FixedSlotFromAttrType: 计算指定属性值的固定值槽位
#define FixedSlotFromAttrType(attrType) ((attrType) * 2 - 1)

enum EQUIP_QUALITY
{
    EQUIP_QA_WHITE = 1,
    EQUIP_QA_GREEN,
    EQUIP_QA_BLUE,
    EQUIP_QA_PUEPLE,
    EQUIP_QA_ORANGE,
};

enum EQUIP_LOCATION
{
    EQUIP_NONE,
    EQUIP_WEAPON,   //武器
    EQUIP_CAP,      //头盔
    EQUIP_HANDS,    //护手
    EQUIP_ARMOR,    //衣服
    EQUIP_PANTS,    //护腿
    EQUIP_FOOT,     //鞋子

    EQUIP_NECKLACE,  //项链
    EQUIP_RING,     //戒指
    EQUIP_BRACELET, //手镯
    EQUIP_DANGLER,  //耳环
    EQUIP_HEADDRESS, //头饰
    EQUIP_TATTOO,    //纹身

    FASHION_ARMS,   //时装武器
    FASHION_HEAD,   //时装头部
    FASHION_FACE,   //时装脸部
    FASHION_COAT,   //时装上衣
    FASHION_TROUSERS,   //时装下装
    FASHION_EFFECT,   //时装特效
};

enum DECORATION_PART
{
    DECORATION_ARMS = 1,    //可见部位-武器
    DECORATION_HEAD,        //可见部位-头部
    DECORATION_FACE,        //可见部位-脸部
    DECORATION_COAT,        //可见部位-上衣
    DECORATION_TROUSERS,    //可见部位-下装
    DECORATION_EFFECT,      //可见部位-特效

    DECORATION_MAX
};

enum BAG_TYPE
{
	BAG_EQUIP = 1,
	BAG_FASHION,
	BAG_ACCESSORY,
	BAG_ITEM,
	BAG_FRAGMENT,
};

enum CHAR_OCCUPATION
{
    FIGHTER = 1,
    WIZARD = 2,
    GUNNER = 3,

    OTHER_OCCUPATION
};

enum CHAR_PROFESSION
{
    FIGHTER_ONE = 101,
    FIGHTER_TWO = 102,
    FIGHTER_THREE = 103,
    FIGHTER_FOUR = 104,

    WIZARD_ONE = 201,
    WIZARD_TWO = 202,
    WIZARD_THREE = 203,
    WIZARD_FOUR = 204,
};

#define MainOccupation(occ) ((occ) / 100)
#define SubOccupation(occ)  ((occ) % 100)

enum NOTICE_TYPE
{
	NOTICE_NONE,
	NOTICE_SKILL,
	NOTICE_GAIN_ITEM,
	NOTICE_GAIN_FASHION,
	NOTICE_GAIN_PET,
	NOTICE_PET_STAR_LVUP,
	NOTICE_PET_EVOLVE,
	NOTICE_CHANGE_OCCUPATION,
	NOTICE_LEVEL_UP,
	NOTICE_POWER_UP,
	NOTICE_EQUIP_FORGE,
	NOTICE_EQUIP_INTENSIFY,
	NOTICE_GM,
	NOTICE_WABAO,
	NOTICE_MONSTER,
	NOTICE_ONLINE,
	NOTICE_ARENA_RESULT = 20,       // 斗兽场战斗结果公告
};

enum LOTTERY_INDEX
{
	LOTTERY_INDEX_GOLD = 1,
	LOTTERY_INDEX_EQUIP,
	LOTTERY_INDEX_PET,
};

enum LOTTERY_TYPE
{
	LOTTERY_NONE,
	LOTTERY_GOLD_NEW,
	LOTTERY_GOLD_NORMAL,
	LOTTERY_GOLD_SPECIAL,
	LOTTERY_EQUIP_NEW,
	LOTTERY_EQUIP_NORMAL,
	LOTTERY_EQUIP_SPECIAL,
	LOTTERY_PET_NEW,
	LOTTERY_PET_NORMAL,
	LOTTERY_PET_SPECIAL,
};

enum EQUIP_USEOCCUPATION
{
    EQUIP_OF_FIGHTER = FIGHTER,
    EQUIP_OF_WIZARD = WIZARD,
    EQUIP_OF_GUNNER = GUNNER,

    EQUIP_OF_ALL = OTHER_OCCUPATION
};

enum GUILD_POSITION
{
	GUILD_NORMAL,
	GUILD_VICE_LEADER,
	GUILD_LEADER,
};

enum eSceneType
{
	eStageScene				= 100,
	eTownScene				= 101,
	ePVPScene				= 102,
	eDefendScene			= 103,
	eTreasuresStageScene	= 104,
	eBottingScene			= 105,
    eYaBiaoScene            = 106,
	eWorldBossScene			= 107,
    eGuideScene             = 108,
    eAbyssStageScene        = 109,
	eArenaTownScene			= 110,
	eArenaScene				= 111,
    eGreedyGamblingScene    = 112,
    eComlicatedStageScene   = 113,
    eWaiZhuanStageScene     = 114,
    eChopinStageScene       = 115,
    eGuildStageScene        = 116,
	eArenaDefenseScene	 = 117,			// 守擂场景, 服务器不实例, 以斗兽场场景与其对应

    eProvingStageScene      = 118,      // 试炼之塔副本
	eWorldPVPScene			= 119,		// 世界跨服战PVP
	eGuildPVPScene				= 120,		// 公会战PVP
	eGuildBattleMonsterScene = 121,	// 公会战怪物副本
	eWorldBattleScene = 122,
	eGoldHomeScene = 123,
	eGoldBattleScene = 124,
	eWorldWarScene = 125,
	eMaxScene,
};

enum OBJECT_TYPE
{
	eNONE,

    eMONSTER = 1,
    eMONSTER_ELITE = 2,
    eMONSTER_BOSS = 3,
    eCARR_VEHICLE = 4,

    eCHARACTER = 5,
    eROLE = 6,
    ePET = 7,
    eNPC = 8,
    ePROTECTEDNPC = 9,		//防守关卡中被保护的NPC

	ePROP = 10,
	eCHEST = 11,
	eCAPTIVE = 12,
	eSTAGEEVENT = 13,
	eBUILDMONSTER = 14,
	eFRUIT = 15,
	eSTAGEAREA = 16,		// 关卡区域管理
	eTRIGGER = 17,			// 触发机关
	eBLOCK = 18,
	eAREAFLAG		= 20,	// 区域门内的路径引导标志
	eBIGBOSS		= 21,	// 组合大BOSS
	ePARTMONSTER	= 22,	// 组合 BOSS 的部件怪物
	eWorldBoss		= 23,	// 世界BOSS

	eResourceDrop = 28,     //资源（除道具 装备）掉落
	eActiveProp = 29,        //活动道具（龙头等）
	eItemDrop = 30,         //道具掉落
	eEquipDrop = 31,        //装备掉落
	ePVPPlayer = 32,

    eYaBiaoChe = 33,
    eAreaCheck = 34,        // 区域开放限制对象
    eSettlementCheck = 35,  // 结算限制对象
    eGuideFlag = 36,        // 引导标志
	eStoryLineMonitor = 37, // 剧情辅助对象
	eGoldMonster = 39,              // 金矿战斗中的怪物
	eGoldTower = 40,                 // 金矿战斗中的塔
	eGoldCoffers = 41,					// 金库
	eOBJECT_MAX,
};


#define CHECK_TYPE(type)	(type>eNONE && type<eOBJECT_MAX)
//-------------------------------------------------------------------------

enum RESOURCE_TYPE
{
	RESOURCE_NONE,
    RESOURCE_DIAMOND,			// 钻石
    RESOURCE_GOLD,				// 金币
    RESOURCE_POWER,				// 体力
	RESOURCE_FRIEND_POINT,		// 好友点
    RESOURCE_ITEM,				// 物品（不包括装备）
	RESOURCE_EQUIP,				// 装备
	RESOURCE_GEM_FORGE_POINT,	// 宝石锻造值
    RESOURCE_SKILL_POINT,		// 技能点
    RESOURCE_PRESTIGE_POINT,    // 声望
	RESOURCE_EXP,				// 经验值
    RESOURCE_FLOWER,			// 鲜花（好友系统使用）
	RESOURCE_PET,				// 宠物
	RESOURCE_HORNOR,			// 荣誉点（竞技场使用）
	RESOURCE_BUFFITEM,			// BUFF
	RESOURCE_VIPEXP,			// vip经验
    RESOURCE_GUILD_FUNDS,       // 公会资金
    RESOURCE_GUILD_EXP,         // 公会经验
    RESOURCE_GUILD_DONATE_PONT, // 公会个人贡献
	RESOURCE_CASH,				// 充值
};

enum RESOURCE_CHANGE_REASON
{
	REASON_NONE,
	REASON_STAGE_DROP,			// 关卡掉落
	REASON_SHOP,				// 商城
	REASON_MAIL,
	REASON_SELL_EQUIP,
	REASON_EQUIP_FORGE,
};


enum MAIL_TYPE
{
	eMAIL_NONE = 0,
	eMAIL_DROP = 1,
	eMAIL_PVP = 2,
	eMAIL_WORLDBOSS_1 = 3,
	eMAIL_WORLDBOSS_2 = 4,
	eMAIL_YABAO_WIN = 5,
	eMAIL_YABAO_FAIL =6,
	eMAIL_CDKEY = 7,
	eMAIL_CITYWAR = 8,
	eMAIL_PVP_FAILED = 9,
	eMAIL_DONATE_OBTAIN = 10,
	eMAIL_VIP = 11,
	eMAIL_CHARGE = 12,
	eMAIL_GUILD_STAGE_PROGRESS = 13,
	eMAIL_GUILD_MEMBERSHIP_KICKED = 14,
	eMAIL_GUILD_LEADER_INHERITED = 15,
	eMAIL_GUILD_BUY_FESTIVAL_CAKE = 16,
	eMAIL_PVP_SEASON_REWARD = 17,

	eMAIL_GUILD_TREASURE_FOUND = 20,
	eMAIL_GUILD_TREASURE_PICK = 21,
	eMAIL_GUILD_TREASURE_ROB = 22,
	eMAIL_GUILD_TREASUER_SETTLE = 23,
	eMAIL_BETA_TEST_DIAMOND = 24,
	ePAY_AMOUNT_ERROR = 25,  // 通知支付金额异常
	eMAIL_WORLD_BATTLE_RESULT = 26,
	eMAIL_WORLD_BATTLE_WIN_RESULT = 27,
	eMAIL_WORLD_BATTLE_REWARD = 28,
	eMAIL_GUILD_BATTLE_RESULT = 29,
	eMAIL_GUILD_BATTLE_NOTIFY = 30,
};

enum TRANSIT_MAILT_TYPE
{
    eTMAIL_FRIEND_INTERACTION      = 1,
    eTMAIL_SYSTEM_AWARD
};

// 抽取模式
enum GROUP_TYPE
{
    GROUP_TYPE_ALL,
    GROUP_TYPE_LOW,
    GROUP_TYPE_HIGH,
};

enum CASH_TYPE
{
	CASH_BEGIN = 1,
	CASH_WEEKLY_CARD = CASH_BEGIN,	// 周卡
	CASH_MONTHLY_CARD,				// 月卡
	CASH_INFINITE_CARD,				// 不限时卡
	CASH_FIRST,						// 首充奖励
	CASH_ONCE,						// 单笔充值
	CASH_TOTAL,						// 累计充值
	CASH_LEVEL_UP,					// 成长基金
	CASH_LOGIN,						// 连续登录
	CASH_SHOP,						// 限时礼包
	CASH_COST,						// 累计消费
	CASH_DOUBLE,					// 充值双倍
	CASH_VIP,						// VIP奖励
	CASH_7DAY,						// 7日充值奖励
    CASH_TIME_LIMIT_MISSIONS,       // 限时积分任务
    CASH_TIME_LIMIT_EXCHANGE,       // 限时兑换
	CASH_END,
};

enum PLAYER_LOG_TYPE
{
	PLAYER_LOG_NONE,
	PLAYER_LOG_LOGIN,
	PLAYER_LOG_RESOURCE,
	PLAYER_LOG_EQUIP,
	PLAYER_LOG_TITLE,
	PLAYER_LOG_TASK,
	PLAYER_LOG_LEVEL_UP,
	PLAYER_LOG_STAGE,
	PLAYER_LOG_CHAT,
	PLAYER_LOG_TEAM,
};

enum ERROR_CODE
{
    ERROR_NONE = 1,
	ERROR_INVALID_REQUEST = 2,
    ERROR_INVALID_EQUIP = 3,						// 无效的装备
	ERROR_LOW_LEVEL = 4,							// 等级不足
    ERROR_INVALID_OCCUPATION = 5,					// 职业不正确
    ERROR_INVALID_GENDER = 6,						// 性别不符
    ERROR_MAX_FORGE_LEVEL = 7,						// 已达到最大精炼等级
    ERROR_NO_ENOUGH_MONEY = 8,						// 没有足够的金钱
    ERROR_NO_ENOUGH_GEM = 9,						// 没有足够的宝石
    ERROR_NO_ENOUGH_DIAMOND = 10,					// 没有足够的钻石
    ERROR_NO_ENOUGH_ITEM = 11,						// 没有足够的物品
	ERROR_NO_ENOUGH_RESOURCE = 12,					// 资源不足
	ERROR_ROLE_NOTEXIST = 13,						// 角色不存在
	ERROR_INVALID_PET = 14,							// 无效的宠物
    ERROR_CONDITION_NOTMATCH = 15,                  // 条件不满足
	ERROR_BAG_FULL = 16,							// 背包已满
	ERROR_PLAYER_OFFLINE = 17,						// 玩家不在线
	ERROR_ACCOUNT_ERROR = 18,						// 帐号或密码错误
	ERROR_ACCOUNT_BAN = 19,							// 帐号被封停
	ERROR_CHAT_BAN = 20,							// 你已被禁言
	ERROR_NAME_EXIST = 21,							// 角色名重复
    ERROR_INVALID_PET_SKILL = 22,                   // 无效的宠物技能
    ERROR_MAX_PET_SKILL = 23,                       // 宠物技能已经达到最大等级
    ERROR_MAX_POWER_BUY_COUNT = 24,                 // 体力购买已经达到最大次数
	ERROR_INVALID_CDKEY = 25,						// 无效的CDKEY
	ERROR_CDKEY_ALREADY_USED = 26,					// 该CDKEY已经被使用
	ERROR_CDKEY_SAME_TYPE = 27,						// 已经使用过同样类型的CDKEY
	ERROR_CDKEY_TIME_OUT = 28,						// 该CDKEY已过时
    ERROR_INVALID_ITEM = 29,                        // 无效的道具
    ERROR_INVALID_FRAGMENT = 30,                    // 无效的碎片
    ERROR_NO_ENOUGH_FRAGMENT = 31,                  // 没有足够的碎片
	ERROR_CDKEY_MAX_NUM = 32,						// 该CDKEY已被领完
    ERROR_INVALID_QUOTA_ID = 33,                    // 无效配额ID
    ERROR_QUOTA_EXTRA_FULL_OR_REJECT = 34,          // 配额不能再增加
	ERROR_NO_PERMISSION = 35,						// 你没有权限
    //道具商城购买错误
    ERROR_SHOP_WRONG_ITEMLIST = 40,				    // 购买道具的列表参数无效
    ERROR_SHOP_OVER_ITEMLIST = 41,				    // 一次购买道具超过50个
    ERROR_SHOP_NOTENOUGH_GOLD = 42,				    // 金币不足,购买失败
    ERROR_SHOP_NOTENOUGH_GEM = 43,				    // 钻石不足,购买失败

    ERROR_NOEXIST_BUY_ITEM_LIST,
    ERROR_NOEXIST_ITEM_CONFIG,
    ERROR_NOEXIST_ITEMPACKET_CONFIG,
    ERROR_NOEXIST_SHOP_CONFIG,

    ERROR_NOT_IS_ITEM_BOX,						    // 不是有效的盒子 或数量不够
    ERROR_ITEM_ID_NOEXIST,
    ERROR_INVALID_INTENSIFY_STONE,                  // 无效的强化石

    ERROR_PET_MAX_LEVEL,                            // 宠物已达到当前阶段最大等级
    ERROR_PET_PLAYER_LEVEL_LOW,                     // 宠物等级已达到玩家当前等级的限制

	ERROR_ALREADY_PAY,								// 已处理过的订单

	ERROR_EQUIP = 100,
	ERROR_EQUIP_REFINE_FAIL = 101,					// 精炼失败
	ERROR_EQUIP_NEED_SELECT_EQUIP = 102,			// 请先选择用来强化的装备
	ERROR_EQUIP_MAX_COUNT = 103,					// 选择的数量已达到最大
	ERROR_EQUIP_NOT_INSET_SMAE_GEM_ID = 104,		// 不能镶嵌相同ID的宝石
	ERROR_EQUIP_NO_ENOUGH_ITEM_PUNCH = 105,			// 打孔所需的道具不足
	ERROR_EQUIP_NEED_SELECT_GEM = 106,				// 请先选择用来镶嵌的宝石
	ERROR_EQUIP_NEED_SELECT_GEM_FOR_DECOMPOSE = 107,            //请先选择用来分解的宝石
	ERROR_EQUIP_NO_ENOUGH_FORGE_POINT = 108,                    //锻造值不足
	ERROR_INVALID_EQUIP_ID = 109,					// 无效的装备Id
    ERROR_EQUIP_GEM_MAX_LEVEL = 110,                // 宝石已达到最大等级
    ERROR_EQUIP_NOTSALABLE = 111,                   // 装备不可出售
    ERROR_EQUIP_NOTDECOMPOSABLE = 112,              // 装备不可分解
    ERROR_EQUIP_NOTEXCHANGABLE = 113,               // 装备不可兑换
    ERROR_EQUIP_CANNOT_USE_INTSTONE = 114,          // 首饰不可用强化石强化
    ERROR_EQUIP_CANNOT_USE_INTEQUIP = 115,          // 被献祭的装备不可用于强化
    ERROR_EQUIP_CANNOT_USE_ITEM_ATTM = 116,         // 被献祭的碎片不可以用于附魔升星
    ERROR_EQUIP_CANNOT_USE_EQUIP_ATTM = 117,        // 被献祭的装备不可以用于附魔升星

    ERROR_TITLE_NO_QUOTA    = 120,                  // 称号佩戴数已经最大
    ERROR_FUWEN_FUSHION_FAILED = 130,               // 符文合成失败

    ERROR_EQUIP_ATTM_ALREADY_EXISTS = 140,          // 装备已经附魔
    ERROR_EQUIP_ATTM_NOT_ALLOWED,                   // 该装备不可以附魔
    ERROR_EQUIP_NO_ATTM,                            // 该装备没有附魔
    ERROR_EQUIP_INVALID_ATTM_INDEX,                 // 无效的附魔ID
    ERROR_EQUIP_ATTM_MAX,                           // 附魔已经达到最大星级

    ERROR_EQUIP_FUSHION_MUST_SAME = 150,            // 必须同一种装备才可以融合觉醒
    ERROR_EQUIP_FUSHION_MUST_NOT_IN_USE,            // 必须是不在使用中的装备才可以被融合
    ERROR_EQUIP_FUSHION_NOT_IN_FONFIG,              // 该装备不可以融合觉醒

    ERROR_NOT_TEAM_STAGE = 202,                     // 组队时不可以进入单人副本
    ERROR_EXTRA_COUNT_NOMORE = 203,					// 通关次数已经购买过
	ERROR_INVALID_SCENE_CODE = 204,					// 场景ID 无效
    ERROR_SCENE_ENTER_CONDITION_FAILE = 205,		// 进入场景的条件不成立
    ERROR_SCENE_LEADER_ONLY  = 206,                 // 普通队伍成员不可以进入场景
    ERROR_SCENE_NOT_UNLOCK = 207,                   // 副本未解锁
    ERROR_PEPOLE_COUNT_TOO_SMALL = 208,             // 进入副本人数不足
    ERROR_PEPOLE_COUNT_TOO_BIG = 209,               // 进入副本人数太多
    ERROR_PEPOLE_LEVEL_TOO_SMALL = 210,             // 进入副本的等级不足
    ERROR_ACCESS_COUNT_EXCESS = 211,                // 进入副本次数过多
    ERROR_CLEAR_COUNT_EXCESS = 212,                 // 通关副本次数过多
    ERROR_STAGE_ALREADY_COMPLETED = 213,            // 副本已经结算
    ERROR_POWER_EXHAUSTED = 214,                    // 体力不足
    ERROR_TEAMATE_LEVEL_TOO_SMALL = 215,            // 玩家{0}等级不足
    ERROR_TEAMATE_ACCESS_COUNT_EXCESS = 216,        // 玩家{0}进入副本次数过多
    ERROR_TEAMATE_CLEAR_COUNT_EXCESS = 217,         // 玩家{0}通关副本次数过多
    ERROR_TEAMATE_SCENE_NOT_UNLOCK = 218,           // 玩家{0}副本未解锁
    ERROR_TEAMATE_POWER_EXHAUSTED = 219,            // 玩家{0}体力不足

    ERROR_SAODANG_STAR_NOT_REACH = 220,             // 副本通关星级不足以扫荡
    ERROR_SAODANG_RESOURCE_INSUFFICIEN = 221,       // 扫荡卷不足

	ERROR_WABAO_STAGE_MAX = 222,					// 每日挖宝通关次数达到上限，无法进入
	ERROR_NO_TICKET = 223,							// 没有入场券，无法进入
    ERROR_TREASUREMAP_STAGE_MAX = 224,              // 每日海贼王通关次数达到上限，无法进入

    ERROR_WORDL_BOSS_COMPLETED = 225,               // 世界 BOSS 已经结束

    ERROR_FAILED_TO_JOIN_ACTIVITY = 230,            // 参加活动失败

    ERROR_WAITING_TEAMMATE  = 240,                  // 等待其他队友准备

    ERROR_YABIAO_NO_BIAOCHE = 250,                  // 没有准备镖车
    ERROR_YABIAO_COUNT_EXCESS = 251,                // 玩家XXX剩余护送次数不足1次

    ERROR_SAODANG_NOT_PRESENT = 252,                // 副本不可扫荡

    ERROR_GUIDE_FAILED      = 256,                  // 战斗引导失败

	ERROR_TEAM_EXIST = 300,						// 已存在队伍
	ERROR_IS_TEAM_MEMBER = 301,					// 已经在队伍中
	ERROR_TEAM_FULL = 302,						// 队伍已满
	ERROR_ALREADY_APPLY = 303,					// 已发送过入队申请
	ERROR_TEAM_NOT_EXIST = 304,					// 队伍不存在
	ERROR_TEAM_NOT_APPLY = 305,					// 该玩家没有申请过入队
	ERROR_IS_NOT_LEADER = 306,					// 你不是队长
    ERROR_TEAM_KICK_MEMBER = 307,               //玩家{0}被踢出队伍
    ERROR_TEAM_AGREE_APPLY = 308,               //玩家{0}同意你加入队伍
    ERROR_TEAM_ACCEPT_INVITE_REFUSE = 309,      //玩家{0}拒绝加入队伍
    ERROR_TEAM_APPLY_TEAM = 310,                //玩家{0}申请加入队伍   
    ERROR_TEAM_INVITE_MEMBER = 311,             //玩家{0}邀请你加入队伍
    ERROR_TEAM_ADD_MEMBER = 312,                //玩家{0}加入队伍
    ERROR_TEAM_LEVEL_TEAM = 313,                //玩家{0}离开队伍
    ERROR_TEAM_MODIFY_TEAM =314,                //队伍规则已修改
    ERROR_TEAM_REMOVE_MEMBER = 315,             //是否确认将{0}移除队伍？
    ERROR_TEAM_APPOINT_TEAM_LEADER = 316,       //玩家{0}被任命为队长
    ERROR_TEAMMATE_NOT_IN_SAME_SCENE  = 317,    // 组员不在同一场景
	ERROR_TEAM_LESS_ACTIVE_TIME = 318,			// 队伍正在活动中，无法加入
	ERROR_OTHER_TEAM_EXIST = 319,				// 对方已有队伍

	ERROR_GUILD_NOT_EXIST = 350,				// 公会不存在
	ERROR_GUILD_EXIST = 351,					// 已经有公会
	ERROR_INVALID_GUILD_NAME = 352,				// 公会名字非法
	ERROR_GUILD_TIME_LIMIT = 353,				// 离开公会不足24小时，无法加入公会
	ERROR_GUILD_FULL = 354,						// 公会已满，无法加入
	ERROR_IS_GUILD_MEMBER = 355,				// 已经是公会成员
	ERROR_IS_GUILD_APPLY = 356,					// 已经申请过加入公会
	ERROR_GUILD_POSITION_APPLY = 357,			// 只有会长和副会长有权同意入会申请
	ERROR_GUILD_NOT_APPLY = 358,				// 该玩家没申请过加入公会
	ERROR_LEADER_CANNOT_LEAVE = 359,			// 公会里还有其它成员，会长无法离开公会
	ERROR_NO_KICK_PERMISSION = 360,				// 你没有权限剔除该玩家
	ERROR_IS_NOT_GUILD_LEADER = 361,			// 只有队长有设置权限
	ERROR_ELDER_NUM_MAX = 362,					// 最多能设置两名副会长
    ERROR_REQUIRE_GUILD_MEMBERSHIP = 363,       // 需要公会成员身份
    ERROR_NO_FRAGMENT_ASK_QUOTA = 364,          // 捐赠请求次数超限
    ERROR_INVALID_DONTE_TARGET = 365,           // 被捐赠者已经无效
    ERROR_INVALID_DONTE_COUNT = 366,            // 无效的捐赠数量
    ERROR_REQUIRE_SAME_GUILD_MEMBERSHIP = 367,  // 需要在同一个公会
	ERROR_IMPEACH_POSITION = 368,				// 只有副会长才能发起弹劾
	ERROR_NOT_IMPEACH_TIME = 369,				// 会长7天以上不在线才能弹劾
	ERROR_ALREADY_IMPEACH = 370,				// 你已经弹劾过了
    ERROR_REQUIRE_HIGH_GUILD_AUTHORITY = 371,   // 需要高级公会成员权限
    ERROR_GUILD_DONATE_REPEATED = 372,          // 重复公会捐献
    ERROR_GUILD_SIGN_REPEATED = 373,            // 重复公会签到
    ERROR_GUILD_STAGE_AWARD_REPEATED = 374,     // 重复领取公会副本进度奖励
    ERROR_GUILD_LEVEL_TOO_LOW = 375,            // 公会等级过低
    ERROR_GUILD_NO_ENOUGH_MONDEY = 376,         // 公会资金不足
	ERROR_GUILD_APPLY_LOW_LEVEL = 377,			// 入会等级不足
    ERROR_GUILD_NO_ENOUGH_EXP = 378,            // 公会经验不足

    ERROR_GUILD_STAGE_REQUIRE_MEMBERSHIP = 380, // 公会副本只允许公会成员进入
    ERROR_GUILD_STAGE_REQUIRE_SAME_MEMBERSHIP,  // 公会副本只允许相同公会成员进入
    ERROR_GUILD_STAGE_PROGRESS_NOT_REACH,       // 公会副本进度未达成
    ERROR_GUILD_STAGE_ACCESS_NO_QUOTA,          // 公会副本已经没有剩余次数

    ERROR_GUILD_FRAGMENT_NOT_DONATABLE,         // 该碎片不可以请求捐赠

    ERROR_GUILD_NOT_IN_TREAUSER_TEAM,           // 玩家不在寻宝团中
    ERROR_GUILD_ALREADY_IN_TREAUSER_TEAM,       // 玩家已经在寻宝团中
    ERROR_GUILD_NO_ROBS_COUNT,                  // 寻宝团抢劫次数超限
    ERROR_GUILD_NOT_TREASER_TEAM_ID,            // 无效的寻宝团ID  
    ERROR_GUILD_NOT_TREASURE_SUONER,            // 玩家并非是寻宝团创建人
    ERROR_GUILD_TREASURE_MUSTI_IN_SAME_GUILD,   // 目标寻宝团必须与自己在同一公会 
    ERROR_GUILD_CAN_NOT_ROB_IN_SAME_GUILD,      // 抢劫的目标寻宝团必须与自己不在同一公会 
    ERROR_GUILD_TREASURE_NOT_ENOUGH_LEFT_TIME,  // 寻宝团剩余时间不足加入新成员
    ERROR_GUILD_TREASURE_ROB_FAILED,            // 打劫寻宝团失败
    ERROR_GUILD_TREASURE_NOT_COMPLETE,          // 寻宝团未结束
    ERROR_GUILD_TREASURE_SETTLED,               // 寻宝已经领取
    ERROR_GUILD_TREASURE_ALREADY_PICKED,        // 已经拾取过该寻宝团
    ERROR_GUILD_TREASURE_NO_PICKABLE_RES,       // 该寻宝团可拾取资源为零
    ERROR_GUILD_TREASURE_NO_QUOTA,              // 今日不可再寻宝

    ERROR_SKILL_NOT_UNLOCK = 400,               //技能未解锁
    ERROR_SKILL_PLEVEL_TOO_LOW = 401,           //玩家学习等级不足
    ERROR_SKILL_EVOLU_MAX_LEVEL = 402,          //已强化到最大等级

    ERROR_GUILD_ONT_IN_BATTLE = 420,            // 公会没有参加公会战
    ERROR_GUILD_BATTLE_ARRANGEMEMT_OUT_OF_TIME, // 该时间段内不可调整布防
    ERROR_GUILD_BATTLE_DEFENCER_OUT_OF_RANGE,   // 布防塔人数已满
    ERROR_GUILD_BATTLE_NOT_IN_DEFENCER,         // 布防塔中没有该成员
	ERROR_GUILD_BATTLE_TOWNER_INDEX_ERROR,
	ERROR_GUILD_BATTLE_STARTING,						// 公会战开始中

    ERROR_ACTION_REWARD_ALREADY_GET = 500,			//活动通用 - 已经领取过奖励了
    ERROR_ACTION_REWARD_NOEXSIT,					//活动通用 - 领取奖励参数无效
    ERROR_ACTION_NO_ACTION_DATA,					//活动通用 - 没有该活动数据(一般是没有初始化)
    ERROR_ACTION_ONTHEHOUR_WRONG_HOUR,				//整点活动 - 领取的小时不是当前小时
    ERROR_ACTION_ONTHEHOUR_OVERTIME,				//整点活动 - 领取超时(过了当前整点的30分钟)
    ERROR_ACTION_ONTHEHOUR_NODATA,					//整点活动 - 没有该活动的数据
    ERROR_ACTION_7DAY_NOTTHISDAY,					//7日登录  - 领取的天数错误
    ERROR_ACTION_STATE_DATA_NOEXIST,
    ERROR_ACTION_TIME_REWARD_CONFIG_NOEXIST,
    ERROR_ACTION_ALREADY_REWARD,
    ERROR_ACTION_REWARD_CONFIG_ERROR,
    ERROR_ACTION_ACCESS_COUNT_EXCESS,				//活动通用 - 活动参加次数已经用完
    ERROR_ACTION_PARTICIPANTS_MINIMUM,              //活动通用 - 活动参加人数不足
    ERROR_ACTION_ACTIVITY_NOT_START,                //活动通用 - 活动没有接取
    ERROR_ACTION_LVEL_TOO_SMALL,                    //活动通用 - 参加活动的等级不足
    ERROR_ACTION_LEADER_ONLY,                       //活动通用 - 只有队长才能参加活动
    ERROR_ACTION_NEED_RESTART,                      //活动通用 - 请重新开启活动
    ERROR_ACTION_POINT_UNENOUGH,                    //活跃度不足
    ERRIO_ACTION_ALREADY_RECEIVE,                   //不能重复领取

	ERROR_WAIZHUAN_NOT_UNLOCK,						//外传模式： 外传未解锁

    ERROR_GENIUS_NOT_ACTIVE = 600,              // 天赋尚未激活
    ERROR_GENIUS_NOT_LEARN  = 601,              // 天赋尚未解锁(激活)
    ERROR_GENIUS_LEVEL_EXCCED  = 602,           // 天赋已达到最大等级

    ERROR_TASK_INVALID  =   700,                // 无效的任务
    ERROR_TASK_INPROGRESS = 701,                // 任务进行中

    ERROR_MISSION_NOT_COMPLETE = 720,           // 每日任务未完成


	ERROR_MAIL = 801,
	ERROR_MAIL_SUCCESS = 802,						//领取成功
	ERROR_MAIL_BAG_FULL = 803,						//背包已满
	ERROR_MAIL_GET_SOME = 804,						//部分物品领取成功
	ERROR_MAIL_NO_MAIL = 805,						//没有邮件可以领取
	ERROR_MAIL_SEND_FAIL,							//邮件发送失败
	ERROR_MAIL_ADD_RESOURCE_FAIL,					//保存邮件物品失败

	ERROR_FRIEND = 901,
	ERROR_FRIEND_SUCCESS = 902,						//恭喜你和{0}成为好友
	ERROR_FRIEND_OTHER_FULL = 903,					//对方好友已满，添加好友失败
	ERROR_FRIEND_FULL = 904,						//好友已满，添加好友失败
	ERROR_FRIEND_IS_FRIEND = 905,					//已经成为好友，添加好友失败
	ERROR_FRIEND_RECOMMEND_SUCCESS = 906,			//申请好友成功
	ERROR_FRIEND_RECOMMEND_FULL = 907,				//好友数量已满，无法添加
	ERROR_FRIEND_RECOMMEND_ISFRIEND = 908,			//已经为好友，无法添加
	ERROR_FRIEND_RECOMMEND_SELF = 909,				//不能添加自己为好友
	ERROR_FRIEND_RECOMMEND_REPEAT = 910,			//已经发送过邀请
	ERROR_FRIEND_REFUSED = 911,						//请问你要拒绝{0}的好友申请吗？
	ERROR_FRIEND_SEARCH_NULL = 912,					//查无此人
	ERROR_FRIEND_DELETE = 913,						//请问你要删除{0}好友吗？
	ERROR_FRIEND_PRAISE = 914,						//请问你要给{0}送体力值吗？
	ERROR_ACCPET_MAIL_NOEXIST = 915,				//接受的邮件不存在, 设计错误
	ERROR_ACCPET_OPERATE_ERROR = 916,				//执行存储操作失败, 设计错误
	ERROR_FRIEND_IS_FOLLOW,							//已经是关注对象,添加关注失败
	ERROR_FRIEND_ISNOT_FOLLOW,						//不是关注对象,取消关注失败
	ERROR_FRIEND_ISNOT_FRIEND,						//不是好友,删除好友失败

	ERROR_ALREADY_EXIST_GUILD = 950,				//已经存在行会中
	ERROR_NOT_IN_GUILD,						
	ERROR_IS_MASTER,						
	ERROR_SCENE_OBJECT_NOEXIST,
	ERROR_SCENE_OBJECT_CONFIG_NOEXIST,
	ERROR_NO_ENOUGH_GOLD,
	ERROR_NO_ENOUGH_FOOD,
	ERROR_NO_ENOUGH_STONE,
	ERROR_MEMBER_LIMITED,
	ERROR_PLAYER_NO_PRIORITY,						//玩家权限不够
	ERROR_NO_CONTRIBUTION_INDEX,					//配置表中不存在的贡献索引
	ERROR_GUILD_LEVEL_LIMITED,
	ERROR_GUILD_ITEM_TIME_LIMITED,
	ERROR_NO_ENOUGH_CONTRIBUTION,

    ERROR_PAY_NOT_FIRSTPAY = 1000,					//不是首充 传入首充参数错误
    ERROR_PAY_IS_FIRSTPAY,							//是首充 传入首充参数错误
    ERROR_PAY_NUM_NO_EXIST,							//套餐数量不存在
    ERROR_PAY_VIP_LEVEL_NO_EXIST,					//VIP等级不存在
    ERROR_PAY_VIP_REWARD_GOT,						//VIP礼包已经领取过了
    ERROR_PAY_VIP_REWARD_NO_EXIST,					//VIP等级对应的礼包不存在
    ERROR_PAY_VIP_REWARD_EMPTY,						//VIP礼包内容为空
    ERROR_VIP_LEVEL_TOO_LOW,                        //VIP 等级不足

    ERROR_INVITE_CODE = 1201,
    ERROR_INVITE_CODE_INPUT_SUCCESS = 1202,   		//推荐码输入成功，请前往成就界面领取奖励
    ERROR_INVITE_CODE_INPUT_SELF = 1203,		  		//不能输入自己的推荐码
    ERROR_INVITE_CODE_INPUT_ERROR = 1204, 	  		//推荐码输入错误
    ERROR_INVITE_CODE_INPUTED = 1205,			  	//您已经输入过玩家推荐码了

	ERROR_MOVE_SPEED_ERROR = 1300,					// 系统检测评估到您使用了移动加速，强行踢除处理，如有疑问联系客服
	ERROR_WORLD_BATTLE_ERROR = 1301,				// 创建跨服战玩家失败
	ERROR_WORLD_BATTLE_OFFLINE = 1302,				// 等待跨服战玩家已经下线
	ERROR_WORLD_BATTLE_GS_ERROR	= 1303,				// 未能正确验证GS WORLD 接入
	ERROR_WORLD_BATTLE_MATCH_ERROR = 1304,			// 请求匹配超时
	ERROR_WORLD_BATTLE_NOT_START = 1305,			// 跨服战还未开始
	ERROR_GUILD_BATTLE_USE_OUT = 1306,				// 攻击次数已经完成
	ERROR_GUILD_BATTLE_FRESH_TIME_ERROR = 1307,		// 刷新时间未到
	ERROR_GUILD_BATTLE_BEING_ATTACK = 1308,			// 被攻击中
	ERROR_GUILD_BATTLE_STATUS_ERROR = 1309,			// 战斗状态未找到
	ERROR_GUILD_BATTLE_STATUS_ATTACK_ERROR = 1310,	// 无攻击状态
	ERROR_GUILD_BATTLE_START = 1311,				// 公会战已经开始
	ERROR_GUILD_BATTLE_SIGNED = 1312,				// 已经报名
	ERROR_GUILD_BATTLE_MEMBER_LESS = 1313,			// 会员人数不足6人
	ERROR_GUILD_BATTLE_LESS_MONEY = 1314,			// 报名资金不够
	ERROR_GUILD_BATTLE_CREATE_FAILED = 1315,		// 创建公会战记录失败
	ERROR_GUILD_BATTLE_IN_BATTLE = 1316,			// 正在战斗中

	ERROR_CONFIG_NOT_EXIST = 5000,			//配置表格不存在
	ERROR_UNKOWN,
};

enum LOGIN_INFO
{	
	eError_NONE,
	eLogin_Succeed,
	eLogin_Version_Too_Old,
	eLogin_Version_NoSet,
	eLogin_SQL_function_Error,
    eLogin_CreateAccount_Succeed,
	eLogin_CreateDBData_Succeed,
	eLogin_CreateDBData_Fail,
	eLogin_NameExist,
	eLogin_NameIllegal,
	eLogin_Account_NoSet,
	eLogin_Account_NoExist,
	eLogin_Account_Repeat,
	eLogin_Account_Ban,
    eLogin_DBData_NoExist,
	eLogin_PassWord_Error,
	eLogin_Need_PassWord,
	eLogin_NoExist_GameServer,
	eLogin_World_NoResponse,
	eLogin_World_NoExist,
	eLogin_Resources_Too_Old,
	eLogin_Server_Logic_Error,
	eLogin_NoExist_ResourcesServer,

	eLogin_Login_Fail,
	eLogin_NoResponse_LoginData,
	eLogin_ConnectLoginServer_Fail,
	eLogin_ConnectLoginServer_OverTime,
	eLogin_ConnectGameServer_Fail,
	eLogin_ChangePsw,
	eLogin_ChangePsw_Success,
	eLogin_ChangePsw_Fail,
	eLogin_Other_Login,
	eLogin_Need_Restart_App,
	eLogin_Update_Resources_Succeed,
	eLogin_NETWORK_DISCONNECT,
	eLogin_NETWORK_DISCONNECT_TITLE,

	eLogin_Role_AlreadyExists,
	eLogin_AlreadyLogin,
	eLogin_ServerMemoryUNENOUGH,		// 服务器内存不够

	eLogic_NoSetSID				= 100,
	eLogic_NoLogicUC,

	eLogin_PlayerFull,					// 服务器人数已满
	eLogin_RequestDB_OverTime,			// 请求DB数据超时
	eLogin_CheckEnterCodeFail,			// 检验登陆验证KEY失败
	eLogin_ServerMaintain,				// 服务器维护
	eSDKUID_CHECK_FAIL,
	eSDK_REQUST_DNS_FAIL,							//请求解析DNS失败
};

enum eBOSS_STATE
{
	eBossActive,
	eBossDead,
	eBossTimeOver,
	eBossNoExist,
};

enum ActionState
{
	eMoveStop,
	eMoveLeft,
	eMoveRight,
	eJump,
};

enum eSkillElementType
{
	eSKILLEL_None = 0,
	eSKILLEL_Icy,
    eSKILLEL_Fire,
	eSKILLEL_Light,
	eSKILLEL_Dark,
};

enum CHAT_CHANNEL
{
    CHAT_SYSTEM = 0,
    CHAT_NEARBY = 1,
    CHAT_WORLD = 2,
    CHAT_GUILD = 3,
    CHAT_TEAM = 4,
    CHAT_ENLIST = 5,
    CHAT_WHISPER = 6,
};

enum MSG_TYPE
{
	MSG_KILL_MONSTER				= 1,		// 杀怪
	MSG_LEVEL_UP					= 2,		// 升级
	MSG_ENTER_STAGE					= 3,		// 进入关卡
	MSG_STAGE_COMPLETE				= 4,		// 通关
	MSG_SKILL_LEVEL_UP				= 5,		// 角色技能升级
	MSG_GAIN_EQUIP					= 6,		// 获取装备
	MSG_GAIN_ITEM					= 7,		// 获取物品
	MSG_GAIN_PET					= 8,		// 获得宠物
    MSG_DIALOGUE					= 9,		// 对话NPC
	MSG_TEAM_CHANGE					= 10,		// 队伍信息变更
	MSG_ARENA						= 11,		// 参加竞技场
	MSG_ESCORT						= 12,		// 参加运镖
	MSG_TREASURE					= 13,		// 参加藏宝图
	MSG_WANTED						= 14,		// 参加通缉
	MSG_ADD_FRIEND					= 15,		// 添加好友
	MSG_FEED_PET					= 16,		// 宠物喂食
	MSG_PET_SKILL_LEVEL_UP			= 17,		// 宠物技能升级
	MSG_USE_EQUIP					= 18,		// 使用装备
    MSG_EQUIP_INTENSIFY				= 19,		// 装备强化(强化已经升级)
    MSG_EQUIP_FORGE					= 20,		// 装备锻造
	MSG_EQUIP_COMPOSITION			= 21,		// 装备合成
    MSG_BUY_ITEM					= 22,		// 购买物品
    MSG_TITLE_CHANGE				= 23,       // 称号变更
    MSG_GENIUS_LEARNED				= 24,		// 天赋解锁
    MSG_SUBOCCUPATION_TRANSFER		= 25,	    // 转职
	MSG_PET_CHANGE					= 26,		// 宠物更换
    MSG_CAPTIVE_SAVED				= 27,       // 解救人质
    MSG_TASK_COMPLETE				= 28,       // 任务完成
    MSG_EQUIP_GEM_INSET				= 29,		// 镶嵌宝石
    MSG_OBJECT_APPLY_ABSTATE		= 30,		// 对象中了异常状态Buff
	MSG_WABAO						= 31,		// 挖宝
    MSG_SAODANG						= 32,		// 扫荡
    MSG_TEAM_BOSS					= 33,		// 组队BOSS
    MSG_MONO_BOSS					= 34,		// 单人BOSS
	MSG_TREASURE_COMPLETE			= 35,		// 完成藏宝图
    MSG_PVP_RANGK_CHANGE			= 36,		// PVP名次变更
	MSG_TEAM_WABAO					= 37,		// 组队挖宝
	MSG_TICKET_STAGE				= 38,		// 王的遗迹
    MSG_BIOGRAPHY_STAGE             = 39,       // 外传

    MSG_GOLD_MINE                   = 40,       // 挖金矿（金手指）
    MSG_GEM_UPGRADE                 = 41,       // 宝石升级

    MSG_CHOPIN_MUSIC                = 42,       // 肖邦音乐副本
    MSG_GREADY_GAMBLING             = 43,       // 贪婪赌场
    MSG_WORLD_BOSS_BATTLE           = 44,       // 参加一次世界BOSS 战斗
    MSG_CITY_WAR                    = 45,       // 参加了一次怪物攻城战
    MSG_ATTEND_OFFLINE_PVP           = 46,       // 参加了一次竞技场竞技
    MSG_ATTEND_ONLINE_PVP           = 47,       // 参加了一次斗兽场竞技

    MSG_OPEN_CHEST                  = 50,       // 打开宝箱

    MSG_SHOPING                     = 60,       // 商店消费
    MSG_EQUIP_INTENSIFY_OP          = 61,       // 强化装备操作(不一定强化升级)
    MSG_LOTTERY_DRAW                = 62,       // 抽奖
    MSG_CASH_CHAGE                  = 63,       // 现金充值

    MSG_ATTEND_GUILD_STAGE          = 70,       // 参加了一次公会副本
    MSG_GUILD_DONATE                = 71,       // 公会捐献
    MSG_GUILD_FRAGMENT_DONAE        = 72,       // 公会碎片捐赠
    MSG_GUILD_CHAT                  = 73,       // 公会聊天频道发言一次
    MSG_GUILD_TREASURE              = 74,       // 公会寻宝一次
    MSG_GUILD_MEMBER_LEVEL_EXP      = 75,       // 公会成员等级成长点增加

    MSG_COMUNICATION_GROUP          = 100,      // 副本内消息交流组消息
    MSG_ACTIVITY_POINT              = 101,      // 需要增加活跃度时
    MSG_DAILY_MISSIONS_STATUSE      = 102,      // 每日任务集合状态更改时

    MSG_TYPE_MAX
};

#define _STR(s)             #s
#define STR(s)              _STR(s)
#define _MSGNAME(MsgType)    MSG_##MsgType

//#define MSGNAME(MsgType)    STR(_MSGNAME(MsgType))
#define MSGNAME(MsgType)    STR(MsgType)

// 伤害修正类型
enum eAlterAffectType
{
    eBeforeHit,
    eInAttackValue,         // 计算攻击数值时
    eAfterAttackValue,      // 在攻击数值算出之后
    eAfterAttackAdded,      // 附加伤害之后
    eAfterAnti,             // 抵抗之后
    eAfterShield,           // 护盾之后
    eAfterRebound           // 反弹之后
};

// 打击类型
enum eAttackHint
{
    eNormal = -1,
    eFloat = 1,     // 浮空打击
    eBlock,         // 格挡打击
};

// 任务类型
enum eTaskType
{
    eTaskMain = 1,
    eTaskBranch,
    eTaskDaily,
    eTaskActivity,
    eTaskLoop,
    eTaskGenius,
    eTaskZhuanZhi
};

enum ACTIVITY_TYPE
{
    TREASURE_MAP = 1,       // 海贼王
    YABIAO,                 // 押镖
    TEAM_BOSS_STAGE,        // 组队BOSS
    MONO_BOSS_STAGE,        // 担任BOSS
    CITY_WAR,               // 怪物攻城
    WABAO,                  // 挖宝

    DRAW_PHY_POWER,         // 领取体力
    WABAO_TEAM,             // 组队挖宝
    WORLD_BOSS,				// 世界BOSS
    TICKET_STAGE,           // 王的遗迹

    CHOPIN_MUSIC = 12,      // 肖邦的音乐(经验副本)
    GREEDY_GAMBLING,        // 贪婪赌场（金币副本）
	WAIZHUAN,				// 外传模式活动

    GOLD_MINE,              // 原野金矿
    GEM_UPGRADE,            // 璀璨宝石
	ONLINE_BOT,				// 在线挂机

    PROVING_TOWER,          // 试炼之塔

	CD_KEY	= 101,

    DUMMY_ACTIVITY1,        // 运营活动
};


enum PERIOD_QUOTA
{
    GUILD_STAGE_QUOTA = 1,        //公会副本剩余次数
    FRAGMENT_ASK_QUOTA = 2,       //碎片捐赠索要次数 
    GUILD_TREASUER_QUOTA = 3,     //公会寻宝次数
};

struct GeneralAccessProperty
{
    bool    mHaveData;
    KEY     mPlayerID;
    int     mPlayerLevel;
    int     mPlayerVIPLevel;
    int     mTeamId;
    int     mGuildId;
    int     mGuildLevel;

public:
    GeneralAccessProperty()
        : mHaveData(false)
        , mPlayerID(0)
        , mPlayerLevel(0)
        , mPlayerVIPLevel(0)
        , mTeamId(0)
        , mGuildId(0)
        , mGuildLevel(0)
    {}
};

enum eDailyMissionsTag
{
    eDMTag_None,
    eDMTag_Common = 1,
    eDMTag_Guild = 2,
};
//-------------------------------------------------------------------------*/
typedef Array<int>	BattleLevelConfig;
// 根据总星数, 查表得到战斗等级及等级星数
int ToWorldBattleLevel(const BattleLevelConfig &config, int starCount, int &levelStar);
//-------------------------------------------------------------------------*/
enum eGuildBattleArrangementTowerIndex
{
    eMainTowerIndex = 0,
    eInsideTowerIndex = 1,
    eMiddleTowerUpIndex = 2,
    eMiddleTowerDownIndex = 3,
    eOutsideTowerUpIndex = 4,
    eOutsideTowerDownIndex = 5,
	eBoxTower = 6,
	eGuildBattleTowerCount  = 7,
};

enum eGuildBattleWideTowerIndex
{
    eCenterTowerIndex = 0,
    eUpTowerIndex = 1,
    eBottomTowerIndex = 2,
    eLeftUpTowerIndex = 3,
    eLeftBottomTowerIndex = 4,
    eRightUpTowerIndex = 5,
    eRightBottomTowerIndex = 6,
	eGuildBattleWideTowerCount,
};

enum TowerState
{
	eGuildTowerDefault,
	eGuildTowerAttacking,
	eGuildTowerDied,
	eGuildTowerNoExist,
};
enum GUILD_BATTLE_RESULT
{
	eGuildBattleTied,
	eGuildBattleWin,
	eGuildBattleFail,
};

enum GUILD_TOWER_STATE
{
	eTowerNormal = 0,
	eTowerAttacking = 1,
	eTowerDied = 2, 
};
//-------------------------------------------------------------------------*/

#endif