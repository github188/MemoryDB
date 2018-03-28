#ifndef _INCLUDE_UDPMESSAGE_H_
#define _INCLUDE_UDPMESSAGE_H_

#include "NetHead.h"
#include "UDPThreadNet.h"

#include "EasyMap.h"
#include "Array.h"
#include <EasyHash.h>


using namespace std;
//using namespace stdext;

//-------------------------------------------------------------------------
#define MAX_WAIT_MSG_COUNT		(5)			// 最大256


//-------------------------------------------------------------------------
// 丢包模式消息处理, 此功能, C# 中也要实现, 用于接收可丢包的数据, 如语音或视频
// UDP包头数据: 发包时间4字节(相对开机毫秒时间）; 1字节 消息编号(用于快速取包,判断丢包); 1字节消息分包编号(0 号包内第三个字节是 分包数量)
// NOTE: 消息头信息保存在包数据后面，目的为了方便处理包数据
//-------------------------------------------------------------------------
class UDPMessageNet;

class Net_Export UDPConnect : public Base<UDPConnect>
{
	friend class UDPMessageNet;

public:

	// DataBuffer 消息的分包数据
	typedef Array<DataBuffer>	MsgDataArray;
	struct MsgData
	{
		UINT			mTime;			// 发包时开机毫秒时间
		byte			mPartCount;
		MsgDataArray	mMsgData;
		bool			mUse;

		MsgData()
		{
			Init();
		}

		void Init()
		{
			mUse = false;
			mTime = 0;
			mPartCount = 0;
			for (size_t i=0; i<mMsgData.size(); ++i)
			{
				mMsgData[i].setDataSize(0);
			}
		}
	};
	typedef Array<MsgData*>	MsgList;

public:
	virtual void OnReceivePacket(DataStream *revPacket, int packetSize){}
	virtual bool Process(UDPMessageNet *pUDPMessageNet){ return true; }
	virtual void Close(){}

	// 出现丢包
	virtual void OnHappenGiveupPacket(int beginCode, int endCode, struct MsgData *msg = NULL)
	{
		WARN_LOG("发现UDP丢包>[%d - %d]", beginCode, endCode);
	}

public:
	UDPConnect();

	~UDPConnect();



public:
	virtual void OnReceiveMsg(DataStream *revData, int revSize);

	void _AppendMsgData(MsgData *msg, byte partCode, DataStream *msgData, int msgDataSize);

	//void _CheckClear();

	void _CheckRunMsg();


	bool _RunMsg(MsgData *msg);

	bool _HaveWaitMsg()
	{
		for (int i=0; i<mWaitMsgList.size(); ++i)
		{
			if (mWaitMsgList[i]->mUse)
				return true;
		}
		return false;
	}

protected:
	// bAddPacketCode 递增UDP连接的包号
	void SendData(UDPMessageNet *net, byte serverMsgCode, DataStream *sendData, UInt64 addr, bool bAddPacketCode);

//	MsgData* GetFreeMsgData();

protected:
	MsgList						mWaitMsgList;	
	EasyMap<UINT, byte>			mTimeSqrtList;			// 用于按时间先后排序

	int							mLastRunMessageCode;	// 仅用于丢包判断,当前执行包序不等于此值+1，即发现丢包

	byte						mSendPacketCode;
};

typedef Hand<UDPConnect>		AutoUDPConnect;

//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
class Net_Export UDPMessageNet : public UDPThreadNet
{
public:
	virtual void OnReceivePacket(UInt64 scrAddr, DataStream *revPacket, int packetSize){}
	virtual AutoUDPConnect NewUDPConnect(UInt64 scrAddr);
	virtual AutoUDPConnect RemoveUDPConnect(UInt64 scrAddr, AutoUDPConnect connect);

	virtual void AppendUDPConnect(UInt64 scrAddr, AutoUDPConnect udpConn);

	virtual AutoUDPConnect OnReceiveNewConnect(UInt64 newAddr);

	virtual AutoUDPConnect GetUDPConnect(UInt64 addr);

	typedef EasyHash<UInt64, AutoUDPConnect>	UDPList;
public:
	virtual void OnReceiveMsg(UInt64 scrAddr, DataStream *revData, int revSize);

	virtual bool SendPacket(UInt64 targetAddr, DataStream *sendData, int sendSize, Array<UInt64> *targetList = NULL);
	virtual bool BocastPacket(DataStream *sendData, int sendSize, UInt64 exclude);

	virtual void Process();

	virtual void Close();

public:
	UDPMessageNet();

protected:
	UDPList			mUDPConnectList;

	DataBuffer		mSendDataBuffer;
	Array<UInt64>	mBocastList;

	byte				mServerPacketCode;
};
//-------------------------------------------------------------------------
class Net_Export DefaultUDPConnect : public UDPConnect
{
public:
	virtual void OnReceivePacket(DataStream *revPacket, int packetSize);

public:
	DefaultUDPConnect(UDPMessageNet *pUdp, UInt64 scrAddr)
		: mpUDPNet(pUdp)
		, mScrAddr(scrAddr)
	{

	}

protected:
	UDPMessageNet	*mpUDPNet;
	UInt64			mScrAddr;
};
//-------------------------------------------------------------------------

#endif //_INCLUDE_UDPMESSAGE_H_