
#ifndef _INCLUDE_EVENTPACKET_H_
#define _INCLUDE_EVENTPACKET_H_

#include "Packet.h"
#include "Event.h"
#include "NiceData.h"
#include "EventCenter.h"

#include "NetHead.h"
#include "PacketFactory.h"


enum NET_PACKET_ID
{
	PACKET_NONE		= 0,
	PACKET_CHECK_REQUEST	= 1,		// ������֤������Ϣ
	PACKET_CHECK_RESPONSE	= 2,		// ������֤�ظ���Ϣ
	PACKET_COMPRESS_EVENT	= 3,		// �Ż�,ֻ�����ݵ��¼���Ϣ,�ֶ���Ϣ�̶���������
	PACKET_PART_PACKET		= 4,		// �ְ�
	PACKET_QUEST_REPEAT_SEND = 5,		// �����ط�ָ���İ�, �����Ƕ���
	PACKET_QUEST_REPEAT_CONNECT = 6,	// ������������
	PACKET_EVENT			= 7,		// Ĭ��һ����¼���Ϣ
	PACKET_TRANSFER_DATA	= 8,
	eNotifyNetEventID		= 9,
	eNotifyHeartBeat        = 10,		// ���������ɷ�����ͳһÿ��Լ10�����������ӷ���һ��������
	PACKET_EVENT_PROCESS = 11, // ֱ�Ӷ�ȡ������¼��� 
	PACKET_MAX,
};	
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class Net_Export DataPacket : public Packet
{
public:
	DataPacket()
		: mbAlreadyEncrypt(false)
	{

	}

	virtual void InitData() override 
	{ 
		mStateData.init(true);
		//mbNeedEncrypt = false; 
		mbAlreadyEncrypt = false; 
		//if (mData)
		//	mData->clear(false);
	}

public:
	virtual bool SetData(AutoData hData, size_t size)
	{
		mData = hData;
		if (mData)
			mData->seek(size);
		return (bool)mData;
	}

	AutoData& GetData() { return mData; }

	void _SetSendData(AutoData hData, size_t size){ mData = hData; if (mData) mData->seek(size); }	

	virtual	VOID		SetPacketID( PacketID_t id )  {};

	//���ü̳нӿ�
	virtual BOOL Read( LoopDataStream& iStream, size_t packetSize ) 
	{
		size_t size = packetSize;
		if (!mData)
			mData = MEM_NEW DataBuffer(size);
		else if (mData->size()<(DSIZE)size)
			mData->resize(size);
		if (iStream.Read(mData->data(), (UINT)size)==size)
		{
			mData->seek(size);
			mData->setDataSize(size);

			_OnReceiveData(mData->data(), size);

			return TRUE;
		}
		return FALSE;
	}
	virtual BOOL Write( LoopDataStream& oStream )const
	{
		if (!mData)
			return FALSE;
		size_t size = mData->dataSize();

		const char *szData = ((DataStream*)mData.getPtr())->data();
		_OnSendData((void*)szData, size);

		if (oStream.Write( szData, (UINT)size)==size)
			return TRUE;

		return FALSE;
	}

	virtual UINT GetPacketSize() const {  if (mData) return (UINT)mData->dataSize(); else return (UINT)0; }

protected:
	bool _OnSendData(void *szSendData, size_t size) const;
	bool _OnReceiveData(void *szRevData, size_t size);

	virtual void SetNeedEncrypt(bool bNeed)const{ mStateData.set(ePacketStateEncrypt, bNeed); }
	bool _EncryptData( void *scrData, DSIZE length ) const;
	bool _DecryptData( void *scrData, DSIZE length ) const;

	virtual UINT		GetState() const { return ((StateDataType)mStateData) & 0xFF; }
	virtual VOID		SetState(UINT stateData) { mStateData = (StateDataType)stateData; }

	bool HasState(StateDataType state) const { return mStateData.isOpen(state); }

protected:
	AutoData				mData;
	mutable StateData		mStateData;
	mutable bool			mbAlreadyEncrypt;
};
//-----------------------------------------------------------------------------
class tNetHandle;


