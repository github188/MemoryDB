#ifndef _INCLUDE_KEYDISTRIBUTIONINFO_H_
#define _INCLUDE_KEYDISTRIBUTIONINFO_H_

#include "MeshedNetNodeData.h"
#include "MemoryDBHead.h"
//-------------------------------------------------------------------------
// ���ݱ�ֲ���Ϣ, ��Node�����Ӻ�, ��ͬ������Ϣ
class MemoryDB_Export TableKeyDistribution : public AutoBase
{
public:
	Hand<NetNodeConnectData> FindNode(Int64 key)
	{
		if (mbStringKey)
			return FindNode(STRING(key));

		return _FindNode((UInt64)key);
	}

	Hand<NetNodeConnectData> FindNode(const char *szKey)
	{
		if (mbStringKey) 
		{
			short slotIndex = STRHASH_SLOT(szKey);
			return mDistributionSlotArray[slotIndex];

			//UInt64 key = (UInt64)(uint)(MAKE_INDEX_ID(szKey));
			//return _FindNode(key);
		}

		return FindNode( TOINT64(szKey) );
	}

	Hand<NetNodeConnectData> _FindNode(UInt64 key)
	{
		//for (size_t i=0; i<mDistributionList.size(); ++i)
		//{
		//	KeyRange &keyInfo = mDistributionList[i];
		//	if ( key>=(UInt64)keyInfo.mMin && key<=(UInt64)keyInfo.mMax )
		//		return keyInfo.mNode;
		//}
		short slotIndex = HASH_SLOT(key);
		return mDistributionSlotArray[slotIndex];
		
		return Hand<NetNodeConnectData>();
	}

	bool AppendNodeDistributionData( Hand<NetNodeConnectData> node, AutoData slotData );

	bool CheckAllHashSlot(const EasyMap<short, bool> &localSlot)
	{
		for (int i=0; i<mDistributionSlotArray.size(); ++i)
		{
			if (!mDistributionSlotArray[i] && !localSlot.exist(i))
				return false;
		}
		return true;
	}

public:
	struct KeyRange
	{
	public:
		AutoData	mSlotData;

	public:
		// DBNode�������� ʹ��KEY>"DBSERVER_IPPORT"����ýڵ㿪�ŵ�DB����IP���˿�
		Hand<NetNodeConnectData>	mNode;	
	};

public:
	TableKeyDistribution()
		: mbStringKey(false)
		, mDistributionSlotArray(DB_HASH_SLOT_COUNT)
	{

	}

public:
	Array<KeyRange>		mDistributionList;
	typedef Hand<NetNodeConnectData>	NodeConnectData;
	Array<NodeConnectData> mDistributionSlotArray;
	bool				mbStringKey;
};

typedef Auto<TableKeyDistribution> AutoDistributionData;
//-------------------------------------------------------------------------

#endif //_INCLUDE_KEYDISTRIBUTIONINFO_H_