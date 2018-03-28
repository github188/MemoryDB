#include "IOCPUDPNet.h"
#include "WorkThread.h"
#include "IOCPCommon.h"
#include "SocketAPI.h"
#include "TimeManager.h"
#include <MSWSock.h>

struct UDPRevOverlapped : public RecvOverlapped
{
	char mRevBuffer[MAX_UDP_MSG_SIZE + _UDP_EX_DATA_LENGTH];
	short mRevSize;
	sockaddr_in mAddr;
	int mAddrLen;

	UDPRevOverlapped()
		: mRevSize(0)
	{
		WsaBuf.buf = mRevBuffer;
		WsaBuf.len = MAX_UDP_MSG_SIZE + _UDP_EX_DATA_LENGTH;
	}
};

struct UDPSendOverlapped : public SendOverlapped
{
	char mSendBuffer[MAX_UDP_MSG_SIZE + _UDP_EX_DATA_LENGTH];
	UInt64 mSendAddr;
	short mSendSize;

	UDPSendOverlapped()
		: mSendSize(0)
		, mSendAddr(0)
	{
		WsaBuf.buf = mSendBuffer;
		WsaBuf.len = MAX_UDP_MSG_SIZE + _UDP_EX_DATA_LENGTH;
	}
};

class UDPSendData : public TempDataBuffer
{
	IOCPUDPNet *mpNet;
	UDPSendOverlapped *mpSendData;

	friend class IOCPUDPNet;
public:
	~UDPSendData()
	{
		if (mpNet!=NULL && mpSendData!=NULL)
		{
			if (dataSize() > 0)
			{
				mpSendData->WsaBuf.len = dataSize();
				mpNet->SendTo(mpSendData);
			}
			else
			{
				mpNet->mSendLock.lock();
				mpNet->mFreeSendList.push(mpSendData);
				mpNet->mSendLock.unlock();
			}
		}
		
	}

	UDPSendData()
		: mpNet(NULL)
		, mpSendData(NULL)
		, TempDataBuffer(0, 0)
	{

	}

	UDPSendData(IOCPUDPNet *pNet, UDPSendOverlapped *pSendData)
		: TempDataBuffer(pSendData->mSendBuffer, MAX_UDP_MSG_SIZE + _UDP_EX_DATA_LENGTH)
		, mpNet(pNet)
		, mpSendData(pSendData)
	{

	}
};

typedef Auto<UDPSendData> AUDPSendData;

class UDPCompletionThread : public WorkThread
{
public:
	UDPCompletionThread(IOCPUDPNet *pNet)
		: mpNet(pNet)
	{

	}

	virtual void backWorkThread() override
	{
		while (IsActive())
		{
			mpNet->_ProcessCompletion();
		}
	}

protected:
	IOCPUDPNet *mpNet;
};

IOCPUDPNet::IOCPUDPNet()
	: mpCompletionThread(NULL)
	, m_hCompletionPort(NULL)
	, mSocketID(INVALID_SOCKET)
	, mbHaveRev(false)
	, mbClosed(false)
	, mPort(0)
{


}

IOCPUDPNet::~IOCPUDPNet()
{
	Close();

	for (int i = 0; i < mAllRevList.size(); ++i)
	{
		delete mAllRevList[i];
	}
}


