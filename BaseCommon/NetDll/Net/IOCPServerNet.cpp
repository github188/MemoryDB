#include "IOCPServerNet.h"
#include "TableTool.h"
#include "SocketAPI.h"
#include "TimeManager.h"

#include "IOCPCommon.h"
#include <MSTcpip.h>

#include "IOCPConnect.h"

#include "NetIndexPacket.h"

#define  USE_IPV6		0

#if USE_IPV6
#include <ws2ipdef.h>
#include <WS2tcpip.h>
#endif
//-------------------------------------------------------------------------
bool IOCPListenThread::InitStart( const char* szIp, int nPort )
{
	Close();
#if USE_IPV6
	SOCKET	sockfd;

	struct sockaddr_in6 my_addr; 

	if ((sockfd = socket(PF_INET6, SOCK_STREAM, 0)) == -1) { // IPv6
		perror("socket");
		exit(1);
	} else
		printf("socket created/n");

	ULONG argp = 0;
	int r = ioctlsocket( sockfd,FIONBIO,&argp);

	memset(&my_addr, 0, sizeof(my_addr));
	/* my_addr.sin_family = PF_INET; */ // IPv4
	my_addr.sin6_family = PF_INET6;    // IPv6
	/* my_addr.sin_port = htons(myport); */ // IPv4
	my_addr.sin6_port = htons(nPort);   // IPv6
	//if (argv[3])
	//	/* my_addr.sin_addr.s_addr = inet_addr(argv[3]); */ // IPv4
	//	inet_pton(AF_INET6, argv[3], &my_addr.sin6_addr);  // IPv6
	//else
	/* my_addr.sin_addr.s_addr = INADDR_ANY; */ // IPv4
	my_addr.sin6_addr = in6addr_any;            // IPv6

	/* if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) */ // IPv4
	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_in6))  // IPv6
		== -1) {
			perror("bind");
			exit(1);
	} else
		printf("binded/n");

	mListenSocket = sockfd;

#else
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
#endif
	// 准备监听Socket连接
	if ( listen(mListenSocket, SOMAXCONN) == SOCKET_ERROR )
	{   
		ERROR_LOG("[%s:%d] Socket 开启监听 失败", szIp, nPort);
		ErrorExit("listen");
		return false;   
	}

	WorkThread::InitThread();
	TABLE_LOG("NET: 初始网络服务成功 [%s:%d]", szIp, nPort);
	return true;
}

void IOCPListenThread::backWorkThread()
{
	sockaddr saClient;
	SocketInfo info;

	while(IsActive())
	{
		// 处理连接请求
#if USE_IPV6
		memset(&saClient, 0, sizeof(saClient));
		struct sockaddr_in6 their_addr;
		int iClientSize = sizeof(their_addr);
		SOCKET acceptSocket = WSAAccept(mListenSocket, (struct sockaddr *)&their_addr, &iClientSize, NULL, NULL);
		if ( acceptSocket == SOCKET_ERROR )
		{
			// 异常错误事件
			//OnError(INVALID_ID, WSAGetLastError());
			Sleep(1);
			continue;
		}			
		char buf[1024];
		printf("server: got connection from %s, port %d\r\n",
			/* inet_ntoa(their_addr.sin_addr), */ // IPv4
			inet_ntop(AF_INET6, &their_addr.sin6_addr, buf, sizeof(buf)), // IPv6
			/* ntohs(their_addr.sin_port), new_fd); */ // IPv4
			their_addr.sin6_port);
#else
		int iClientSize = sizeof(saClient);
		SOCKET acceptSocket = WSAAccept(mListenSocket, &saClient, &iClientSize, NULL, NULL);
		if ( acceptSocket == SOCKET_ERROR )
		{
			// 异常错误事件
			OnError(INVALID_ID, WSAGetLastError());
			continue;
		}			
#endif
		mConnectListLock.lock();
		info.mSocket = acceptSocket;
		info.mAddr = saClient;
		mConnectList.push_back(info);
		mbHasConnect = true;
		mConnectListLock.unlock();
	}
}

void IOCPListenThread::Process()
{
	if (mbHasConnect)
	{
		mConnectListLock.lock();
		mMainConnectList.swap(mConnectList);
		mbHasConnect = false;
		mConnectListLock.unlock();

		for (size_t i=0; i<mMainConnectList.size(); ++i)
		{
			HandConnect connect = _CreateNewConnect(mMainConnectList[i]);
			OnAcceptConnect(connect);
		}
		mMainConnectList.clear(false);

	}
}

