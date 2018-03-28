#include "UDPMessage.h"
#include "TimeManager.h"
#include "ServerIPInfo.h"

#define _TEST_DEBUG_RAND_SEND_		0

#if _TEST_DEBUG_RAND_SEND_
#	include "RandTool.h"
#	include "TimeManager.h"
#endif

const int UDP_MSG_HEAD_SIZE = sizeof(int)+sizeof(byte)*2;
//-------------------------------------------------------------------------
UDPConnect::UDPConnect()
	: mLastRunMessageCode(0)
	, mSendPacketCode(0)
{
	mWaitMsgList.resize(MAX_WAIT_MSG_COUNT);
	for (size_t i=0; i<MAX_WAIT_MSG_COUNT; ++i)
	{
		mWaitMsgList[i] = new MsgData();
		mWaitMsgList[i]->Init();
	}
}

UDPConnect::~UDPConnect()
{	
	for (size_t i=0; i<MAX_WAIT_MSG_COUNT; ++i)
	{
		delete mWaitMsgList[i];			
	}
	mWaitMsgList.clear();
}

void UDPConnect::OnReceiveMsg( DataStream *revData, int revSize )
{	
	if (revSize<=UDP_MSG_HEAD_SIZE)
		return;
	
	revData->seek(revSize-UDP_MSG_HEAD_SIZE);	

	int sendTime = 0;

	revData->read(sendTime);
	
	byte msgCode = 0;
	revData->read(msgCode);

	if (msgCode >= MAX_WAIT_MSG_COUNT)
	{
		ERROR_LOG("msg code too big [%d]", msgCode);
		return;
	}

	byte partCode = 0;
	revData->read(partCode);

	printf("[%d: %d]\n", (int)msgCode, (int)partCode);

	revSize -= UDP_MSG_HEAD_SIZE;
	byte partCount = 1;
	if (partCode==0)
	{
		// 注意分包数量保存在消息头前面的位置
		--revSize;
		revData->seek(revSize);

		revData->read(partCount);
		if (partCount<=1 && !_HaveWaitMsg())
		{		
			int nextCode = (mLastRunMessageCode+1)%MAX_WAIT_MSG_COUNT;
			if (msgCode!=nextCode)
				OnHappenGiveupPacket(nextCode, (msgCode-1)%MAX_WAIT_MSG_COUNT);

			mLastRunMessageCode = msgCode;
			revData->seek(0);
			revData->setDataSize(revSize);
			OnReceivePacket(revData, revSize);
#if _TEST_DEBUG_RAND_SEND_
			TABLE_LOG("Rev msg >%d", msgCode);
#endif
			return;
		}
	}	

	MsgData *msg = mWaitMsgList[msgCode];
	if (msg->mUse)
	{			
		if (msg->mTime!=sendTime)
		{
			OnHappenGiveupPacket(msgCode, msgCode, msg);
			// 包编号一样，但时间不一至时，说明原包已经过期, 直接丢弃	
			msg->Init();		
			msg->mUse = true;
			msg->mTime = sendTime;	
		}			
	}
	else
	{
		msg->mUse = true;
		msg->mTime = sendTime;
	}

	if (partCode==0)
		msg->mPartCount = partCount;

	_AppendMsgData(msg, partCode, revData, revSize);

#if _TEST_DEBUG_RAND_SEND_
	int count = 0;
	for (size_t i=0; i<msg->mMsgData.size(); ++i)
	{
		if (msg->mMsgData[i].dataSize()>0)
			++count;
	}
	TABLE_LOG("%d > %d = %d of %d", msgCode, partCode, count, msg->mPartCount);
#endif
}

void UDPConnect::_AppendMsgData( MsgData *msg, byte partCode, DataStream *revData, int msgDataSize )
{
	if (partCode>=msg->mMsgData.size())
		msg->mMsgData.resize(partCode+1);

	msg->mMsgData[partCode].seek(0);
	msg->mMsgData[partCode]._write(revData->data(), msgDataSize);

	_CheckRunMsg();
}

void UDPConnect::_CheckRunMsg()
{
	mTimeSqrtList.clear(false);
	for (size_t i=0; i<mWaitMsgList.size(); ++i)
	{								
		if (mWaitMsgList[i]->mUse)
			mTimeSqrtList.insert(mWaitMsgList[i]->mTime, i);
	}

	for (size_t i=0; i<mTimeSqrtList.size(); ++i)
	{
		byte msgCode = mTimeSqrtList.get(i);
		MsgData *msg = mWaitMsgList[ msgCode ];

		if (!_RunMsg(msg))
			return;
		int nextCode = (mLastRunMessageCode+1)%MAX_WAIT_MSG_COUNT;
		if (msgCode!=nextCode)
			OnHappenGiveupPacket(nextCode, (msgCode-1)%MAX_WAIT_MSG_COUNT);

		mLastRunMessageCode = msgCode;

		msg->Init();

#if _TEST_DEBUG_RAND_SEND_
		TABLE_LOG("Rev msg >%d", mLastRunMessageCode);
#endif
	}
}

