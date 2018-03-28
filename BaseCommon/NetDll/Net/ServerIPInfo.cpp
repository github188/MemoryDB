#include "ServerIPInfo.h"

#define _USE_SOCKET_API_		1

#ifdef __LINUX__
#	include <netdb.h>
#	include <arpa/inet.h>
#else 
#	include "winsock2.h"
#endif

AString ServerIPInfo::Num2IP(UInt64 scrNum, int &port, int &gsPort)
{
#if _USE_SOCKET_API_
	in_addr scr;
	scr.S_un.S_addr = (scrNum>>32) & 0xFFFFFFFF;	


	AString re = inet_ntoa(scr);

#else
	AString re = Num2IP((scrNum>>32) & 0xFFFFFFFF);
#endif
	port  = (scrNum & 0xFFFF0000)>>16;	
	gsPort = scrNum & 0xFFFF;

	return re;
}

AString ServerIPInfo::Num2IP( uint nNumIp )
{
#if _USE_SOCKET_API_
	in_addr scr;
	scr.S_un.S_addr = nNumIp;
	AString re = inet_ntoa(scr);
	return re;
#else
	uint s0 = (nNumIp & 0x000000FF);
	uint s1 = (nNumIp & 0x0000FF00)>>8;
	uint s2 = (nNumIp & 0X00FF0000)>>16;	
	uint s3 = (nNumIp & 0xFF000000)>>24;	

	AString strIp;
	strIp.Format("%u.%u.%u.%u", s0, s1, s2, s3);
	return strIp;
#endif
}

UInt64 ServerIPInfo::IP2Num(const char *szIP, int port, int gsPort)
{
	unsigned int t = port;
	t = (t<<16) + gsPort;
#if _USE_SOCKET_API_
	UInt64 reNum =  inet_addr(szIP);
#else
	UInt64 reNum =  IP2Num(szIP); 
#endif
	reNum = (reNum<<32) + t;
	return reNum;
}

UInt64 ServerIPInfo::IP2Num( uint ipNum, int port )
{

	UInt64 reNum = ipNum;

	reNum = (reNum<<32) + port; 
	return reNum;
}

uint ServerIPInfo::IP2Num( const char *szIP )
{
#if _USE_SOCKET_API_
	return inet_addr(szIP);
#else
	AString ip = szIP;

	Array<AString> ipPart;

	int n = AString::Split(szIP, ipPart, ".", 8);
	if (n==4)
	{
		uint s0 = atoi(ipPart[0].c_str());

		uint s1 = atoi(ipPart[1].c_str());
		s1 = s1 << 8;

		uint s2 = atoi(ipPart[2].c_str());
		s2 = s2 << 16;

		uint s3 = atoi(ipPart[3].c_str());
		s3 = s3 << 24;

		uint resultNum = s0 + s1 + s2 + s3;
		return resultNum;
	}
	else
		return 0;
#endif
}


ServerIPInfo::ServerIPInfo(const char* szIp, int port)
{
	mIpPort = IP2Num(szIp);  
	mIpPort = (mIpPort<<32) + port;
}



AString ServerIPInfo::Ip()
{
	return Num2IP((mIpPort>>32) & 0xFFFFFFFF);
}

AString ServerIPInfo::DNS2IP( const char *szDNSName )
{
	AString result;
	struct hostent *hp;
	struct in_addr in;
	struct sockaddr_in local_addr;
	if(!(hp=gethostbyname(szDNSName)))
	{
		result = ("Error dns name");	
	}
	else
	{
		memcpy(&local_addr.sin_addr.s_addr, hp->h_addr, 4);
		in.s_addr=local_addr.sin_addr.s_addr;
		result = inet_ntoa(in);
	}
	return result;
}

AString ServerIPInfo::GetAddrInfo( UInt64 netKey )
{
	int p1, p2;
	AString ip = Num2IP(netKey, p1, p2);
	AString info;
	info.Format("[%s:%d]", ip.c_str(), p1+p2);
	return info;
}
