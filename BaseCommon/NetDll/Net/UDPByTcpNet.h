#ifndef _INCLUDE_UDPBYTCPNET_H_
#define _INCLUDE_UDPBYTCPNET_H_

#include "IOCPServerNet.h"
#include "IOCPConnect.h"
#include "CEvent.h"

#define  _WAIT_RESEND_TIME          60              // 在此时间内未接收到回复则进行重发
//-------------------------------------------------------------------------*/
enum
{
	UDP_PART_PACKET	 = 100,
	UDP_RESPONSE_PACKET = 101,
	UDP_REQUEST_RESEND_PACKET = 102,
	UDP_CHECK_KEY,
	TCP_NOTIFY_CHECK_KEY,		// 服务端发向客户端
	TCP_NOTIFY_READY_OK,
	TCP_RESPONSE_EMPTY_PART,
};

//-------------------------------------------------------------------------*/
/*/ UDP结合Tcp实现无丢包传输
测试效果优良 (前端向ResourcesServer上传文件) 2017.9.28
	1 当所以数据发送完成后, 计时等待回应序号, 如果未回应对应的序号则使用TCP将最后发送的序号发给对方, 
		对方收到后此时理论上UDP应该已经全部接收到, 通过接收列表分析还未接收的数据包,请求重发
	2 当接收到数据包, 且待处理的数据为空时, 将接收到的最后序号发给对方, 对方接收后清除等待重发的数据
	3 当接收到数据包后, 如果接收列表不为空, 则分析是否存在序号与最后序号相差为3 的包, 存在则请求重发

优点:
	1 使用方便, 抽象为一般的TCP连接, UDP依附在TCP连接上, 自动完成UDP准备及传输接管
	2 提高消息响应速度, 约 20(wifi)~80(4G)
	3 确保无丢失数据传输, 可继承OnConnectFault() 处理网络故障

NOTE: 包序使用 uint 未实现溢出处理, 一次连接可传输约4200G, 理论上可以满足临时P2P房间需求

优化修改 (2017.9.30)
	1 发送一个包就将包序及发送时间和包，加入到发送列表。
	2 接送一段数据后，即把已处理序号和已接收序号UDP回复
	3 等待判断第一个包如果未回复则重发一次，且移除判断重发列表
//-------------------------------------------------------------------------*/
class Net_Export UDPByTcpConnect : public IOCPServerConnect
{
	friend class UDP_RequestResendPart;
	friend class UDP_NotifySendCode;
	friend class UDPPartPacket;

public:
	static void RegisterMsgPacket(tNetProtocol *pPro);

public:
	// 连接出现问题, TCP传输已经出现超时
	virtual void OnConnectFault() { NOTE_LOG("[%s] connet happy fault, now stop net work",  GetIp(), GetPort()); }

	virtual bool Process() override
	{
		_udpConnectProcess();
		return IOCPServerConnect::Process();		
	}

	virtual void _udpConnectProcess();

public:
	virtual HandPacket _createPartPacket() = 0;
	virtual bool _sendByUDP(HandPacket dataPacket) = 0;
	virtual bool _sendByTcp(HandPacket dataPacket) = 0;
	virtual bool UdpReadyOk() const { return mbUDPReadyOk;  }

	virtual void StartSendUDPCheck(UInt64 checkKey);

	void _sendUdpCheck(UInt64 checkKey)
	{
		Auto<DataPacket> pak = _createPartPacket();
		pak->GetData()->seek(0);
		pak->GetData()->write((byte)UDP_CHECK_KEY);
		pak->GetData()->write(checkKey);
		_sendByUDP(pak);
	}

	virtual void SetUDPReadyOk(bool bOk)
	{
		mbUDPReadyOk = bOk;
		mWaitResponeOkEvent._free();
	}

	// 接管所有的发送消息包,使用UDP发送
	virtual bool Send(const Packet *msgPacket, bool bEncrypt) override
	{
		if (!UdpReadyOk())
			return IOCPServerConnect::Send(msgPacket, bEncrypt);

		// 分包
		bool b = GetNetHandle()->GetNetProtocol()->WritePacket(msgPacket, &mUDPSendData);
		if (b)
		{
			_sendAllData();
			return true;
		}
		else
			ERROR_LOG("UDP send [%d] WritePacket fail", msgPacket->GetPacketID());
		return false;
	}

public:
	virtual void _sendAllData();

	// UDP接收数据处理
	virtual void OnReceiveUDPData(DataBuffer *receiveData, int size, bool bUdp);

	void _CheckProcessWaitReceivePartList();

	// 处理合并最终接收的数据流
	void _ProcessUDPReceive();

	// 请求重发, UDP和TCP同时发送
	void _requestResend(uint nPartCode);

	//// NOTE: 由TCP通知当前发送所有包的最后序号，因为TCP速度慢于UDP,此时理论上都已经接收到所有包,所以未接收到的全部进行重发处理
	//void OnNotifyLastSendCode(uint lastSendCode)
	//{
	//	if (lastSendCode>mReceiveCode)
	//	{			
	//		for (uint i=mReceiveCode+1; i<=lastSendCode; ++i)
	//		{
	//			if (!mReceivePartDataList.exist(i))
	//				_requestResend(i);
	//		}
	//	}
	//}