bool IOCPUDPNet::Start(const char *szIP, int nPort, int threadCount)
{
	Close();
	mIp = szIP;
	mPort = nPort;

	mSocketID = (int)SocketAPI::socket_ex(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	DWORD dw = 0;
	BOOL b = FALSE;
	DWORD status = WSAIoctl(mSocketID, SIO_UDP_CONNRESET, &b, sizeof(b), NULL, 0, &dw, NULL, NULL);
	if (status == SOCKET_ERROR)
	{
		DWORD err = GetLastError();
		ERROR_LOG("Init udp socket error %u", err);
	}
	else
	{
		SocketAPI::setsocketnonblocking_ex(mSocketID, TRUE);
		// 创建IO完成端口
		m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
		if (m_hCompletionPort == NULL)
		{
			TABLE_LOG("NET ERROR: CreateIoCompletionPort");
			return false;
		}
	}

    sockaddr_in	sAddr;
    memset(&sAddr, 0, sizeof(sAddr));
    sAddr.sin_family = AF_INET;
    if (szIP==NULL || strlen(szIP)<=0)
        sAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        sAddr.sin_addr.s_addr = inet_addr(szIP);
    sAddr.sin_port = htons(nPort);

    BOOL result = SocketAPI::bind_ex(mSocketID, (const struct sockaddr *)&sAddr, sizeof(sAddr));
    if (result)
	{
		mbClosed = false;
		mIocpHandle = CreateIoCompletionPort((HANDLE)mSocketID, m_hCompletionPort, (ULONG_PTR)this, 0);
		if (mIocpHandle == NULL)
		{
			ERROR_LOG("NET ERROR: [%s:%d] CreateIoCompletionPort", mIp.c_str(), nPort);
			Close();
			return false;
		}
		for (int i = 0; i < threadCount; ++i)
		{
			WorkThread *pThread = MEM_NEW UDPCompletionThread(this);
			mpCompletionThread.push_back(pThread);
			pThread->InitThread();
		}
        _UDP_LOCK_RECEIVE_DATA
		for (int i=0; i<IOCP_POST_UDP_RECEIVE_COUNT; ++i)
		{
            UDPRevOverlapped *pRev = new UDPRevOverlapped();
            mAllRevList.push_back(pRev);
			_BeginReceive(pRev);
		}

	}
    NOTE_LOG("UDP start [%s:%d] %s", mIp.c_str(), mPort, result==TRUE ? "OK":"FAIL");
	return result == TRUE;
}

void IOCPUDPNet::Close()
{
	mbClosed = true;
	mpCompletionThread.clear(false);
	mIocpHandle = NULL;

	if (mSocketID != INVALID_SOCKET)
    {
        NOTE_LOG("UDP [%s:%d] closing", mIp.c_str(), mPort);
		SocketAPI::closesocket_ex(mSocketID);
    }
	mSocketID = INVALID_SOCKET;

	UInt64 now = TimeManager::NowTick();

	bool bHaveRev = false;
	mRevDataLock.lock();
	bHaveRev = !mAllRevList.empty();
	mRevDataLock.unlock();
	while (bHaveRev)
	{
		Process();

		if (TimeManager::NowTick() - now > 3000)
		{
			ERROR_LOG("Close all rev over time 3000");
			break;
		}
		mRevDataLock.lock();
		for (int i = 0; i < mAllRevList.size(); ++i)
		{
			if (mAllRevList[i]->mbWaiting)
			{
				mRevDataLock.unlock();
				TimeManager::Sleep(10);
				continue;
			}
		}
		mRevDataLock.unlock();
		break;
	}
	now = TimeManager::NowTick();
	bool bHaveSend = false;
	mSendLock.lock();
	bHaveRev = !mAllSendBufferList.empty();
	mSendLock.unlock();
	while (bHaveSend)
	{
		Process();
		if (TimeManager::NowTick() - now > 3000)
		{
			ERROR_LOG("Close all send over time 3000");
			break;
		}
		mSendLock.lock();
		for (int i = 0; i < mAllSendBufferList.size(); ++i)
		{
			if (mAllSendBufferList[i]->mbWaiting)
			{
				mSendLock.unlock();
				TimeManager::Sleep(10);
				continue;
			}
		}
		mSendLock.unlock();
		break;
	}

	if (m_hCompletionPort != NULL)
		CloseHandle(m_hCompletionPort);
	m_hCompletionPort = NULL;

	for (int i = 0; i < mpCompletionThread.size(); ++i)
	{
		delete mpCompletionThread[i];
	}

	//NOTE_LOG("*** UDP [%s:%d] closed", mIp.c_str(), mPort);
}

bool IOCPUDPNet::Send(UInt64 targetAddr, const char *szSendData, int sendSize)
{
	if (sendSize <= 0 || sendSize > MAX_UDP_MSG_SIZE + _UDP_EX_DATA_LENGTH)
	{
		ERROR_LOG("Send data size error %d", sendSize);
		return false;
	}
	UDPSendOverlapped *pSendOverlapped = NULL;
	
	mSendLock.lock();
	if (mFreeSendList.empty())
	{
		pSendOverlapped = new UDPSendOverlapped();
		mAllSendBufferList.push_back(pSendOverlapped);
	}
	else
		pSendOverlapped = mFreeSendList.pop();
	mSendLock.unlock();

	if (memcpy_s(pSendOverlapped->mSendBuffer, MAX_UDP_MSG_SIZE + _UDP_EX_DATA_LENGTH, szSendData, sendSize) != 0)
	{
		mSendLock.lock();
		mFreeSendList.push(pSendOverlapped);
		mSendLock.unlock();
		return false;
	}
	pSendOverlapped->WsaBuf.len = sendSize;
	pSendOverlapped->mSendAddr = targetAddr;
	SendTo(pSendOverlapped);

	return false;
}

AutoData IOCPUDPNet::ReadySend(UInt64 targetAddr)
{
	UDPSendOverlapped *pSendOverlapped = NULL;
	mSendLock.lock();
	if (mFreeSendList.empty())
	{
		pSendOverlapped = new UDPSendOverlapped();
		mAllSendBufferList.push_back(pSendOverlapped);
	}
	else
		pSendOverlapped = mFreeSendList.pop();
	mSendLock.unlock();
	pSendOverlapped->mSendAddr = targetAddr;
	return MEM_NEW UDPSendData(this, pSendOverlapped);
}

void IOCPUDPNet::Process()
{
	if (mbHaveRev)
	{	
		if (mMainReceiveDataList.empty())
		{
			mRevDataLock.lock();
			//if (!mThreadReceiveDataList.empty())
				mMainReceiveDataList.swap(mThreadReceiveDataList);
				mbHaveRev = false;
			mRevDataLock.unlock();
		}

		for (int i = 0; i < mMainReceiveDataList.size(); ++i)
		{			
			UDPRevOverlapped *d = mMainReceiveDataList[i];
			UInt64 revAddr = d->mAddr.sin_addr.s_addr;
			revAddr = revAddr << 32;
			revAddr += ntohs(d->mAddr.sin_port);
			OnReceive(revAddr, d->mRevBuffer, d->mRevSize);
		}
		mRevDataLock.lock();
		for (int i = 0; i < mMainReceiveDataList.size(); ++i)
		{
			UDPRevOverlapped *d = mMainReceiveDataList[i];
			mFreeRevData.push(d);
		}
		mMainReceiveDataList.clear(false);
		mRevDataLock.unlock();
	}
   // if (mbHaveRev)
   // {
   //     mRevDataLock.lock();
   //     for (int i = 0; i < mThreadReceiveDataList.size(); ++i)
   //     {
   //         UDPRevOverlapped *d = mThreadReceiveDataList[i];
   //         UInt64 revAddr = d->mAddr.sin_addr.s_addr;
   //         revAddr = revAddr << 32;
   //         revAddr += ntohs(d->mAddr.sin_port);
   //         OnReceive(revAddr, d->mRevBuffer, d->mRevSize);
			//mFreeRevData.push(d);
   //         //if (!mbClosed)
   //         //    _BeginReceive(d);
   //     }
   //     mbHaveRev = false;
   //     mThreadReceiveDataList.clear(false);
   //     mRevDataLock.unlock();
   // }
}

bool IOCPUDPNet::SendTo(UDPSendOverlapped *pSendData)
{
	pSendData->mbWaiting = true;
	DWORD SendBytes = 0;

	sockaddr_in sAddr;
	static int s = sizeof(SOCKADDR_IN);
	sAddr.sin_family = AF_INET;
	sAddr.sin_addr.s_addr = pSendData->mSendAddr >> 32;
	uint port = (uint)(pSendData->mSendAddr & 0xFFFFFFFF);
	sAddr.sin_port = htons(port);

	if (WSASendTo(mSocketID, &pSendData->WsaBuf, 1, &SendBytes, 0,
		(sockaddr*)&sAddr, s,
		(LPOVERLAPPED)pSendData, NULL) == SOCKET_ERROR)
	{
		int iError = WSAGetLastError();
		if (iError != ERROR_IO_PENDING)
		{
			if (iError == 10054)
			{
				// WSAECONNRESET(10054) 远程客户端强制关闭连接
				TABLE_LOG("[%d] [%s:%d] Other part connect close", iError, mIp.c_str(), mPort);
			}
			else
				ERROR_LOG("IOCPUDPNet::SendTo - WSASend() 发生了如下错误：   %d\n", iError);
			pSendData->mbWaiting = false;
			mSendLock.lock();
			mFreeSendList.push(pSendData);
			mSendLock.unlock();
			return false;
		}
	}
	return true;
}

void IOCPUDPNet::_ProcessCompletion()
{
	while (m_hCompletionPort != NULL)
	{
		PULONG_PTR netConnectPtr = NULL;
		DWORD   BytesTransferred = 0;
		BaseOverlapped* pSOverlapped = NULL;

		if (GetQueuedCompletionStatus(m_hCompletionPort, &BytesTransferred,
			(PULONG_PTR)&netConnectPtr, (LPOVERLAPPED*)&pSOverlapped, 1000) == 0)

		{
			if (netConnectPtr == NULL)
				return;


			DWORD dwLastError = GetLastError();
			if (dwLastError == WAIT_TIMEOUT )
				break;

			WARN_LOG("WARN: IOCP error>[%u]", dwLastError);

			if (dwLastError == ERROR_OPERATION_ABORTED || dwLastError == ERROR_NETNAME_DELETED)
			{
				
					//if (pSOverlapped->IoMode == IoSend || pSOverlapped->IoMode == IoRecv)
					//{
					//	pSOverlapped->mbWaiting = false;				
					//}
					//CloseConnect(pConnect);
			}
			// 64错误号表示"指定的网络名不再可用"，客户端异常退出会产生这个错误号
			else if (dwLastError != WAIT_TIMEOUT)
			{
				
				DWORD dwTrans;
				DWORD dwFlags;
				if (FALSE == WSAGetOverlappedResult(mSocketID, pSOverlapped, &dwTrans, FALSE, &dwFlags))
					ERROR_LOG("GetQueuedCompletionStatus   发生了如下错误： %d\n", WSAGetLastError());
				
					ERROR_LOG("GetQueuedCompletionStatus   发生了如下错误： %d\n", GetLastError());
				//if (pSOverlapped->IoMode == IoSend || pSOverlapped->IoMode == IoRecv)
				//{
				//	pSOverlapped->mbWaiting = false;				
				//}
				//CloseConnect(pConnect);
			}

        }

		if (pSOverlapped == NULL) {
			//printf("pSOverlapped == NULL\n");
			break;
		}

		if (pSOverlapped->IoMode == IoSend)
		{
			// 发送事件
			UDPSendOverlapped* pSendOverlapped = (UDPSendOverlapped*)pSOverlapped;
			if (pSOverlapped->WsaBuf.len != BytesTransferred)
			{
				ERROR_LOG("Send size %u no finish %u", BytesTransferred, pSOverlapped->WsaBuf.len);
			}
			pSendOverlapped->mbWaiting = false;

			mSendLock.lock();
			mFreeSendList.push(pSendOverlapped);
			mSendLock.unlock();
		}
        else if (pSOverlapped->IoMode == IoRecv)
        {
			if (BytesTransferred==0)
			{
				mRevDataLock.lock();
				mFreeRevData.push( (UDPRevOverlapped*)pSOverlapped);
				mRevDataLock.unlock();
			}
			else
			{
				UDPRevOverlapped* pRecvOverlapped = (UDPRevOverlapped*)pSOverlapped;
				pSOverlapped->mbWaiting = false;
				pRecvOverlapped->mRevSize = (short)BytesTransferred;
				mRevDataLock.lock();
				mThreadReceiveDataList.push_back(pRecvOverlapped);
				mbHaveRev = true;
				if (!mbClosed)
				{
					_BeginReceive(NULL);
				}
				mRevDataLock.unlock();
			}
		}
	}

}


bool IOCPUDPNet::_BeginReceive(UDPRevOverlapped *pRev)
{
	if (pRev==NULL)
	{
		if (mFreeRevData.empty())
		{
			pRev = new UDPRevOverlapped();
			mAllRevList.push_back(pRev);
			NOTE_LOG("All rev data count %llu", mAllRevList.size());
		}
		else
			pRev = mFreeRevData.pop();
	}

    pRev->mbWaiting = true;
    DWORD RecvBytes = 0;
    DWORD Flags = 0;
    pRev->mAddrLen = sizeof(SOCKADDR_IN);

    if (WSARecvFrom(mSocketID, &pRev->WsaBuf, 1, &RecvBytes, &Flags,
        (sockaddr*) &pRev->mAddr, &pRev->mAddrLen,
        (LPOVERLAPPED)pRev, NULL) == SOCKET_ERROR)
    {
        int iError = WSAGetLastError();
        if (iError != ERROR_IO_PENDING)
        {
            pRev->mbWaiting = false;
            // 异常错误
            NOTE_LOG("NET ERROR: [%s:%d], [%u]", mIp, mPort, iError);
            return false;
        }
    }

    return true;

}

