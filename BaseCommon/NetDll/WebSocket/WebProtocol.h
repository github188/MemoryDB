#ifndef _INCLUDE_WEBPROTOCOL_H_
#define _INCLUDE_WEBPROTOCOL_H_

#include "sha1.h"
#include <string>
#include "NetHead.h"

using namespace std;

enum WS_Status
{
    WS_STATUS_CONNECT = 0,
    WS_STATUS_UNCONNECT = 1,
};

enum WS_FrameType
{
    WS_EMPTY_FRAME = 0xF0,
    WS_ERROR_FRAME = 0xF1,
    WS_TEXT_FRAME   = 0x01,
    WS_BINARY_FRAME = 0x02,
    WS_PING_FRAME = 0x09,
    WS_PONG_FRAME = 0x0A,
    WS_OPENING_FRAME = 0xF3,
    WS_CLOSING_FRAME = 0x08,
    WS_PART_FRAME = 0xFFFF,
};
class DataStream;
class Net_Export WebProtocol
{
public:
    static int wsHandshake(string &request, string &response);

    //NOTE: 未判断源数据长度是否 >= 接收数据长度, 在使用后, 需要进行检查长度信息是否正确
    static int wsDecodeFrame(DataStream *pScrMsgFrameData, DataStream *destData, WS_FrameType &frameType, bool &bLastFrame, bool &bMask);

    static int wsEncodeFrame(DataStream *pScrSendData, DataStream *pDestData, enum WS_FrameType frameType);

};

#endif //_INCLUDE_WEBPROTOCOL_H_