bool UDPConnect::_RunMsg( MsgData *msg )
{
	if (msg->mPartCount>0 && msg->mMsgData.size()>=msg->mPartCount)
	{
		for (size_t i=0; i<msg->mPartCount; ++i)
		{
			if (msg->mMsgData[i].dataSize()<=0)
				return false;
		}
		if (msg->mPartCount<=1)
		{
			msg->mMsgData[0].seek(0);
			OnReceivePacket(&(msg->mMsgData[0]), msg->mMsgData[0].dataSize());				
		}
		else
		{
			DataBuffer temp(MAX_UDP_MSG_SIZE * msg->mPartCount);
			for (size_t i=0; i<msg->mPartCount; ++i)
			{
				temp._write(msg->mMsgData[i].data(), msg->mMsgData[i].dataSize());
			}
			temp.seek(0);
			OnReceivePacket(&temp, temp.dataSize());
		}
		return true;
	}
	return false;
}

void UDPConnect::SendData( UDPMessageNet *net, byte serverMsgCode, DataStream *sendData, UInt64 addr, bool bAddPacketCode )
{
	if (bAddPacketCode)
		mSendPacketCode = (mSendPacketCode+1)%MAX_WAIT_MSG_COUNT;

	int s = sendData->dataSize();
	sendData->seek(s-2); // 移动到包序号位置
	sendData->write(mSendPacketCode);
	if (!net->Send(sendData, addr))
		ERROR_LOG("UDP send fail>%s", ServerIPInfo::GetAddrInfo(addr));

	sendData->seek(s-2);
	sendData->write(serverMsgCode);
}

//UDPConnect::MsgData* UDPConnect::GetFreeMsgData()
//{
//	for (size_t i=0; i<MAX_WAIT_MSG_COUNT+2; ++i)
//	{
//		if (!mMsgDataList[i]->mUse)
//			return mMsgDataList[i];			
//	}
//	AssertEx(0, "编码逻辑错误, 原理上不应该多于 %d", MAX_WAIT_MSG_COUNT+2);
//	return NULL;
//}





//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
void UDPMessageNet::OnReceiveMsg( UInt64 scrAddr, DataStream *revData, int revSize )
{
	AutoUDPConnect udpConn = mUDPConnectList.find(scrAddr);
	if (!udpConn) //(it==mUDPConnectList.end())
	{
		udpConn = OnReceiveNewConnect(scrAddr); 
		if (!udpConn)
			return;
	}
	//else
	//	udpConn = it->second;

	udpConn->OnReceiveMsg(revData, revSize);
}

bool UDPMessageNet::SendPacket( UInt64 targetAddr, DataStream *sendData, int sendSize, Array<UInt64> *targetList )
{
	if (sendSize<=0)
		return true;

	if (sendSize>sendData->size())
		return false;

	int msgCount = (sendSize + (MAX_UDP_MSG_SIZE-1))/MAX_UDP_MSG_SIZE;
	if (msgCount>0xFF)
	{
		ERROR_LOG("发送消息包大小大于限制[%d]", (int)(MAX_UDP_MSG_SIZE*0xFF));
		return false;
	}

	int	nowTime = (int)TimeManager::NowTick();

	mServerPacketCode = (mServerPacketCode+1)%MAX_WAIT_MSG_COUNT;

#if _TEST_DEBUG_RAND_SEND_
	RandTool<AutoData>	tempList;

	int alreadySize = 0;
	for (int i=0; i<msgCount; ++i)
	{
		alreadySize = i*MAX_UDP_MSG_SIZE;
		int lastSize = sendSize - alreadySize;
		if (lastSize>MAX_UDP_MSG_SIZE)
			lastSize = MAX_UDP_MSG_SIZE;

		AutoData d = MEM_NEW DataBuffer(MAX_UDP_MSG_SIZE+16);
		DataStream &data = *d;
		
		data._write(sendData->data()+(alreadySize), lastSize );		
		
		if (i==0)
			data.write((byte)msgCount);

		// 写入发包时间
		data.write(nowTime);
		data.write(mSendPacketCode);
		data.write((byte)i);


		tempList.Push(d);
	}

	if (mSendPacketCode%2==0)
	{
		while (true)
		{
			AutoData d;
			tempList.RandPop(d);
			if (d)
			{
				if (targetList!=NULL)
				{
					for (size_t i=0; i<targetList->size(); ++i)
					{
						Send (d.getPtr(), (*targetList)[i]);
					}
				}
				else
					Send(d.getPtr(), targetAddr);
			}
			else
				break;

			TimeManager::Sleep(2);
		}
	}

#else

	int alreadySize = 0;
	for (int i=0; i<msgCount; ++i)
	{
		mSendDataBuffer.clear();		

		alreadySize = i*MAX_UDP_MSG_SIZE;
		int lastSize = sendSize - alreadySize;
		if (lastSize>MAX_UDP_MSG_SIZE)
			lastSize = MAX_UDP_MSG_SIZE;
		mSendDataBuffer._write(sendData->data()+(alreadySize), lastSize );		

		if (i==0)
			mSendDataBuffer.write((byte)msgCount);

		// 写入发包时间
		mSendDataBuffer.write(nowTime);
		mSendDataBuffer.write(mServerPacketCode);
		mSendDataBuffer.write((byte)i);

		if (targetList!=NULL)
		{
			for (size_t m=0; m<targetList->size(); ++m)
			{
				UInt64 addr = (*targetList)[m];
				AutoUDPConnect udp = GetUDPConnect(addr);
				if (udp)				
					udp->SendData(this, mServerPacketCode, &mSendDataBuffer, addr, i==0);
				else
					Send (&mSendDataBuffer, addr);
			}
		}
		else			
		{
			AutoUDPConnect udp = GetUDPConnect(targetAddr);
			if (udp)				
			{				
				udp->SendData(this, mServerPacketCode, &mSendDataBuffer, targetAddr, i==0);
			}
			else
				Send (&mSendDataBuffer, targetAddr);
		}		
	}
#endif
	return true;
}