void IOCPListenThread::ErrorExit( const char *szInfo )
{
	NOTE_LOG("NET ERROR: [%s]", szInfo);
}
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
IOCPBaseNet::IOCPBaseNet( size_t connectCount /*= 1*/, int threadNum/* = _IOCP_THREAD_NUM*/ ) 
	: m_hCompletionPort(NULL)
	, mbStop(false)
#if _DEBUG_NET_
	, mRevPacketCount(0)
#endif
{
	//if (connectCount>1)
	//	mConnectList.resize(16);
	//else
	//	mConnectList.resize(connectCount);

	Init();

#if _IOCP_THREAD_RECEIVE_SEND
	mCompletionThreadList.resize(threadNum);
	for (int i=0; i<mCompletionThreadList.size(); ++i)
	{
		mCompletionThreadList[i] = MEM_NEW IocpProcessCompletionThread(this);
		mCompletionThreadList[i]->InitThread();
	}
#endif
}

bool IOCPBaseNet::Init( void )
{
	mbStop  = false;
	// 创建IO完成端口
	m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (m_hCompletionPort == NULL)
	{
		TABLE_LOG("NET ERROR: CreateIoCompletionPort");
		return false; 
	}
	
	return true;
}

void IOCPBaseNet::_AppendConnect( HandConnect newConnect )
{
	if (mbStop)
	{
		newConnect->Close();
		return;
	}
//_append:
//	bool bAdd = false;
//	int count = 0;
//	for (size_t i=0; i<mConnectList.size(); ++i)
//	{
//		 if (mConnectList[i])
//			 ++count;
//		else //if (!bAdd)
//		{
//			mConnectList[i] = newConnect;
//			newConnect->SetNetID(i);
//			bAdd = true;
//			break;
//		}			
//	}
//	
//	if (!bAdd)
//	{
//		mConnectList.resize(mConnectList.size()*2);
//		goto _append;
//		//TABLE_LOG("NET_ERROR : Connect count over max num [%d], Now remove connect [%s:%d]", NET_CONNECT_MAX, newConnect->GetIp(), newConnect->GetPort());
//		//newConnect->SetRemove(true);
//		//return;
//	}
//	++count;	

	mConnectList.add(newConnect);
	int count = mConnectList.size();

	IOCPConnect *pRecConnect = dynamic_cast<IOCPConnect*>(newConnect.getPtr());
	AssertEx(pRecConnect!=NULL, "创建连接必须继承 IOCPConnect ");

	// 修改缓存限制
	pRecConnect->mSendData.SetMaxLength(SendBufferMax());
	pRecConnect->mReceiveData.SetMaxLength(ReceiveBufferMax());

	// 给当前连接进来的客户端SOCKET创建IO完成端口
	if (pRecConnect->mIocpHandle)
		CloseHandle((HANDLE)pRecConnect->mIocpHandle);
	pRecConnect->mIocpHandle = CreateIoCompletionPort((HANDLE)pRecConnect->mSocketID, m_hCompletionPort, (ULONG_PTR)newConnect.getPtr(), 0);
	if ( pRecConnect->mIocpHandle == NULL )
	{
		ERROR_LOG("NET ERROR: [%s:%d] CreateIoCompletionPort", pRecConnect->GetIp(), pRecConnect->GetPort());
		pRecConnect->SetRemove(true);
		return;
	}
	
	TABLE_LOG("NET: ++[%s:%d] [%d] Succeed accept connect", pRecConnect->GetIp(), pRecConnect->GetPort(), count);
	// 发送一个接收请求
	pRecConnect->_Start();
	_OnConnectStart(pRecConnect);
}
// 分离包含的连接，之后可以将连接再次加入到其他IOCP网络管理里
void IOCPBaseNet::_RemoveConnect(HandConnect conn)
{
	IOCPConnect *pRecConnect = dynamic_cast<IOCPConnect*>(conn.getPtr());
	AssertEx(pRecConnect!=NULL, "创建连接必须继承 IOCPConnect ");
	if (pRecConnect->mIocpHandle!=NULL)
		CloseHandle(pRecConnect->mIocpHandle);
	pRecConnect->mIocpHandle = NULL;
	for (int i=0; i<mConnectList.size(); ++i)
	{
		if (mConnectList[i]==conn)
		{
			mConnectList.removeAt(i);
			break;
		}
	}
	pRecConnect->mNet.setNull();
}

