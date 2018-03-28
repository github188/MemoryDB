/********************************************************************
	created:	2014/12/31
	created:	31:12:2014   10:49
	filename: 	E:\Home\Code\Server\GameServer\Server\GateNet.h
	file path:	E:\Home\Code\Server\GameServer\Server
	file base:	GateNet
	file ext:	h
	author:		Yang Wenge
	
	purpose:	��ת�ͻ�����DB�ڵ�������
				�������ڲ�ֱ�ӽ����յ�������ת��Ŀ�����ӵķ��ͻ�����
				ʵ�� Client>Gate>DB
					DB>Gata>Client
				Դ��Ϣ���͵İ�ͷ�ڱ����ṩĿ�����ӱ�ʶ, ������תʱ�ҵ�Ŀ������

				ÿ���¼��� _getTarget()�б��淢�͵�Ŀ����Ϣ, 0��ֵ��ʾ���յ�����ִ��
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
		//  ������ת�¼�����ʹ��Э���Ż����޷���ȷ�ж϶Է��Ƿ��Ѿ����յ�Э��
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
	//byte	mTransferTime;	// ��ת����, ����������ڹ涨ֵ��, ���ж���, Ŀǰʹ���������ڵؽ��
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