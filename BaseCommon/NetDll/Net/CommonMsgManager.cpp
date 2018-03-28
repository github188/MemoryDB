

#include "CommonMsgManager.h"
//#include "PacketFactoryManager.h"
#include "A_CommonData.h"
#include "TableManager.h"

#include "Packet.h"

#ifdef SHOW_TABLE_PACKET_REV
#include "TimeManager.h"
#endif

// 大约一秒钟发10SEND_SLEEP_TIME个包
#define	 SEND_SLEEP_TIME	50
//--------------------------------------------------------------------------------------
CommonMsgManager::CommonMsgManager()
{

}
//--------------------------------------------------------------------------------------

unsigned int CommonMsgManager::OnCommonMsg( Player *pPlayer, Packet *pPacket )
{
	A_CommonData *pData = dynamic_cast<A_CommonData*>(pPacket);
	if (pData)
	{		
		const AString &type = pData->mEventName;
		if (type.empty())
			return PACKET_EXE_ERROR;
#ifdef SHOW_TABLE_PACKET_REV
		printf("接收到数据包消息[%s]-->[%u]-->[%d:%d:%d]\n",type.c_str(),pData->mData->size(),g_pTimeManager->GetHour(),g_pTimeManager->GetMinute(),g_pTimeManager->GetSecond());
#endif
		AutoData hData;
		// 检查是否是独立包
		if ( pData->mIsBegin && pData->mIsEnd )
		{
			hData = pData->mData;
		}
		else
		{
			if (pData->mIsBegin)
			{
				mRevDataMap[MAKE_INDEX_ID(type.c_str())] = pData->mData;
				pData->mData->seek(0,SEEK_END);
			}
			else
			{
				AutoDataMap::iterator it = mRevDataMap.find(MAKE_INDEX_ID(type.c_str()));
				if (it!=mRevDataMap.end())
				{
					hData = it->second;
					hData->write(pData->mData->data(),pData->mSize);
					if (pData->mIsEnd)
					{
						mRevDataMap.erase(it);
					}
				}
			}
		}

		if (hData)
		{
			//hTable->_SaveToCSV(("e:/"+type+".csv").c_str());
			return ExeCommonPacket(pPlayer, pPacket, type, hData);
		}
		return PACKET_EXE_CONTINUE;		
	}
	return PACKET_EXE_ERROR;
}
//--------------------------------------------------------------------------------------


void CommonMsgManager::Tick()
{
	// 发送缓冲中的表格 [杨文鸽 2010/6/9]
	if (!m_waitSendTableList.empty())
	{
		SendEventData &sendT = m_waitSendTableList.front();		
		if (GetTickCount()-sendT.upTime>SEND_SLEEP_TIME) 
		{
			if (__SendData(sendT))
			{
				m_waitSendTableList.pop_front();
			}
			else
				sendT.upTime = GetTickCount();
		}
	}
}


//----------------------------------------------------------------------------------------------
