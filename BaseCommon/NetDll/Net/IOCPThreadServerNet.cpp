#include "IOCPThreadServerNet.h"

#include "IOCPCommon.h"
#include <WinSock2.h>
#include "TimeManager.h"
#include "ServerIPInfo.h"
//-------------------------------------------------------------------------*/
// 监听
class ListenThreadEx : public WorkThread
{
	friend class IOCPProcessThread;
	friend class IOCPThreadServerNet;
public:
	ListenThreadEx(IOCPThreadServerNet *pServerNet)
		: mListenSocket(INVALID_SOCKET)
		, mbHasConnect(false)
		, mpThreadServerNet(pServerNet)
	{		

	}

	virtual ~ListenThreadEx()
	{
		Close();
		if (m_hCompletionPort!=0)
		{
			CloseHandle(m_hCompletionPort);
			m_hCompletionPort = NULL;
		}
	}

	bool InitStart(const char* szIp, int nPort)
	{
		// 创建一个监听用的Socket 
		// 第3个参数IPPROTO_TCP
		mListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (mListenSocket == INVALID_SOCKET)
		{
			ERROR_LOG("[%s:%d]创建监听Socket失败", szIp, nPort);
			ErrorExit("WSASocket");
			return false;   
		}
		INT opt = 1;
		SocketAPI::setsockopt_ex( mListenSocket , SOL_SOCKET , SO_REUSEADDR , &opt , sizeof(opt) );

		// 绑定IP和端口
		SOCKADDR_IN SockAddr;
		SockAddr.sin_family = AF_INET;
		//SockAddr.sin_addr.s_addr = inet_addr(localIP);	// 只允许指定IP的客户端连接
		SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		SockAddr.sin_port = htons(nPort);
		if ( bind(mListenSocket, (SOCKADDR*)&SockAddr, sizeof(SockAddr)) == SOCKET_ERROR )
		{   
			ERROR_LOG("[%s:%d]绑定监听端口Socket失败", szIp, nPort);
			ErrorExit("bind");
			return false;   
		}

		// 准备监听Socket连接
		if ( listen(mListenSocket, SOMAXCONN) == SOCKET_ERROR )
		{   
			ERROR_LOG("[%s:%d] Socket 开启监听 失败", szIp, nPort);
			ErrorExit("listen");
			return false;   
		}

		WorkThread::InitThread();
		printf("NET: 初始网络服务成功 [%s:%d]\r\n", szIp, nPort);

		// 创建完成端口
		m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
		if (m_hCompletionPort == NULL)
		{
			TABLE_LOG("NET ERROR: CreateIoCompletionPort");
			return false; 
		}

		return true;
	}

	virtual void Close() override
	{
		_ForceClose(10);		
		mConnectListLock.lock();

		for (int i=0; i<mConnectList.size(); ++i)
		{
			SocketAPI::closesocket_ex(mConnectList[i].mSocket);
		}

		mConnectList.clear();
		mMainConnectList.clear();
		mConnectListLock.unlock();
		WorkThread::Close();

		if (mListenSocket!=0)
			SocketAPI::closesocket_ex(mListenSocket);
		mListenSocket = INVALID_SOCKET;
	}

	void ErrorExit(const char *szInfo)
	{
		NOTE_LOG("NET ERROR: [%s]", szInfo);
	}

public:
	// NOTE: 在后台线程内调用
	virtual void backWorkThread()
	{
		sockaddr saClient;
		//SocketInfo info;

		while(IsActive())
		{
			// 处理连接请求
			int iClientSize = sizeof(saClient);
			SOCKET acceptSocket = WSAAccept(mListenSocket, &saClient, &iClientSize, NULL, NULL);
			if ( acceptSocket == SOCKET_ERROR )
			{
				// 异常错误事件
				OnError(INVALID_ID, WSAGetLastError());
				continue;
			}						

			mpThreadServerNet->OnAcceptConnect(acceptSocket);

			//mConnectListLock.lock();
			//info.mSocket = acceptSocket;
			//info.mAddr = saClient;
			//mConnectList.push_back(info);
			//mbHasConnect = true;
			//mConnectListLock.unlock();
		}
	}
	void OnError(int netID, int error){}

protected:
	IOCPThreadServerNet *mpThreadServerNet;
	SOCKET				mListenSocket;