UDPMessageNet::UDPMessageNet() 	
	: mSendDataBuffer(MAX_UDP_MSG_SIZE+16)
	, mServerPacketCode(0)
{
	mUDP.SetReceiveBufferSize(1024*64);
	mUDP.SetSendBufferSize(1024*128);
}

AutoUDPConnect UDPMessageNet::NewUDPConnect( UInt64 scrAddr )
{
	return  MEM_NEW DefaultUDPConnect(this, scrAddr);
}

void UDPMessageNet::Process()
{
	UDPThreadNet::Process();

	//for (UDPList::iterator it=mUDPConnectList.begin(); it!=mUDPConnectList.end(); )
	for (auto it = mUDPConnectList.begin(); it; )
	{
		if (it.get()->Process(this))
			++it;
		else
			it.erase();
			//it = mUDPConnectList.erase(it);
	}
}

AutoUDPConnect UDPMessageNet::RemoveUDPConnect( UInt64 scrAddr, AutoUDPConnect connect )
{
	AutoUDPConnect conn = mUDPConnectList.find(scrAddr);
	//if (conn)
	{
		if (!connect || connect==conn)
		{
			//conn = it->second;
			if (conn)
				conn->Close();
			mUDPConnectList.erase(scrAddr);
		}		
	}
	return conn;
}

void UDPMessageNet::AppendUDPConnect( UInt64 scrAddr, AutoUDPConnect udpConn )
{
	AutoUDPConnect existConn = mUDPConnectList.find(scrAddr);
	if (existConn && existConn!=udpConn)
		existConn->Close();
	mUDPConnectList[scrAddr] = udpConn;
}

AutoUDPConnect UDPMessageNet::OnReceiveNewConnect( UInt64 scrAddr )
{
	AutoUDPConnect udpConn = NewUDPConnect(scrAddr); 
	if (!udpConn)
		return udpConn;

	mUDPConnectList.insert(scrAddr, udpConn);
	return udpConn;
}

void UDPMessageNet::Close()
{
	for (UDPList::iterator it = mUDPConnectList.begin(); it; )
	{
		if (it.get())
			it.get()->Close();
		it.erase();
	}
}

bool UDPMessageNet::BocastPacket(DataStream *sendData, int sendSize, UInt64 exclude)
{
	mBocastList.clear(false);
	for (UDPList::iterator it=mUDPConnectList.begin(); it; ++it)
	{
		if (it.key()!=exclude && it.get())
			mBocastList.push_back(it.key());
	}
	return SendPacket(0, sendData, sendSize, &mBocastList);
}

AutoUDPConnect UDPMessageNet::GetUDPConnect( UInt64 addr )
{
	return mUDPConnectList.find(addr);
}

//-------------------------------------------------------------------------

void DefaultUDPConnect::OnReceivePacket( DataStream *revPacket, int packetSize )
{
	mpUDPNet->OnReceivePacket(mScrAddr, revPacket, packetSize);
}
