#ifndef _INCLUDE_WEBPLAYER_H_
#define _INCLUDE_WEBPLAYER_H_

#include "WebGameNet.h"
#include "GameDefine.h"

//-------------------------------------------------------------------------
class WaitReceiveBigMsgData : public AutoBase
{
public:
	Array<AutoData> mReceiveBigData;
};

typedef Auto<WaitReceiveBigMsgData> AReceiveBigData;

//-------------------------------------------------------------------------

class WebPlayer : public WebPlayerConnect
{
public:
	WebPlayer()
		: WebPlayerConnect(9*1024, 32*1024)
		, mDBID(0)
		, mNowShowID(0)
        , mLastBigMsgIndexID(0)
	{
		
	}

public:
    void OnReceiveFrame(DataStream *revFrameData, int frameType, bool bLastFrame) override;

	void ProcessMsg(DataStream *revFrameData);

public:
	int mDBID;
	int mNowShowID;
    int mLastBigMsgIndexID;

	EasyHash<int, AReceiveBigData> mWaitReceiveDataList;
};


#endif //_INCLUDE_WEBPLAYER_H_