void IOCPBaseNet::Process( void )
{	
#if !_IOCP_THREAD_RECEIVE_SEND
	_ProcessCompletion();
#endif
	bool bRemove = false;
	for (size_t i=0; i<mConnectList.size(); )
	{
		if (mConnectList[i])
		{		
			if (!mConnectList[i]->Process())
			{
				TABLE_LOG("NET: --[%s:%d] [%d] Now free connect socket", mConnectList[i]->GetIp(), mConnectList[i]->GetPort(), GetConnectCount()-1);

				IOCPConnect *pC = dynamic_cast<IOCPConnect*>(mConnectList[i].getPtr());
				AssertEx(pC!=NULL, "Must is IOCPConnect");
				pC->_Free();

				mConnectList[i]->OnDisconnect();
				OnCloseConnect(mConnectList[i].getPtr());

				mConnectList.removeAt(i);
				//HandConnect lastConnect;
				//for (int n=mConnectList.size()-1; n>=0; --n)
				//{
				//	if (mConnectList[n])
				//	{
				//		if (mConnectList[n]!=mConnectList[i])
				//			lastConnect = mConnectList[n];
				//		mConnectList[n].setNull();
				//		break;
				//	}
				//}
				//mConnectList[i].setNull();
				//if (lastConnect)
				//	mConnectList[i] = lastConnect;
				bRemove = true;
				continue;
			}
		}
		else
			break;

		++i;
	}
	if (bRemove)
		OnRemovedConnect();
}

bool IOCPBaseNet::_ProcessCompletion()
{
	while(m_hCompletionPort!=NULL) 
	{
		PULONG_PTR netConnectPtr = NULL;
		DWORD   BytesTransferred = 0;
		BaseOverlapped* pSOverlapped = NULL;
#if _IOCP_THREAD_RECEIVE_SEND
		if ( GetQueuedCompletionStatus(m_hCompletionPort, &BytesTransferred,   
			(PULONG_PTR)&netConnectPtr, (LPOVERLAPPED*)&pSOverlapped, 2000) == 0 ) 
#else
		if ( GetQueuedCompletionStatus(m_hCompletionPort, &BytesTransferred,   
			(PULONG_PTR)&netConnectPtr, (LPOVERLAPPED*)&pSOverlapped, 0) == 0 ) 
#endif 
		{   
			if (netConnectPtr==NULL)                            
				    return false;            

			IOCPConnect *pConnect = dynamic_cast<IOCPConnect*>((tNetConnect*)netConnectPtr);						

			DWORD dwLastError = GetLastError();
			if (dwLastError==WAIT_TIMEOUT || pConnect==NULL)
				break;

            if (dwLastError!=995)
			    WARN_LOG("WARN: IOCP error>[%u]", dwLastError);

			if (dwLastError==ERROR_OPERATION_ABORTED || dwLastError==ERROR_NETNAME_DELETED)
			{			
				CloseConnect(pConnect);
			}
			// 64错误号表示"指定的网络名不再可用"，客户端异常退出会产生这个错误号
			else if (dwLastError!=WAIT_TIMEOUT ) 
			{
				Log("GetQueuedCompletionStatus   发生了如下错误： %d\n",   GetLastError());
				CloseConnect(pConnect);
			}
			
			if (pSOverlapped!=NULL)
				pSOverlapped->mbWaiting = false;

			break;
		}

		IOCPConnect *pConnect = dynamic_cast<IOCPConnect*>((tNetConnect*)netConnectPtr);
        if (pSOverlapped == NULL || pConnect==NULL) {
            printf("pSOverlapped == NULL or connect is null\n");
            break;
        }
		
		if (pSOverlapped->IoMode == IoSend) 
		{
			// 发送事件
			SendOverlapped* pSendOverlapped = (SendOverlapped*)pSOverlapped;
            pConnect->_OnThreadSendFinish(pSendOverlapped, BytesTransferred);
            //NOTE_LOG("[%s:%d] sended >%u", pConnect->GetIp(), pConnect->GetPort(), BytesTransferred);
		} 
		else if (pSOverlapped->IoMode == IoRecv) 
		{			
#if _IOCP_THREAD_RECEIVE_SEND
            pConnect->mRevLock.lock();
#endif
			pSOverlapped->mbWaiting = false;
			if (BytesTransferred == 0) 
			{							
				if (pConnect!=NULL)
				{
					TABLE_LOG("NET: May be other party connect close [%s:%d], Now set remove", pConnect->GetIp(), pConnect->GetPort());
					pConnect->Close();
				}		
			}
			else
			{	
                //NOTE_LOG("== rev size > %d", BytesTransferred);
 				AddReceiveTotalSize(BytesTransferred);
               RecvOverlapped* pRecvOverlapped = (RecvOverlapped*)pSOverlapped;
                pConnect->_OnThreadRevFinish(pRecvOverlapped, BytesTransferred);
			}			
#if _IOCP_THREAD_RECEIVE_SEND
            pConnect->mRevLock.unlock();
#endif
		}
	}

	return true;
}

