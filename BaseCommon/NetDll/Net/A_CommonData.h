
#pragma once

#include "Type.h"
#include "Packet.h"
#include "PacketFactory.h"
#include "DataBuffer.h"
#include "EasyString.h"

#define MAX_TABLE_BUFFER_SIZE	1024

class A_CommonData : public EasyPacket<A_CommonData>
{
public:
	A_CommonData()
		:mSize(0)
		,mIsBegin(true)
		,mIsEnd(true)
	{

	}
	~A_CommonData()
	{
	}
public:
	//公用继承接口
	virtual BOOL Read( SocketInputStream& iStream )
	{
		__ENTER_FUNCTION
			iStream.Read((CHAR*)&mIsEnd,sizeof(bool));
		iStream.Read((CHAR*)&mIsBegin,sizeof(bool));
		char s = 0;
		iStream.Read(&s,1);		
		mEventName.resize(s+1);
		iStream.Read(mEventName._str(),s);		
		iStream.Read( (CHAR*)&mSize, sizeof(size_t));
		mData = new DataBuffer(mSize);
		iStream.Read((CHAR*)mData->data(),(UINT)mSize);

		return TRUE;

		__LEAVE_FUNCTION

			return FALSE ;
	}
	virtual BOOL Write( SocketOutputStream& oStream )const
	{
		__ENTER_FUNCTION
			oStream.Write((CHAR*)&mIsEnd,sizeof(bool));
		oStream.Write((CHAR*)&mIsBegin,sizeof(bool));
		char s = (char)mEventName.length();
		oStream.Write(&s,1);
		oStream.Write(mEventName.c_str(),s);
		oStream.Write( (CHAR*)(&mSize), sizeof(size_t));
		if (mSize>0)
		{
			DataBuffer *data = (DataBuffer*)mData.getPtr();
			oStream.Write(data->data()+data->tell(),(UINT)mSize);
		}

		return TRUE ;

		__LEAVE_FUNCTION

			return FALSE ;
	}

	virtual UINT GetPacketSize() const 
	{
		return	(UINT)(mSize + _getExclusionTableSize()); 
	}

	// 返回是否可以保存完全记录
	bool SetEventData(const AString& eventName,AutoData data,size_t sizeLen,bool isBegin)
	{		
		mEventName = eventName;
		mData = data;
		mIsBegin = isBegin;
		if (mData && mData->tell()<sizeLen)
		{
			size_t waitSize = sizeLen-mData->tell();
			mSize = MAX_TABLE_BUFFER_SIZE-_getExclusionTableSize();
			if (waitSize<=mSize)
			{
				mSize = waitSize;
				mIsEnd = true;
			}
			else
			{
				mIsEnd = false;
				return false;
			}
		}
		else
			mSize = 0;

		return true;
	}

protected:
	size_t _getExclusionTableSize(void) const
	{
		return sizeof(size_t) + /*sizeof(bool)*/2 + sizeof(char)+ mEventName.length();
	}

public:
	AString		mEventName;		// 事件名称
	size_t		mSize;			// 要发送的表格大小
	bool		mIsEnd;			// 是否为最后一个分包
	bool		mIsBegin;		// 是否是第一个分包
	AutoData	mData;			// 数据流

};

MAKE_FACTORY(A_CommonDataFactory,Packets::PACKET_GCG_COMMON,A_CommonData,MAX_TABLE_BUFFER_SIZE)