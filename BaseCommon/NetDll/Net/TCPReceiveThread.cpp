
#include "TCPReceiveThread.h"

#include "SocketInputStream.h"
#include "SocketOutputStream.h"
#include "Packet.h"

#include "Socket.h"
#include "TimeManager.h"
#include "TableTool.h"
#include "ServerIPInfo.h"

#include "TCPThreadNet.h"

TCPThreadReceiveConnect::TCPThreadReceiveConnect() 
	: mMainThreadReceiveData(NULL)
	, mNeedRemove(false)
	, mbDataReceived(false)
	, mBackData(NULL)
	, mStartTime(0)
	, mReceiveTotalSize(0)
{
	mMainThreadReceiveData = MEM_NEW SocketInputStream();
	mMainThreadSendData = MEM_NEW SocketOutputStream();
}

bool TCPThreadReceiveConnect::Connect( const char *ip, unsigned int port )
{
	if (mBackData->mSocket)
	{			
		mBackData->mSocket->close();
		mBackData->mSocket->create();
		mBackData->mSocket->setNonBlocking(FALSE);		

		bool bOk = mBackData->mSocket->connect(ip, port)==TRUE;
		if (bOk)
		{		
			Log("成功连接 > [%s : %u]", ip, port);			

			mBackData->mSocket->setSendBufferSize( 1024*100 );
			mBackData->mSocket->setReceiveBufferSize( 1024*100 );
			mBackData->mSocket->setNonBlocking(TRUE);
			mBackData->mSocket->setLinger(0);							

			return true;
		}
	}		
	Log("连接失败 > [%s : %u]", ip, port);
	return false;
}

bool TCPThreadReceiveConnect::Send( const Packet *msgPacket, bool )
{
	tNetHandle *p = GetNetHandle();
	if (p!=NULL)
	{
		DSIZE nowSize = mMainThreadSendData->Length();
		bool re = p->GetNetProtocol()->WritePacket(msgPacket, mMainThreadSendData);
		GetNetHandle()->AddSendTotalSize( mMainThreadSendData->Length()-nowSize );

		mBackData->mSendDataLock.lock();
		if (mBackData->mWillSendData->IsEmpty())
		{
			SocketOutputStream *pTemp = mBackData->mWillSendData;
			mBackData->mWillSendData = mMainThreadSendData;
			mMainThreadSendData = pTemp;
		}
		else
		{
			mBackData->mWillSendData->Write(mMainThreadSendData->GetBuffer(), mMainThreadSendData->Size());
			mMainThreadSendData->CleanUp();
		}
		mBackData->mHasSendData = true;
		mBackData->mSendDataLock.unlock();
		return re;
	}
	return false;
}

bool TCPThreadReceiveConnect::SendEvent( Logic::tEvent *pEvent )
{
	if (GetNetHandle())
		return EventNetProtocol::SendEvent(this, pEvent);

	return false;
}

bool TCPThreadReceiveConnect::InitCreate()
{
	mbDataReceived = false;
	if (mBackData==NULL)
		mBackData = MEM_NEW BackReceiveData();	

	return true;
}

bool TCPThreadReceiveConnect::Process()
{
	if (IsNeedRemove() || mBackData->IsDisconnect())
		return false;

	if (mBackData->mHasReceiveData)
	{
		UINT nowDataSize = mMainThreadReceiveData->Length();
		mBackData->mLock.lock();
		if (mMainThreadReceiveData->IsEmpty())
		{
			// 如果主线接收数据为空, 直接交换
			SocketInputStream *p = mMainThreadReceiveData;
			mMainThreadReceiveData = mBackData->mBackThreadReceiveData;
			mBackData->mBackThreadReceiveData = p;
		}
		else
		{
			// 如果不为空, 直接写到最后
			bool b = mMainThreadReceiveData->WriteData(mBackData->mBackThreadReceiveData);
			AssertEx(b, "转存数据错误");

			//size_t s = mBackData->mBackThreadReceiveData->Size();
			//Buffer		tempData(s);
			//bool b = mBackData->mBackThreadReceiveData->Read(tempData.data(), s)==s;
			//AssertEx(b, "获取数据错误");
			//b = mMainThreadReceiveData->WriteData(tempData.data(), s);
			//AssertEx(b, "转存数据错误");
			mBackData->mBackThreadReceiveData->CleanUp();
		}
		mBackData->mHasReceiveData = false;
		mBackData->mLock.unlock();

		if (!mbDataReceived)
		{
			if (!CheckSafeConnect())
				return false;
		}
		mReceiveTotalSize += mMainThreadReceiveData->Length()-nowDataSize;
		GetNetHandle()->AddReceiveTotalSize( mMainThreadReceiveData->Length()-nowDataSize );
		ProcessReceiveData();
	}

	if (!mbDataReceived)
	{
		CheckSafeOverTime();
	}

	return true;
}

