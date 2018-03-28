#ifndef _INCLUDE_IOCPUDP_H_
#define _INCLUDE_IOCPUDP_H_
#include "Hand.h"
#include "NetHead.h"
#include "BaseCommon.h"
#include "DataBuffer.h"
#include "EasyStack.h"

#define IOCP_POST_UDP_RECEIVE_COUNT   16
#define _UDP_EX_DATA_LENGTH					20

#define _UDP_LOCK_RECEIVE_DATA	CsLockTool l(mRevDataLock);
#define _UDP_LOCK_SEND_DATA		CsLockTool l(mSendLock);

class WorkThread;
class DataBuffer;

struct UDPSendOverlapped;
struct UDPRevOverlapped;

class Net_Export IOCPUDPNet : public Base<IOCPUDPNet>
{
	friend class UDPCompletionThread;
	friend class UDPSendData;
public:
	bool Start(const char *szIP, int port, int threadCount = 2);
	void Close();

	bool Send(UInt64 targetAddr, const char *szSendData, int sendSize);
	AutoData ReadySend(UInt64 targetAddr);

	virtual void OnReceive(UInt64 targetAddr, const char *revData, int revSize) = 0;
	
	virtual void Process();

	bool SendTo(UDPSendOverlapped *pSendData);

protected:
	void _ProcessCompletion();
	bool _BeginReceive(UDPRevOverlapped *pRev);

public:
	IOCPUDPNet();
	~IOCPUDPNet();

protected:
	void				*m_hCompletionPort;
	void*				mIocpHandle;
	Array<WorkThread*>	mpCompletionThread;
	size_t			mSocketID;

	bool			mbClosed;
	bool			mbHaveRev;
	UInt64			mLastSendTime;

	AString			mIp;
	int				mPort;

protected:
	Array<UDPRevOverlapped*> mThreadReceiveDataList;
	Array<UDPRevOverlapped*> mMainReceiveDataList;

	EasyStack<UDPRevOverlapped*>	mFreeRevData;
	Array<UDPRevOverlapped*>		mAllRevList;

	Array<UDPSendOverlapped*>		mAllSendBufferList;
	EasyStack<UDPSendOverlapped*>	mFreeSendList;
	
	CsLock			mRevDataLock;
	CsLock			mSendLock;

};

#endif // _INCLUDE_IOCPUDP_H_
