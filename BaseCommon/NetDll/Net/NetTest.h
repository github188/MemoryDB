#ifndef _INCLUDE_NETTEST_H_
#define _INCLUDE_NETTEST_H_

#include "ServerEvent.h"
#include "ClientEvent.h"
#include "ClientNet.h"
#include "NetHead.h"


#include "WorkThread.h"

class Net_Export NetTest
{
public:
	NetTest()
		: msCode(0)
		, msLastCode(0)
		, mbFinish(false)
	{

	}

public:
	void Run( const char *szIp, int nPort );

	UInt64 msCode;
	UInt64 msLastCode;

	bool mbFinish;
};

#define TEST_SEND_DATA_SIZE		1024

class Net_Export CS_NetTestEvent : public Logic::tServerEvent
{
public:
	virtual bool _DoEvent()
	{
		mNetID = _getTarget();
		mTestCode = mpNetTest->msCode++;
		set("TEST", mTestCode);

		//AutoData testData = MEM_NEW DataBuffer(TEST_SEND_DATA_SIZE);
		//testData->setDataSize(TEST_SEND_DATA_SIZE);
		//set("DATA", testData);

		WaitTime(200);
		return true;
	}

	virtual void _OnResp(AutoEvent &respEvent);

	virtual void Log( const char* lpszFormat, ... ){}

public:

	UInt64 mTestCode;
	int mNetID;

	NetTest *mpNetTest;
};

class Net_Export CS_NetTestEvent_S : public Logic::tClientEvent
{
public:
	virtual bool _DoEvent()
	{
		AutoData d;
		get("DATA", d);
		if (d && d->dataSize()==TEST_SEND_DATA_SIZE)
		{
			Log("OK Revceive data [%d]", TEST_SEND_DATA_SIZE);
		}
		else
			Log("XX TEST send data fail X ");

		Finish();
		return true;
	}	

	virtual void Log( const char* lpszFormat, ... ){}
};


class NetTestThread : public WorkThread
{
public:
	void StartTest(const char *szIp, int nPort)
	{
		mIp = szIp;
		mPort = nPort;

		WorkThread::InitThread();
	}

public:
	virtual void backWorkThread(void)
	{
		mNetTest.Run(mIp.c_str(), mPort);
	}

public:
	NetTest mNetTest;

	EasyString mIp;
	int mPort;
};

#endif