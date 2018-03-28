/********************************************************************
	created:	2014/12/31
	created:	31:12:2014   10:49
	filename: 	E:\Home\Code\Server\GameServer\Server\GateNet.h
	file path:	E:\Home\Code\Server\GameServer\Server
	file base:	GateNet
	file ext:	h
	author:		Yang Wenge
	
	purpose:	中转客户端与DB节点的网络层
				在连接内部直接将接收的数据中转到目标连接的发送缓存内
				实现 Client>Gate>DB
					DB>Gata>Client
				源消息发送的包头内必须提供目标连接标识, 用来中转时找到目标连接

				每个事件的 _getTarget()中保存发送的目标信息, 0或负值表示接收的连接执行
*********************************************************************/

#ifndef _INLCUDE_GATENET_H_
#define _INLCUDE_GATENET_H_

#include "NetHead.h"
#include "EventProtocol.h"


namespace Logic
{
	class tEventCenter;
}

//-------------------------------------------------------------------------
class Net_Export TransferPacket : public EventPacket
{
	friend class GateEventProtocol;

public:
	TransferPacket()
		: mTargetFlag(0)
		, mTargetID(0)
		, mEventPacketID(PACKET_EVENT)
	{

	}

	virtual void InitData() override
	{
		EventPacket::InitData();
		mTargetFlag = 0;
		mTargetID = 0;
		mEventPacketID = PACKET_EVENT;
	}

	virtual int SetEvent(AutoEvent &hEvent) override
	{
		//  对于中转事件，不使用协议优化，无法正确判断对方是否已经接收到协议
		hEvent->setState(STATE_EVENT_USE_PROTOCOL_SAVE, false);
		hEvent->setState(STATE_EVENT_NEED_SAVE_PROTOCOL, false);
		return EventPacket::SetEvent(hEvent);
	}

public:
	HandPacket CreateSendDataPacket(tNetConnect *pConnect);
	virtual void OnDoEventBefore(Logic::tEvent *pNetEvent){	}

public:
	virtual	PacketID_t	GetPacketID( ) const { return PACKET_TRANSFER_DATA; }

	virtual UINT Execute( tNetConnect* pConnect );

	virtual BOOL Read( LoopDataStream& iStream, size_t packetSize ) 
	{
		if (iStream.Read((char*)&mTargetFlag, sizeof(mTargetFlag))==FALSE)
			return FALSE;
		if (iStream.Read((char*)&mTargetID, sizeof(mTargetID))==FALSE)
			return FALSE;

		if (iStream.Read((char*)&mEventPacketID, sizeof(mEventPacketID))==FALSE)
			return FALSE;

		return EventPacket::Read(iStream, packetSize-_TargetSize());
	}
	virtual BOOL Write( LoopDataStream& oStream )const
	{
		oStream.Write((const char*)&mTargetFlag, sizeof(mTargetFlag));
		oStream.Write((const char*)&mTargetID, sizeof(mTargetID));
		oStream.Write((const char*)&mEventPacketID, sizeof(mEventPacketID));
		return EventPacket::Write(oStream);
	}

	virtual UINT GetPacketSize() const {  return DataPacket::GetPacketSize() + _TargetSize(); }

	int _TargetSize() const { return sizeof(mEventPacketID) + sizeof(mTargetFlag)+sizeof(mTargetID); }


public:
	byte	mEventPacketID;
	byte	mTargetFlag;
	int		mTargetID;
	//byte	mTransferTime;	// 中转次数, 如果次数大于规定值后, 进行丢弃, 目前使用数据所在地解决
};


//-------------------------------------------------------------------------
class Net_Export GateEventProtocol : public EventNetProtocol
{
public:	
	virtual bool Transfer(TransferPacket *pPacket, tNetConnect *pScrConnect) = 0;
	virtual void OnPacketExecuteError(tNetConnect *pConnect, Packet *pPacket)
	{
		WARN_LOG("Net packet execute error>[%d]", pPacket->GetPacketID());		
		//pConnect->SetRemove(true);
	}
	virtual int GetNetFlag() const = 0;
	virtual bool CheckInLocal(TransferPacket *pTransferPacket, tNetConnect* pConnect) = 0;

	virtual void OnNetEventBegin(TransferPacket *sTransferPacket, Logic::tEvent *pNetEvent){}

	virtual Logic::tEventCenter* GetTransferEventCenter(tNetConnect *pConnect);

public:
	virtual HandPacket GenerateEventPacket(tNetConnect *pConnect, Logic::tEvent *pSendEvent, bool bNeedZip = true) override ;

public:
	GateEventProtocol()
	{
		RegisterNetPacket(MEM_NEW DefinePacketFactory<TransferPacket, PACKET_TRANSFER_DATA>());
	}
};
//-------------------------------------------------------------------------

#endif  //_INLCUDE_GATENET_H_