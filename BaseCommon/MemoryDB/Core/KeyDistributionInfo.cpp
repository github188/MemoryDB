#include "KeyDistributionInfo.h"

#include "ServerIPInfo.h"


bool TableKeyDistribution::AppendNodeDistributionData( Hand<NetNodeConnectData> node, AutoData slotData )
{
	// ���Ƴ���ǰ��ͬ�Ľڵ�
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

	// NOTE: ʹ��HASH Slot �ֲ�
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
	//	ERROR_LOG("���ش���: ָ���������KEYֵ[%s]��������СKEYֵ[%s]", STRING(maxRange), STRING(minRange));
	//	return false;
	//}	

	//// ���Ƴ���ǰ��ͬ�Ľڵ�
	//for (size_t i=0; i<mDistributionList.size(); ++i)
	//{
	//	KeyRange &keyInfo = mDistributionList[i];
	//	if ( node==keyInfo.mNode )
	//	{
	//		mDistributionList.remove(i); 
	//		break;
	//	}
	//}
	//// ����Ƿ���ڽ������, �������, ���ط�, ���������ش���
	//for (size_t i=0; i<mDistributionList.size(); ++i)
	//{
	//	KeyRange &keyInfo = mDistributionList[i];

	//	if ( (minRange>=keyInfo.mMin && minRange<=keyInfo.mMax)	// ��Сֵ��������
	//		|| (maxRange>=keyInfo.mMin && maxRange<=keyInfo.mMax) // ���ֵ��������
	//		)
	//	{ 
	//		ERROR_LOG("���ش���: %sָ���������KEYֵ[%s]��������СKEYֵ[%s], �뵱ǰ%s[%s~%s]���ڽ���",
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
	////		TABLE_LOG("WARN: ��ǰ�Ѿ����ڷֲ���Ϣ, �����ݲ�һ��, ���ڽ��и��� [%s]~[%s]", STRING(minRange), STRING(maxRange));

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
