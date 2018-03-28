#ifndef __GAMESTRUCT_H__
#define __GAMESTRUCT_H__

#include "BaseCommon.h"
#include <cmath>
#include <math.h>
#include "EasyMap.h"
#include "Array.h"

using namespace std;
//-------------------------------------------------------------------------
// �趨����Э��,����1 ʹ��TCPЭ��; ����0 ʹ��UDPЭ��
#define Net_Connect_Server		(10)		// �첽���ӷ�����, ����ID ����Ϊ��ʱͬ����������, NOTE:���ܺ����е�����ID��ͬ
#define	USE_THREAD_FINDPATH		1			// ��̨�߳�Ѱ·
#define NET_HEART_CHECK			1			// �������������
#define USE_DYNAMIC_IMAGE_LAOD  0           //�Ƿ�̬����ͼƬ
// ������ȴ�ʱ��, ������ʱ���, ��������ߴ���, (10����)
#define PLAYER_NET_CHECK_OVERTIME	(600)
// �ͻ���ÿ��30�뷢��һ��������������Ϣ
#define NET_CHECK_ONCE_TIME			(10)
#define	WAIT_RESUME_OVER_TIME		(480)		// �ͻ�����С������8����, �������ȴ�10����, ������ÿ30��һ��
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

// ��󸱱�ƥ���鿴ʱ��
#define WORLD_HOME_WAIT_LOOK		(20)
#define WORLD_HOME_BATTLE_TIME		(3*60)
#define WORLD_HOME_LOCK_TIME			(WORLD_HOME_WAIT_LOOK+WORLD_HOME_BATTLE_TIME+10)

// ս����Ѫ����ID
#define BATTLE_USE_ITEM_ID		(43)

// ����������
#define MAX_FRIEND_NUM      50

// �������ֵ
#define MAX_POINT           10

#define MAX_DAILY_POINT     50

// ÿ�ָ�1������ֵ����Ҫ��ʱ��
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
		BIT_SIZE = nSize, //λ��ǵĳ��ȣ���λ�Ƕ�����λ
		BYTE_SIZE = 1+BIT_SIZE/8, //��Ϣ��ռ�õ��ֽ���
	};
	BitFlagSet_T(VOID)
	{
		memset((VOID*)m_aBitFlags, '\0', sizeof(m_aBitFlags));
	};
	//���ƹ�����
	BitFlagSet_T(BitFlagSet_T const& rhs)
	{
		memcpy((VOID*)m_aBitFlags, (VOID*)(rhs.GetFlags()), sizeof(m_aBitFlags));
	};
	~BitFlagSet_T() {};
	//���Ʋ�����
	BitFlagSet_T& operator=(BitFlagSet_T const& rhs)
	{
		memcpy((VOID*)m_aBitFlags, (VOID*)(rhs.GetFlags()), sizeof(m_aBitFlags));
		return *this;
	};
	//�������б��λ
	VOID MarkAllFlags(VOID)
	{
		memset((VOID*)m_aBitFlags, 0xFF, sizeof(m_aBitFlags));
	};
	//������б��λ
	VOID ClearAllFlags(VOID)
	{
		memset((VOID*)m_aBitFlags, 0x00, sizeof(m_aBitFlags));
	};
	//ȡָ���ı��λ
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
	//���ָ���ı��λ
	void ClearFlagByIndex(INT const nIdx)
	{
		if(0>nIdx||BIT_SIZE<=nIdx)
		{
			AssertEx(FALSE,"[BitFlagSet_T::ClearFlagByIndex]: Index out of range!");
			return;
		}
		m_aBitFlags[nIdx>>3] &= ~(0x01<<(nIdx%8));
	}
	//�趨ָ���ı��λ
	VOID MarkFlagByIndex(INT const nIdx)
	{
		if(0>nIdx||BIT_SIZE<=nIdx)
		{
			AssertEx(FALSE,"[BitFlagSet_T::MarkFlagByIndex]: Index out of range!");
			return;
		}
		m_aBitFlags[nIdx>>3] |=	0x01<<(nIdx%8);
	}
	//��ռ�õ��ֽ���
	UINT GetByteSize(VOID) const {return BYTE_SIZE;}
	//��֧�ֵı����
	UINT GetBitSize(VOID) const {return BIT_SIZE;}
	//ȡ��������ָ��
	CHAR const* const GetFlags(VOID) const {return m_aBitFlags;}
protected:
private:
	CHAR m_aBitFlags[BYTE_SIZE]; //���ݴ洢��
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

	// ���һ���㴹ֱ�ཻ��˷�Χ���ϵ�λ��
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
    bool    mIsHeadKill;        // նɱ

    int     mBlocked;           // ��
    int     mCompensate;        // ���ܵ���

    int     mAdditionDamages[ABS_MAX_COUNT];     // �����˺�ֵ(���������ա��е�...��
    float   mAdditionDamagesF[ABS_MAX_COUNT];     // �����˺�ֵ(���������ա��е�...��

    int     mSelfHpAlter;        // ����(��Ѫ)
    int     mSelfDamage;         // �˺�����

    int     mAttackerId;

    int     mHitNum;            //�ͻ�����ֵ���ͻ����Լ�����

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
	UInt64 GetLeaveTime(UInt64 uNow)//ʣ��ʱ��;
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
	ATTR_ATT = ATTR_BASE_BEGIN,						// ����
	ATTR_DEF,										// ����
	ATTR_MAX_HP,									// �������
	ATTR_CRIT,										// ������
	ATTR_HIT,										// ������
	ATTR_MISS,										// ������
	ATTR_IGNORE_DEF_ATT,							// ���ӷ�������
	ATTR_CRIT_PERCENT,								// �����˺��ٷֱ�
	ATTR_PENETRATE,									// ������͸
	ATTR_ATT_DISTANCE,								// ��������
	ATTR_MOVE_SPEED,								// �ƶ��ٶ�
	ATTR_ATT_SPEED,									// �����ٶ�
	ATTR_IMMUNITY,									// �˺�����
	ATTR_IMMUNITY_PERCENT,							// �˺�����ٷֱ�
	ATTR_HURT,										// �˺���ǿ
	ATTR_HURT_PERCENT,								// �˺���ǿ�ٷֱ�
	ATTR_JUMP,										// ��Ծ��
	ATTR_CRIT_IMMUNITY,								// �����ֿ�
	ATTR_HARD,										// Ӳֱ
	ATTR_ATT_FIRE,									// �����Թ���
	ATTR_ATT_COLD,									// �����Թ���
	ATTR_ATT_LIGHT,									// �����Թ���
	ATTR_ATT_DARK,									// �����Թ���
	ATTR_FIRE_IMMUNITY,								// �����Կ���
	ATTR_COLD_IMMUNITY,								// �����Կ���
	ATTR_LIGHT_IMMUNITY,							// �����Կ���
	ATTR_DARK_IMMUNITY,								// �����Կ���
    ATTR_CONTROL_IMMUNITY,                          // �Կ���Ч����������ѣ�Ρ����ٵȣ��Ƿ�����
	ATTR_BASE_END,

    // ��������
	ATTR_EXTRA_BEGIN = ATTR_BASE_END + 9,			// Ԥ��һЩ�ռ��Ա��Ժ����ӻ�����������
	ATTR_BUFF_ATTR	= ATTR_EXTRA_BEGIN,			    // Buffer������;�������ܴ�����Buffer������
	ATTR_ALL_ELEMENT_ATTACK,						// ������,��,��,�����Լ��ܵĹ���
    ATTR_ALL_ELEMENT_IMMUNITY,					    // �����ܵ���,��,��,�����Լ��ܹ���ʱ���˺�

	ATTR_GENERATE_HP,								// �����������ظ�// ����ɾ������
	ATTR_CLEAR_ALL_BUFF,							// �������״̬

    ATTR_DROP_GOLD_MULTIPLE,                        // �����ҷ�������
    ATTR_HUNT_EXP_MULTIPLE,                         // ���Ծ��鷭������
    ATTR_STAGE_EXP_MULTIPLE,                        // ͨ�ؾ��鷭������
    
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