	CsLock				mConnectListLock;
	Array<SocketInfo>	mConnectList;
	Array<SocketInfo>	mMainConnectList;
	bool				mbHasConnect;

private:
	void				*m_hCompletionPort;
};
//-------------------------------------------------------------------------*/

void IOCPProcessThread::backWorkThread(void)
{
	while (IsActive())
	{
		// 处理完成端口
		while(mpListen->m_hCompletionPort!=NULL) 
		{
			Int64 netConnectPtr = NULL;
			DWORD   BytesTransferred = 0;
			BaseOverlapped* pSOverlapped = NULL;
//#if _IOCP_THREAD_RECEIVE_SEND
			if ( GetQueuedCompletionStatus(mpListen->m_hCompletionPort, &BytesTransferred,   
				(PULONG_PTR)&netConnectPtr, (LPOVERLAPPED*)&pSOverlapped, INFINITE) == 0 ) 
//#else
//			if ( GetQueuedCompletionStatus(mpListen->m_hCompletionPort, &BytesTransferred,   
//				(PULONG_PTR)&netConnectPtr, (LPOVERLAPPED*)&pSOverlapped, 0) == 0 ) 
//#endif 
			{   
				//if (netConnectPtr==NULL)
				//	break;

				CsLockTool l(mpListen->mpThreadServerNet->mLock);				
				if (netConnectPtr>=(Int64)mpListen->mpThreadServerNet->mConnectList.size())
					break;

				IOCPThreadConnect *pConnect = mpListen->mpThreadServerNet->mConnectList[netConnectPtr].mpConnect;						

				DWORD dwLastError = GetLastError();
				if (dwLastError==WAIT_TIMEOUT || pConnect==NULL)
					break;

				WARN_LOG("WARN: IOCP error>[%u]", dwLastError);

				if (pSOverlapped!=NULL)
					pSOverlapped->mbWaiting = false;

				if (dwLastError==ERROR_OPERATION_ABORTED || dwLastError==ERROR_NETNAME_DELETED)
				{			
					//???_IOCP_AUTO_LOCK_CONNECT_EX
						//if (pSOverlapped->IoMode == IoSend || pSOverlapped->IoMode == IoRecv)
						//{
						//	pSOverlapped->mbWaiting = false;				
						//}
						pConnect->Close();
				}
				// 64错误号表示"指定的网络名不再可用"，客户端异常退出会产生这个错误号
				else if (dwLastError!=WAIT_TIMEOUT ) 
				{
					//???_IOCP_AUTO_LOCK_CONNECT_EX
						Log("GetQueuedCompletionStatus   发生了如下错误： %d\n",   GetLastError());
					//if (pSOverlapped->IoMode == IoSend || pSOverlapped->IoMode == IoRecv)
					//{
					//	pSOverlapped->mbWaiting = false;				
					//}
					pConnect->Close();
				}				

				break;
			}

			if (pSOverlapped == NULL) {
				printf("pSOverlapped == NULL\n");
				break;
			}

			mpListen->mpThreadServerNet->mLock.lock();
			if (netConnectPtr>=(Int64)mpListen->mpThreadServerNet->mConnectList.size())
			{
				mpListen->mpThreadServerNet->mLock.unlock();
				break;
			}
			IOCPThreadConnect *pConnect = mpListen->mpThreadServerNet->mConnectList[netConnectPtr].mpConnect;				
			CsLockTool l(mpListen->mpThreadServerNet->mConnectList[netConnectPtr].mConnectLock);
			mpListen->mpThreadServerNet->mLock.unlock();
			//IOCPThreadConnect *pConnect = (IOCPThreadConnect*)netConnectPtr;		

			if (pSOverlapped->IoMode == IoSend) 
			{
				//???_IOCP_AUTO_LOCK_CONNECT_EX
					// 发送事件
					SendOverlapped* pSendOverlapped = (SendOverlapped*)pSOverlapped;
				if (pSendOverlapped->dwSentBytes + BytesTransferred < pSendOverlapped->dwTotalBytes) 
				{
					if (pConnect->mSocketID!=0)
					{
						//printf("========出现发送数据不完整的情况！！！========\n");
						pSendOverlapped->dwSentBytes += BytesTransferred;
						pSendOverlapped->WsaBuf.buf = &pSendOverlapped->buffer[pSendOverlapped->dwSentBytes];
						pSendOverlapped->WsaBuf.len = pSendOverlapped->dwTotalBytes - pSendOverlapped->dwSentBytes;

						// 发送事件
						DWORD SendBytes = 0;
						if ( WSASend(pConnect->mSocketID, &pSendOverlapped->WsaBuf, 1, &SendBytes, 0,
							(LPOVERLAPPED)pSendOverlapped, NULL)   ==   SOCKET_ERROR )
						{   
							int iError = WSAGetLastError();
							if ( iError != ERROR_IO_PENDING )
							{   
								pSendOverlapped->mbWaiting = false;
								TABLE_LOG("IOCPServer::ServerWorkerThread - WSASend() 发生了如下错误：   %d\n",   iError);

								// 关闭SOCKET
								pConnect->Close();
								continue;
							}   
						}  
					}
					else
						pSendOverlapped->mbWaiting = false;
				} 
				else 
				{
					//??? NOTE_LOG(">>> 发送 = %d", BytesTransferred);
					//pServer->OnSend(nClient, pSendOverlapped->pBuffer, pSendOverlapped->dwTotalBytes);
					pSendOverlapped->mbWaiting = false;
					if (pConnect!=NULL)
						pConnect->_SendTo();				
				}

			} 
			else if (pSOverlapped->IoMode == IoRecv) 
			{			
				//???_IOCP_AUTO_LOCK_CONNECT_EX

					pSOverlapped->mbWaiting = false;

				if (BytesTransferred == 0) 
				{							
					if (pConnect!=NULL)
					{
						TABLE_LOG("NET: May be other party connect close [%s:%d], Now set remove", pConnect->GetIp(), pConnect->GetPort());
						pConnect->Close();
						break;
					}
					//CloseConnect(netID);
					//pServer->Disconnect(nClient);					
				}
				else
				{	
					//??? NOTE_LOG("Rev >[%u]\n", BytesTransferred);
					// 接收事件
					RecvOverlapped* pRecvOverlapped = (RecvOverlapped*)pSOverlapped;
					char* pBuf = pRecvOverlapped->buffer;

					if (pConnect!=NULL)
					{
						//???_IOCP_AUTO_LOCK_RECEIVE_EX

							//AddReceiveTotalSize(BytesTransferred);
						if (!pConnect->IsRemove())
						{
							//if (pConnect->mReceiveData.dataSize()+BytesTransferred>(uint)ReceiveBufferMax())
							//{
							//	NOTE_LOG("[%s:%d] 接收数据太大 超过[%d]，移除", pConnect->GetIp(), pConnect->GetPort(), MAX_RECEIVE_DATA_SIZE);
							//	pConnect->SetRemove(true);
							//}
							//else
							{
								pConnect->OnReceiveData(pBuf, BytesTransferred);
								//OnReceiveData(pConnect, pBuf, BytesTransferred);
								//pConnect->mReceiveData.Write(pBuf, BytesTransferred);	
								pConnect->_PostReceive();

//#if _IOCP_THREAD_RECEIVE_SEND
//								// 直接在接收线程解析接收到的消息数据
//								if (pConnect->_checkOk())
//									EventNetProtocol::ReadAnalyzeEventData(pConnect, &pConnect->mReceiveData);
//#endif
							}
						}
					}	
				}			
			}

			//if (TimeManager::NowTick()!=nowTime)
			//	break;
		}
		TimeManager::Sleep(1);
	}
}

