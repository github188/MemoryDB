#include "EventPacket.h"

#include "Assertx.h"

#include "NetDataBuffer.h"
#include "TimeManager.h"
#include "ResoursePack.h"
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

UINT EventPacket::Execute( tNetConnect* pConnect )
{
	UInt64 nowTime = TimeManager::NowTick();
	Logic::tEventCenter *pCenter = pConnect->GetNetHandle()->GetEventCenter();
	AssertEx(NULL!=pCenter, "事件中心不应为空");
	GetData()->seek(0);
	AutoEvent hRevEvent = pCenter->RestoreMsg(GetData().getPtr());

	if (hRevEvent)
	{		
		//NOTE_LOG(" Run net msg [%s]", hRevEvent->GetEventName());
		hRevEvent->_OnBindNet(pConnect);
		pConnect->OnReceiveEvent(hRevEvent.getPtr(), this);
		pConnect->GetNetHandle()->OnReceiveEvent(pConnect, hRevEvent.getPtr());

		int tickDiff = (int)(TimeManager::NowTick() - nowTime);
		if (tickDiff > 1)
		{
			NOTE_LOG("EventPacket::Execute %s tick=%d", hRevEvent->GetEventName(), tickDiff);
		}
	}
	else
	{
		WARN_LOG("恢复网络事件失败 >[%d] size [%llu]", GetPacketID(), mData->dataSize());
		pConnect->GetNetHandle()->GetNetProtocol()->OnPacketExecuteError(pConnect, this);
		
		return -1;
	}

	return 0;
}
//-----------------------------------------------------------------------------
int EventPacket::SetEvent(AutoEvent &hEvent)
{
	mData->clear(false);
	if (hEvent->StandardDataEvent())
		SetState(eEventDataPacket);
	if ( hEvent->GetEventCenter()->SerializeMsg(hEvent, mData.getPtr()) )
	{
		mData->setDataSize(mData->tell());
		return mData->tell();
	}
	return 0;
}
//-----------------------------------------------------------------------------
EventPacket::EventPacket()
{
	mData = MEM_NEW DataBuffer(EVENTPACKET_DEFAULT_BUFFER_SIZE);
}


