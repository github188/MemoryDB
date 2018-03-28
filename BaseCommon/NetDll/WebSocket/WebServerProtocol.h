#ifndef _INCLUDE_WEBSERVERPROTOCOL_H_
#define _INCLUDE_WEBSERVERPROTOCOL_H_

#include "NetHead.h"

#include "WebProtocol.h"
#include "IOCPConnect.h"
#include "BaseEventNet.h"

class Net_Export WebConnect : public IOCPServerConnect
{
public:
    virtual void OnReceiveFrame(DataStream *revFrameData, int frameType, bool bLastFrame) = 0;
    virtual bool SendMsgData(DataStream *pSendMsgData, int frameType);

public:
    virtual void _ProcessReceiveData() override;
    virtual bool SendEvent(Logic::tEvent *pEvent) override{ ERROR_LOG("WebConnect::SendEvent Need override"); return false; }

    WebConnect(int revBufferLen = 0, int sendBufferLen = 0)
		: IOCPServerConnect(revBufferLen, sendBufferLen)
        , mRevFrameData(128)
        , mTempRevBuffer(128)
        , mSendFramData(128)
    {}

protected:
    DataBuffer  mRevFrameData;
    DataBuffer  mSendFramData;
    DataBuffer  mTempRevBuffer;
};

class Net_Export WebServerProtocol : public EventNetProtocol
{
public:
	virtual bool WritePacket( const Packet* pPacket, LoopDataStream *mSocketOutputStream ) override
	{
        return false;
	}
	virtual HandPacket ReadPacket( tNetConnect *pConnect, LoopDataStream *mSocketInputStream ) override
	{
        return HandPacket();
	}

public:

};

#endif //_INCLUDE_WEBSERVERPROTOCOL_H_