class Net_Export EventPacket : public DataPacket
{
public:
	EventPacket();

	~EventPacket()
	{
	}

public:
	virtual int SetEvent(AutoEvent &hEvent);
	virtual AutoData GetData(void){ return mData; }

	virtual UINT Execute( tNetConnect* pConnect );

	virtual	PacketID_t	GetPacketID( ) const { return PACKET_EVENT; }

	virtual AutoData& GetEventData(){ return mData; }
	virtual void SwapEventData( AutoData &eventData );

	virtual bool ReadySendData(){ return true; }

};
//-----------------------------------------------------------------------------
// �Ż���ȡ�ָ���Ϣ�¼�
class Net_Export EventProcessPacket : public EventPacket
{
public:
	virtual	PacketID_t	GetPacketID( ) const { return PACKET_EVENT_PROCESS; }
	virtual BOOL		Read( LoopDataStream& iStream, size_t packetSize )
	{ AssertEx(0, "����ֱ��ʹ��Read"); return FALSE; }
	// ��������
	virtual BOOL ReadEvent(tNetConnect *pConnect,  LoopDataStream& iStream, size_t packetSize );

	virtual UINT Execute( tNetConnect* pConnect ) override;

	virtual void InitData() override 
	{ 
		EventPacket::InitData();
		mStateData.init(true);
		mMsgEvent.setNull();
	}

public:
	AutoEvent mMsgEvent;
};

//-----------------------------------------------------------------------------
class CheckRequestPacket : public DataPacket
{
public:
	CheckRequestPacket()
	{
		mData = MEM_NEW DataBuffer();
	}
	bool SetNiceData(const NiceData &niceData)
	{		
		return niceData.serialize(mData.getPtr(), true);
	}

	virtual UINT Execute( tNetConnect* pConnect );

	virtual	PacketID_t	GetPacketID( ) const { return PACKET_CHECK_REQUEST; }

};

//-----------------------------------------------------------------------------
class CheckResponsePacket : public DataPacket
{
public:
	CheckResponsePacket()
	{
		mData = MEM_NEW DataBuffer();
	}
	bool SetNiceData(const NiceData &niceData)
	{		
		return niceData.serialize(mData.getPtr(), true);
	}

	virtual UINT Execute( tNetConnect* pConnect );

	virtual	PacketID_t	GetPacketID( ) const { return PACKET_CHECK_RESPONSE; }

};
//-----------------------------------------------------------------------------
class CompressPacket : public EventPacket
{
public:
	CompressPacket();

	virtual void InitData() override 
	{
		EventPacket::InitData();
		//if (mZipBuffer)
		//	mZipBuffer->clear(false);
	}

public:
	//virtual bool SetEvent(AutoEvent &hEvent);

	//virtual UINT Execute( tNetConnect* pConnect );

	virtual	PacketID_t	GetPacketID( ) const { return PACKET_COMPRESS_EVENT; }

	virtual BOOL Read( LoopDataStream& iStream, size_t packetSize );
	virtual bool FullZipData(AutoData zipData);

	virtual int SetEvent(AutoEvent &hEvent);

	virtual AutoData& GetEventData(){ return mZipBuffer; }

	virtual bool ReadySendData();

	virtual void SwapEventData( AutoData &eventData );

	virtual bool SetData(AutoData hData, size_t size){ return FullZipData(hData); }

public:
	static bool UnZip(AutoData zipBuffer, AutoData &resultData);

protected:
	AutoData		mZipBuffer;

};

#define CHECK_READ_STREAM(a, b) \
	if (iStream.Read(a, b)!=b) return FALSE;

#define CHECK_WRITE_STREAM(a, b) \
	if (oStream.Write(a, b)!=b) return FALSE;

