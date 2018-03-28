
#include "ClientNet.h"

#include "Socket.h"
#include "NetSafeCheck.h"
#include "NetHandle.h"
#include "TimeManager.h"

#include "WorkThread.h"

#ifdef __LINUX__
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include "SocketInputStream.h"
#include "SocketOutputStream.h"
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------
tClientConnect::tClientConnect()
{
}
//----------------------------------------------------------------------------------------------

tClientConnect::~tClientConnect()
{

}
//----------------------------------------------------------------------------------------------

void tClientConnect::Close(void)
{
	if (mSocket)
		mSocket->close();
	SetState(NET_INIT_CONNECT);
	//tConnect::Close();
}
//----------------------------------------------------------------------------------------------


bool tClientConnect::InitConnect()
{
	CreateSocket();
	return true;
}

void tClientConnect::BindSocket(Socket *pSocket)
{
	mSocketInputStream->CleanUp();
	mSocketOutputStream->CleanUp();
    if (mSocket!=NULL)
        delete mSocket;
    
    mSocket = pSocket;
   
}
//----------------------------------------------------------------------------------------------

bool tClientConnect::Connect(const char *ip, unsigned int port, bool bWait)
{
	if (mSocket)
	{
		mSocket->close();
		mSocket->create();

		if (bWait)		
			mSocket->setNonBlocking(FALSE);
		else
			mSocket->setNonBlocking(TRUE);

		bool bOk = mSocket->connect(ip, port)==TRUE;
		if (bOk)
		{		
#if defined(__WINDOWS__)
			UINT flag = MSG_DONTROUTE ;
#elif defined(__LINUX__)
			UINT flag = MSG_DONTROUTE ;
#endif
			// 直接发送4字节安全验证
			int checkCode = GetNetHandle()->GetSafeCode();
			UINT re = GetSocket()->send((void*)&checkCode, sizeof(checkCode), flag);
			if ( re == sizeof(checkCode) )
				Log("连接成功-->[%s : %u]", ip, port);				
			else
				bOk = false;
		}
		else if (!bWait)
		{
			Log("等待连接-->[%s : %u], 非阻塞模式,连接中...", ip, port);
		}

		if (bOk || !bWait)
		{
			mSocket->setSendBufferSize( 1024*100 );
			mSocket->setReceiveBufferSize( 1024*100 );
			mSocket->setNonBlocking(TRUE);
			mSocket->setLinger(0);

			SetConnectBegin();

			SetState(NET_CONNECT_BEGIN);

			return true;
		}
	}
	Log("连接失败-->[%s : %u]", ip, port);
	return false;
}
//----------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------
void tClientConnect::SetNetHandle( tNetHandle *pNetServerTool )
{
	mNetHandle = pNetServerTool;
}

AString tClientConnect::GetHost( void )
{
	if (GetNetHandle())
		return GetNetHandle()->GetIp();
	return AString();
}

int tClientConnect::GetPort( void )
{
	if (GetNetHandle())
		return GetNetHandle()->GetPort();
	return 0;
}

bool tClientConnect::IsConnect(int checkTime)
{
	return mSocket!=NULL && mSocket->canWrite(checkTime);

	//struct timeval timeout; 
	//fd_set r;          
	//FD_ZERO(&r);
	//FD_SET(sockfd,&r); 
	//timeout.tv_sec=0;    
	//timeout.tv_usec=100;
	//int retval = select(sockfd+1,NULL,&r,NULL,&timeout);
	//if(retval==-1)
	//{
	//	perror("select");
	//	return 0;
	//}
	//else if(retval == 0)
	//{
	//	fprintf(stderr,"timeout/n");
	//	return 0;
	//}
	//printf("%sconnected/n",server);
}

//----------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------

// 进行网络连接线程
class ConnectThread : public WorkThread
{
public:
	ConnectThread()
		: mSocket(NULL)
		, mPort(0)
		, mOverTime(10000)
		, mFinish(false)
		, mResult(false)
		, mLive(true)
		, mSafeCode(0)
	{

	}

	~ConnectThread()
	{
		if (mSocket!=NULL)
			delete mSocket;
		mSocket = NULL;
	}
    

public:
	void StartConnect(const char *szIP, int port, int overTime, int nSafeCode)
	{
		if (mSocket!=NULL)
			delete mSocket;
		mSocket = new Socket();
        mSocket->create();
		mSocket->setNonBlocking(FALSE);
        mSocket->setLinger(0);
        
		mIp = szIP;
		mPort = port;
		mOverTime = overTime;
		mFinish = false;
		mResult = false;
		mBeginTime = TimeManager::NowTick();
		mSafeCode = nSafeCode;
		InitThread();
                
	}

	virtual void backWorkThread(void)
	{
		bool result = mSocket->connect(mIp.c_str(), mPort)==TRUE;
		if (result)
		{
#if defined(__WINDOWS__)
			UINT flag = MSG_DONTROUTE ;
#elif defined(__LINUX__)
			UINT flag = MSG_DONTROUTE ;
#endif
			// 直接发送4字节安全验证
			int checkCode = mSafeCode;
			int i = 0;
			// 约 30秒 = 50 * 600
			while (++i<600)	
			{
				UINT re = mSocket->send((void*)&checkCode, sizeof(checkCode), flag);
				result = re == sizeof(checkCode);
				if (result)
					break;
				TimeManager::Sleep(50);
			}
		}
		mResult = result;
		TABLE_LOG("Connect [%s : %d] > [%s]", mIp.c_str(), mPort, (mResult ? "Succeed" : "Fail") );
		mFinish = true;
		mClose = true;
	}