bool IOCPThreadServerNet::SendData(int connectID, void *checkParam, const char *pSendData, int sendSize)
{
	mLock.lock();
	if (connectID>=0 && connectID<mConnectList.size())
	{
		ConnectData *pData = NULL;
		if (mConnectList[connectID].mpConnect!=NULL && CheckConnect(mConnectList[connectID].mpConnect, checkParam) )
		{
			//mConnectList[connectID].mConnectLock.lock();
			bool b = mConnectList[connectID].mpConnect->Send((void*)pSendData, sendSize);
			//mConnectList[connectID].mConnectLock.unlock();
			mLock.unlock();
			return b;
		}
	}
	mLock.unlock();
	return false;
}

void IOCPThreadServerNet::ProcessReceive(int connectID, void *checkParam, void *processTool)
{
	mLock.lock();
	if (connectID>=0 && connectID<mConnectList.size())
	{
		if (mConnectList[connectID].mpConnect!=NULL && CheckConnect(mConnectList[connectID].mpConnect, checkParam) )
		{
			mConnectList[connectID].mConnectLock.lock();
			mLock.unlock();
			mConnectList[connectID].mpConnect->ProcessReceive(processTool);
			mConnectList[connectID].mConnectLock.unlock();
			return;
		}
	}
	mLock.unlock();
}

