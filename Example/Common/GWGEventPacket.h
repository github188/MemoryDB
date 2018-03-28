#ifndef _INCLUDE_GWGEVENTPACKET_H_
#define _INCLUDE_GWGEVENTPACKET_H_

#include "EventPacket.h"
#include "CommonDefine.h"

//-------------------------------------------------------------------------*/
class GWGEventPacket : public EventPacket
{
public:
	GWGEventPacket()
		: mTargetServerID(0)
		, mScrServerID(0)
	{ }

	virtual void InitData() override 
	{
		EventPacket::InitData();
		mTargetServerID = 0;
		mScrServerID = 0;
	}

	virtual	PacketID_t	GetPacketID( ) const override { return GWG_EVENT_PACKET_ID; }

	virtual BOOL Read( LoopDataStream& iStream, size_t packetSize ) 
	{
		if (iStream.Read((char*)&mTargetServerID, sizeof(mTargetServerID))==FALSE)
			return FALSE;
		if (iStream.Read((char*)&mScrServerID, sizeof(mScrServerID))==FALSE)
			return FALSE;

		return EventPacket::Read(iStream, packetSize-_TargetSize());
	}

	virtual BOOL Write( LoopDataStream& oStream )const
	{
		oStream.Write((const char*)&mTargetServerID, sizeof(mTargetServerID));
		oStream.Write((const char*)&mScrServerID, sizeof(mScrServerID));
		return EventPacket::Write(oStream);
	}

	virtual UINT GetPacketSize() const {  return EventPacket::GetPacketSize() + _TargetSize(); }

	int _TargetSize() const { return sizeof(mTargetServerID) + sizeof(mScrServerID); }

public:
	UInt64 mTargetServerID;
	UInt64 mScrServerID;
};
//-------------------------------------------------------------------------*/

#endif //_INCLUDE_GWGEVENTPACKET_H_