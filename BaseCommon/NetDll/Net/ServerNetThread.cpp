//#include "ServerNetThread.h"
//#include "Socket.h"
//#include "ServerSocket.h"
//
//
//ConnectTask::ConnectTask()
//{
//	mSocket = new Socket;
//}
//
//ConnectTask::~ConnectTask()
//{
//	delete mSocket;
//	mSocket = NULL;
//}
//
//
//ServerNetThread::ServerNetThread()
//	: mServerSocket(NULL)
//{
//	
//}
//
//void ServerNetThread::InitServerSocket( UINT port )
//{
//	mServerSocket = new TCPServerSocket();
//	mServerSocket->InitCreate(port);
//}
//
//void ServerNetThread::backWorkTread(void)
//{
//	while (!mClose)
//	{
//		ConnectTask *p = new ConnectTask();
//		AutoTask hTask = p;
//		if ( mServerSocket->accept(p->mSocket) )
//		{
//			hTask->SetFinish(true);
//			mTaskListLock.lock();
//			mTaskList.insert(hTask);
//			hTask.setNull();
//			mTaskListLock.unlock();
//		}
//	}
//}
//
//void ServerNetThread::Process( void )
//{
//	mTaskListLock.lock();
//	if (!mTaskList.empty())
//	{			
//		for (TaskList::iterator it=mTaskList.begin(); it!=mTaskList.end(); ++it)
//		{
//			if ((*it)->IsFinish())
//			{
//				if ((*it)->TryLock())
//				{
//					ConnectTask *p = dynamic_cast<ConnectTask*>((*it).getPtr());
//					if (p)
//						OnConnect( p->mSocket );
//					(*it)->UnLock();
//				}
//			}
//		}
//		mTaskList.clear();
//	}
//	mTaskListLock.unlock();
//}