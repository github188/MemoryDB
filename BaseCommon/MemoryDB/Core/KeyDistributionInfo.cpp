#include "KeyDistributionInfo.h"

#include "ServerIPInfo.h"


bool TableKeyDistribution::AppendNodeDistributionData( Hand<NetNodeConnectData> node, AutoData slotData )
{
	// 先移除当前相同的节点
	for (size_t i=0; i<mDistributionList.size(); ++i)
	{
		KeyRange &keyInfo = mDistributionList[i];
		if ( node==keyInfo.mNode )
		{
			mDistributionList.remove(i); 
			break;
		}
	}
	KeyRange k;
	//k.mMin = minRange;
	//k.mMax = maxRange;
	k.mNode = node;
	k.mSlotData = slotData;
	mDistributionList.push_back(k);

	// NOTE: 使用HASH Slot 分布
	if (slotData->dataSize()<=0)
	{
		for (int i=0; i<mDistributionSlotArray.size(); ++i)
		{
			mDistributionSlotArray[i] = node;
		}
		return true;
	}

	slotData->seek(0);
	short slotIndex = 0;
	while (true)
	{
		if (slotData->read(slotIndex))
		{
			if (slotIndex>=mDistributionSlotArray.size())
			{
				ERROR_LOG("slot index more then %d", DB_HASH_SLOT_COUNT);
				return false;
			}
			mDistributionSlotArray[slotIndex] = node;
		}
		else
			break;
	}
	return true;
	//if (maxRange!=-1 && maxRange<=minRange)
	//{
	//	ERROR_LOG("严重错误: 指定区间最大KEY值[%s]不大于最小KEY值[%s]", STRING(maxRange), STRING(minRange));
	//	return false;
	//}	

	//// 先移除当前相同的节点
	//for (size_t i=0; i<mDistributionList.size(); ++i)
	//{
	//	KeyRange &keyInfo = mDistributionList[i];
	//	if ( node==keyInfo.mNode )
	//	{
	//		mDistributionList.remove(i); 
	//		break;
	//	}
	//}
	//// 检查是否存在交集情况, 如果存在, 返回否, 并报告严重错误
	//for (size_t i=0; i<mDistributionList.size(); ++i)
	//{
	//	KeyRange &keyInfo = mDistributionList[i];

	//	if ( (minRange>=keyInfo.mMin && minRange<=keyInfo.mMax)	// 最小值在区间内
	//		|| (maxRange>=keyInfo.mMin && maxRange<=keyInfo.mMax) // 最大值在区间内
	//		)
	//	{ 
	//		ERROR_LOG("严重错误: %s指定区间最大KEY值[%s]不大于最小KEY值[%s], 与当前%s[%s~%s]存在交集",
	//			ServerIPInfo::GetAddrInfo(node->mServerNetKey).c_str(),
	//			STRING(maxRange), 
	//			STRING(minRange),
	//			ServerIPInfo::GetAddrInfo(keyInfo.mNode->mServerNetKey).c_str(),
	//			STRING(keyInfo.mMin),
	//			STRING(keyInfo.mMax)
	//			);
	//		return false;
	//	}
	//}
	////for (size_t i=0; i<mDistributionList.size(); ++i)
	////{
	////	KeyRange &keyInfo = mDistributionList[i];
	////	if ( node==keyInfo.mNode )
	////	{
	////		if (minRange==keyInfo.mMin && maxRange==keyInfo.mMax)
	////			return true;
	////		TABLE_LOG("WARN: 当前已经存在分布信息, 且数据不一至, 现在进行更新 [%s]~[%s]", STRING(minRange), STRING(maxRange));

	////		keyInfo.mMin = minRange;
	////		keyInfo.mMax = maxRange;
	////		return true;
	////	}
	////}
	//KeyRange k;
	//k.mMin = minRange;
	//k.mMax = maxRange;
	//k.mNode = node;
	//mDistributionList.push_back(k);

	//return true;
}
