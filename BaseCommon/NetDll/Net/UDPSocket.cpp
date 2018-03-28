
#include "UDPSocket.h"
#include "Assertx.h"
#ifndef __LINUX__
#   include "winsock2.h"
#else
#   include <Sys/socket.h>
#endif
#include <string>

using namespace std;

UDPSocket::UDPSocket()
{
	FD_ZERO(&mFdSet);
	mTimeVal.tv_sec = 0;
	mTimeVal.tv_usec = 10;
}


BOOL UDPSocket::create()
{
	__ENTER_FUNCTION_FOXNET

	m_SocketID = SocketAPI::socket_ex( AF_INET , SOCK_DGRAM , IPPROTO_IP );

	memset( &m_SockAddr , 0 , sizeof(m_SockAddr) );

	m_SockAddr.sin_family = AF_INET;

	log("Info: create udp socket.");

	if( isValid() ) 
	{
		//1.用FD_ZEOR初始化fd_set，timeval  
		//2.用FD_SET将SOCKET加入集合  
		//3.调用select函数  
		//4.调用FD_ISSET判断  
		    
		FD_ZERO(&mFdSet);  
		FD_SET(m_SocketID, &mFdSet);

		// 设置为阻塞方式
		setNonBlocking(FALSE);

		return TRUE ;
	}
	else 
		return FALSE ;

	__LEAVE_FUNCTION_FOXNET 

		return FALSE ;
}

BOOL UDPSocket::connect()
{
	__ENTER_FUNCTION_FOXNET

	//	m_SockAddr.sin_addr.s_addr = inet_addr( m_Host );

	//// set sockaddr's port
	//m_SockAddr.sin_port = htons(m_Port);

	m_SockAddr.sin_addr.s_addr = inet_addr( m_Host );
	// set sockaddr's port
	m_SockAddr.sin_port = htons(m_Port);

	log("info: start udp connect demo, then try send demo msg to target net");

	return TRUE;

	__LEAVE_FUNCTION_FOXNET

		return FALSE;
}

BOOL UDPSocket::connect( const char* szIp, int port )
{
	return Socket::connect(szIp, port);
}

UINT UDPSocket::send( const VOID* buf, UINT len, UINT flags )
{	
	static const int s = sizeof(SOCKADDR_IN);
	int re = sendto(CurrentSendSocketID(), (const char*)buf, len, 0, (const sockaddr*)&(m_SockAddr), s);
	if (re==SOCKET_ERROR)
	{
		log("Error: udp send error");
	}
	return re; 
}

UINT UDPSocket::receive( VOID* buf, UINT len, UINT flags /*= 0*/ )
{
	static int size = sizeof(SOCKADDR_IN);
	
	if (available()<=0)
		return 0;
#ifdef __LINUX__
    socklen_t l = size;
    int re = recvfrom(m_SocketID, (void*)buf, len, 0, (sockaddr*)&(m_SockAddr), &l);
    if (re==SOCKET_ERROR )
		re = 0;
#else
	int re = recvfrom(m_SocketID, (char*)buf, len, 0, (sockaddr*)&(m_SockAddr), &size);
    if (re==SOCKET_ERROR && GetLastError()==10035)
		re = 0;
#endif
    
		 
	else if (re>0)
	{
		static char szTemp[128];
		sprintf(szTemp, "----> ip [%s : %u] > R [%d]\n", inet_ntoa(m_SockAddr.sin_addr), ntohs(m_SockAddr.sin_port), re);
		log(szTemp);
	}
	return re;
}

void UDPSocket::ReadyAddr( const SOCKADDR_IN &in )
{
	m_SockAddr = in;
	strncpy( m_Host, inet_ntoa(in.sin_addr), IP_SIZE);
	m_Port = in.sin_port;
}


UINT UDPSocket::__send( const VOID* buf, UINT len, UINT flags, const sockaddr_in *targetSockAddr )
{
	static const int s = sizeof(SOCKADDR_IN);
	int re = sendto(CurrentSendSocketID(), (const char*)buf, len, flags, (const sockaddr*)targetSockAddr, s);
	if (re==SOCKET_ERROR)
	{
#ifndef __LINUX__
		char szTemp[128];
		sprintf_s(szTemp, 128, "Error: udp send error > %u ", GetLastError());
		log(szTemp);
#else
        log("Error: udp send error");
#endif
	}
	return re;
}

const int ADDR_SIZE = sizeof(SOCKADDR);

UINT UDPSocket::__receive( VOID* buf, UINT len, UINT flags /*= 0*/, sockaddr *revSockAddr, bool bNeedWait )
{
	if (!bNeedWait && available()<=0)
		return 0;

	int size = ADDR_SIZE;	

#ifdef __LINUX__
    socklen_t l = size;
    int re = recvfrom(m_SocketID, (void*)buf, len, 0, revSockAddr, &l);
    //size = l;
    if (re==SOCKET_ERROR )
		re = 0;
#else
	int re = recvfrom(m_SocketID, (char*)buf, len, 0, revSockAddr, &size);
    if (re==SOCKET_ERROR && GetLastError()==10035) 
		re = 0;
#endif
	 
#ifdef __WINDOWS__
	if (re>0)
	{
		sockaddr_in *pAddr = (sockaddr_in*)revSockAddr;
		static char szTemp[128]; 
		sprintf_s(szTemp, 127, "----> ip [%s : %u] > R [%d]\n", inet_ntoa(pAddr->sin_addr), ntohs(pAddr->sin_port), re);
		log(szTemp); 
	}
	else if (re<0)
	{
		static char szTemp[128]; 
		sprintf_s(szTemp, 127, " >>> receive error > [%u]", ::GetLastError());
		log(szTemp);
		re = 0;
	}
#else
		if (re<0)
			re = 0;
#endif
	return re;
}

void UDPSocket::setSOCKET( SOCKET sock )
{
	AssertEx(0, "只用于UDP服务连接, 其他不可使用");
}

SocketInputStream* UDPSocket::GetReceiveFromThread()
{
	AssertEx(0, "必须应用于 UDPThreadReceiveSocket");
	return NULL;
}


