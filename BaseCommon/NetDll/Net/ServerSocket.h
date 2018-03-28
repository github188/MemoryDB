
#ifndef __SERVER_SOCKET__
#define __SERVER_SOCKET__

//////////////////////////////////////////////////
// include files
//////////////////////////////////////////////////
#include "BaseCommon.h"
#include "Socket.h"

#include "NetHead.h"


//#ifdef __WINDOWS__
//	#pragma comment(lib, "../../Common/KeyDog.lib")
//	extern BOOL GetMacByCmd(char *lpszMac);
////#else
////	#pragma comment(lib, "../../Common/libCFG.a")
////	extern int		 CheckSystem();
//#endif
//////////////////////////////////////////////////////////////////////
//
// class ServerSocket
//
//////////////////////////////////////////////////////////////////////
class tServerNet;
class tConnect;

class Net_Export ServerSocket 
{
public :
	ServerSocket()
		: m_Socket(NULL)
	{

	}		
	// destructor
	virtual ~ServerSocket () ;
	
	virtual bool InitCreate(UINT port, UINT backlog = 5) = 0;
	virtual void Process(tServerNet *){}
//////////////////////////////////////////////////
// methods
//////////////////////////////////////////////////
public :
	
	// close socket
	VOID close () ;
	
	// accept new connection
	virtual BOOL accept ( Socket* socket );

	// get/set socket's linger status
    UINT getLinger () { return m_Socket->getLinger(); }
    VOID setLinger (UINT lingertime) { m_Socket->setLinger(lingertime); }
 
     // get/set socket's nonblocking status
    BOOL isNonBlocking () const { return m_Socket->isNonBlocking(); }
    VOID setNonBlocking (BOOL on = TRUE) { m_Socket->setNonBlocking(on); }
 
    // get/set receive buffer size
    UINT getReceiveBufferSize () const { return m_Socket->getReceiveBufferSize(); }
    VOID setReceiveBufferSize (UINT size) { m_Socket->setReceiveBufferSize(size); }
 
    // get/set send buffer size
    UINT getSendBufferSize () const { return m_Socket->getSendBufferSize(); }
    VOID setSendBufferSize (UINT size) { m_Socket->setSendBufferSize(size); }
 
	SOCKET getSOCKET () { return m_Socket->getSOCKET(); }
	Socket* GetSocket(){ return m_Socket; }

	virtual bool RemoveConnect(tConnect *pConnect){ return false; }

//////////////////////////////////////////////////
// attributes
//////////////////////////////////////////////////
protected :
	
	// socket implementation object
	Socket* m_Socket;


};

class TCPServerSocket : public ServerSocket
{
public:
	virtual bool InitCreate(UINT port, UINT backlog = 5);
};

//------------------------------------------------------------------------
#endif