//-------------------------------------------------------------------------*/
IOCPThreadConnect* IOCPThreadServerNet::CreateConnect()
{
	return MEM_NEW  IOCPThreadConnect(this);
}

//bool IOCPThreadServerNet::AppendConnect(IOCPThreadConnect *pConnect)
//{
//	pConnect->LeaveNet();
//mLock.lock();
//	for (int i=0; i<mConnectList.size(); ++i)
//	{
//		if (mConnectList[i]==NULL)
//		{
//			mConnectList[i] = pConnect;
//			mLock.unlock();
//			return true;
//		}
//	}
//	mConnectList.push_back(pConnect);
//mLock.unlock();
//	return true;
//}

void IOCPThreadServerNet::OnAcceptConnect(size_t connectSocket)
{
	IOCPThreadConnect *pConnect = CreateConnect();
	pConnect->mSocketID = connectSocket;
	pConnect->mPort = pConnect->GetLocalIPPort(pConnect->mIp);

	printf("New connect [%s:%d]\r\n", pConnect->GetIp(), pConnect->GetPort());
	//AppendConnect(pConnect);
	mLock.lock();
	for (int i=0; i<mConnectList.size(); ++i)
	{
		if (mConnectList[i].mpConnect==NULL)
		{
			pConnect->mNetID = i;
			mConnectList[i].mpConnect = pConnect;

			// 绑定到完成端口
			CreateIoCompletionPort((HANDLE)connectSocket, mpListen->m_hCompletionPort, (ULONG_PTR)pConnect->mNetID, 0);
			pConnect->_PostReceive();

			mLock.unlock();
			return;
		}
	}
	ConnectData d;
	d.mpConnect = pConnect;
	pConnect->mNetID = mConnectList.size();
	mConnectList.push_back(d);

	// 绑定到完成端口
	CreateIoCompletionPort((HANDLE)connectSocket, mpListen->m_hCompletionPort, (ULONG_PTR)pConnect->mNetID, 0);
	pConnect->_PostReceive();

	mLock.unlock();
}

bool IOCPThreadServerNet::InitStart(const char *szIp, int port, int threadCount /*= 2*/)
{
	Close();

	mpListen = MEM_NEW ListenThreadEx(this);
	if (!mpListen->InitStart(szIp, port))
	{
		delete mpListen;
		mpListen = NULL;
		return false;
	}
	mIOCPThreadList.resize(threadCount);
	for (int i=0; i<mIOCPThreadList.size(); ++i)
	{
		mIOCPThreadList[i] = MEM_NEW IOCPProcessThread(mpListen);
		mIOCPThreadList[i]->InitThread();
	}
	WorkThread::InitThread();
	return true;
}

void IOCPThreadServerNet::Close()
{
	WorkThread::Close();
	while (!CloseAllConnect(6))
	{
		ERROR_LOG("Net close all connect over time 6 sec");
		TimeManager::Sleep(10);
	}
	mLock.lock();
	mConnectList.clear();
	mLock.unlock();
	for (int i=0; i<mIOCPThreadList.size(); ++i)
	{
		mIOCPThreadList[i]->Close();
		delete mIOCPThreadList[i];
	}
	mIOCPThreadList.clear();

	if (mpListen!=NULL)
	{
		mpListen->Close();
		mpListen = NULL;
	}
	printf("Server net close finish\r\n");
}

bool IOCPThreadServerNet::CloseAllConnect(int waitSecondTime /*= 10*/)
{
	mLock.lock();
	for (int i=0; i<mConnectList.size(); ++i)
	{
		IOCPThreadConnect *pConnect = mConnectList[i].mpConnect;
		if (pConnect!=NULL)
			pConnect->SetRemove(true);
	}
	mLock.unlock();

	UInt64 lastTime = TimeManager::Now();
	while (true)
	{
		Process();
		bool bAll = true;
		mLock.lock();
		for (int i=0; i<mConnectList.size(); ++i)
		{
			if (mConnectList[i].mpConnect!=NULL)
			{
				bAll = false;
				break;
			}
		}
		mLock.unlock();
		if (bAll)
			return true;
		TimeManager::Sleep(10);
		if (TimeManager::Now()-lastTime>waitSecondTime)
			return false;
	}
	return false;
}

