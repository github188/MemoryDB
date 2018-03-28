
#include "TCPListenThread.h"
#include "TCPReceiveThread.h"
#include "ServerSocket.h"

#include "TableTool.h"

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
void TCPListenThread::InitStart( const char* szIp, int nPort )
{
	mListenSocket = new TCPServerSocket();	
	bool b = mListenSocket->InitCreate(nPort);
	TABLE_LOG("初始服务网络 [%s:%d] > [%s]", szIp, nPort, b? "成功":"失败");
	if (!b)
	{
#if __WINDOWS__
		throw (std::exception("初始创建侦听网络失败"));
#elif __LINUX__
		throw (1);
#endif
		return;
	}

	// NOTE: 当前使用阻塞方式监听
	mListenSocket->setNonBlocking(FALSE);	

	WorkThread::InitThread();
}

TCPListenThread::~TCPListenThread()
{
	Close();
	if (mListenSocket)
		delete mListenSocket;

	mListenSocket = NULL;
}

void TCPListenThread::backWorkThread()
{
	while (IsActive())
	{
		Socket *pSocket = new Socket();
		if (pSocket->create()==FALSE)
		{
			AssertEx(0, "初始创建Socket失败");
			return;
		}

		pSocket->setNonBlocking(TRUE);	
		if (TRUE==mListenSocket->accept(pSocket))
		{
			mConnectListLock.lock();
			mConnectList.push_front(pSocket);
			mbHasConnect = true;
			mConnectListLock.unlock();
		}
		else
			delete pSocket;
	}
}

void TCPListenThread::Process()
{
	if (mbHasConnect)
	{
		mConnectListLock.lock();
		mMainConnectList.swap(mConnectList);
		mbHasConnect = false;
		mConnectListLock.unlock();

		for (PoolList<Socket*>::iterator it=mMainConnectList.begin(); it!=mMainConnectList.end(); ++it)
		{
			HandConnect connect = _CreateNewConnect();
			TCPThreadReceiveConnect *p = dynamic_cast<TCPThreadReceiveConnect*>(connect.getPtr());
			AssertEx(p, "必须使用 TCPThreadReceiveConnect");
			if (p->mBackData->mSocket!=NULL)
				delete p->mBackData->mSocket;

			p->mBackData->mSocket = *it;

			OnAcceptConnect(connect);
		}
		mMainConnectList.clear();
		
	}
}
//-------------------------------------------------------------------------