// AffectSlot: ��������������͵�Ӱ���λ����λ����װ�����ԡ��������ԡ��츳���Եȵ�����
// attrType: CHAR_ATTR_INDEX
#define AffectSlot(attrType, isByRate) ((isByRate) ? ((attrType) * 2) : ((attrType) * 2 - 1))

// IsRateSlot: �ǰٷֱȲ�λ���ǹ̶�ֵ��λ
#define IsRateSlot(attrSlot) ((attrSlot) % 2 == 0)

// AttrTypeFromSlot: �����λ��Ӱ�����������
#define AttrTypeFromSlot(attrSlot)  ((attrSlot + 1) / 2)

// RateSlotFromAttrType: ����ָ������ֵ�İٷֱȲ�λ
#define RateSlotFromAttrType(attrType)  ((attrType) * 2)

// FixedSlotFromAttrType: ����ָ������ֵ�Ĺ̶�ֵ��λ
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
    EQUIP_WEAPON,   //����
    EQUIP_CAP,      //ͷ��
    EQUIP_HANDS,    //����
    EQUIP_ARMOR,    //�·�
    EQUIP_PANTS,    //����
    EQUIP_FOOT,     //Ь��

    EQUIP_NECKLACE,  //����
    EQUIP_RING,     //��ָ
    EQUIP_BRACELET, //����
    EQUIP_DANGLER,  //����
    EQUIP_HEADDRESS, //ͷ��
    EQUIP_TATTOO,    //����

    FASHION_ARMS,   //ʱװ����
    FASHION_HEAD,   //ʱװͷ��
    FASHION_FACE,   //ʱװ����
    FASHION_COAT,   //ʱװ����
    FASHION_TROUSERS,   //ʱװ��װ
    FASHION_EFFECT,   //ʱװ��Ч
};

enum DECORATION_PART
{
    DECORATION_ARMS = 1,    //�ɼ���λ-����
    DECORATION_HEAD,        //�ɼ���λ-ͷ��
    DECORATION_FACE,        //�ɼ���λ-����
    DECORATION_COAT,        //�ɼ���λ-����
    DECORATION_TROUSERS,    //�ɼ���λ-��װ
    DECORATION_EFFECT,      //�ɼ���λ-��Ч

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
	NOTICE_ARENA_RESULT = 20,       // ���޳�ս���������
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
	eArenaDefenseScene	 = 117,			// ���޳���, ��������ʵ��, �Զ��޳����������Ӧ

    eProvingStageScene      = 118,      // ����֮������
	eWorldPVPScene			= 119,		// ������սPVP
	eGuildPVPScene				= 120,		// ����սPVP
	eGuildBattleMonsterScene = 121,	// ����ս���︱��
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
    ePROTECTEDNPC = 9,		//���عؿ��б�������NPC

	ePROP = 10,
	eCHEST = 11,
	eCAPTIVE = 12,
	eSTAGEEVENT = 13,
	eBUILDMONSTER = 14,
	eFRUIT = 15,
	eSTAGEAREA = 16,		// �ؿ��������
	eTRIGGER = 17,			// ��������
	eBLOCK = 18,
	eAREAFLAG		= 20,	// �������ڵ�·��������־
	eBIGBOSS		= 21,	// ��ϴ�BOSS
	ePARTMONSTER	= 22,	// ��� BOSS �Ĳ�������
	eWorldBoss		= 23,	// ����BOSS

	eResourceDrop = 28,     //��Դ�������� װ��������
	eActiveProp = 29,        //����ߣ���ͷ�ȣ�
	eItemDrop = 30,         //���ߵ���
	eEquipDrop = 31,        //װ������
	ePVPPlayer = 32,

    eYaBiaoChe = 33,
    eAreaCheck = 34,        // ���򿪷����ƶ���
    eSettlementCheck = 35,  // �������ƶ���
    eGuideFlag = 36,        // ������־
	eStoryLineMonitor = 37, // ���鸨������
	eGoldMonster = 39,              // ���ս���еĹ���
	eGoldTower = 40,                 // ���ս���е���
	eGoldCoffers = 41,					// ���
	eOBJECT_MAX,
};


#define CHECK_TYPE(type)	(type>eNONE && type<eOBJECT_MAX)
//-------------------------------------------------------------------------

