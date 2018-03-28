#ifndef _INCLUDE_UDPNET_H_
#define _INCLUDE_UDPNET_H_

#include "NetHead.h"
#include "DataStream.h"
#include "Hand.h"
#include "DataBuffer.h"

class Net_Export UDPNet : public Base<UDPNet>
{
public:
	UDPNet();
	~UDPNet();

public:
	bool Bind(const char *szIP, int nPort);
	void Close();

	int ProcessReceive(DataStream *dataBuffer, UInt64 &scrAddr, bool bBlock);

	bool Send(DataStream *sendData, UInt64 destAddr);
	bool Send(DataStream *sendData, const char *szIp, int port);

	bool Send(UInt64 destAddr, const char *sendData, int sendSize);

	bool Send(const char *szIp, int port, const char *sendData, int sendSize );

	bool Send(DataStream *sendData, int startPos, int sendSize, UInt64 targetAddr);

	bool SendPacket(UInt64 packedCode, DataStream *sendData, int sendSize, Array<UInt64> &targetList, int nSleepMilSec);

	bool SetBlock(bool bBlock);
	bool SetReceiveBufferSize(int size);
	bool SetSendBufferSize(int size);

	AString getIpPort(int &port);

protected:
	size_t			mSocketID;
	DataBuffer		mSendDataBuffer;

};



#endif //_INCLUDE_UDPNET_H_