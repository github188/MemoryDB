
#include "UDPThreadNet.h"
#include "ServerIPInfo.h"
#include "TimeManager.h"


//-------------------------------------------------------------------------
UDPThreadNet::UDPThreadNet(bool bThreadSend) :mRevAddr(0)
	, mbHaveReceiveData(false)
	, mpNewNode(NULL)
	, mUDPSender(NULL)
{
	mThreadReceiveDataList = MEM_NEW DataList(0);
	mMainReceiveDataList = MEM_NEW DataList(0);

	if (bThreadSend)
		mUDPSender = MEM_NEW UDPSenderTool(this);	
	mpUDPReceiveThread = MEM_NEW UDPReceiveThread(this);
}
//-------------------------------------------------------------------------
UDPThreadNet::~UDPThreadNet()
{
	if (mUDPSender!=NULL)
	{	
		mUDPSender->Close();
		delete mUDPSender;
		mUDPSender = NULL;
	}
	mpUDPReceiveThread->Close();
	delete mpUDPReceiveThread;
	mpUDPReceiveThread = NULL;
	
	if (mpNewNode!=NULL)
	{
		mThreadReceiveDataList->_FreeNode(mpNewNode);
		mpNewNode = NULL;
	}

	delete mThreadReceiveDataList;
	delete mMainReceiveDataList;
	mThreadReceiveDataList = NULL;
	mMainReceiveDataList = NULL;

}
//-------------------------------------------------------------------------
bool UDPThreadNet::StartNet(const char *szIP, int lowPort, int highPort)
{
	mUDP.SetBlock(true);

	bool b = false;
	if (highPort==0)
		b = mUDP.Bind(szIP, lowPort);
	else
	{
		for (int i=lowPort; i<highPort; ++i)
		{
			if (mUDP.Bind(szIP, i))
			{
				b = true;
				break;
			}
		}
	}
	if (b)
	{
		if (highPort<=0 && lowPort<=0)
		{			
			getIpPort(lowPort);
		}
		TABLE_LOG("UDP Login server net [%s:%d] init succeed", szIP, lowPort);
	}
	else
		ERROR_LOG("UDP Login server net [%s:%d] init fail", szIP, lowPort);

	if (b)
		mpUDPReceiveThread->InitThread();

	return b;
}
//-------------------------------------------------------------------------
void UDPThreadNet::Process()
{
	if (mMainReceiveDataList->empty() && mbHaveReceiveData)
	{
		mDataLock.lock();
		DataList *pTemp = mMainReceiveDataList;
		mMainReceiveDataList = mThreadReceiveDataList;
		mThreadReceiveDataList = pTemp;
		mbHaveReceiveData = false;
		mDataLock.unlock();
	}

	static const int headSize = sizeof(UInt64);

	if (!mMainReceiveDataList->empty())
	{
		while (!mMainReceiveDataList->empty())
		{		
			DataList::iterator it=mMainReceiveDataList->begin();
			DataBuffer &d = it.get();
			it.erase();
			short len = d.dataSize()-headSize;			
			UInt64 *pAddr = (UInt64 *)(d.data()+len);
			d.seek(0);
			OnReceiveMsg(*pAddr, &d, len);
			//it.erase();
		}
	}
}
//-------------------------------------------------------------------------
void UDPThreadNet::_ProcessNet()
{
	if (mpNewNode==NULL)
	{
		mDataLock.lock();
		mpNewNode = mThreadReceiveDataList->_NewNode();
		mDataLock.unlock();
		if (mpNewNode->mValue.size()<=0)
			mpNewNode->mValue.resize(MAX_UDP_MSG_SIZE+16);
	}
	static const int headSize = sizeof(UInt64);
	short revLen = (short)mUDP.ProcessReceive(&mpNewNode->mValue, mRevAddr, true);
	if (revLen>0)
	{
		mpNewNode->mValue.seek(revLen);
		mpNewNode->mValue.write(mRevAddr);
		mpNewNode->mValue.setDataSize(headSize+revLen);
		_onThreadReceive(mRevAddr, &mpNewNode->mValue, revLen);
		mDataLock.lock();
		mThreadReceiveDataList->_insert(mpNewNode);
		mpNewNode = NULL;
		mbHaveReceiveData = true;
		mDataLock.unlock();
	}
	//TimeManager::Sleep(1);
}

bool UDPThreadNet::Send( DataStream *sendData, UInt64 destAddr )
{
#if _USE_THREAD_SEND_
	if (mUDPSender!=NULL)
		return mUDPSender->Send(destAddr, sendData->data(), sendData->dataSize());
#endif
	return mUDP.Send(sendData, destAddr);
}

