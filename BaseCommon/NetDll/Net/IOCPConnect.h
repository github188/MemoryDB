/*/-------------------------------------------------------------------------
NOTE:
当前使用发送和接收的缓存直接投递到完成端口线程
在执行期内不可以对发送和接收的缓存进行动态修改空间大小
//-------------------------------------------------------------------------*/
#ifndef _INCLUDE_IOCPCONNECT_H_
#define _INCLUDE_IOCPCONNECT_H_

#include "NetHead.h"
#include "LoopDataStream.h"
#include "NetHandle.h"
#include "Lock.h"
#include "LoopList.h"
//-------------------------------------------------------------------------
// 
#if _IOCP_THREAD_RECEIVE_SEND
#	define _IOCP_LOCK_SEND		mSendLock.lock();
#	define _IOCP_UNLOCK_SEND		mSendLock.unlock();

#	define _IOCP_LOCK_RECEIVE		mRevLock.lock(); //mReceiveLock.lock();
#	define _IOCP_UNLOCK_RECEIVE		mRevLock.unlock(); //mReceiveLock.unlock();

#	define _IOCP_AUTO_LOCK_SEND	CsLockTool t(mSendLock); //CsLockTool t(mReceiveLock);
#	define _IOCP_AUTO_LOCK_RECEIVE	CsLockTool t(mRevLock);

#else
//#	define _IOCP_LOCK_SEND
//#	define _IOCP_UNLOCK_SEND		
#	define _IOCP_LOCK_RECEIVE		
#	define _IOCP_UNLOCK_RECEIVE		

#	define _IOCP_LOCK_SEND		
#	define _IOCP_UNLOCK_SEND	

#  define _IOCP_AUTO_LOCK_SEND
#	define _IOCP_AUTO_LOCK_RECEIVE	

#endif
//-------------------------------------------------------------------------
struct SendOverlapped;
struct RecvOverlapped;
struct PacketData;
class EventData;

class NoResizeLoopBuffer : public LoopDataStream
{
public:
	virtual BOOL	Resize( INT size ) override { return FALSE; }

	NoResizeLoopBuffer(UINT BufferSize = DEFAULTSOCKETINPUTBUFFERSIZE, UINT MaxBufferSize = DISCONNECTSOCKETINPUTSIZE)
		: LoopDataStream(BufferSize, MaxBufferSize){}
};

class Net_Export IOCPConnect : public tNetConnect
{
	friend class IOCPBaseNet;
	friend class IOCPServerNet;
	friend class IOCPClientNet;
	friend class IOCPClientSetNet;
	friend class MeshedNodeNet;
	friend class EventNetProtocol;
public:
	IOCPConnect(int revBufferLen = 0, int sendBufferLen = 0);
	virtual ~IOCPConnect();

public:
	void Init(AutoNet ownerNet, UInt64 socketID, const char *szIp, int port)
	{
		mNet = ownerNet;
		mSocketID = socketID;
		mIp = szIp;
		mPort = port;
	}

	//NOTO: NET ID 可以由逻辑层进行自由设定, 连接管理中不使用此ID, 所有可以与连接列表下标不相符
	virtual void SetNetID(int netID) 
	{ 
		mID = netID; 
	}
	virtual int GetNetID() const { return mID; }

	virtual void SetRemove(bool bNeedRemove);
	virtual bool IsRemove(){ return mNeedRemove; }
	virtual bool IsDisconnect() const { return mNeedRemove; }
	virtual const char* GetIp(){ return mIp.c_str(); }
	virtual int GetPort(){ return mPort; }
	virtual int GetLocalPort();
	virtual AString GetRemoteAddr(int &port);
	virtual void Close(){ SetRemove(true); }

	virtual bool _checkOk() const { return true; }

public:
	virtual bool SendEvent(Logic::tEvent *pEvent);
	virtual bool Send(const Packet  *msgPacket, bool bEncrypt) override;

	virtual tNetHandle* GetNetHandle(void){ return mNet.getPtr(); }

	virtual bool CheckSameConnect(tNetConnect *pConnect) { return pConnect==this; }


public:
	virtual void OnConnected(){}
	virtual void OnDisconnect(){ /*WARN_LOG("[%s:%d] disconnect!", GetIp(), GetPort());*/ }
	virtual void OnSucceedSendEvent(Logic::tEvent *pEvt, Packet *p) {}

	virtual bool Process();

	void Log( const char *szInfo, ... );

protected:
	virtual void _SendTo();
	virtual void _PostReceive();
	virtual void _Free();
	virtual void _ProcessReceiveData();

	// NOTE: 多线接收时, 逻辑线程处理接收到的消息数据
	virtual void _ProcessMsgData();
    // NOTE: IOCP线程回调
    void _OnThreadSendFinish(SendOverlapped* pSendOverlapped, DWORD size);
    void _OnThreadRevFinish(RecvOverlapped* pRecvOverlapped, DWORD size);

	void _Start();

    void _Send(const char *szSendData, DWORD size)
    {
        _IOCP_LOCK_SEND
            mSendData.Write(szSendData, size);		
        _IOCP_UNLOCK_SEND
    }

protected:
	int			mID;
	AutoNet		mNet;
	EasyString	mIp;
	int			mPort;

protected:
    bool		mNeedRemove;
    bool        mbHaveReceiveEventMsg;
    bool        mbHaveRevData;
	LoopDataStream			mReceiveData;
	NoResizeLoopBuffer		mSendData;
	UInt64				mLastSendTime;

	UInt64				mSocketID;

	void*				mIocpHandle;

	SendOverlapped		*mSendOverlapped;
	RecvOverlapped		*mRecvOverlapped;	
	LoopList<HandPacket> mWaitSendPacketList;

#if _IOCP_THREAD_RECEIVE_SEND
	//CsLock			mSendLock;
	//CsLock			mReceiveLock;
	CsLock				        mRevLock;
    CsLock                      mSendLock;

	struct MsgData
	{
		EventData *mpEventData;
		PacketData *mpPacketData;

		MsgData() : mpEventData(NULL), mpPacketData(NULL) {};
		MsgData(EventData *pEvtData, PacketData *pPakData) : mpEventData(pEvtData), mpPacketData(pPakData) {}
	};
	Array<MsgData> mReceiveDataList;
	Array<MsgData> mProcessDataList;			// 用于快速交换的, 目的减少处理时锁时间

#endif

#if _DEBUG_NET_
	UInt64				mRemoveTime;
#endif
};
//-------------------------------------------------------------------------
class Net_Export IOCPServerConnect : public IOCPConnect
{
	friend class IOCPServerNet;

public:
	IOCPServerConnect(int revBufferLen = 0, int sendBufferLen = 0)
		: IOCPConnect(revBufferLen, sendBufferLen)
		, mbReceiveData(false)
		, mStartTime(0)		
	{

	}

protected:
	virtual void _ProcessReceiveData();
	virtual bool Process();

	virtual void OnReceivePacket(Packet *pPacket);

	virtual bool _checkOk() const override { return mbReceiveData; }

	virtual void OnConnectSafeCodeError(const char *szIp, int port, int safeCode){}

protected:
	bool					mbReceiveData;
	UInt64					mStartTime;
};

//-------------------------------------------------------------------------

#endif //_INCLUDE_IOCPCONNECT_H_