void TCPThreadReceiveConnect::ProcessReceiveData()
{
	tNetHandle *pNet = GetNetHandle();
	AssertEx(pNet!=NULL && pNet->GetNetProtocol()!=NULL, "网络管理不存在或协议为空");

	while(mMainThreadReceiveData->IsEmpty()==FALSE)
	{
		HandPacket packet = pNet->GetNetProtocol()->ReadPacket(this, mMainThreadReceiveData);
		if (packet)
			packet->Execute(this);				
		else
			break;
	}
}



void TCPThreadReceiveConnect::Log( const char *szInfo, ... )
{
#if DEVELOP_MODE
	va_list va;
	va_start(va, szInfo);
	LOG_TOOL(va, szInfo);
#endif
}

TCPThreadReceiveConnect::~TCPThreadReceiveConnect()
{	
	if (mBackData!=NULL)
	{
		mBackData->SetNeedFree();
	}

	if (mMainThreadReceiveData)
		delete mMainThreadReceiveData;
	mMainThreadReceiveData = NULL;

	delete mMainThreadSendData;
	mMainThreadSendData = NULL;
}

UInt64 TCPThreadReceiveConnect::GetIPKey()
{
	if (GetSocket()!=NULL)
		return ServerIPInfo::IP2Num(GetSocket()->getHost(), GetSocket()->getPort(), 0);

	return 0;
}

const char* TCPThreadReceiveConnect::GetIp()
{
	return GetSocket()->getHost();
}

int TCPThreadReceiveConnect::GetPort()
{
	return GetSocket()->getPort();
}

bool TCPThreadReceiveConnect::CanRemove()
{
	return mBackData->mWillSendData->IsEmpty()==TRUE;
}

void TCPThreadReceiveConnect::OnDisconnect()
{
	if (GetNetHandle())
		GetNetHandle()->OnCloseConnect(this);
}

void TCPThreadReceiveConnect::OnConnected()
{
	mStartTime = TimeManager::Now();
}

void TCPThreadReceiveConnect::OnSafeCheckSucceed()
{
	GetNetHandle()->OnAddConnect(this);
}

bool TCPThreadReceiveConnect::CheckSafeConnect()
{
	int checkCode = 0;
	if (mMainThreadReceiveData->Peek((CHAR*)&checkCode, sizeof(checkCode))==TRUE)
	{
		if (checkCode == GetNetHandle()->GetSafeCode())
		{
			mMainThreadReceiveData->Skip(sizeof(checkCode));
			mbDataReceived = true;
			OnSafeCheckSucceed();
		}
		else
		{
			TABLE_LOG("WARN: [%s : %d] connect safe check fail, then now remove", GetIp(), GetPort());
			SetRemove(true);
			return false;
		}
	}
	return true;
}