enum RESOURCE_TYPE
{
	RESOURCE_NONE,
    RESOURCE_DIAMOND,			// ��ʯ
    RESOURCE_GOLD,				// ���
    RESOURCE_POWER,				// ����
	RESOURCE_FRIEND_POINT,		// ���ѵ�
    RESOURCE_ITEM,				// ��Ʒ��������װ����
	RESOURCE_EQUIP,				// װ��
	RESOURCE_GEM_FORGE_POINT,	// ��ʯ����ֵ
    RESOURCE_SKILL_POINT,		// ���ܵ�
    RESOURCE_PRESTIGE_POINT,    // ����
	RESOURCE_EXP,				// ����ֵ
    RESOURCE_FLOWER,			// �ʻ�������ϵͳʹ�ã�
	RESOURCE_PET,				// ����
	RESOURCE_HORNOR,			// �����㣨������ʹ�ã�
	RESOURCE_BUFFITEM,			// BUFF
	RESOURCE_VIPEXP,			// vip����
    RESOURCE_GUILD_FUNDS,       // �����ʽ�
    RESOURCE_GUILD_EXP,         // ���ᾭ��
    RESOURCE_GUILD_DONATE_PONT, // ������˹���
	RESOURCE_CASH,				// ��ֵ
};

enum RESOURCE_CHANGE_REASON
{
	REASON_NONE,
	REASON_STAGE_DROP,			// �ؿ�����
	REASON_SHOP,				// �̳�
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
	ePAY_AMOUNT_ERROR = 25,  // ֪֧ͨ������쳣
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

// ��ȡģʽ
enum GROUP_TYPE
{
    GROUP_TYPE_ALL,
    GROUP_TYPE_LOW,
    GROUP_TYPE_HIGH,
};

enum CASH_TYPE
{
	CASH_BEGIN = 1,
	CASH_WEEKLY_CARD = CASH_BEGIN,	// �ܿ�
	CASH_MONTHLY_CARD,				// �¿�
	CASH_INFINITE_CARD,				// ����ʱ��
	CASH_FIRST,						// �׳佱��
	CASH_ONCE,						// ���ʳ�ֵ
	CASH_TOTAL,						// �ۼƳ�ֵ
	CASH_LEVEL_UP,					// �ɳ�����
	CASH_LOGIN,						// ������¼
	CASH_SHOP,						// ��ʱ���
	CASH_COST,						// �ۼ�����
	CASH_DOUBLE,					// ��ֵ˫��
	CASH_VIP,						// VIP����
	CASH_7DAY,						// 7�ճ�ֵ����
    CASH_TIME_LIMIT_MISSIONS,       // ��ʱ��������
    CASH_TIME_LIMIT_EXCHANGE,       // ��ʱ�һ�
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
    ERROR_INVALID_EQUIP = 3,						// ��Ч��װ��
	ERROR_LOW_LEVEL = 4,							// �ȼ�����
    ERROR_INVALID_OCCUPATION = 5,					// ְҵ����ȷ
    ERROR_INVALID_GENDER = 6,						// �Ա𲻷�
    ERROR_MAX_FORGE_LEVEL = 7,						// �Ѵﵽ������ȼ�
    ERROR_NO_ENOUGH_MONEY = 8,						// û���㹻�Ľ�Ǯ
    ERROR_NO_ENOUGH_GEM = 9,						// û���㹻�ı�ʯ
    ERROR_NO_ENOUGH_DIAMOND = 10,					// û���㹻����ʯ
    ERROR_NO_ENOUGH_ITEM = 11,						// û���㹻����Ʒ
	ERROR_NO_ENOUGH_RESOURCE = 12,					// ��Դ����
	ERROR_ROLE_NOTEXIST = 13,						// ��ɫ������
	ERROR_INVALID_PET = 14,							// ��Ч�ĳ���
    ERROR_CONDITION_NOTMATCH = 15,                  // ����������
	ERROR_BAG_FULL = 16,							// ��������
	ERROR_PLAYER_OFFLINE = 17,						// ��Ҳ�����
	ERROR_ACCOUNT_ERROR = 18,						// �ʺŻ��������
	ERROR_ACCOUNT_BAN = 19,							// �ʺű���ͣ
	ERROR_CHAT_BAN = 20,							// ���ѱ�����
	ERROR_NAME_EXIST = 21,							// ��ɫ���ظ�
    ERROR_INVALID_PET_SKILL = 22,                   // ��Ч�ĳ��＼��
    ERROR_MAX_PET_SKILL = 23,                       // ���＼���Ѿ��ﵽ���ȼ�
    ERROR_MAX_POWER_BUY_COUNT = 24,                 // ���������Ѿ��ﵽ������
	ERROR_INVALID_CDKEY = 25,						// ��Ч��CDKEY
	ERROR_CDKEY_ALREADY_USED = 26,					// ��CDKEY�Ѿ���ʹ��
	ERROR_CDKEY_SAME_TYPE = 27,						// �Ѿ�ʹ�ù�ͬ�����͵�CDKEY
	ERROR_CDKEY_TIME_OUT = 28,						// ��CDKEY�ѹ�ʱ
    ERROR_INVALID_ITEM = 29,                        // ��Ч�ĵ���
    ERROR_INVALID_FRAGMENT = 30,                    // ��Ч����Ƭ
    ERROR_NO_ENOUGH_FRAGMENT = 31,                  // û���㹻����Ƭ
	ERROR_CDKEY_MAX_NUM = 32,						// ��CDKEY�ѱ�����
    ERROR_INVALID_QUOTA_ID = 33,                    // ��Ч���ID
    ERROR_QUOTA_EXTRA_FULL_OR_REJECT = 34,          // ����������
	ERROR_NO_PERMISSION = 35,						// ��û��Ȩ��
    //�����̳ǹ������
    ERROR_SHOP_WRONG_ITEMLIST = 40,				    // ������ߵ��б������Ч
    ERROR_SHOP_OVER_ITEMLIST = 41,				    // һ�ι�����߳���50��
    ERROR_SHOP_NOTENOUGH_GOLD = 42,				    // ��Ҳ���,����ʧ��
    ERROR_SHOP_NOTENOUGH_GEM = 43,				    // ��ʯ����,����ʧ��

    ERROR_NOEXIST_BUY_ITEM_LIST,
    ERROR_NOEXIST_ITEM_CONFIG,
    ERROR_NOEXIST_ITEMPACKET_CONFIG,
    ERROR_NOEXIST_SHOP_CONFIG,

    ERROR_NOT_IS_ITEM_BOX,						    // ������Ч�ĺ��� ����������
    ERROR_ITEM_ID_NOEXIST,
    ERROR_INVALID_INTENSIFY_STONE,                  // ��Ч��ǿ��ʯ

    ERROR_PET_MAX_LEVEL,                            // �����Ѵﵽ��ǰ�׶����ȼ�
    ERROR_PET_PLAYER_LEVEL_LOW,                     // ����ȼ��Ѵﵽ��ҵ�ǰ�ȼ�������

