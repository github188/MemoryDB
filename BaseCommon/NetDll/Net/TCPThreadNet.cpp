
#include "TCPThreadNet.h"
#include "TimeManager.h"

#include "Socket.h"
#include "Winsock2.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"
//-------------------------------------------------------------------------
class ClientThreadConnect : public TCPThreadReceiveConnect
{
public:
	ClientThreadConnect(tNetHandle *pOwnerNet)
		: mpOwnerNet(pOwnerNet)
	{

	}
	~ClientThreadConnect()
	{
		if (mBackData!=NULL)
			delete mBackData;
		mBackData = NULL;
	}

public:
	virtual bool InitCreate()
	{
		if ( TCPThreadReceiveConnect::InitCreate() )
		{
			Socket *pSocket = new Socket();
			if (pSocket->create()==FALSE)
			{
				AssertEx(0, "初始创建Socket失败");
				return false;
			}

			pSocket->setNonBlocking(TRUE);	
			mBackData->mSocket = pSocket;

			return true;
		}
		return false;
	}

	virtual void SetNetHandle(AutoNet net){ mpOwnerNet = net.getPtr(); }
	virtual tNetHandle* GetNetHandle(void) { return mpOwnerNet; }

	virtual bool CheckSafeConnect(){ mbDataReceived = true; return true; }
	virtual void CheckSafeOverTime(){}

protected:
	tNetHandle *mpOwnerNet;
};
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
void TCPThreadClientNet::backWorkThread( void )
{
	while (IsActive())
	{
		if (!mConnected)
		{
			if (mNeedConnect && !mbConnectFail)
			{
				mThreadConnect->mBackData->mSocket->setNonBlocking(FALSE);
				bool b = mThreadConnect->Connect(mIp.c_str(), mPort);
				if (b)
				{
					mThreadConnect->mBackData->mSendDataLock.lock();
					mThreadConnect->mBackData->mSocket->setNonBlocking(TRUE);
					mThreadConnect->mBackData->mWillSendData->CleanUp();
					int checkCode = GetSafeCode();
					mThreadConnect->mBackData->mWillSendData->Write((const CHAR*)&checkCode, sizeof(checkCode));
					mThreadConnect->mBackData->mHasSendData = true;
					mThreadConnect->mBackData->mSendDataLock.unlock();

					mConnected = true;
					mNeedConnect = false;
				}
				else
				{
					mbConnectFail = true;
					mThreadConnect->Close();
				}
			}
			else
				TimeManager::Sleep(10);
		}
		else if (!mThreadConnect->IsDisconnect())
		{		
			SOCKET s = mThreadConnect->GetSocket()->getSOCKET();
			if (s!=INVALID_SOCKET)
			{
				FD_ZERO( m_ReadFDs );
				FD_ZERO( m_WriteFDs );
				FD_ZERO( m_ExceptFDs );

				FD_SET(s, m_ReadFDs);
				FD_SET(s, m_WriteFDs );
				FD_SET(s, m_ExceptFDs );

				INT iRet = SocketAPI::select_ex( (INT)s+1, 
					m_ReadFDs,
					m_WriteFDs,												// 是否可发送在主线进行检测
					m_ExceptFDs, 
					m_Timeout );
				AssertEx( iRet!=SOCKET_ERROR, "网络状态获取错误" );

				if( FD_ISSET(s, m_ExceptFDs) )
				{
					TABLE_LOG( "[%s : %u] 网络异常, 可能断开或关闭", mThreadConnect->GetSocket()->getHost(), mThreadConnect->GetSocket()->getPort() ) ;				
					mThreadConnect->mBackData->m_bDisconnect = true;
				}
				if (FD_ISSET(s, m_ReadFDs))
					mThreadConnect->mBackData->ReceiveDataOnThread();

				if (FD_ISSET(s, m_WriteFDs))
					mThreadConnect->mBackData->SendDataOnThread();
			}
		}
		//TimeManager::Sleep(10);
	}
}

bool TCPThreadClientNet::Connect( const char *szIp, int nPort, bool bThread )
{
	if (IsConnected())
	{
		Log("WARN: 已经连接成功 [%s:%d], Now [%s:%d]", szIp, nPort, mThreadConnect->GetIp(), mThreadConnect->GetPort());
		return true;
	}

	mbConnectFail = false;

	mConnected = false;
	mAlreadyConnect = false;
	mIp = szIp;
	mPort = nPort;
	mNeedConnect = true;
	//if (!bThread)
	//{
	//	mConnected = mThreadConnect->Connect(szIp, nPort);
	//	return mConnected;
	//}

	return true;
}

TCPThreadClientNet::TCPThreadClientNet() 
	: mConnected(false)
	, mNeedConnect(false)
	, mAlreadyConnect(false)
	, mPort(0)
	, mbDisconnect(false)
	, mbConnectFail(false)
{
	mThreadConnect = MEM_NEW ClientThreadConnect(this);

	m_ReadFDs = new fd_set;
	m_WriteFDs = new fd_set;
	m_ExceptFDs = new fd_set;

	FD_ZERO( m_ReadFDs );
	FD_ZERO( m_WriteFDs );
	FD_ZERO( m_ExceptFDs );

	m_Timeout = new timeval;
	m_Timeout->tv_sec = 0;
	m_Timeout->tv_usec = 20;

	if (mThreadConnect->InitCreate())
	{
		InitThread();
	}
	else
	{
		AssertEx(0, "Init create thread connect fail");
	}
}

TCPThreadClientNet::~TCPThreadClientNet()
{
	Close();
	mThreadConnect._free();

	delete m_ReadFDs;
	delete m_WriteFDs;
	delete m_ExceptFDs;
	delete m_Timeout;
}

void TCPThreadClientNet::Process()
{	
	if ( mThreadConnect->IsDisconnect() )
	{
		if (!mbDisconnect)
		{
			mbDisconnect = true;
			if (mbConnectFail)
				OnConnectFail();
			else
				OnCloseConnect(mThreadConnect.getPtr());
		} 
	}
	else if (mConnected)
	{
		if (!mAlreadyConnect)
		{
			mAlreadyConnect = true;
			OnConnected();
		}
		mThreadConnect->Process();
	}
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
bool TCPThreadServerNet::StartNet( const char* szIp, int nPort )
{
	mReceiveThread.InitThread();
	TCPListenThread::InitStart(szIp, nPort);
	TABLE_LOG("初始网络成功>[%s:%d]", szIp, nPort);

	return true;
}

void TCPThreadServerNet::OnAcceptConnect( HandConnect newConnect )
{
	TCPThreadReceiveConnect *p = dynamic_cast<TCPThreadReceiveConnect*>(newConnect.getPtr());
	if (p!=NULL)
		p->OnConnected();
	mReceiveThread.AppendConnect(newConnect);

}

void TCPThreadServerNet::StopNet()
{
	OnStop();

	mReceiveThread.Close();
	TCPListenThread::Close();
}