void IOCPThreadServerNet::RemoveConnect(IOCPThreadConnect *pConn, bool pDelete /*= true*/)
{
	if (pDelete)
	{
		pConn->SetRemove(true);			
	}
	//_RemoveConnect(pConn, pDelete);
}

void IOCPThreadServerNet::Process()
{
	mLock.lock();
	for (int i=0; i<mConnectList.size(); ++i)
	{
		ConnectData &connData = mConnectList[i];
		if (connData.mpConnect!=NULL && connData.mpConnect->CheckRemove())
		{
			connData.mConnectLock.lock();
			delete connData.mpConnect;
			connData.mpConnect = NULL;
			connData.mConnectLock.unlock();
		}
	}
	mLock.unlock();
}

void IOCPThreadServerNet::backWorkThread()
{
	while (IsActive())
	{
		Process();
		TimeManager::Sleep(500);
	}
}

void IOCPThreadConnect::OnReceiveData(void *pData, int size)
{
	DWORD id = GetCurrentThreadId();
	printf("IOCP receive data [%d], Thread %lu\r\n", size, id);
}

//void IOCPThreadServerNet::_RemoveConnect(IOCPThreadConnect *pConn, bool pDelete)
//{
//	mLock.lock();
//	for (int i=0; i<mConnectList.size(); ++i)
//	{
//		if (mConnectList[i]==pConn)
//		{
//			mConnectList[i] = NULL;
//			break;
//		}
//	}
//	mLock.unlock();
//}

void IOCPThreadConnect::_SendTo()
{
	if (mSocketID!=0 && !mNeedRemove)
	{		
		if (mSendOverlapped->mbWaiting)
		{
			if (TimeManager::Now()-mLastSendTime>_IOCP_SEND_OVERTIME)
			{
				WARN_LOG("[%d][%s:%d]连接发送超时>[%d] Sec, 强制关闭连接", GetNetID(), GetIp(), GetPort(), _IOCP_SEND_OVERTIME);
				SetRemove(true);
				return;
			}
		}
		else
		{				
			//CsLockTool l(mSendLock);
			UINT s = mSendData.Length();
			if (s<=0)
			{			
				return;
			}
			UINT len = mSendData.Read(mSendOverlapped->buffer, s<MAX_PACKAGE_SIZE ? s : MAX_PACKAGE_SIZE );
			if (len <= 0)
			{			
				return;			
			}
			mSendOverlapped->mbWaiting = true;		
			mLastSendTime = TimeManager::Now();
			// 创建一个发送完成事件
			SendOverlapped* pSendOverlapped = mSendOverlapped;

			pSendOverlapped->WsaBuf.buf = pSendOverlapped->buffer;
			pSendOverlapped->WsaBuf.len = len;
			pSendOverlapped->dwSentBytes = 0;
			pSendOverlapped->dwTotalBytes = len;

			// 发送事件
			DWORD SendBytes = 0;
			if ( WSASend(mSocketID, &pSendOverlapped->WsaBuf, 1, &SendBytes, 0,
				(LPOVERLAPPED)pSendOverlapped, NULL)   ==   SOCKET_ERROR )
			{   
				int iError = WSAGetLastError();
				if ( iError != ERROR_IO_PENDING )
				{   
					if (iError==10054)
					{
						// WSAECONNRESET(10054) 远程客户端强制关闭连接
						TABLE_LOG("[%d] [%s:%d] Other part connect close", iError, GetIp(), GetPort());
					}
					else
						ERROR_LOG("IOCPServer::SendData - WSASend() 发生了如下错误：   %d\n",   iError);
					// 异常错误事件
					//OnError(nClient, iError);
					// 关闭SOCKET
					mSendOverlapped->mbWaiting = false;
					Close();
					//Disconnect(nClient);
					return;
				}   
			}   		
		}
	}
}

