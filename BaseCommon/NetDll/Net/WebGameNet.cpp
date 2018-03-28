#include "WebGameNet.h"
#include "CEvent.h"
#include "ClientEvent.h"

enum
{
    WEB_MSG_RESPNSE_MSG = 1,
    CS_TEST_WEB_MSG  = 2,
    SC_TEST_REQUEST_MSG = 3,
};
//#include "GameDefine.h"
//----------------------------------------------------------------------
class TestWebMsg : public tClientEvent
{
public:
    virtual bool _DoEvent() override
    {
        //GetResponseEvent()["KKK"] = "ok";
        //GetResponseEvent()["AAA"] = 8765321;
        //GetResponseEvent()["AAZ"] = 8.12345678f;
        AutoArrayData d = MEM_NEW ArrayNiceData();
        d[0] = 99;
        d[2] = 101;
        d[3] = 220;
        GetResponseEvent()["TEST_ARR"] = d;
        Finish();
        Hand<tServerEvent> evt = GetEventCenter()->StartEvent(SC_TEST_REQUEST_MSG);
        evt->mNetConnect = mNetConnect;
        evt->Start();
        return false;
    }
};

class TestResponseMsg : public tServerEvent
{
public:
    virtual bool _DoEvent()
    {
        set("GGG", 9883);
        set("INFO", "TestEgretRequest");
        WaitTime(10);
        return true;
    }

    virtual void _OnResp(AutoEvent &respEvent) override
    {
        NOTE_LOG("Client response >\r\n%s", respEvent->GetData().dump().c_str());
    }
};
//----------------------------------------------------------------------

//-------------------------------------------------------------------------
WebGameNet::WebGameNet()
{    
    mEventCenter = MEM_NEW WebNetEventCenter();
    mEventCenter->_SetNetTool(0, GetSelf());

    //RegisterMsg(CS_TEST_WEB_MSG, MEM_NEW EventFactory<TestWebMsg>());
    //RegisterMsg(SC_TEST_REQUEST_MSG, MEM_NEW EventFactory<TestResponseMsg>());
}

void WebGameNet::RegisterMsg(int eventIndex, AutoEventFactory factory)
{
    factory->SetID(eventIndex);
    AString name;
    name.Format("WebMsgEvent_%d", eventIndex);
    factory->SetEventName(name.c_str());
    Hand<WebNetEventCenter> webCenter = mEventCenter;
    webCenter->RegisterMsg(factory);
}

HandConnect WebGameNet::CreateConnect()
{
    return MEM_NEW WebPlayerConnect();
}
//----------------------------------------------------------------------
void WebNetEventCenter::RegisterMsg(AutoEventFactory hFactory)
{
    int id = hFactory->GetID();
    if (id<mArrayIndex.size() && mArrayIndex[id])
        ERROR_LOG("Already exist event factory %d >%s", id, mArrayIndex[id]->GetEventName());
    RegisterEvent(hFactory->GetEventName(), hFactory);
    hFactory->SetID(id);
    if (id>=mArrayIndex.size())
        mArrayIndex.resize(id+8);        

    mArrayIndex[id] = hFactory;    
}

void WebNetEventCenter::RegisterMsg(int eventIndex, AutoEventFactory factory)
{
    factory->SetID(eventIndex);
    AString name;
    name.Format("WebMsgEvent_%d", eventIndex);
    factory->SetEventName(name.c_str());
    RegisterMsg(factory);
}

bool WebNetEventCenter::SerializeMsg(AutoEvent &hEvent, DataStream *destData)
{
    int id = hEvent->GetEventFactory()->GetID();
    if (id>0xffff>>1)
    {
        ERROR_LOG("[%s] Event ID over 65535", hEvent->GetEventName());
        return false;
    }

    destData->write((short)id);		

    if (hEvent->_Serialize(destData))
        return true;

    ERROR_LOG("get event [%s] data stream error", hEvent->GetEventNameInfo().c_str());
    return false;
}

AutoEvent WebNetEventCenter::RestoreMsg(DataStream *scrData)
{
    AutoEvent hEvent;
    short id = 0;
    if (!scrData->read(id))
    {
        ERROR_LOG("Read event  id fail");
        return AutoEvent();
    }
    if (id>0)
    {
        hEvent = _Start(id, 0);
        if (!hEvent)
        {                    
            ERROR_LOG("根据ID创建事件失败>[%d], 未注册事件", id);			                    
        }
        else
        {
            if (hEvent->_Restore(scrData))
                return hEvent;
            ERROR_LOG("Error: restor event [%s] form data stream", hEvent->GetEventName());
        }
    }
    else
        ERROR_LOG("恢复事件 ID 为 0");

    return hEvent;
}

AutoEvent WebNetEventCenter::_Start(ushort eventID, int eventNameIndex)
{
    if (eventID>0 && eventID<mArrayIndex.size())
    {
        AutoEventFactory hF = mArrayIndex[eventID];
        if (hF)
        {
            return hF->StartEvent();
        }
    }
    return AutoEvent();
}

WebNetEventCenter::WebNetEventCenter()    
{
    mResponseEventFactory->SetID(WEB_MSG_RESPNSE_MSG);
    if (mArrayIndex.size()<=1)
        mArrayIndex.resize(8);
    mArrayIndex[WEB_MSG_RESPNSE_MSG] = mResponseEventFactory;
}

//----------------------------------------------------------------------

void WebPlayerConnect::OnReceiveFrame(DataStream *revFrameData, int frameType, bool bLastFrame)
{
    if (frameType==WS_TEXT_FRAME)
    {                
        NOTE_LOG("Rev WEB >%s", AString::getANIS(revFrameData->data()).c_str());            
        //SendMsgData(revFrameData, WS_TEXT_FRAME);
    }
    else if (frameType==WS_BINARY_FRAME)
    {
        //static int x = 0;
        //NOTE_LOG(">>> Rev %d %d", ++x, (int)revFrameData->dataSize());
        revFrameData->seek(0);
        AutoEvent evt = GetNetHandle()->GetEventCenter()->RestoreMsg(revFrameData);
        if (evt)
        {
            NOTE_LOG("Rev msg event >%s", evt->GetEventName());
            NOTE_LOG("%s", evt->GetData().dump().c_str());
            evt->_OnBindNet(this);
            evt->DoEvent(true);
        }
        else
            ERROR_LOG("Restore msg event fail >%d", revFrameData->dataSize());

        //AString info = revFrameData->readString();
        ////info.toANIS();
        //NiceData d;
        //d.restore(revFrameData);
        //NOTE_LOG("INFO >%s", info.toANIS());
        //NOTE_LOG("%s", d.dump().c_str());
        //NOTE_LOG("------------------------");
    }
}

bool WebPlayerConnect::SendEvent(Logic::tEvent *pEvent)
{
    mSendMsgBuffer.clear(false);
    bool b = pEvent->GetEventCenter()->SerializeMsg(pEvent->GetSelf(), &mSendMsgBuffer);
    if (b)
        return SendMsgData(&mSendMsgBuffer, WS_BINARY_FRAME);
    return false;
}
//----------------------------------------------------------------------