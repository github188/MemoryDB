
#ifndef _INCLUDE_TESTSERVER_H_
#define _INCLUDE_TESTSERVER_H_

#include "TCPThreadNet.h"
#include "EventCenter.h"
#include "NetTest.h"
#include "TableManager.h"
#include "IOCPServerNet.h"
#include "TimeManager.h"

using namespace Logic;

class Net_Export TestServerNet : public IOCPServerNet
{
public:
	TestServerNet()
		: mReceiveEventNum(0)
	{
		mStartTime = TimeManager::Now();
	}

public:
	void StartTest(const char *szIP, int port)
	{
		mIp = szIP;
		mPort = port;

		TableManager::getSingleton();

		eventCenter = MEM_NEW EventCenter();
				
		eventCenter->RegisterEvent("CS_NetTestEvent", MEM_NEW EventFactory<CS_NetTestEvent_S>());

		StartNet(szIP, port);

		while (true)
		{
			eventCenter->ProcessEvent();
			Process();
		}
	}

public:
	virtual const char* GetIp(void) const { return mIp.c_str(); }
	virtual int		GetPort(void) const { return mPort; }	

	virtual Logic::tEventCenter* GetEventCenter(void) const { return (tEventCenter*)eventCenter.getPtr(); }

	//virtual tNetConnect* GetConnect(int netID) { return NULL; }

	virtual bool OnReceiveEvent(tNetConnect *revConnect, Logic::tEvent *revEvent)
	{
		revEvent->DoEvent();
		++mReceiveEventNum;

		if (TimeManager::Now()-mStartTime>10)
		{
			printf("*** 10 s receive [%d]\n", mReceiveEventNum);
			mStartTime = TimeManager::Now();
			mReceiveEventNum = 0;
		}

		return true;
	}

	virtual int GetSafeCode(){ return 1; }

protected:
	EasyString	mIp;
	int			mPort;

	int			mReceiveEventNum;

	UInt64		mStartTime;

	AutoEventCenter eventCenter;
};




#endif //_INCLUDE_TESTSERVER_H_