void IOCPBaseNet::StopNet()
{
	mbStop = true;
	//IOCPListenThread::Close();
	for (size_t i=0; i<mConnectList.size(); ++i)
	{
		if (mConnectList[i])
		{		
			mConnectList[i]->Close();
		}
	}

	while (GetConnectCount()>0)
	{
		Sleep(100);
		Process();
	}
}

int IOCPBaseNet::GetConnectCount()
{
	return (int)mConnectList.size();
	//int count = 0;
	//for (size_t i=0; i<mConnectList.size(); ++i)
	//{
	//	if (mConnectList[i])
	//		++count;
	//	else
	//		break;
	//}
	//return count;
}

//void IOCPBaseNet::CloseConnect( int netID )
//{
//	tNetConnect *p = GetConnect(netID);
//	if (p!=NULL)
//	{			
//		p->Close();
//	}
//}

void IOCPBaseNet::CloseConnect( tNetConnect *pConnect )
{
	if (pConnect!=NULL)
	{			
		pConnect->Close();
	}
}

IOCPBaseNet::~IOCPBaseNet()
{
	for (size_t i=0; i<mConnectList.size(); ++i)
	{
		if (mConnectList[i])
		{		
			mConnectList[i]->Close();
		}
	}

	while (true)
	{
		if (GetConnectCount()<=0)
			break;

		Process();
	}

#if _IOCP_THREAD_RECEIVE_SEND
	// 需要发送一个退出线程的指令
	PostQueuedCompletionStatus(m_hCompletionPort, 0, NULL, NULL);

	for (int i=0; i<mCompletionThreadList.size(); ++i)
	{
		if (mCompletionThreadList[i]!=NULL)
		{
			mCompletionThreadList[i]->Close();
			delete mCompletionThreadList[i];
			mCompletionThreadList[i] = NULL;
		}
	}
	mCompletionThreadList.clear(true);
#endif

	CloseHandle(m_hCompletionPort);
	m_hCompletionPort = NULL;
}

HandConnect IOCPBaseNet::CreateConnect()
{
	return MEM_NEW IOCPConnect();
}

//-------------------------------------------------------------------------*/
//-------------------------------------------------------------------------
HandConnect IOCPServerNet::_CreateNewConnect( SocketInfo *acceptSocket )
{
	HandConnect conn = CreateConnect();

	IOCPConnect *pRecConnect = dynamic_cast<IOCPConnect*>(conn.getPtr());
	AssertEx(pRecConnect!=NULL, "创建连接必须继承 IOCPConnect ");
	pRecConnect->mNet = GetSelf();
	pRecConnect->mSocketID = acceptSocket->mSocket;
	sockaddr_in *p = (sockaddr_in *)&(acceptSocket->mAddr);
	pRecConnect->mIp = inet_ntoa(p->sin_addr);
	pRecConnect->mPort = ntohs( p->sin_port );

	return conn;
}

IOCPServerNet::IOCPServerNet( size_t maxConnectCount /*= NET_CONNECT_MAX*/, int threadNum ) 
	: IOCPBaseNet(maxConnectCount, threadNum)
	, mPort(0)
	, mStartSucceed(false)
{
	mListenThread = MEM_NEW IOCPListenThread(this);
}

IOCPServerNet::~IOCPServerNet()
{
	mListenThread->Close();
	delete mListenThread;
	mListenThread = NULL;
}

//bool IOCPServerNet::StartNet( void )
//{
//	return StartNet(GetIp(), GetPort()); 
//}

bool IOCPServerNet::StartNet( const char *szIP, int nPort )
{
	mIp = szIP;
	mPort = nPort;
	
	if (GetSafeCode()==0)
	{
		WARN_LOG("[%s:%d]网络连接安全验证码为0, 默认不进行安全验证", szIP, nPort); 
		WARN_LOG("NOTE: 连接[%s:%d]请不要再发送验证码, 否则解包错误", szIP, nPort);
	}

	{
		mStartSucceed = mListenThread->InitStart(GetIp(), GetPort());
		//mStartSucceed = true;
		return mStartSucceed;
	}
	return false;
}

