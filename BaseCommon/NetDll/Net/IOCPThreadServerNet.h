#ifndef _INCLUDE_IOCPTHREADSERVERNET_H_
#define _INCLUDE_IOCPTHREADSERVERNET_H_

#include "MemBase.h"
#include "NetHead.h"
#include "Array.h"
#include "WorkThread.h"
#include "LoopDataStream.h"

//-------------------------------------------------------------------------
// 支持连接转移的IOCP服务网络
// NOTE:  连接绑定一次端口会不能更新，因此必须使用多线接收及发送
// 1 监听接收后，绑定端口， 然后分配到指定线程中的网络管理中, 只需要其中一个线程启动监听即可
// 2 每个线程中的管理都支持转移功能
// 3 为了支持多线,不使用任何智能对象
//-------------------------------------------------------------------------
class IOCPThreadConnect;
class ListenThreadEx;
class IOCPProcessThread;
class Net_Export IOCPThreadServerNet :  public WorkThread
{
	friend class IOCPThreadConnect;
	friend class IOCPProcessThread;
public:	
	virtual bool SendData(int connectID, void *checkParam, const char *pSendData, int sendSize);

	virtual void ProcessReceive(int connectID, void *checkParam, void *processTool);

	virtual bool CheckConnect(IOCPThreadConnect *pConn, void *checkParam){ return true; }

public:
	virtual IOCPThreadConnect* CreateConnect();

	// 追加连接 NOTE: 需要保证线程安全
	// virtual bool AppendConnect(IOCPThreadConnect *pConnect);

	// NOTE: 监听线程内回调
	void OnAcceptConnect(size_t connectSocket);

	bool InitStart(const char *szIp, int port, int threadCount = 2);

	void Close();

	// NOTE: 阻塞关闭所有连接
	bool CloseAllConnect(int waitSecondTime = 10);

	void RemoveConnect(IOCPThreadConnect *pConn, bool pDelete = true);

	//void _RemoveConnect(IOCPThreadConnect *pConn, bool pDelete);

	//NOTE: 低速线程调用, 只用来处理连接移除
	void Process();

	virtual void backWorkThread() override;

public:
	IOCPThreadServerNet()
		: mpListen(NULL)
	{

	}
	~IOCPThreadServerNet()
	{
		Close();
	}

protected:
	struct ConnectData
	{
		IOCPThreadConnect	*mpConnect;
		CsLock	mConnectLock;

		ConnectData()
			: mConnectLock()
			, mpConnect(NULL)
		{

		}
	};
	Array<ConnectData> mConnectList;
	Array<IOCPProcessThread*> mIOCPThreadList;

	CsLock	mLock;

private:
	ListenThreadEx		*mpListen;
};
//-------------------------------------------------------------------------
struct SendOverlapped;
struct RecvOverlapped;
//-------------------------------------------------------------------------
class Net_Export  IOCPThreadConnect :  public MemBase
{
public:
	virtual void ProcessReceive(void *processTool){}

public:
	// 脱离当前网络 NOTE: 必须确保线程安全
	//virtual bool LeaveNet()
	//{
	//	mConnectLock.lock();
	//	mpNet->RemoveConnect(this, false);
	//	mConnectLock.unlock();
	//	return true;
	//}

	// 在接收线程回调
	virtual void OnReceiveData(void *pData, int  size);

	// NOTE: 这个会把此连接实例直接释放掉, 如果有端口未完成, 则会在完成后在接收线程内进行删除
	void Close()
	{
		mpNet->RemoveConnect(this, true);
	}

	bool Send( void *sendData, size_t size )
	{
		CsLock *pLock = getLock();
		if (pLock==NULL)
			return false;

		pLock->lock();
		bool b = mSendData.Write((const char*)sendData, size)==size;
		_SendTo();
		pLock->unlock();
		return b;
	}

	void _SendTo();
	void _PostReceive();

	bool IsRemove(){ return false; }

	void SetRemove(bool bRemove);

	bool CheckRemove();

	const char* GetIp(){ return  mIp; }
	int GetPort(){ return mPort; }
	int GetLocalIPPort(char *ip);

	int GetNetID() const { return mNetID; }

	CsLock* getLock();

	void lock()
	{
		CsLock *pLock = getLock();
		if (pLock!=NULL)
			pLock->lock();
	}

	void unlock()
	{
		CsLock *pLock = getLock();
		if (pLock!=NULL)
			pLock->unlock();
	}

public:
	IOCPThreadConnect(IOCPThreadServerNet *pNet);
	virtual ~IOCPThreadConnect();

public:
	SendOverlapped		*mSendOverlapped;
	RecvOverlapped		*mRecvOverlapped;	

	//CsLock mRevLock;
	//CsLock mSendLock;
	LoopDataStream		mSendData;

	size_t mSocketID;
	char mIp[20];
	int mPort;

	UInt64 mLastSendTime;
	bool mNeedRemove;
	IOCPThreadServerNet *mpNet;
	int mNetID;
};
//-------------------------------------------------------------------------
// IOCP 处理线程， 实例多个，保存在开户监听的网络管理中
class IOCPProcessThread : public WorkThread
{
public:
	IOCPProcessThread(ListenThreadEx *pListen)
		: mpListen(pListen)
	{

	}

	virtual void onBeforeClose(void) override { _ForceClose(10); };

	virtual void backWorkThread(void);

	void Log(const char *sz, ...){}

protected:
	ListenThreadEx	*mpListen;
};
//-------------------------------------------------------------------------
#endif //_INCLUDE_IOCPTHREADSERVERNET_H_