void EventPacket::SwapEventData( AutoData &eventData )
{
	AutoData temp = mData;
	mData = eventData;
	eventData = temp;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UINT CheckRequestPacket::Execute( tNetConnect* pConnect )
{
	// 读取ID, 需要验证的信息

	return 0;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
UINT CheckResponsePacket::Execute( tNetConnect* pConnect )
{
	// 设置网络ID, 生成验证ID
	// ...
	return 0;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define SCR_SIZE_TYPE	unsigned int
#define SCR_SIZE_INFO_LENGTH	sizeof(SCR_SIZE_TYPE)

BOOL CompressPacket::Read( LoopDataStream& iStream, size_t packetSize )
{
	size_t size = packetSize;

	if (mZipBuffer->size()<(DSIZE)size)
		mZipBuffer->resize(size);

	if (iStream.Read(mZipBuffer->data(), (UINT)size)==size)
	{
		mZipBuffer->seek(size);

		_OnReceiveData(mZipBuffer->data(), size);

		AssertEx (size>sizeof(size_t), "压缩数据大小错误,不可能小于信息数据长度");
		mZipBuffer->seek(0);
		SCR_SIZE_TYPE scrSize = 0;
		if (!mZipBuffer->read(scrSize))
		{
			ERROR_LOG("解压数据出错, 从压缩数据中读取源始大小失败");
			return FALSE;
		}
		HandDataBuffer d = mZipBuffer;
		// 解压缩
		if (d->UnZipData(mData, 0, scrSize, SCR_SIZE_INFO_LENGTH, size-SCR_SIZE_INFO_LENGTH))		
		{
			//INFO_LOG("OK >>> 成功解压网络数据 scr size [%u], Zip size [%u], RATE[%.4f]", scrSize, size-SCR_SIZE_INFO_LENGTH, (float)(size-SCR_SIZE_INFO_LENGTH)/(float)scrSize);
		}
		else
		{
			ERROR_LOG("XXX >>> 解压缩失败, 检索压缩数据及大小信息是否正确.");
			return FALSE;
		}		

	}
	return TRUE;
}

//-------------------------------------------------------------------------
CompressPacket::CompressPacket()
	: EventPacket()
{
	//AssertEx(m_pNetProtocol!=NULL, "网络协议为空");
	mZipBuffer = MEM_NEW DataBuffer(EVENTPACKET_DEFAULT_BUFFER_SIZE);
}

int CompressPacket::SetEvent( AutoEvent &hEvent )
{
	mZipBuffer->clear(false);
	
	if (! hEvent->GetEventCenter()->SerializeMsg(hEvent, mZipBuffer.getPtr()) )
	{
		ERROR_LOG("发送事件错误, 序列化事件数据失败");
		return 0;
	}
	mZipBuffer->setDataSize(mZipBuffer->tell());
	return mZipBuffer->tell();
}

bool CompressPacket::ReadySendData()
{
	size_t size = mZipBuffer->dataSize();

	AssertEx(size<0x00FFFFFF, "发送数据必须 < 65535");

	// 使用 zip 压缩
	HandDataBuffer d = mZipBuffer;
	int zipSize = d->ZipData(mData, SCR_SIZE_INFO_LENGTH, 0, size, MAX_SPEED);	
	if (zipSize<=0)
	{
		ERROR_LOG("压缩消息包数据失败, 可能目标空间不够");
		return false;
	}

	mData->seek(0);
	// 写入原数据大小
	mData->write((SCR_SIZE_TYPE)size);

	// 将当前位置移动到发送数据之后, 以确定发送大小
	mData->seek(SCR_SIZE_INFO_LENGTH + zipSize);

	//INFO_LOG("消息数据压缩 >>> Size[%u] >>> Zip size [%d], RATE[%.4f]", size, zipSize, (float)zipSize/(float)size);

	return true;
}

void CompressPacket::SwapEventData( AutoData &eventData )
{
	AutoData temp = mZipBuffer;
	mZipBuffer = eventData;
	eventData = temp;
}

bool CompressPacket::UnZip(AutoData zipBuffer,  AutoData &resultData )
{
	AssertEx(resultData, "返回数据必须先进行初始, 以便考虑使用 NetDataBuffer");
	
	size_t size = zipBuffer->tell();
	zipBuffer->seek(0);
	SCR_SIZE_TYPE scrSize = 0;
	if (!zipBuffer->read(scrSize))
	{
		TABLE_LOG("解压数据出错, 从压缩数据中读取源始大小失败", "");
		return false;
	}

	HandDataBuffer zipData = zipBuffer;
	// 解压缩
	if (zipData->UnZipData(resultData, 0, scrSize, SCR_SIZE_INFO_LENGTH, size-SCR_SIZE_INFO_LENGTH))		
	{
		//!!! INFO_LOG("OK >>> 成功解压网络数据 scr size [%u], Zip size [%u], RATE[%.4f]", scrSize, size-SCR_SIZE_INFO_LENGTH, (float)(size-SCR_SIZE_INFO_LENGTH)/(float)scrSize);
	}
	else
	{
		TABLE_LOG("XXX >>> 解压缩失败, 检索压缩数据及大小信息是否正确.", "");
		return false;
	}

	return true;
}

bool CompressPacket::FullZipData( AutoData zipData )
{
	size_t size = zipData->dataSize();
	if (size<=SCR_SIZE_INFO_LENGTH)
	{
		ERROR_LOG("压缩数据长度太小");
		return false;
	}
	zipData->seek(0);
	SCR_SIZE_TYPE scrSize = 0;
	if (!zipData->read(scrSize))
	{
		INFO_LOG("解压数据出错, 从压缩数据中读取源始大小失败");
		return FALSE;
	}
	HandDataBuffer d = zipData;
	// 解压缩
	if (d->UnZipData(mData, 0, scrSize, SCR_SIZE_INFO_LENGTH, size-SCR_SIZE_INFO_LENGTH))		
	{
		//!!! INFO_LOG("OK >>> 成功解压网络数据 scr size [%u], Zip size [%u], RATE[%.4f]", scrSize, size-SCR_SIZE_INFO_LENGTH, (float)(size-SCR_SIZE_INFO_LENGTH)/(float)scrSize);
	}
	else
	{
		ERROR_LOG("XXX >>> 解压缩失败, 检索压缩数据及大小信息是否正确.");
		return FALSE;
	}		

	return TRUE;
}



PartPacket::PartPacket()
{
	mData = MEM_NEW DataBuffer(MAX_UDP_PACKET_SIZE);
}

UINT PartPacket::Execute( tNetConnect* pConnect )
{
	return 0;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

bool DataPacket::_OnSendData( void *szSendData, size_t size ) const
{
	if (HasState(ePacketStateEncrypt) && !mbAlreadyEncrypt)
	{
		mbAlreadyEncrypt = true;
		bool b = _EncryptData(szSendData, size);
		if (!b)
		{
			TABLE_LOG("Error: Encrypt net Data fail");
			return false;
		}
	}
	return true;
}

bool DataPacket::_OnReceiveData( void *szRevData, size_t size )
{
	if (HasState(ePacketStateEncrypt))
	{
		bool b = _DecryptData(szRevData, size);
		if (!b)
		{
			TABLE_LOG("Error: Decrypt net Data fail");
			return false;
		}
	}
	return true;
}

bool DataPacket::_EncryptData( void *scrData, DSIZE length ) const
{
	return ResourcesPack::_EncryptData(scrData, length);
}

bool DataPacket::_DecryptData( void *scrData, DSIZE length ) const
{
	return ResourcesPack::_DecryptData(scrData, length);
}

//-------------------------------------------------------------------------

BOOL EventProcessPacket::ReadEvent( tNetConnect *pConnect, LoopDataStream& iStream, size_t packetSize )
{
	Logic::tEventCenter *pCenter = pConnect->GetNetHandle()->GetEventCenter();
	AssertEx(NULL!=pCenter, "事件中心不应为空");

	mMsgEvent = pCenter->RestoreMsg(&iStream);

	if (!mMsgEvent)
	{
		WARN_LOG("恢复网络事件失败 >[%d] size [%llu]", GetPacketID(), packetSize);
		pConnect->GetNetHandle()->GetNetProtocol()->OnPacketExecuteError(pConnect, this);

		return false;
	}		

	return true;
}

UINT EventProcessPacket::Execute( tNetConnect* pConnect )
{
	if (mMsgEvent)
	{		
		//NOTE_LOG(" Run net msg [%s]", hRevEvent->GetEventName());
		mMsgEvent->_OnBindNet(pConnect);
		pConnect->OnReceiveEvent(mMsgEvent.getPtr(), this);
		pConnect->GetNetHandle()->OnReceiveEvent(pConnect, mMsgEvent.getPtr());
	}

	return 0;
}
//-------------------------------------------------------------------------