//
//文件名称：	SocketInputStream.h
//功能描述：	消息数据的接收缓存，提供数据的接收和格式化读取功能
//				
//
//


#ifndef __SOCKETINPUTSTREAM_H__
#define __SOCKETINPUTSTREAM_H__

#include "BaseCommon.h"
#include "NetHead.h"
#include "AutoString.h"
#include "LoopDataStream.h"


//-------------------------------------------------------------------------
class Socket;
class Packet;
class tNetObject;

class Net_Export SocketInputStream : public LoopDataStream
{
public :
	SocketInputStream(UINT BufferSize = DEFAULTSOCKETINPUTBUFFERSIZE,
		UINT MaxBufferSize = DISCONNECTSOCKETINPUTSIZE);
	//SocketInputStream( Socket* sock, 
	//				   UINT BufferSize = DEFAULTSOCKETINPUTBUFFERSIZE,
	//				   UINT MaxBufferSize = DISCONNECTSOCKETINPUTSIZE ) ;
	virtual ~SocketInputStream( ) ;

public :
	BOOL						Find( CHAR* buf ) ;
		
	INT							Fill( Socket *m_pSocket ) ;
	

//private :	
//	CHAR*		m_Buffer ;
//	
//	UINT		m_BufferLen ;
//	UINT		m_MaxBufferLen ;
//	
//	UINT		m_Head ;
//	UINT		m_Tail ;



};




#endif
