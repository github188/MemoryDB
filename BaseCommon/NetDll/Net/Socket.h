//
//文件名称：	Socket.h
//功能描述：	封装网络Socket的功能，通过接口实现所有的网络操作
//				
//
#include "BaseCommon.h"
#if defined NET_EXPORTS || defined USE_SOCKET || defined __LINUX__

#ifndef __SOCKET_H__
#define __SOCKET_H__


#include "SocketAPI.h"

#include "NetHead.h"
#include "Lock.h"

#ifndef LPCSTR
#   define LPCSTR const char*
#endif

//////////////////////////////////////////////////////////////////////////////
// class Socket
//
// TCP Client Socket
//
// 
//
// 
//////////////////////////////////////////////////////////////////////////////

class Net_Export Socket 
{
	//friend class UDPServerSocket;
//////////////////////////////////////////////////
// constructor/destructor
//////////////////////////////////////////////////
public :
	
	// constructor
	Socket () ;
	Socket (const CHAR* host, UINT port) ;
	
	// destructor
	virtual ~Socket () ;

public:
	virtual void log(const char* szInfo);

	virtual bool ReadyConnect();
//////////////////////////////////////////////////
// methods
//////////////////////////////////////////////////
public :
	virtual BOOL create() ;
	
	// close connection
	VOID close () ;
	
	// try connect to remote host
	virtual BOOL connect () ;
	virtual BOOL connect (const CHAR* host, UINT port) ;

	virtual void ReadyAddr(const SOCKADDR_IN &in){}
	// close previous connection and connect to another socket
	BOOL reconnect (const CHAR* host, UINT port) ;
	
	// send data to peer
	virtual UINT send (const VOID* buf, UINT len, UINT flags = 0) ;
	
	// receive data from peer
	virtual UINT receive (VOID* buf, UINT len, UINT flags = 0) ;
	
	UINT available ()const;
	BOOL canWrite(int checkTime);

	SOCKET accept( struct sockaddr* addr, UINT* addrlen ) ;

	BOOL bind( ) ;
	BOOL bind( UINT port ) ;

	BOOL listen( INT backlog ) ;

//////////////////////////////////////////////////
// methods
//////////////////////////////////////////////////
public : 
    // get/set socket's linger status
    UINT getLinger ()const ;
    BOOL setLinger (UINT lingertime) ;

	BOOL isReuseAddr ()const ;
	BOOL setReuseAddr (BOOL on = TRUE) ;

	// get is Error
    UINT getSockError()const ;
 
    // get/set socket's nonblocking status
    BOOL isNonBlocking ()const ;
    BOOL setNonBlocking (BOOL on = TRUE) ;
 
    // get/set receive buffer size
    UINT getReceiveBufferSize ()const ;
    BOOL setReceiveBufferSize (UINT size) ;
 
    // get/set send buffer size
    UINT getSendBufferSize ()const ;
    BOOL setSendBufferSize (UINT size) ;
 
	void setHost(const char* szIp);
	void setPort(int port){ m_Port = port; }
	UINT getPort ()const;
	IP_t getHostIP ()const;
	LPCSTR getHost() const;
	SOCKADDR_IN& getSocketAddr(){ return m_SockAddr; }

	// check if socket is valid
	BOOL isValid ()const ;

	virtual void setSOCKET(SOCKET sock);
	// get socket descriptor
	SOCKET getSOCKET ()const ;

	BOOL isSockError()const ;

	

protected:

	SOCKET m_SocketID;
	
	// socket address structure
	SOCKADDR_IN m_SockAddr;
	
	// peer host
	CHAR m_Host[IP_SIZE];
	
	// peer port
	UINT m_Port;

public:
#if DEVELOP_MODE
	static CsLock		mDebugLock;
	static int		mSocketCount;
#endif
};

#endif


#endif //NET_EXPORTS