	virtual bool CheckFinishOrOverTime()
	{          
		if ( mFinish )
			return true;

        if ( (TimeManager::NowTick()-mBeginTime)>=(UINT)mOverTime )
		{
            mFinish = true;
		}

		return mFinish;
	}

	virtual bool IsConnectSucceed(){ return mLive && mResult; }

	virtual void SetInvalid(){ mLive = false; }

	virtual bool IsInvalid(){ return !mLive; }

public:
	Socket		*mSocket;
	EasyString	mIp;
	int			mPort;
	int			mOverTime;
	UInt64		mBeginTime;
	bool		mResult;
	bool		mFinish;
	bool		mLive;
	int			mSafeCode;
};

//-------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------

bool EventClientNet::ConnectDNS( const char *szDnsName, int port, int overTime )
{
	struct hostent *hp;
	struct in_addr in;
	struct sockaddr_in local_addr;
	if(!(hp=gethostbyname(szDnsName)))
	{
		printf("error");
		return false;
	}
	memcpy(&local_addr.sin_addr.s_addr, hp->h_addr, 4);
	in.s_addr=local_addr.sin_addr.s_addr;

	bool b = Connect(inet_ntoa(in), port, overTime);

	if (overTime>0)
	{
		TABLE_LOG("Begin connect > [%s : %d]", szDnsName, port );
	}
	else
	{
		TABLE_LOG("connect [%s : %d] > [%s]", szDnsName, port, (b ? "成功" : "失败") );
	}

	return b;
}

EventClientNet::EventClientNet()
	: mbDisconnect(false)
{
	mClientConnect = MEM_NEW BaseClientConnect();	
	mClientConnect->SetNetHandle(this);
	mClientConnect->InitConnect();
}

bool EventClientNet::Connect( const char *ip, unsigned int port, int overTime )
{
	if (overTime<=0)
	{
		if ( mClientConnect->Connect(ip, port, true) )
		{
			mClientConnect->SetOK();
			OnConnected();
			return true;
		}		
		return false;
	}

	_StartConnectThread(ip, port, overTime);

	return true;
}

bool EventClientNet::CheckConnect(bool &bFinish)
{
	if (mClientConnect)
		if ( mClientConnect->IsOk() )		
		{
			return true;
		}

	bool bHaveNoFinish = false;
	for (size_t i=0; i<mConnectThreadList.size(); ++i)
	{
		if (!mConnectThreadList[i]->IsInvalid())
		{
			if (mConnectThreadList[i]->CheckFinishOrOverTime())
			{
				if (mConnectThreadList[i]->IsConnectSucceed())
				{
					mClientConnect->BindSocket(mConnectThreadList[i]->mSocket);
					mConnectThreadList[i]->mSocket = NULL;
					mClientConnect->GetSocket()->setNonBlocking(TRUE);
					mClientConnect->SetOK();
					OnConnected();
					//mConnectThreadList.clear(true);				
					for (size_t n=0; n<mConnectThreadList.size(); ++n)
					{
						mConnectThreadList[n]->SetInvalid();
					}

					return true;
				}
			}
			else
				bHaveNoFinish = true;
		}
	}

	bFinish = !bHaveNoFinish;

	return false;

}

bool EventClientNet::CheckConnect()
{
	// 使用接收来检查SOCKET是否正常, 主要用于客户端, 当应用与IPHONE时, 按了休眠后, 一段时间回来时, SOCKET一般都是错误的
	return mClientConnect->ProcessInput();
}


void EventClientNet::_StartConnectThread( const char *szIP, int port, int overTime )
{
	ConnectThread *pConnectThread = MEM_NEW ConnectThread();
	pConnectThread->StartConnect( szIP, port, overTime, GetSafeCode() );

	mConnectThreadList.push_back(pConnectThread);
}

void EventClientNet::Process()
{
	if (!mConnectThreadList.empty())
	{
		for (size_t i=0; i<mConnectThreadList.size(); ++i)
		{
			if (mConnectThreadList[i]->IsInvalid() && !mConnectThreadList[i]->IsActive())
			{
				delete mConnectThreadList[i];
				mConnectThreadList[i] = NULL;
				mConnectThreadList.remove(i);
				break;
			}
		}
	}

	if (!mbDisconnect)
	{
		if (!mClientConnect->Process())
		{
			OnCloseConnect(mClientConnect.getPtr());
			mbDisconnect = true;
		}		
	}	
}

void EventClientNet::InitReset()
{
	if (mClientConnect)
	{
		mClientConnect->Close();
		for (size_t i=0; i<mConnectThreadList.size(); ++i)
		{
			mConnectThreadList[i]->SetInvalid();
		}
	}
}

bool EventClientNet::CanWrite( int checkTime )
{
	if (mClientConnect && mClientConnect->GetSocket())
    {
		return mClientConnect->GetSocket()->canWrite(checkTime)==TRUE;
    }
	return false;
}

AString EventClientNet::GetIPByDNS( const char *szDNS )
{
	struct hostent *hp;
	struct in_addr in;
	struct sockaddr_in local_addr;
	if(!(hp=gethostbyname(szDNS)))
	{
		printf("error");
		return AString();
	}
	memcpy(&local_addr.sin_addr.s_addr, hp->h_addr, 4);
	in.s_addr=local_addr.sin_addr.s_addr;

	AString resultIP = inet_ntoa(in);
	return resultIP;
}






//----------------------------------------------------------------------------------------------