void IOCPThreadConnect::_PostReceive()
{
	if (mNeedRemove)
	{		
		return;
	}
	//mConnectLock.lock();
	if (mSocketID!=0 && !mRecvOverlapped->mbWaiting)
	{		
		DWORD RecvBytes = 0;
		DWORD Flags = 0; 
		RecvOverlapped* pRecvOverlapped = mRecvOverlapped;

		if ( WSARecv(mSocketID, &pRecvOverlapped->WsaBuf, 1, &RecvBytes, &Flags,
			(LPOVERLAPPED)pRecvOverlapped, NULL)   ==   SOCKET_ERROR )   
		{   
			int iError = WSAGetLastError();
			if ( iError != ERROR_IO_PENDING )
			{   
				// 异常错误
				TABLE_LOG("NET ERROR: [%d], [%u]", GetNetID(), iError);
				SetRemove(true);
				//mConnectLock.unlock();
				return;
			}   
		}

		mRecvOverlapped->mbWaiting = true;
	}
	//mConnectLock.unlock();
}

void IOCPThreadConnect::SetRemove(bool bRemove)
{
	CsLock *pLock = getLock();
	if (pLock!=NULL)
		pLock->lock();
	mNeedRemove = bRemove;

	if (mNeedRemove)
	{		
		if (mSocketID!=0)
		{		
			if (SocketAPI::closesocket_ex(mSocketID)==FALSE)
				//if (closesocket(mSocketID) == SOCKET_ERROR) 
			{   
				ERROR_LOG("Close socket error >[%u]", GetLastError());				
			}				
			mSocketID = 0;
		}	
	}

	//if (CheckRemove())
	//{
	//	mpNet->_RemoveConnect(this, true);
	//	return;
	//}
	if (pLock!=NULL)
		pLock->unlock();
}

bool IOCPThreadConnect::CheckRemove()
{
	if (!mNeedRemove || mSendOverlapped->mbWaiting)
		return false;

	if (mRecvOverlapped->mbWaiting)
		return false;
	
	return true;
}

int IOCPThreadConnect::GetLocalIPPort(char *ip)
{
	struct sockaddr addr;
	struct sockaddr_in* addr_v4;
	int addr_len = sizeof(addr);
	int port = 0;
	//获取remote ip and port
	ZeroMemory(&addr, sizeof(addr));
	if (0 == getpeername(mSocketID, &addr, &addr_len))
	{
		if (addr.sa_family == AF_INET)
		{
			addr_v4 = (sockaddr_in*)&addr;
			port = ntohs(addr_v4->sin_port);
			strcpy(ip, ServerIPInfo::Num2IP(addr_v4->sin_addr.S_un.S_addr).c_str());			
		}
	}
	return port;

	//unsigned short port = 0;

	//struct sockaddr_storage sa;  
	//int salen = sizeof(sa);  

	//if (::getsockname((SOCKET)mSocketID, (struct sockaddr*)&sa, &salen) == -1) {  
	//	ip = "?";  
	//	port = 0;  
	//	return port;  
	//}  

	//if (sa.ss_family == AF_INET) {  
	//	struct sockaddr_in *s = (struct sockaddr_in*)&sa;  
	//	strcpy(ip, ::inet_ntoa(s->sin_addr));  
	//	port = ::ntohs(s->sin_port);  
	//	return port;  
	//}  
	//return 0;
}

CsLock* IOCPThreadConnect::getLock()
{
	mpNet->mLock.lock();
	if (mNetID>=0 && mNetID<mpNet->mConnectList.size() && mpNet->mConnectList[mNetID].mpConnect==this)
	{
		mpNet->mLock.unlock();
		return &mpNet->mConnectList[mNetID].mConnectLock;
	}
	return NULL;
}

//-------------------------------------------------------------------------*/
IOCPThreadConnect::IOCPThreadConnect(IOCPThreadServerNet *pNet) 
	: mSocketID(0)
	, mLastSendTime(0)
	, mNeedRemove(false)
	, mpNet(pNet)
	, mNetID(-1)
{
	mSendOverlapped = new SendOverlapped();
	mRecvOverlapped = new RecvOverlapped();
	mLastSendTime = TimeManager::Now();
	mIp[0] = '\0';
}

IOCPThreadConnect::~IOCPThreadConnect()
{
	delete mSendOverlapped;
	delete mRecvOverlapped;

	printf("*** Free connect [%s:%d]\r\n", GetIp(), GetPort());
}
//-------------------------------------------------------------------------*/