void IOCPServerNet::StopNet()
{
	mListenThread->Close();
	IOCPBaseNet::StopNet();
}

void IOCPServerNet::OnMsgRegistered(int eventNameIndex)
{
	// 通知给所有的连接
	NotifyEventIndexPacket	indexPacket;

	if (GetEventCenter()->GenerateMsgIndex(indexPacket.GetData(), eventNameIndex))
	{
		for (size_t i=0; i<mConnectList.size(); ++i)
		{
			if (mConnectList[i])
			{
				mConnectList[i]->Send(&indexPacket, false);
			}
		}
	}

}

void IOCPServerNet::Process()
{
	if (mStartSucceed)
	{
		mListenThread->Process();
		IOCPBaseNet::Process();
	}
}

void IOCPServerNet::OnAcceptConnect( HandConnect newConnect )
{
	IOCPServerConnect *p = dynamic_cast<IOCPServerConnect*>(newConnect.getPtr());
	p->mStartTime = TimeManager::Now();

	IOCPBaseNet::_AppendConnect(newConnect);
}

HandConnect IOCPServerNet::CreateConnect()
{
	return MEM_NEW IOCPServerConnect();
}

void IOCPServerNet::_OnConnectStart( tNetConnect *pConnect )
{
	// 通知给所有的连接
	NotifyEventIndexPacket	indexPacket;

	if (GetEventCenter()->GenerateMsgIndex(indexPacket.GetData(), 0))
	{
		pConnect->Send(&indexPacket, false);
	}
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
bool IOCPClientNet::Connect( const char *szIp, int nPort, int overmilSecond )
{
	mIp = szIp;
	mPort = nPort;

	if (mIp=="" || mPort<=0)
		return false;

	if (mWaitConnectThread!=NULL)
	{
		mWaitConnectThread->Close();
		delete mWaitConnectThread;
	}
	mWaitConnectThread = MEM_NEW ConnectNetThread();
	mWaitConnectThread->StartConnect(szIp, nPort, overmilSecond);

	return true;
}

void IOCPClientNet::StopNet( void )
{
	if (mWaitConnectThread!=NULL)
	{
		mWaitConnectThread->Close();
		delete mWaitConnectThread;
		mWaitConnectThread = NULL;
	}

	for (size_t i=0; i<mConnectList.size(); ++i)
	{
		if (mConnectList[i])
			mConnectList[i]->Close();
	}
	Process();
	if (GetClientConnect())
	{
		Sleep(10);
		Process();
	}
	//IOCPBaseNet::StopNet();
}

void IOCPClientNet::Process()
{
	if (mWaitConnectThread!=NULL)
	{
		if (mWaitConnectThread->IsConnectFinish())
		{
			if (mWaitConnectThread->mSocket!=0)
			{
				HandConnect conn = CreateConnect();

				IOCPConnect *pRecConnect = dynamic_cast<IOCPConnect*>(conn.getPtr());
				AssertEx(pRecConnect!=NULL, "创建连接必须继承 IOCPConnect ");
				pRecConnect->mNet = GetSelf();
				pRecConnect->mSocketID = mWaitConnectThread->mSocket;
				mWaitConnectThread->mSocket = 0;
				pRecConnect->mIp = mIp.c_str();
				pRecConnect->mPort = mPort;
				//tNetConnect *p = GetClientConnect().getPtr(); //GetConnect(0);
				//if (p!=NULL)
				//	p->Close();
				for (size_t i=0; i<mConnectList.size(); ++i)
				{
					if (mConnectList[i])
					{		
						mConnectList[i]->Close();
					}
				}
				while( GetConnectCount()>0 )
				{
					IOCPBaseNet::Process();
					TimeManager::Sleep(10);
				}
				_AppendConnect(conn);
				_SendSafeCode(pRecConnect);
				conn->OnConnected();
				OnAddConnect(conn.getPtr());
				OnConnected();
			}
			else
			{
				Log("NET: WARN Connect fail [%s:%d]", mIp.c_str(), mPort);
				OnConnectFail();
			}
			if (mWaitConnectThread!=NULL)
			{
				mWaitConnectThread->Close();
				delete mWaitConnectThread;
				mWaitConnectThread = NULL;
			}
		}
		else if (mWaitConnectThread->IsOverTime())
		{
			Log("NET: WARN Connect over time [%s:%d]", mIp.c_str(), mPort);
			mWaitConnectThread->Close();
			delete mWaitConnectThread;
			mWaitConnectThread = NULL;

			OnConnectFail();

		}

	}
	else
		IOCPBaseNet::Process();
}

void IOCPClientNet::_SendSafeCode(IOCPConnect *pConnect)
{
	if (GetSafeCode()!=0)
	{		
		int safe = GetSafeCode();
		if (safe!=0)
		{
            pConnect->_Send((const CHAR*)&safe, sizeof(int));
		}
		pConnect->_SendTo();
	}
}


//-------------------------------------------------------------------------

ConnectNetThread::ConnectNetThread() 
	: mPort(0)
	, mSocket(0)
{
	mOverTime = TimeManager::NowTick();	
}

void ConnectNetThread::StartConnect( const char *sIp, int nPort, int milSecondOverTime )
{
	mIP = sIp;
	mPort = nPort;
	//??? TABLE_LOG("开始连接[%s:%d]", sIp, nPort);
	mOverTime = TimeManager::NowTick() + milSecondOverTime*1000;
	InitThread();
}

bool ConnectNetThread::IsConnectFinish()
{
	if (mBackTread==NULL)
		return true;
	//!!! 注意，此处如果存在尝试连接时，耗时1毫秒判断完成
	DWORD dw = WaitForSingleObject( (HANDLE)mBackTread, 1 );
	if (dw==WAIT_OBJECT_0 || dw==WAIT_FAILED)
	{
		CloseHandle((HANDLE)mBackTread);
		mBackTread = NULL;
		return true;
	}
	return false;
}

bool ConnectNetThread::IsOverTime()
{
	return TimeManager::NowTick()>mOverTime;
}

void ConnectNetThread::backWorkThread( void )
{
	mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED); 
	if (mSocket == INVALID_SOCKET)
	{
		TABLE_LOG("NET ERROR: IOCP client socket create fail > [%u]", GetLastError());
		return;
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr( mIP.c_str() );
	addr.sin_port = htons(mPort);

	SocketAPI::setsocketnonblocking_ex(mSocket, FALSE);
	if (SocketAPI::connect_ex( mSocket , (const struct sockaddr *)&addr , sizeof(addr) )==TRUE)
		SocketAPI::setsocketnonblocking_ex(mSocket, TRUE);
	else
	{
		SocketAPI::closesocket_ex(mSocket);
		mSocket = 0;
	}
}

ConnectNetThread::~ConnectNetThread()
{
	if (mSocket!=0)
		SocketAPI::closesocket_ex(mSocket);
	mSocket = 0;
}
//-------------------------------------------------------------------------

void NetListen::ProcessToNet( tNetHandle *destHandle )
{

	if (mbHasConnect)
	{
		mConnectListLock.lock();
		mMainConnectList.swap(mConnectList);
		mbHasConnect = false;
		mConnectListLock.unlock();

		for (size_t i=0; i<mMainConnectList.size(); ++i)
		{
			IOCPServerNet *pNet = dynamic_cast<IOCPServerNet*>(destHandle);
			HandConnect connect = pNet->_CreateNewConnect(&mMainConnectList[i]);
			pNet->OnAcceptConnect(connect);		
		}
		mMainConnectList.clear(false);
	}

	//if (mbHasConnect)
	//{
	//	mConnectListLock.lock();

	//	if (mConnectList.empty())
	//	{
	//		mbHasConnect = false;
	//		mConnectListLock.unlock();
	//		return;
	//	}

	//	if (mCheckCount<mNetList.size() && mConnectList.size()<mNetList.size())
	//	{
	//		if (mNetList.size()>0 && mNetList.get(0)!=destHandle)
	//		{
	//			mCheckCount++;
	//			mConnectListLock.unlock();
	//			return;
	//		}
	//	}						
	//	IOCPServerNet *pNet = dynamic_cast<IOCPServerNet*>(destHandle);
	//	HandConnect connect = pNet->_CreateNewConnect(&mConnectList.back());
	//	pNet->OnAcceptConnect(connect);			
	//	mConnectList.pop_back();

	//	mbHasConnect = !mConnectList.empty();
	//	mCheckCount = 0;
	//	mConnectListLock.unlock();
	//}
}
