


#ifndef __PACKETFACTORY_H__
#define __PACKETFACTORY_H__

#include "Packet.h"
#include "EasyStack.h"
#include "Auto.h"
#include "Hand.h"
//-------------------------------------------------------------------------*/
class PacketFactoryPtr : public AutoBase
{
public:
	PacketFactoryPtr()
		: mpFactory(NULL){}
	PacketFactory *mpFactory;
};
//-------------------------------------------------------------------------*/
class PacketFactory : public AutoBase
{
public :
	PacketFactory()
	{
		mOwnerPtr = MEM_NEW PacketFactoryPtr();
		mOwnerPtr->mpFactory = this;
	}
	virtual ~PacketFactory (){  mOwnerPtr->mpFactory = NULL; }

	virtual HandPacket	CreatePacket ()  = 0;
	virtual PacketID_t	GetPacketID ()const  = 0;
	virtual	void		destroyPacket(Packet* pPacket) = 0;

public:
	Auto<PacketFactoryPtr> mOwnerPtr;
};

typedef Auto<PacketFactory> AutoPacketFactory;

template<typename T, int typeID>
class DefinePacketFactory : public PacketFactory
{
public:
	virtual HandPacket	CreatePacket ()  
	{
		if (mFreeList.empty())
		{
			//static int i = 0;
			//printf("*++ [%d]\n", ++i);
			Packet *p = MEM_NEW T();
			p->mFactoryPtr = mOwnerPtr;
			return p;
		}

		return mFreeList.pop();
	}
	virtual PacketID_t	GetPacketID ()const{  return typeID; }	
	virtual	void		destroyPacket(Packet* pPacket)
	{
		//static int i = 0;
		//printf("@-- [%d]\n", ++i);
		pPacket->InitData();
		mFreeList.push(pPacket);
	}

public:
	~DefinePacketFactory()
	{
		while (!mFreeList.empty())
		{
			Packet *p = mFreeList.pop();
			delete p;
		}
	}

public:
	EasyStack<Packet*>	mFreeList;
};
//-------------------------------------------------------------------------*/
#endif
