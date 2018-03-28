#ifndef _INCLUDE_WEBGAMENET_H_
#define _INCLUDE_WEBGAMENET_H_

#include "EventCenter.h"
#include "EventFactory.h"
#include "IOCPServerNet.h"
#include "../WebSocket/WebServerProtocol.h"
#include "NetHead.h"
//#include "DdzRoom.h"

using namespace Logic;


class Net_Export WebGameNet : public IOCPServerNet
{
public:
    void RegisterMsg(int eventIndex, AutoEventFactory factory);

public:
    WebGameNet();

public:
    virtual void _OnConnectStart(tNetConnect *pConnect) override{}
    virtual void OnMsgRegistered(int eventNameIndex){}
    virtual int GetSafeCode() override { return 0; }

    virtual HandConnect CreateConnect() override;

    Logic::tEventCenter* GetEventCenter( void ) const
    {
        return ((WebGameNet*)(this))->mEventCenter.getPtr();
    }

    // 不再自动产生ID
    virtual bool NeedUpdateMsgIndex() const override { return false; }
    virtual bool NeedMsgEventIndex(const char *msgName, int msgNameIndex) const override { return false; }

public:
    AutoEventCenter mEventCenter;
};

class Net_Export WebPlayerConnect : public WebConnect
{
public:
	WebPlayerConnect(int revBufferLen = 0, int sendBufferLen = 0)
		: WebConnect(revBufferLen, sendBufferLen){}

public:
    virtual void OnReceiveFrame(DataStream *revFrameData, int frameType, bool bLastFrame) override;

    virtual bool SendEvent(Logic::tEvent *pEvent) override;

public:
    DataBuffer  mSendMsgBuffer;
};

class Net_Export WebNetEventCenter : public EventCenter
{
public:
    void RegisterMsg(int eventIndex, AutoEventFactory factory);
    void RegisterMsg(AutoEventFactory hFactory);
    virtual AutoEvent StartEvent(int nEventNameIndex) override
    {
        return _Start(nEventNameIndex, 0);
    }

public:
    // 停止自动维护事件ID优化
    virtual void UpdateNetMsgIndex(AutoData msgIndexData) override {}
    // 生成消息事件索引数据，msgNameIndex = 0 表示生成所有的
    virtual bool GenerateMsgIndex(AutoData &destData, int msgNameIndex = 0) override { return false; }
    bool SerializeMsg( AutoEvent &hEvent, DataStream *destData );
    AutoEvent RestoreMsg( DataStream *scrData ) override;

    AutoEvent _Start( ushort eventID, int eventNameIndex ) override;

public:
    WebNetEventCenter();
};

#endif //_INCLUDE_WEBGAMENET_H_