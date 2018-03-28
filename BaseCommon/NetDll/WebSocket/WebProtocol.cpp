#include "WebProtocol.h"
//#ifndef _DLL_CPPLIB
//#   define _DLL_CPPLIB
//#endif
#include <sstream>
#include <iosfwd>
#include <Windows.h>
#include <stdint.h>
#include "base64.h"
#include "AutoString.h"
#include "DataStream.h"

int WebProtocol::wsHandshake(string &request, string &response)
{
    // 解析http请求头信息
    int ret = WS_STATUS_UNCONNECT;
    std::istringstream stream(request.c_str());
    std::string reqType;
    std::getline(stream, reqType);
    if (reqType.substr(0, 4) != "GET ")
    {
        return ret;
    }

    std::string header;
    std::string::size_type pos = 0;
    std::string websocketKey;
    while (std::getline(stream, header) && header != "\r")
    {
        header.erase(header.end() - 1);
        pos = header.find(": ", 0);
        if (pos != std::string::npos)
        {
            std::string key = header.substr(0, pos);
            std::string value = header.substr(pos + 2);
            if (key == "Sec-WebSocket-Key")
            {
                ret = WS_STATUS_CONNECT;
                websocketKey = value;
                break;
            }
        }
    }

    if (ret != WS_STATUS_CONNECT)
    {
        return ret;
    }

    // 填充http响应头信息
    response = "HTTP/1.1 101 Switching Protocols\r\n";
    response += "Upgrade: websocket\r\n";
    response += "Connection: upgrade\r\n";
    response += "Sec-WebSocket-Accept: ";

    const std::string magicKey("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    //258EAFA5-E914-47DA-95CA-C5AB0DC85B11
    //websocketKey = "c9RUauXe5LIPHXV7fpMd9w=="; //???
    std::string serverKey = websocketKey + magicKey;

    byte shaHash[32];
    memset(shaHash, 0, sizeof(shaHash));
    //sha1::calc(serverKey.c_str(), serverKey.size(), (unsigned char *) shaHash);
    CSHA1 sh1;
    sh1.Reset();
    //sh1<<serverKey.c_str();
    //AString uKey = AString::getUTF8(serverKey.c_str());
    //sh1.Input(uKey.c_str(), (unsigned int)uKey.length());
    //sh1.Result((unsigned int *) shaHash);

    sh1.Update((const unsigned char*)serverKey.c_str(), serverKey.length());
    sh1.Final();
    sh1.GetHash(shaHash);
    
    serverKey = base64_encode((const unsigned char*)shaHash, (unsigned int)20) + "\r\n\r\n";
    string strtmp(serverKey.c_str());
    response += strtmp;

    return ret;
}

int WebProtocol::wsDecodeFrame(DataStream *pScrMsgFrameData, DataStream *destData, WS_FrameType &frameType, bool &bLastFrame, bool &bMask)
{
    const char *frameData = pScrMsgFrameData->data()+pScrMsgFrameData->tell();
    const int frameLength = (int)pScrMsgFrameData->dataSize()-pScrMsgFrameData->tell();
    if (frameLength < 2)
    {
        return 0;
    }

    // 检查扩展位并忽略
    if ((frameData[0] & 0x70) != 0x0)
    {
        //ret = WS_ERROR_FRAME;
    }

    // fin位: 为1表示已接收完整报文, 为0表示继续监听后续报文    
    bLastFrame = (frameData[0] & 0x80) == 0x80;    

    // mask位, 为1表示数据被加密
    bMask = (frameData[1] & 0x80) == 0x80;

    // 操作码
    uint16_t payloadLength = 0;
    uint8_t payloadFieldExtraBytes = 0;
    uint8_t opcode = static_cast<uint8_t >(frameData[0] & 0x0f);
    frameType = (WS_FrameType)opcode;
    //if (opcode == WS_TEXT_FRAME || opcode==WS_BINARY_FRAME)
    if (opcode!=WS_CLOSING_FRAME)
    {
        // 处理utf-8编码的文本帧
        payloadLength = static_cast<uint16_t >(frameData[1] & 0x7f);
        if (payloadLength == 0x7e)
        {
            uint16_t payloadLength16b = 0;
            payloadFieldExtraBytes = 2;
            memcpy(&payloadLength16b, &frameData[2], payloadFieldExtraBytes);
            payloadLength = ntohs(payloadLength16b);
        }
        else if (payloadLength == 0x7f)
        {
            frameType = (WS_FrameType)WS_ERROR_FRAME;
            // 数据过长,暂不支持
            //ret = WS_ERROR_FRAME;
        }
    }
    //else if (opcode == WS_BINARY_FRAME || opcode == WS_PING_FRAME || opcode == WS_PONG_FRAME)
    //{
    //    // 二进制/ping/pong帧暂不处理
    //    frameType = (WS_FrameType)opcode;
    //}
    //else if (opcode == WS_CLOSING_FRAME)
    //{
    //    ret = WS_CLOSING_FRAME;
    //}
    //else
    //{
    //    ret = WS_ERROR_FRAME;
    //}

    // 数据解码
    if (payloadLength > 0)
    {
        if (pScrMsgFrameData->lastDataSize()<(2 + payloadFieldExtraBytes + 4)+payloadLength)
            return 0;
        // header: 2字节, masking key: 4字节
        const char *maskingKey = &frameData[2 + payloadFieldExtraBytes];
        if (destData->lastSize()<payloadLength)
            destData->resize(destData->tell()+payloadLength);
        char *payloadData = destData->data()+destData->tell();
        memset(payloadData, 0, payloadLength);
        //memcpy(payloadData, &frameData[2 + payloadFieldExtraBytes + 4], payloadLength);
        const char *scrData = frameData+(2 + payloadFieldExtraBytes + 4);
        for (int i = 0; i < payloadLength; i++)
        {
            payloadData[i] = scrData[i] ^ maskingKey[i % 4];
        }
        
        destData->setDataSize(destData->tell()+payloadLength);
        destData->seek(destData->tell()+payloadLength);
    }

    return 2 + payloadFieldExtraBytes + 4 + payloadLength;
}

int WebProtocol::wsEncodeFrame(DataStream *pScrSendData, DataStream *pDestData, enum WS_FrameType frameType)
{
    int ret = WS_EMPTY_FRAME;
    const uint32_t messageLength = (uint32_t)pScrSendData->dataSize();
    if (messageLength > 32767)
    {
        // 暂不支持这么长的数据
        ERROR_LOG("Msg data length more then 32767");
        return 0;
    }

    uint8_t payloadFieldExtraBytes = (messageLength <= 0x7d) ? 0 : 2;
    // header: 2字节, mask位设置为0(不加密), 则后面的masking key无须填写, 省略4字节
    uint8_t frameHeaderSize = 2 + payloadFieldExtraBytes;
    size_t s = frameHeaderSize;
    uint8_t *frameHeader = (uint8_t*)ALLOCT_NEW(s); //new uint8_t[frameHeaderSize];
    memset(frameHeader, 0, frameHeaderSize);
    // fin位为1, 扩展位为0, 操作位为frameType
    frameHeader[0] = static_cast<uint8_t>(0x80 | frameType);

    // 填充数据长度
    if (messageLength <= 0x7d)
    {
        frameHeader[1] = static_cast<uint8_t>(messageLength);
    }
    else
    {
        frameHeader[1] = 0x7e;
        uint16_t len = htons(messageLength);
        memcpy(&frameHeader[2], &len, payloadFieldExtraBytes);
    }

    // 填充数据
    uint32_t frameSize = frameHeaderSize + messageLength;
    if (pDestData->size()<(int)frameSize+1)
        pDestData->resize(frameSize+1);
    char *frame = pDestData->data(); // new char[frameSize + 1];
    memcpy(frame, frameHeader, frameHeaderSize);
    memcpy(frame + frameHeaderSize, pScrSendData->data(), messageLength);
    frame[frameSize] = '\0';
    pDestData->setDataSize(frameSize);
    //outFrame = frame;

    //delete[] frame;
    //delete[] frameHeader;
    ALLOCT_FREE(frameHeader);
    return frameSize;
}