	void _checkResend()
	{
		if (mReceivePartDataList.empty())
			return;
		uint lastCode = mReceivePartDataList.last()->mKey;
		for (uint i=mReceiveCode+1; i<lastCode; ++i)
		{
			if (!mReceivePartDataList.exist(i) && (lastCode-i)>3)
				_requestResend(i);
		}
	}

	HandPacket _findSendPacket(int nCode)
	{
		for (auto it=mSendPacketList.begin(); it!=mSendPacketList.end(); ++it)
		{
			if ((*it).mCode==nCode)
				return (*it).mPartPacket;
		}
		return HandPacket();
	}

public:
	UDPByTcpConnect()
		: mSendCode(0)
		, mReceiveCode(0)
		, mbUDPReadyOk(false)
		, mTargetAddr(0)
	{

	}
	~UDPByTcpConnect()
	{
		mWaitResponeOkEvent._free();
	}

public:
	UInt64			mTargetAddr;						// 远端地址

protected:
	// UDP 待发送的数据流
	LoopDataStream		mUDPSendData;
	LoopDataStream		mUDPReceiveData;
	struct SendPartData
	{
		uint				mCode;
		UInt64			mSendTime;
		HandPacket	mPartPacket;
		bool				mbResend;

		SendPartData()
			: mbResend(false)
			, mCode(0)
			, mSendTime(0) {}
	};
	PoolList<SendPartData>				mSendPacketList;
	//EasyMap<uint, HandPacket>		mSendPacketList;
	struct RevPartData
	{
		HandPacket	mPartPacket;
		bool				mbResponse;

		RevPartData()
			: mbResponse(false){}
	};
	EasyMap<uint, RevPartData>		mReceivePartDataList;
	EasyMap<uint, AutoEvent>			mWaitResendList;
	//EasyMap<uint, AutoEvent>			mWaitSendLastSendCodeList;
	uint		mSendCode;
	uint		mReceiveCode;
	AutoEvent	mWaitResponeOkEvent;
	bool mbUDPReadyOk;
};

//-------------------------------------------------------------------------*/
class UDPPartPacket : public DataPacket
{
public:	
	virtual UINT Execute( tNetConnect* pConnect );
	virtual	PacketID_t	GetPacketID( ) const { return UDP_PART_PACKET; }

public:
	UDPPartPacket()
	{
		mData = MEM_NEW DataBuffer(MAX_UDP_MSG_SIZE+60);
	}
};

//-------------------------------------------------------------------------*/
class UDP_RequestResendPart : public DataPacket
{
public:
	virtual UINT Execute( tNetConnect* pConnect );
	virtual	PacketID_t	GetPacketID( ) const { return UDP_REQUEST_RESEND_PACKET; }

	UDP_RequestResendPart(){ mData = MEM_NEW DataBuffer(64); }
};

//-------------------------------------------------------------------------*/
class TM_WaitTcpResponseResendPart : public Logic::CEvent
{
public:
	virtual bool _OnTimeOver() override
	{
		ERROR_LOG("[%s:%d] Connect may be disconnect, part code >%u", mConnect->GetIp(), mConnect->GetPort(), mWaitCode);
		Hand<UDPByTcpConnect> conn = mConnect;
		if (conn)
			conn->OnConnectFault();
		return true;
	}

public:
	int mWaitCode;
	HandConnect mConnect;
};
class TM_WaitSendUDPCheckEvent : public Logic::CEvent
{
public:
	virtual bool _DoEvent() override
	{
		_OnTimeOver();
		return true;
	}

	virtual bool _OnTimeOver() override
	{
		if (++mCount<=10)
		{
			mpConnect->_sendUdpCheck(mCheckKey);
			WaitTime(2);
		}
		else
			ERROR_LOG("尝试发送UDP准备验证超时, 超过10次");
		return true;
	}

public:
	TM_WaitSendUDPCheckEvent()
		: mCount(0){}

	virtual void InitData() override
	{
		Logic::CEvent::InitData();
		mCount = 0;
	}

public:
	int mCount;
	UDPByTcpConnect *mpConnect;
	UInt64	mCheckKey;
};
//-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*
// 准备时用到的消息
// 1 TCP连接后, 发送到客户端的要求UDP开始发送验证
class TCP_NotifyUDPCheckKeyMsg : public DataPacket
{
public:
	virtual UINT Execute( tNetConnect* pConnect )
	{
		mData->seek(0);
		UInt64 checkKey = 0;
		mData->read(checkKey);
		Hand<UDPByTcpConnect> conn = pConnect;
		conn->StartSendUDPCheck(checkKey);
		return 0;
	}
	virtual	PacketID_t	GetPacketID( ) const { return TCP_NOTIFY_CHECK_KEY; }

public:
	TCP_NotifyUDPCheckKeyMsg()
	{
		mData = MEM_NEW DataBuffer(16);
	}
};
// 2 UDP服务端准备好后,通知客户端UDP准备OK, 开始UDP传输
class TCP_NotifyUDPReadyOkMsg : public DataPacket
{
public:
	virtual UINT Execute( tNetConnect* pConnect );
	virtual	PacketID_t	GetPacketID( ) const { return TCP_NOTIFY_READY_OK; }

	TCP_NotifyUDPReadyOkMsg(){ mData = MEM_NEW DataBuffer(); }
};
//-------------------------------------------------------------------------*/
#endif //_INCLUDE_UDPBYTCPNET_H_