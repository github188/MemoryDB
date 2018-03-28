#include "WebServerProtocol.h"
#include "TableTool.h"

bool WebConnect::SendMsgData(DataStream *pSendMsgData, int frameType)
{
    mSendFramData.clear(false);
    if (WebProtocol::wsEncodeFrame(pSendMsgData, &mSendFramData, (WS_FrameType)frameType)>0)
    {
        if (!mSendData._write((void*)mSendFramData.data(), mSendFramData.dataSize()))
		{
			ERROR_LOG("Write msg data fail, %d more then send buffer size %d", (int)mSendFramData.dataSize(), (int)mSendData.size());
			return false;
		}
        _SendTo();
        return true;
    }
    ERROR_LOG("加密WEB消息数据失败");
    return false;
}

void WebConnect::_ProcessReceiveData()
{
    if (!mbHaveRevData)
        return;

    if (!mbReceiveData)
    {
        _IOCP_AUTO_LOCK_RECEIVE;
        if (mReceiveData.IsEmpty()==TRUE)
        {
            return;
        }
        // NOTE: HTTP   连接握手
        std::string request = mReceiveData.GetBuffer();
        std::string response;
        if (WebProtocol::wsHandshake(request, response)==WS_STATUS_CONNECT)
        {
            mReceiveData.Skip(request.length());
            mSendData._write((void*)response.c_str(), response.length());
            //NOTE_LOG("%s", request.c_str());
            //NOTE_LOG("----------------------%d\r\n%s", (int)response.length(), response.c_str());
            _SendTo();
            mbReceiveData = true;
            //mbHaveRevData = false;
            mNet->OnAddConnect(this);
            OnConnected();
            //mRevFrameData.clear(false);
            //mRevFrameData.write((ushort)0xFFFF);
            //SendMsgData(&mRevFrameData, WS_BINARY_FRAME);
            return;
        }
//        else
//        {	
//#if DEVELOP_MODE
//            ERROR_LOG("WARN: [%s : %d] Connect request error , then now remove, request >%s", GetIp(), GetPort(), request.c_str());
//#else
//            TABLE_LOG("WARN: [%s : %d] Connect request error , then now remove, request >%s", GetIp(), GetPort(), request.c_str());
//#endif                
//            OnConnectSafeCodeError(GetIp(), GetPort(), 0);
//            SetRemove(true);
//        }
    }
    else
    {
        _IOCP_AUTO_LOCK_RECEIVE
            if (mReceiveData.IsEmpty()==TRUE)
                return;

            if (mTempRevBuffer.size()<(int)mReceiveData.Length())
                mTempRevBuffer.resize(mReceiveData.Length()+128);
        mTempRevBuffer.clear(false);
        if (mReceiveData.Peek(mTempRevBuffer.data(), mReceiveData.Length())==TRUE)
        {            
            mTempRevBuffer.setDataSize(mReceiveData.Length());           
            do
            {
                WS_FrameType frameType = WS_EMPTY_FRAME;
                bool bLastFrame = true;
                bool bMask = true;
                mRevFrameData.clear(false);
                int dataSize = mTempRevBuffer.dataSize();
                int msgUseLen = WebProtocol::wsDecodeFrame(&mTempRevBuffer, &mRevFrameData, frameType, bLastFrame, bMask);
                if (dataSize<msgUseLen)
                    break;
                if (frameType==WS_CLOSING_FRAME)
                {
                    mRevFrameData.clear(false);
                    SendMsgData(&mRevFrameData, WS_CLOSING_FRAME);
                    mReceiveData.Skip(msgUseLen);
                  
                    //TABLE_LOG("[%s:%d] Web client request close, now remove connect", GetIp(), GetPort());
                    SetRemove(true);
                    return;
                }
                if (msgUseLen>0)
                {
                    mTempRevBuffer.seek(mTempRevBuffer.tell()+msgUseLen);
                  
                    if (mReceiveData.Skip(msgUseLen)==FALSE)
                    {
                        ERROR_LOG(" Web skip seek fail >%d", msgUseLen);
                        break;
                    }
                    if (!bMask)
                    {
                        ERROR_LOG("[%s:%d] WEB server must receive mask msg", GetIp(), GetPort());        
                        SetRemove(true);
                        return;
                    }
                    if (!bLastFrame)
                    {
                        ERROR_LOG("Now can do part frame function");
                        continue;
                    }
                    //static int x = 0;
                    //NOTE_LOG(" *** Rev %d size %d, pos %d, msgLen %d, data size %d", ++x, mRevFrameData.dataSize(), mTempRevBuffer.tell()-msgUseLen, msgUseLen, dataSize);
                    OnReceiveFrame(&mRevFrameData, frameType, bLastFrame);
                }
                else
                    break;
            } while (mTempRevBuffer.lastDataSize()>0);
        }
        mbHaveRevData = false;
    }
}