void TCPThreadReceiveConnect::CheckSafeOverTime()
{
	// 如果从来没有接收过数据, 进行检查是否超时
	if (TimeManager::Now()-mStartTime>SERVER_NET_CHECK_CONNECT_WAITTIME)
	{
		TABLE_LOG("WARN: [%s : %d] connect wait receive data over time, then now remove", GetIp(), GetPort());
		SetRemove(true);
	}
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
void TCPReceiveThread::AppendConnect( HandConnect connect )
{
	mConnectDataListLock.lock();

	TCPThreadReceiveConnect *pconnect = dynamic_cast<TCPThreadReceiveConnect*>( connect.getPtr() );
	AssertEx(pconnect, "必须是 TCPThreadReceiveConnect");

	if (mEnterConnectDataList.size()+mConnectCount+1>=NET_CONNECT_MAX)
	{
		TABLE_LOG("Error: 当前连接数量已满 [%d], [%s][%d]连接设置移除", NET_CONNECT_MAX, connect->GetIp(), connect->GetPort());
		pconnect->mBackData->mSocket->close();
		delete pconnect->mBackData;
		pconnect->mBackData = NULL;
		return;
	}
	mHasEnterConnect = true;
	mEnterConnectDataList.insert(pconnect->mBackData);

	TABLE_LOG("成功接受连接>[%s:%d], Now Count [%d]", connect->GetIp(), connect->GetPort(), mEnterConnectDataList.size()+mConnectCount+1 );

	mConnectDataListLock.unlock();
	mMainThreadConnectList.insert(connect);

}

void TCPReceiveThread::backWorkThread( void )
{
	while(IsActive())
	{

		if (m_MaxFD!=INVALID_SOCKET)
		{
			memcpy(m_ReadFDs, m_FDsBak, sizeof(fd_set));
			memcpy(m_WriteFDs, m_FDsBak, sizeof(fd_set));
			memcpy(m_ExceptFDs, m_FDsBak, sizeof(fd_set));
			
			INT iRet = SocketAPI::select_ex( (INT)m_MaxFD+1, 
				m_ReadFDs,
				m_WriteFDs,												// 是否可发送在主线进行检测
				m_ExceptFDs, 
				m_Timeout );
			AssertEx( iRet!=SOCKET_ERROR, "网络状态获取错误" );
		}


		if (mHasEnterConnect)
		{
			mConnectDataListLock.lock();
			for (EasyList<BackReceiveData*>::iterator it=mEnterConnectDataList.begin(); it!=mEnterConnectDataList.end(); ++it)
			{
				mBackConnectDataList.push_back(*it);
				int s = (INT)((*it)->mSocket->getSOCKET());
				if (m_MaxFD==INVALID_SOCKET || s>m_MaxFD)
				{
					m_MaxFD = s;
				}
				FD_SET(s, m_FDsBak);
			}
			mEnterConnectDataList.clear();
			mConnectCount = mBackConnectDataList.size();

			mHasEnterConnect = false;
			mConnectDataListLock.unlock();
		}		

		for (EasyList<BackReceiveData*>::iterator it=mBackConnectDataList.begin(); it!=mBackConnectDataList.end(); )
		{
			BackReceiveData *sData = *it;
			if (sData == NULL)
				continue;

			if (sData->mbNeedFree)
			{
				SOCKET s = sData->mSocket->getSOCKET();
				FD_CLR(s, m_FDsBak);

				delete sData;
				it = mBackConnectDataList.erase(it);
				if (s>=m_MaxFD)
				{
					m_MaxFD = (INVALID_SOCKET);
					for (EasyList<BackReceiveData*>::iterator it2=mBackConnectDataList.begin(); it2!=mBackConnectDataList.end(); ++it2)
					{
						BackReceiveData *sD = *it2;
						int s2 = (int)(sD->mSocket->getSOCKET());
						if (m_MaxFD==INVALID_SOCKET || s2>m_MaxFD)						
							m_MaxFD = s2;
					}
				}
				continue;
			}
			else
			{
				SOCKET s = sData->mSocket->getSOCKET() ;

				if( FD_ISSET(s, m_ExceptFDs) )
				{
					TABLE_LOG( "[%s : %u] 网络异常, 可能断开或关闭", sData->mSocket->getHost(), sData->mSocket->getPort() ) ;				
					sData->m_bDisconnect = true;
				}

				if( FD_ISSET(s, m_ReadFDs) )
					sData->ReceiveDataOnThread();

				if (FD_ISSET(s, m_WriteFDs))
					sData->SendDataOnThread();
			}
			++it;
		}
		TimeManager::Sleep(RECEIVE_THREAD_LOOP_SPACETIME);

	}
}

void TCPReceiveThread::Process()
{
	for (EasyList<HandConnect>::iterator it=mMainThreadConnectList.begin(); it!=mMainThreadConnectList.end(); )
	{
		Hand<TCPThreadReceiveConnect> connect = *it;
		//AssertEx((*it) && connect, "必须是 TCPThreadReceiveConnect");

		if (!connect) 
		{
			it = mMainThreadConnectList.erase(it);
			continue;
		}
		else if ( connect->IsNeedRemove() 
			&& connect->CanRemove()  
			|| connect->IsDisconnect()
			)
		{
			TABLE_LOG("=== Connect set remove, then now remove connect [%s:%d]", connect->GetIp(), connect->GetPort());
			it = mMainThreadConnectList.erase(it);
			//connect._free(); // 这里不作强行释放, 原因处理等待状态的相关事件, 需要使用此连接获取信息, 如 BaseThread			
			connect->OnDisconnect();
		}
		else
		{
			connect->Process();
			++it;
		}
	}
}

void TCPReceiveThread::CpuSleep()
{
	TimeManager::Sleep(1);
}

bool TCPReceiveThread::RemoveConnect( tNetConnect *pConnect )
{
	pConnect->SetRemove(true);
	//for (EasyList<HandConnect>::iterator it=mMainThreadConnectList.begin(); it!=mMainThreadConnectList.end(); ++it)
	//{
	//	if ( (*it).getPtr()==pConnect )
	//	{
	//		it = mMainThreadConnectList.erase(it);
	//		return true;
	//	}
	//}
	return false;
}

void TCPReceiveThread::Close( void )
{
	for (EasyList<HandConnect>::iterator it=mMainThreadConnectList.begin(); it!=mMainThreadConnectList.end(); ++it)
	{
		//HandConnect hConnect = (*it);
		//if (hConnect)
		//	hConnect->SetRemove(true);
		(*it)._free();
	}
	Process();
	mMainThreadConnectList.clear();

	TimeManager::Sleep(1000);

	WorkThread::Close();
}

TCPReceiveThread::TCPReceiveThread()
	: mHasEnterConnect(false)
	, mConnectCount(0)
{
	m_ReadFDs = new fd_set;
	m_WriteFDs = new fd_set;
	m_ExceptFDs = new fd_set;

	FD_ZERO( m_ReadFDs );
	FD_ZERO( m_WriteFDs );
	FD_ZERO( m_ExceptFDs );

	m_FDsBak = new fd_set;

	FD_ZERO( m_FDsBak );

	m_Timeout = new timeval;
	m_Timeout->tv_sec = 0;
	m_Timeout->tv_usec = 20;

	m_MaxFD = (INVALID_SOCKET);
}

TCPReceiveThread::~TCPReceiveThread()
{
	mConnectDataListLock.lock();
	for (EasyList<BackReceiveData*>::iterator it=mEnterConnectDataList.begin(); it!=mEnterConnectDataList.end(); ++it)
	{
		delete (*it);		
	}
	mEnterConnectDataList.clear();
	mConnectDataListLock.unlock();
	delete m_ReadFDs;
	delete m_WriteFDs;
	delete m_ExceptFDs;
	delete m_Timeout;

	delete m_FDsBak;

}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------


BackReceiveData::BackReceiveData() 
	: mHasReceiveData(false)
	, m_bDisconnect(false)
	, mHasSendData(false)
	, mbNeedFree(false)
	, mSocket(NULL)
{
	//mBackThreadReceiveData = NULL;
	//mSocket = new Socket();
	//if (mSocket->create()==FALSE)
	//	return;

	//mSocket->setNonBlocking(TRUE);	
	mBackThreadReceiveData = MEM_NEW SocketInputStream();

	mWillSendData = MEM_NEW SocketOutputStream();
	mBackThreadSendData = MEM_NEW SocketOutputStream();

}

BackReceiveData::~BackReceiveData()
{
 	mSocket->close();
	delete mSocket; 
	mSocket = NULL;

	delete mBackThreadReceiveData;
	mBackThreadReceiveData = NULL;

	delete mWillSendData;
	mWillSendData = NULL;

	delete mBackThreadSendData;
	mBackThreadSendData = NULL;

}

void BackReceiveData::ReceiveDataOnThread()
{
	if (m_bDisconnect)
		return;

	mLock.lock();
	if (mBackThreadReceiveData->Fill(mSocket)>=0)
		mHasReceiveData = true;
	else
	{
		TABLE_LOG("Error: Receive data fail, then set remove [%s : %d]", mSocket->getHost(), mSocket->getPort());
		m_bDisconnect = true;
	}
	mLock.unlock();
}

void BackReceiveData::InitReset()
{
	mLock.lock();
	mSocket->close();
	mBackThreadReceiveData->CleanUp();
	m_bDisconnect = false;
	mHasReceiveData = false;
	m_bNeedRemove = false;
	mLock.unlock();
}

void BackReceiveData::SendDataOnThread( )
{
	if (m_bDisconnect)
		return;

	if (mBackThreadSendData->IsEmpty()==FALSE)
	{
		INT sendDataSize = mBackThreadSendData->Flush(mSocket);
		if (sendDataSize<0)
		{
			TABLE_LOG("Error: Send data fail, then set remove [%s : %d]", mSocket->getHost(), mSocket->getPort());
			//mWillSendData->CleanUp();
			m_bDisconnect = true;
			return;
		}
	}
	if (mHasSendData && mBackThreadSendData->IsEmpty()==TRUE)
	{
		mSendDataLock.lock();
		SocketOutputStream *pTemp = mBackThreadSendData;
		mBackThreadSendData = mWillSendData;
		mWillSendData = pTemp;
		mHasSendData = false;
		mSendDataLock.unlock();
	}
}

// 在主线执行会使用线程死亡
//bool BackReceiveData::IsDisconnect()
//{
//	if (mSocket->available()>0) 
//		return false;
//
//	bool bDisconnect = false;
//	char mCheckDisconnect;
//
//	mLock.lock();
//	bDisconnect = mSocket->receive(&mCheckDisconnect, 1)==0;
//	mLock.unlock();
//
//	return bDisconnect;
//}

//-------------------------------------------------------------------------