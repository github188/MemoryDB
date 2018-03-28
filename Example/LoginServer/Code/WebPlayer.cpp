#include "WebPlayer.h"

#include "LGWebNet.h"
#include "LoginThread.h"

void WebPlayer::OnReceiveFrame(DataStream *revFrameData, int frameType, bool bLastFrame)
{
    if (frameType==WS_TEXT_FRAME)
    {                
        NOTE_LOG("Rev WEB >%s", AString::getANIS(revFrameData->data()).c_str());            
        //SendMsgData(revFrameData, WS_TEXT_FRAME);
    }
    else if (frameType==WS_BINARY_FRAME)
    {
        //NOTE_LOG(">>> Rev %d %d", ++mMsgCode, (int)revFrameData->dataSize());
        revFrameData->seek(0);

        short id = 0;
        if (!revFrameData->read(id))
        {
            ERROR_LOG("Rev data error less short");
            return;
        }
        if (id>WEB_MSG_GATE_BEGIN && id<WEB_MSG_GATE_END)
        {

            return;
        }
        
		ProcessMsg(revFrameData);

        //AString info = revFrameData->readString();
        ////info.toANIS();
        //NiceData d;
        //d.restore(revFrameData);
        //NOTE_LOG("INFO >%s", info.toANIS());
        //NOTE_LOG("%s", d.dump().c_str());
        //NOTE_LOG("------------------------");
    }
}

void WebPlayer::ProcessMsg( DataStream *revFrameData )
{
	revFrameData->seek(0);
	AutoEvent evt = GetNetHandle()->GetEventCenter()->RestoreMsg(revFrameData);
	if (evt)
	{
		//NOTE_LOG("Rev msg event >%s", evt->GetEventName());
		//NOTE_LOG("%s", evt->GetData().dump().c_str());
		evt->_OnBindNet(this);
		evt->DoEvent(true);
	}
	else
	{
		ERROR_LOG("Restore msg event fail >%d, then now remove", revFrameData->dataSize());
		SetRemove(true);
	}
}