bool UDPThreadNet::Send( DataStream *sendData, const char *szIp, int port )
{
#if _USE_THREAD_SEND_
	UInt64 destAddr = ServerIPInfo::IP2Num(szIp, 0, port);
	if (mUDPSender!=NULL)
		return mUDPSender->Send(destAddr, sendData->data(), sendData->dataSize());
#endif
	return mUDP.Send(sendData, szIp, port);
}

bool UDPThreadNet::SendPacket(DataStream *sendData, Array<UInt64> &destAddr, UInt64 packetCode)
{
#if _USE_THREAD_SEND_
	if (mUDPSender!=NULL)
		return mUDPSender->Send(destAddr, sendData->data(), sendData->dataSize(), true, packetCode);
#endif
	AssertNote(0, "不支持直接发送消息包");
	return false;
}

void UDPThreadNet::Close()
{
	mpUDPReceiveThread->Close();
	if (mUDPSender!=NULL)
		mUDPSender->Close();
	mUDP.Close();
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
UDPSenderTool::UDPSenderTool( UDPThreadNet *pUDP ) : mOwnerUDPNet(pUDP)
{
	mSendThreadEvent = event_create(false, true);
	mThreadSendList = MEM_NEW SendList(0);
	mMainSendList = MEM_NEW SendList(0);

	WorkThread::InitThread();
}

UDPSenderTool::~UDPSenderTool()
{
	WorkThread::Close();
	delete mThreadSendList;
	delete mMainSendList;
	mThreadSendList = NULL;
	mMainSendList = NULL;

	event_destroy(mSendThreadEvent);
}

bool UDPSenderTool::Send( UInt64 targetAddr, const char *szSendData, int sendSize )
{
	if (sendSize<=0)
		return true;

	mSendLock.lock();
	SendList::Node *pNewNode = mMainSendList->_NewNode();
	pNewNode->mValue.mSendAddr.push_back(targetAddr);
	pNewNode->mValue.mbIsPacket = false;
	pNewNode->mValue.mSendBuffer.clear();
	pNewNode->mValue.mSendBuffer._write((void*)szSendData, sendSize);
	mMainSendList->_insert(pNewNode);
	mSendLock.unlock();

	event_set(mSendThreadEvent);

	return true;
}

bool UDPSenderTool::Send(Array<UInt64> &targetAddr, const char *szSendData, int sendSize, bool bPacket, UInt64 packetCode)
{
	if (sendSize<=0)
		return true;

	mSendLock.lock();
	SendList::Node *pNewNode = mMainSendList->_NewNode();	
	pNewNode->mValue.mSendAddr.swap(targetAddr);
	pNewNode->mValue.mbIsPacket = bPacket;
	pNewNode->mValue.mPacketCode = packetCode;
	pNewNode->mValue.mSendBuffer.clear();
	pNewNode->mValue.mSendBuffer._write((void*)szSendData, sendSize);
	mMainSendList->_insert(pNewNode);
	mSendLock.unlock();

	event_set(mSendThreadEvent);

	return true;
}

void UDPSenderTool::backWorkThread( void )
{
	while (IsActive())
	{
		event_timedwait(mSendThreadEvent, 10000);
		if (!mThreadSendList->empty())
			_ProcessSendThread();

		mSendLock.lock();
		SendList *pTemp = mMainSendList;
		mMainSendList = mThreadSendList;
		mThreadSendList = pTemp;
		mSendLock.unlock();

		_ProcessSendThread();
	}
}

void UDPSenderTool::_ProcessSendThread()
{
	static __declspec(thread) int x = 0;
	while (!mThreadSendList->empty())
	{
		SendList::iterator it = mThreadSendList->begin();
		SendData &sendData = *it;

		if (sendData.mbIsPacket)
		{
			if (!mOwnerUDPNet->mUDP.SendPacket(sendData.mPacketCode, &(sendData.mSendBuffer), sendData.mSendBuffer.dataSize(), sendData.mSendAddr, _UDP_SENDTHREAD_ONCE_TIME))
				ERROR_LOG("UDP thread send udp data fail");
		}
		else
		{
			for (int i=0; i<sendData.mSendAddr.size(); ++i)
			{
				if (!mOwnerUDPNet->mUDP.Send(&(sendData.mSendBuffer), sendData.mSendAddr[i]))
					ERROR_LOG("UDP thread send udp data fail");
			}
			//TimeManager::Sleep(_UDP_SENDTHREAD_ONCE_TIME);
		}

		sendData.mSendAddr.clear(false);

		TABLE_LOG("%d Send >[%d]", ++x, sendData.mSendBuffer.dataSize()); 
		it.erase();

	}
}
//-------------------------------------------------------------------------