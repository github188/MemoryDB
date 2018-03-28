//#include "stdafx.h"

#include "PacketFactory.h"

#include "Packet.h"

void Packet::Release()
{
	if (mFactoryPtr && mFactoryPtr->mpFactory!=NULL) 
		mFactoryPtr->mpFactory->destroyPacket(this); 
	else 
		delete this;
}