	ERROR_ALREADY_PAY,								// �Ѵ�����Ķ���

	ERROR_EQUIP = 100,
	ERROR_EQUIP_REFINE_FAIL = 101,					// ����ʧ��
	ERROR_EQUIP_NEED_SELECT_EQUIP = 102,			// ����ѡ������ǿ����װ��
	ERROR_EQUIP_MAX_COUNT = 103,					// ѡ��������Ѵﵽ���
	ERROR_EQUIP_NOT_INSET_SMAE_GEM_ID = 104,		// ������Ƕ��ͬID�ı�ʯ
	ERROR_EQUIP_NO_ENOUGH_ITEM_PUNCH = 105,			// �������ĵ��߲���
	ERROR_EQUIP_NEED_SELECT_GEM = 106,				// ����ѡ��������Ƕ�ı�ʯ
	ERROR_EQUIP_NEED_SELECT_GEM_FOR_DECOMPOSE = 107,            //����ѡ�������ֽ�ı�ʯ
	ERROR_EQUIP_NO_ENOUGH_FORGE_POINT = 108,                    //����ֵ����
	ERROR_INVALID_EQUIP_ID = 109,					// ��Ч��װ��Id
    ERROR_EQUIP_GEM_MAX_LEVEL = 110,                // ��ʯ�Ѵﵽ���ȼ�
    ERROR_EQUIP_NOTSALABLE = 111,                   // װ�����ɳ���
    ERROR_EQUIP_NOTDECOMPOSABLE = 112,              // װ�����ɷֽ�
    ERROR_EQUIP_NOTEXCHANGABLE = 113,               // װ�����ɶһ�
    ERROR_EQUIP_CANNOT_USE_INTSTONE = 114,          // ���β�����ǿ��ʯǿ��
    ERROR_EQUIP_CANNOT_USE_INTEQUIP = 115,          // ���׼���װ����������ǿ��
    ERROR_EQUIP_CANNOT_USE_ITEM_ATTM = 116,         // ���׼�����Ƭ���������ڸ�ħ����
    ERROR_EQUIP_CANNOT_USE_EQUIP_ATTM = 117,        // ���׼���װ�����������ڸ�ħ����

    ERROR_TITLE_NO_QUOTA    = 120,                  // �ƺ�������Ѿ����
    ERROR_FUWEN_FUSHION_FAILED = 130,               // ���ĺϳ�ʧ��

    ERROR_EQUIP_ATTM_ALREADY_EXISTS = 140,          // װ���Ѿ���ħ
    ERROR_EQUIP_ATTM_NOT_ALLOWED,                   // ��װ�������Ը�ħ
    ERROR_EQUIP_NO_ATTM,                            // ��װ��û�и�ħ
    ERROR_EQUIP_INVALID_ATTM_INDEX,                 // ��Ч�ĸ�ħID
    ERROR_EQUIP_ATTM_MAX,                           // ��ħ�Ѿ��ﵽ����Ǽ�

    ERROR_EQUIP_FUSHION_MUST_SAME = 150,            // ����ͬһ��װ���ſ����ںϾ���
    ERROR_EQUIP_FUSHION_MUST_NOT_IN_USE,            // �����ǲ���ʹ���е�װ���ſ��Ա��ں�
    ERROR_EQUIP_FUSHION_NOT_IN_FONFIG,              // ��װ���������ںϾ���

    ERROR_NOT_TEAM_STAGE = 202,                     // ���ʱ�����Խ��뵥�˸���
    ERROR_EXTRA_COUNT_NOMORE = 203,					// ͨ�ش����Ѿ������
	ERROR_INVALID_SCENE_CODE = 204,					// ����ID ��Ч
    ERROR_SCENE_ENTER_CONDITION_FAILE = 205,		// ���볡��������������
    ERROR_SCENE_LEADER_ONLY  = 206,                 // ��ͨ�����Ա�����Խ��볡��
    ERROR_SCENE_NOT_UNLOCK = 207,                   // ����δ����
    ERROR_PEPOLE_COUNT_TOO_SMALL = 208,             // ���븱����������
    ERROR_PEPOLE_COUNT_TOO_BIG = 209,               // ���븱������̫��
    ERROR_PEPOLE_LEVEL_TOO_SMALL = 210,             // ���븱���ĵȼ�����
    ERROR_ACCESS_COUNT_EXCESS = 211,                // ���븱����������
    ERROR_CLEAR_COUNT_EXCESS = 212,                 // ͨ�ظ�����������
    ERROR_STAGE_ALREADY_COMPLETED = 213,            // �����Ѿ�����
    ERROR_POWER_EXHAUSTED = 214,                    // ��������
    ERROR_TEAMATE_LEVEL_TOO_SMALL = 215,            // ���{0}�ȼ�����
    ERROR_TEAMATE_ACCESS_COUNT_EXCESS = 216,        // ���{0}���븱����������
    ERROR_TEAMATE_CLEAR_COUNT_EXCESS = 217,         // ���{0}ͨ�ظ�����������
    ERROR_TEAMATE_SCENE_NOT_UNLOCK = 218,           // ���{0}����δ����
    ERROR_TEAMATE_POWER_EXHAUSTED = 219,            // ���{0}��������

    ERROR_SAODANG_STAR_NOT_REACH = 220,             // ����ͨ���Ǽ�������ɨ��
    ERROR_SAODANG_RESOURCE_INSUFFICIEN = 221,       // ɨ������

	ERROR_WABAO_STAGE_MAX = 222,					// ÿ���ڱ�ͨ�ش����ﵽ���ޣ��޷�����
	ERROR_NO_TICKET = 223,							// û���볡ȯ���޷�����
    ERROR_TREASUREMAP_STAGE_MAX = 224,              // ÿ�պ�����ͨ�ش����ﵽ���ޣ��޷�����

    ERROR_WORDL_BOSS_COMPLETED = 225,               // ���� BOSS �Ѿ�����

    ERROR_FAILED_TO_JOIN_ACTIVITY = 230,            // �μӻʧ��

    ERROR_WAITING_TEAMMATE  = 240,                  // �ȴ���������׼��

    ERROR_YABIAO_NO_BIAOCHE = 250,                  // û��׼���ڳ�
    ERROR_YABIAO_COUNT_EXCESS = 251,                // ���XXXʣ�໤�ʹ�������1��

    ERROR_SAODANG_NOT_PRESENT = 252,                // ��������ɨ��

    ERROR_GUIDE_FAILED      = 256,                  // ս������ʧ��

	ERROR_TEAM_EXIST = 300,						// �Ѵ��ڶ���
	ERROR_IS_TEAM_MEMBER = 301,					// �Ѿ��ڶ�����
	ERROR_TEAM_FULL = 302,						// ��������
	ERROR_ALREADY_APPLY = 303,					// �ѷ��͹��������
	ERROR_TEAM_NOT_EXIST = 304,					// ���鲻����
	ERROR_TEAM_NOT_APPLY = 305,					// �����û����������
	ERROR_IS_NOT_LEADER = 306,					// �㲻�Ƕӳ�
    ERROR_TEAM_KICK_MEMBER = 307,               //���{0}���߳�����
    ERROR_TEAM_AGREE_APPLY = 308,               //���{0}ͬ����������
    ERROR_TEAM_ACCEPT_INVITE_REFUSE = 309,      //���{0}�ܾ��������
    ERROR_TEAM_APPLY_TEAM = 310,                //���{0}����������   
    ERROR_TEAM_INVITE_MEMBER = 311,             //���{0}������������
    ERROR_TEAM_ADD_MEMBER = 312,                //���{0}�������
    ERROR_TEAM_LEVEL_TEAM = 313,                //���{0}�뿪����
    ERROR_TEAM_MODIFY_TEAM =314,                //����������޸�
    ERROR_TEAM_REMOVE_MEMBER = 315,             //�Ƿ�ȷ�Ͻ�{0}�Ƴ����飿
    ERROR_TEAM_APPOINT_TEAM_LEADER = 316,       //���{0}������Ϊ�ӳ�
    ERROR_TEAMMATE_NOT_IN_SAME_SCENE  = 317,    // ��Ա����ͬһ����
	ERROR_TEAM_LESS_ACTIVE_TIME = 318,			// �������ڻ�У��޷�����
	ERROR_OTHER_TEAM_EXIST = 319,				// �Է����ж���

	ERROR_GUILD_NOT_EXIST = 350,				// ���᲻����
	ERROR_GUILD_EXIST = 351,					// �Ѿ��й���
	ERROR_INVALID_GUILD_NAME = 352,				// �������ַǷ�
	ERROR_GUILD_TIME_LIMIT = 353,				// �뿪���᲻��24Сʱ���޷����빫��
	ERROR_GUILD_FULL = 354,						// �����������޷�����
	ERROR_IS_GUILD_MEMBER = 355,				// �Ѿ��ǹ����Ա
	ERROR_IS_GUILD_APPLY = 356,					// �Ѿ���������빫��
	ERROR_GUILD_POSITION_APPLY = 357,			// ֻ�л᳤�͸��᳤��Ȩͬ���������
	ERROR_GUILD_NOT_APPLY = 358,				// �����û��������빫��
	ERROR_LEADER_CANNOT_LEAVE = 359,			// �����ﻹ��������Ա���᳤�޷��뿪����
	ERROR_NO_KICK_PERMISSION = 360,				// ��û��Ȩ���޳������
	ERROR_IS_NOT_GUILD_LEADER = 361,			// ֻ�жӳ�������Ȩ��
	ERROR_ELDER_NUM_MAX = 362,					// ����������������᳤
    ERROR_REQUIRE_GUILD_MEMBERSHIP = 363,       // ��Ҫ�����Ա���
    ERROR_NO_FRAGMENT_ASK_QUOTA = 364,          // ���������������
    ERROR_INVALID_DONTE_TARGET = 365,           // ���������Ѿ���Ч
    ERROR_INVALID_DONTE_COUNT = 366,            // ��Ч�ľ�������
    ERROR_REQUIRE_SAME_GUILD_MEMBERSHIP = 367,  // ��Ҫ��ͬһ������
	ERROR_IMPEACH_POSITION = 368,				// ֻ�и��᳤���ܷ�����
	ERROR_NOT_IMPEACH_TIME = 369,				// �᳤7�����ϲ����߲��ܵ���
	ERROR_ALREADY_IMPEACH = 370,				// ���Ѿ���������
    ERROR_REQUIRE_HIGH_GUILD_AUTHORITY = 371,   // ��Ҫ�߼������ԱȨ��
    ERROR_GUILD_DONATE_REPEATED = 372,          // �ظ��������
    ERROR_GUILD_SIGN_REPEATED = 373,            // �ظ�����ǩ��
    ERROR_GUILD_STAGE_AWARD_REPEATED = 374,     // �ظ���ȡ���ḱ�����Ƚ���
    ERROR_GUILD_LEVEL_TOO_LOW = 375,            // ����ȼ�����
    ERROR_GUILD_NO_ENOUGH_MONDEY = 376,         // �����ʽ���
	ERROR_GUILD_APPLY_LOW_LEVEL = 377,			// ���ȼ�����
    ERROR_GUILD_NO_ENOUGH_EXP = 378,            // ���ᾭ�鲻��

    ERROR_GUILD_STAGE_REQUIRE_MEMBERSHIP = 380, // ���ḱ��ֻ�������Ա����
    ERROR_GUILD_STAGE_REQUIRE_SAME_MEMBERSHIP,  // ���ḱ��ֻ������ͬ�����Ա����
    ERROR_GUILD_STAGE_PROGRESS_NOT_REACH,       // ���ḱ������δ���
    ERROR_GUILD_STAGE_ACCESS_NO_QUOTA,          // ���ḱ���Ѿ�û��ʣ�����

    ERROR_GUILD_FRAGMENT_NOT_DONATABLE,         // ����Ƭ�������������

    ERROR_GUILD_NOT_IN_TREAUSER_TEAM,           // ��Ҳ���Ѱ������
    ERROR_GUILD_ALREADY_IN_TREAUSER_TEAM,       // ����Ѿ���Ѱ������
    ERROR_GUILD_NO_ROBS_COUNT,                  // Ѱ�������ٴ�������
    ERROR_GUILD_NOT_TREASER_TEAM_ID,            // ��Ч��Ѱ����ID  
    ERROR_GUILD_NOT_TREASURE_SUONER,            // ��Ҳ�����Ѱ���Ŵ�����
    ERROR_GUILD_TREASURE_MUSTI_IN_SAME_GUILD,   // Ŀ��Ѱ���ű������Լ���ͬһ���� 
    ERROR_GUILD_CAN_NOT_ROB_IN_SAME_GUILD,      // ���ٵ�Ŀ��Ѱ���ű������Լ�����ͬһ���� 
    ERROR_GUILD_TREASURE_NOT_ENOUGH_LEFT_TIME,  // Ѱ����ʣ��ʱ�䲻������³�Ա
    ERROR_GUILD_TREASURE_ROB_FAILED,            // ���Ѱ����ʧ��
    ERROR_GUILD_TREASURE_NOT_COMPLETE,          // Ѱ����δ����
    ERROR_GUILD_TREASURE_SETTLED,               // Ѱ���Ѿ���ȡ
    ERROR_GUILD_TREASURE_ALREADY_PICKED,        // �Ѿ�ʰȡ����Ѱ����
    ERROR_GUILD_TREASURE_NO_PICKABLE_RES,       // ��Ѱ���ſ�ʰȡ��ԴΪ��
    ERROR_GUILD_TREASURE_NO_QUOTA,              // ���ղ�����Ѱ��

    ERROR_SKILL_NOT_UNLOCK = 400,               //����δ����
    ERROR_SKILL_PLEVEL_TOO_LOW = 401,           //���ѧϰ�ȼ�����
    ERROR_SKILL_EVOLU_MAX_LEVEL = 402,          //��ǿ�������ȼ�

    ERROR_GUILD_ONT_IN_BATTLE = 420,            // ����û�вμӹ���ս
    ERROR_GUILD_BATTLE_ARRANGEMEMT_OUT_OF_TIME, // ��ʱ����ڲ��ɵ�������
    ERROR_GUILD_BATTLE_DEFENCER_OUT_OF_RANGE,   // ��������������
    ERROR_GUILD_BATTLE_NOT_IN_DEFENCER,         // ��������û�иó�Ա
	ERROR_GUILD_BATTLE_TOWNER_INDEX_ERROR,
	ERROR_GUILD_BATTLE_STARTING,						// ����ս��ʼ��

    ERROR_ACTION_REWARD_ALREADY_GET = 500,			//�ͨ�� - �Ѿ���ȡ��������
    ERROR_ACTION_REWARD_NOEXSIT,					//�ͨ�� - ��ȡ����������Ч
    ERROR_ACTION_NO_ACTION_DATA,					//�ͨ�� - û�иû����(һ����û�г�ʼ��)
    ERROR_ACTION_ONTHEHOUR_WRONG_HOUR,				//���� - ��ȡ��Сʱ���ǵ�ǰСʱ
    ERROR_ACTION_ONTHEHOUR_OVERTIME,				//���� - ��ȡ��ʱ(���˵�ǰ�����30����)
    ERROR_ACTION_ONTHEHOUR_NODATA,					//���� - û�иû������
    ERROR_ACTION_7DAY_NOTTHISDAY,					//7�յ�¼  - ��ȡ����������
    ERROR_ACTION_STATE_DATA_NOEXIST,
    ERROR_ACTION_TIME_REWARD_CONFIG_NOEXIST,
    ERROR_ACTION_ALREADY_REWARD,
    ERROR_ACTION_REWARD_CONFIG_ERROR,
    ERROR_ACTION_ACCESS_COUNT_EXCESS,				//�ͨ�� - ��μӴ����Ѿ�����
    ERROR_ACTION_PARTICIPANTS_MINIMUM,              //�ͨ�� - ��μ���������
    ERROR_ACTION_ACTIVITY_NOT_START,                //�ͨ�� - �û�н�ȡ
    ERROR_ACTION_LVEL_TOO_SMALL,                    //�ͨ�� - �μӻ�ĵȼ�����
    ERROR_ACTION_LEADER_ONLY,                       //�ͨ�� - ֻ�жӳ����ܲμӻ
    ERROR_ACTION_NEED_RESTART,                      //�ͨ�� - �����¿����
    ERROR_ACTION_POINT_UNENOUGH,                    //��Ծ�Ȳ���
    ERRIO_ACTION_ALREADY_RECEIVE,                   //�����ظ���ȡ

	ERROR_WAIZHUAN_NOT_UNLOCK,						//�⴫ģʽ�� �⴫δ����

    ERROR_GENIUS_NOT_ACTIVE = 600,              // �츳��δ����
    ERROR_GENIUS_NOT_LEARN  = 601,              // �츳��δ����(����)
    ERROR_GENIUS_LEVEL_EXCCED  = 602,           // �츳�Ѵﵽ���ȼ�

    ERROR_TASK_INVALID  =   700,                // ��Ч������
    ERROR_TASK_INPROGRESS = 701,                // ���������

    ERROR_MISSION_NOT_COMPLETE = 720,           // ÿ������δ���


	ERROR_MAIL = 801,
	ERROR_MAIL_SUCCESS = 802,						//��ȡ�ɹ�
	ERROR_MAIL_BAG_FULL = 803,						//��������
	ERROR_MAIL_GET_SOME = 804,						//������Ʒ��ȡ�ɹ�
	ERROR_MAIL_NO_MAIL = 805,						//û���ʼ�������ȡ
	ERROR_MAIL_SEND_FAIL,							//�ʼ�����ʧ��
	ERROR_MAIL_ADD_RESOURCE_FAIL,					//�����ʼ���Ʒʧ��

	ERROR_FRIEND = 901,
	ERROR_FRIEND_SUCCESS = 902,						//��ϲ���{0}��Ϊ����
	ERROR_FRIEND_OTHER_FULL = 903,					//�Է�������������Ӻ���ʧ��
	ERROR_FRIEND_FULL = 904,						//������������Ӻ���ʧ��
	ERROR_FRIEND_IS_FRIEND = 905,					//�Ѿ���Ϊ���ѣ���Ӻ���ʧ��
	ERROR_FRIEND_RECOMMEND_SUCCESS = 906,			//������ѳɹ�
	ERROR_FRIEND_RECOMMEND_FULL = 907,				//���������������޷����
	ERROR_FRIEND_RECOMMEND_ISFRIEND = 908,			//�Ѿ�Ϊ���ѣ��޷����
	ERROR_FRIEND_RECOMMEND_SELF = 909,				//��������Լ�Ϊ����
	ERROR_FRIEND_RECOMMEND_REPEAT = 910,			//�Ѿ����͹�����
	ERROR_FRIEND_REFUSED = 911,						//������Ҫ�ܾ�{0}�ĺ���������
	ERROR_FRIEND_SEARCH_NULL = 912,					//���޴���
	ERROR_FRIEND_DELETE = 913,						//������Ҫɾ��{0}������
	ERROR_FRIEND_PRAISE = 914,						//������Ҫ��{0}������ֵ��
	ERROR_ACCPET_MAIL_NOEXIST = 915,				//���ܵ��ʼ�������, ��ƴ���
	ERROR_ACCPET_OPERATE_ERROR = 916,				//ִ�д洢����ʧ��, ��ƴ���
	ERROR_FRIEND_IS_FOLLOW,							//�Ѿ��ǹ�ע����,��ӹ�עʧ��
	ERROR_FRIEND_ISNOT_FOLLOW,						//���ǹ�ע����,ȡ����עʧ��
	ERROR_FRIEND_ISNOT_FRIEND,						//���Ǻ���,ɾ������ʧ��

	ERROR_ALREADY_EXIST_GUILD = 950,				//�Ѿ������л���
	ERROR_NOT_IN_GUILD,						
	ERROR_IS_MASTER,						
	ERROR_SCENE_OBJECT_NOEXIST,
	ERROR_SCENE_OBJECT_CONFIG_NOEXIST,
	ERROR_NO_ENOUGH_GOLD,
	ERROR_NO_ENOUGH_FOOD,
	ERROR_NO_ENOUGH_STONE,
	ERROR_MEMBER_LIMITED,
	ERROR_PLAYER_NO_PRIORITY,						//���Ȩ�޲���
	ERROR_NO_CONTRIBUTION_INDEX,					//���ñ��в����ڵĹ�������
	ERROR_GUILD_LEVEL_LIMITED,
	ERROR_GUILD_ITEM_TIME_LIMITED,
	ERROR_NO_ENOUGH_CONTRIBUTION,

    ERROR_PAY_NOT_FIRSTPAY = 1000,					//�����׳� �����׳��������
    ERROR_PAY_IS_FIRSTPAY,							//���׳� �����׳��������
    ERROR_PAY_NUM_NO_EXIST,							//�ײ�����������
    ERROR_PAY_VIP_LEVEL_NO_EXIST,					//VIP�ȼ�������
    ERROR_PAY_VIP_REWARD_GOT,						//VIP����Ѿ���ȡ����
    ERROR_PAY_VIP_REWARD_NO_EXIST,					//VIP�ȼ���Ӧ�����������
    ERROR_PAY_VIP_REWARD_EMPTY,						//VIP�������Ϊ��
    ERROR_VIP_LEVEL_TOO_LOW,                        //VIP �ȼ�����

    ERROR_INVITE_CODE = 1201,
    ERROR_INVITE_CODE_INPUT_SUCCESS = 1202,   		//�Ƽ�������ɹ�����ǰ���ɾͽ�����ȡ����
    ERROR_INVITE_CODE_INPUT_SELF = 1203,		  		//���������Լ����Ƽ���
    ERROR_INVITE_CODE_INPUT_ERROR = 1204, 	  		//�Ƽ����������
    ERROR_INVITE_CODE_INPUTED = 1205,			  	//���Ѿ����������Ƽ�����

	ERROR_MOVE_SPEED_ERROR = 1300,					// ϵͳ�����������ʹ�����ƶ����٣�ǿ���߳���������������ϵ�ͷ�
	ERROR_WORLD_BATTLE_ERROR = 1301,				// �������ս���ʧ��
	ERROR_WORLD_BATTLE_OFFLINE = 1302,				// �ȴ����ս����Ѿ�����
	ERROR_WORLD_BATTLE_GS_ERROR	= 1303,				// δ����ȷ��֤GS WORLD ����
	ERROR_WORLD_BATTLE_MATCH_ERROR = 1304,			// ����ƥ�䳬ʱ
	ERROR_WORLD_BATTLE_NOT_START = 1305,			// ���ս��δ��ʼ
	ERROR_GUILD_BATTLE_USE_OUT = 1306,				// ���������Ѿ����
	ERROR_GUILD_BATTLE_FRESH_TIME_ERROR = 1307,		// ˢ��ʱ��δ��
	ERROR_GUILD_BATTLE_BEING_ATTACK = 1308,			// ��������
	ERROR_GUILD_BATTLE_STATUS_ERROR = 1309,			// ս��״̬δ�ҵ�
	ERROR_GUILD_BATTLE_STATUS_ATTACK_ERROR = 1310,	// �޹���״̬
	ERROR_GUILD_BATTLE_START = 1311,				// ����ս�Ѿ���ʼ
	ERROR_GUILD_BATTLE_SIGNED = 1312,				// �Ѿ�����
	ERROR_GUILD_BATTLE_MEMBER_LESS = 1313,			// ��Ա��������6��
	ERROR_GUILD_BATTLE_LESS_MONEY = 1314,			// �����ʽ𲻹�
	ERROR_GUILD_BATTLE_CREATE_FAILED = 1315,		// ��������ս��¼ʧ��
	ERROR_GUILD_BATTLE_IN_BATTLE = 1316,			// ����ս����

	ERROR_CONFIG_NOT_EXIST = 5000,			//���ñ�񲻴���
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
	eLogin_ServerMemoryUNENOUGH,		// �������ڴ治��

	eLogic_NoSetSID				= 100,
	eLogic_NoLogicUC,

