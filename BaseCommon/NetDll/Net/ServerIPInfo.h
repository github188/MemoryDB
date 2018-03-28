
#ifndef _INCLUDE_SERVERIPINFO_H_
#define _INCLUDE_SERVERIPINFO_H_

#include "AutoString.h"
#include "NetHead.h"
#include "BaseCommon.h"


class Net_Export ServerIPInfo
{

public:
	static AString Num2IP(UInt64 scrNum, int &port, int &gsPort);
	static UInt64 IP2Num(const char *szIP, int port, int gsPort);
	static UInt64 IP2Num(uint ipNum, int port);

	static AString Num2IP(uint nNumIp);
	static uint IP2Num(const char *szIP);

	static AString DNS2IP(const char *szDNSName);

	static AString GetAddrInfo(UInt64 netKey);

public:
	ServerIPInfo()
		: mIpPort(0)
	{

	}

	ServerIPInfo(const char* szIp, int port);

	int GetIp(){ return (int)(mIpPort>>32 & 0xFFFFFFFF); }
	int GetPort(){ return (int)(mIpPort & 0xFFFFFFFF); }

	AString Ip();

	operator UInt64 ()
	{
		return mIpPort;
	}


protected:
	UInt64	mIpPort;
};



#endif //_INCLUDE_SERVERIPINFO_H_