// �����ݷְ�
class PartPacket : public Packet
{
public:
	PartPacket();

public:
	virtual	PacketID_t	GetPacketID( ) const { return PACKET_PART_PACKET; }
	virtual UINT GetPacketSize() const 
	{		
		size_t size = _GetInfoSize();
		if (mData)
			size += (UINT)mData->tell(); 
		
		return size; 
	}

	virtual BOOL Read( LoopDataStream& iStream, size_t packetSize ) 
	{
		CHECK_READ_STREAM((char*)&mPartCode, sizeof(mPartCode));
		CHECK_READ_STREAM((char*)&mPartCount, sizeof(mPartCount));
		CHECK_READ_STREAM((char*)&mPacketFlag, sizeof(mPacketFlag));
		if (mData->size()<(DSIZE)(packetSize-_GetInfoSize()))
			mData->resize((DSIZE)packetSize-_GetInfoSize());
		 
		CHECK_READ_STREAM(mData->data(), packetSize-_GetInfoSize());
		mData->seek(packetSize-_GetInfoSize());
		return TRUE;
	}
	virtual BOOL Write( LoopDataStream& oStream )const
	{
		CHECK_WRITE_STREAM((char*)&mPartCode, sizeof(mPartCode));
		CHECK_WRITE_STREAM((char*)&mPartCount, sizeof(mPartCount));
		CHECK_WRITE_STREAM((char*)&mPacketFlag, sizeof(mPacketFlag));
		DataStream *p = (DataStream*)mData.getPtr();
		char *szD = p->data(); 
		CHECK_WRITE_STREAM(szD, mData->tell());

		return TRUE;
	}

	virtual UINT		Execute( tNetConnect* pConnect );

	size_t _GetInfoSize() const
	{
		static size_t s = sizeof(mPartCode) + sizeof(mPartCount) + sizeof(mPacketFlag);
		return s;
	}
public:
	byte			mPartCode;
	byte			mPartCount;
	UInt64			mPacketFlag;

	AutoData		mData;
};


class NET_QuestRepeatSendPacket : public Packet
{
public:
	NET_QuestRepeatSendPacket()
		: mQuestSendCode(0) {}

public:
	virtual BOOL	Read( LoopDataStream& iStream, size_t packetSize ) 
	{
		return iStream.Read((CHAR*)&mQuestSendCode, sizeof(mQuestSendCode))==sizeof(mQuestSendCode);
		return TRUE; 
	}	
	virtual BOOL	Write( LoopDataStream& oStream ) const 
	{
		return oStream.Write((CHAR*)&mQuestSendCode, sizeof(mQuestSendCode))==sizeof(mQuestSendCode);
	}

	//����ֵΪ��PACKET_EXE �е����ݣ�
	//PACKET_EXE_ERROR ��ʾ�������ش��󣬵�ǰ������Ҫ��ǿ�ƶϿ�
	//PACKET_EXE_BREAK ��ʾ���غ�ʣ�µ���Ϣ�����ڵ�ǰ����ѭ���ﴦ��
	//PACKET_EXE_CONTINUE ��ʾ�����ڵ�ǰѭ����ִ��ʣ�µ���Ϣ
	//PACKET_EXE_NOTREMOVE ��ʾ�����ڵ�ǰѭ����ִ��ʣ�µ���Ϣ,���ǲ����յ�ǰ��Ϣ
	virtual UINT		Execute( tNetConnect* pConnect )
	{
		AssertEx(0, "��Ӧִ�е���");
		return 0;
	}

	virtual	PacketID_t	GetPacketID( ) const { return PACKET_QUEST_REPEAT_SEND; }
	virtual	VOID		SetPacketID( PacketID_t id )  {}

	virtual	UINT		GetPacketSize( ) const { return sizeof(mQuestSendCode); }

	BYTE				GetPacketIndex( ) const { return 0; }

public:
	byte				mQuestSendCode;
};
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

#endif //_INCLUDE_EVENTPACKET_H_