	eLogin_PlayerFull,					// ��������������
	eLogin_RequestDB_OverTime,			// ����DB���ݳ�ʱ
	eLogin_CheckEnterCodeFail,			// �����½��֤KEYʧ��
	eLogin_ServerMaintain,				// ������ά��
	eSDKUID_CHECK_FAIL,
	eSDK_REQUST_DNS_FAIL,							//�������DNSʧ��
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
	MSG_KILL_MONSTER				= 1,		// ɱ��
	MSG_LEVEL_UP					= 2,		// ����
	MSG_ENTER_STAGE					= 3,		// ����ؿ�
	MSG_STAGE_COMPLETE				= 4,		// ͨ��
	MSG_SKILL_LEVEL_UP				= 5,		// ��ɫ��������
	MSG_GAIN_EQUIP					= 6,		// ��ȡװ��
	MSG_GAIN_ITEM					= 7,		// ��ȡ��Ʒ
	MSG_GAIN_PET					= 8,		// ��ó���
    MSG_DIALOGUE					= 9,		// �Ի�NPC
	MSG_TEAM_CHANGE					= 10,		// ������Ϣ���
	MSG_ARENA						= 11,		// �μӾ�����
	MSG_ESCORT						= 12,		// �μ�����
	MSG_TREASURE					= 13,		// �μӲر�ͼ
	MSG_WANTED						= 14,		// �μ�ͨ��
	MSG_ADD_FRIEND					= 15,		// ��Ӻ���
	MSG_FEED_PET					= 16,		// ����ιʳ
	MSG_PET_SKILL_LEVEL_UP			= 17,		// ���＼������
	MSG_USE_EQUIP					= 18,		// ʹ��װ��
    MSG_EQUIP_INTENSIFY				= 19,		// װ��ǿ��(ǿ���Ѿ�����)
    MSG_EQUIP_FORGE					= 20,		// װ������
	MSG_EQUIP_COMPOSITION			= 21,		// װ���ϳ�
    MSG_BUY_ITEM					= 22,		// ������Ʒ
    MSG_TITLE_CHANGE				= 23,       // �ƺű��
    MSG_GENIUS_LEARNED				= 24,		// �츳����
    MSG_SUBOCCUPATION_TRANSFER		= 25,	    // תְ
	MSG_PET_CHANGE					= 26,		// �������
    MSG_CAPTIVE_SAVED				= 27,       // �������
    MSG_TASK_COMPLETE				= 28,       // �������
    MSG_EQUIP_GEM_INSET				= 29,		// ��Ƕ��ʯ
    MSG_OBJECT_APPLY_ABSTATE		= 30,		// ���������쳣״̬Buff
	MSG_WABAO						= 31,		// �ڱ�
    MSG_SAODANG						= 32,		// ɨ��
    MSG_TEAM_BOSS					= 33,		// ���BOSS
    MSG_MONO_BOSS					= 34,		// ����BOSS
	MSG_TREASURE_COMPLETE			= 35,		// ��ɲر�ͼ
    MSG_PVP_RANGK_CHANGE			= 36,		// PVP���α��
	MSG_TEAM_WABAO					= 37,		// ����ڱ�
	MSG_TICKET_STAGE				= 38,		// �����ż�
    MSG_BIOGRAPHY_STAGE             = 39,       // �⴫

    MSG_GOLD_MINE                   = 40,       // �ڽ�󣨽���ָ��
    MSG_GEM_UPGRADE                 = 41,       // ��ʯ����

    MSG_CHOPIN_MUSIC                = 42,       // Ф�����ָ���
    MSG_GREADY_GAMBLING             = 43,       // ̰���ĳ�
    MSG_WORLD_BOSS_BATTLE           = 44,       // �μ�һ������BOSS ս��
    MSG_CITY_WAR                    = 45,       // �μ���һ�ι��﹥��ս
    MSG_ATTEND_OFFLINE_PVP           = 46,       // �μ���һ�ξ���������
    MSG_ATTEND_ONLINE_PVP           = 47,       // �μ���һ�ζ��޳�����

    MSG_OPEN_CHEST                  = 50,       // �򿪱���

    MSG_SHOPING                     = 60,       // �̵�����
    MSG_EQUIP_INTENSIFY_OP          = 61,       // ǿ��װ������(��һ��ǿ������)
    MSG_LOTTERY_DRAW                = 62,       // �齱
    MSG_CASH_CHAGE                  = 63,       // �ֽ��ֵ

    MSG_ATTEND_GUILD_STAGE          = 70,       // �μ���һ�ι��ḱ��
    MSG_GUILD_DONATE                = 71,       // �������
    MSG_GUILD_FRAGMENT_DONAE        = 72,       // ������Ƭ����
    MSG_GUILD_CHAT                  = 73,       // ��������Ƶ������һ��
    MSG_GUILD_TREASURE              = 74,       // ����Ѱ��һ��
    MSG_GUILD_MEMBER_LEVEL_EXP      = 75,       // �����Ա�ȼ��ɳ�������

    MSG_COMUNICATION_GROUP          = 100,      // ��������Ϣ��������Ϣ
    MSG_ACTIVITY_POINT              = 101,      // ��Ҫ���ӻ�Ծ��ʱ
    MSG_DAILY_MISSIONS_STATUSE      = 102,      // ÿ�����񼯺�״̬����ʱ

    MSG_TYPE_MAX
};

#define _STR(s)             #s
#define STR(s)              _STR(s)
#define _MSGNAME(MsgType)    MSG_##MsgType

//#define MSGNAME(MsgType)    STR(_MSGNAME(MsgType))
#define MSGNAME(MsgType)    STR(MsgType)

// �˺���������
enum eAlterAffectType
{
    eBeforeHit,
    eInAttackValue,         // ���㹥����ֵʱ
    eAfterAttackValue,      // �ڹ�����ֵ���֮��
    eAfterAttackAdded,      // �����˺�֮��
    eAfterAnti,             // �ֿ�֮��
    eAfterShield,           // ����֮��
    eAfterRebound           // ����֮��
};

// �������
enum eAttackHint
{
    eNormal = -1,
    eFloat = 1,     // ���մ��
    eBlock,         // �񵲴��
};

// ��������
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
    TREASURE_MAP = 1,       // ������
    YABIAO,                 // Ѻ��
    TEAM_BOSS_STAGE,        // ���BOSS
    MONO_BOSS_STAGE,        // ����BOSS
    CITY_WAR,               // ���﹥��
    WABAO,                  // �ڱ�

    DRAW_PHY_POWER,         // ��ȡ����
    WABAO_TEAM,             // ����ڱ�
    WORLD_BOSS,				// ����BOSS
    TICKET_STAGE,           // �����ż�

    CHOPIN_MUSIC = 12,      // Ф�������(���鸱��)
    GREEDY_GAMBLING,        // ̰���ĳ�����Ҹ�����
	WAIZHUAN,				// �⴫ģʽ�

    GOLD_MINE,              // ԭҰ���
    GEM_UPGRADE,            // �貱�ʯ
	ONLINE_BOT,				// ���߹һ�

    PROVING_TOWER,          // ����֮��

	CD_KEY	= 101,

    DUMMY_ACTIVITY1,        // ��Ӫ�
};


enum PERIOD_QUOTA
{
    GUILD_STAGE_QUOTA = 1,        //���ḱ��ʣ�����
    FRAGMENT_ASK_QUOTA = 2,       //��Ƭ������Ҫ���� 
    GUILD_TREASUER_QUOTA = 3,     //����Ѱ������
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
// ����������, ���õ�ս���ȼ����ȼ�����
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