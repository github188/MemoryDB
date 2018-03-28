//#include "stdafx.h"


#include "SocketOutputStream.h"
#include "Socket.h"
#include "Packet.h"
#include "Assertx.h"
#include "AutoString.h"
#include <stdlib.h>
#include "PacketFactory.h"

#if __LINUX__
#   define MSG_NOSIGNAL  0
#endif



SocketOutputStream::SocketOutputStream( UINT BufferSize /*= DEFAULTSOCKETOUTPUTBUFFERSIZE*/, UINT MaxBufferSize /*= DISCONNECTSOCKETOUTPUTSIZE */ )
	: LoopDataStream(BufferSize, MaxBufferSize) //m_Buffer(NULL)
{
	__ENTER_FUNCTION_FOXNET


	__LEAVE_FUNCTION_FOXNET
}

SocketOutputStream::~SocketOutputStream( ) 
{
__ENTER_FUNCTION_FOXNET

		
__LEAVE_FUNCTION_FOXNET
}


//UINT SocketOutputStream::Write( const CHAR* buf )
//{
//    __ENTER_FUNCTION_FOXNET
//    return Write( buf, strlen(buf) ); 
//    __LEAVE_FUNCTION_FOXNET
//}


//UINT SocketOutputStream::Write( const CHAR* buf, UINT len ) 
//{
//__ENTER_FUNCTION_FOXNET
//
//		//					//
//        //     T  H			//    H   T			LEN=10
//        // 0123456789		// 0123456789
//        // abcd...efg		// ...abcd...
//        //					//
//			
//	UINT nFree = ( (m_Head<=m_Tail)?(m_BufferLen-m_Tail+m_Head-1):(m_Head-m_Tail-1) ) ;
//
//	if( len>=nFree )
//	{
//		if( !Resize( len-nFree+1 ) )
//			return 0 ;
//	}
//		
//	if( m_Head<=m_Tail ) 
//	{	
//		if( m_Head==0 ) 
//		{
//			nFree = m_BufferLen - m_Tail - 1;
//			memcpy( &m_Buffer[m_Tail], buf, len ) ;
//		} 
//		else 
//		{
//			nFree = m_BufferLen-m_Tail ;
//			if( len<=nFree )
//			{
//				memcpy( &m_Buffer[m_Tail], buf, len ) ;
//			}
//			else 
//			{
//				memcpy( &m_Buffer[m_Tail], buf, nFree ) ;
//				memcpy( m_Buffer, &buf[nFree], len-nFree ) ;
//			}
//		}
//	} 
//	else 
//	{	
//		memcpy( &m_Buffer[m_Tail], buf, len ) ;
//	}
//	
//	m_Tail = (m_Tail+len)%m_BufferLen ;
//		
//	return len;
//	
//__LEAVE_FUNCTION_FOXNET
//
//	return 0 ;
//}


//VOID SocketOutputStream::InitSize( )
//{
//	m_Head = 0 ;
//	m_Tail = 0 ;
//
//	_AllotBuffer(DEFAULTSOCKETINPUTBUFFERSIZE);
//
//	m_BufferLen = DISCONNECTSOCKETINPUTSIZE ;
//
//}

INT SocketOutputStream::Flush( Socket *m_pSocket ) 
{
__ENTER_FUNCTION_FOXNET

	Assert( m_pSocket != NULL );

	UINT nFlushed = 0;
	UINT nSent = 0;
	UINT nLeft;

	if( m_BufferLen>m_MaxBufferLen )
	{
		//如果单个客户端的缓存太大，则重新设置缓存，并将此客户端断开连接
		InitSize( ) ;
		
		return SOCKET_ERROR-1 ;
	}

#if defined(__WINDOWS__)
	UINT flag = MSG_DONTROUTE ;
#elif defined(__LINUX__)
	UINT flag = MSG_NOSIGNAL ;
#endif

	if ( m_Head < m_Tail ) 
	{
		nLeft = m_Tail - m_Head;

		while ( nLeft > 0 ) 
		{
			nSent = m_pSocket->send( &m_Buffer[m_Head] , nLeft , flag );
			if (nSent==SOCKET_ERROR_WOULDBLOCK) return 0 ; 
			if (nSent==SOCKET_ERROR) return SOCKET_ERROR;
			if (nSent==0) return 0;

			nFlushed += nSent;
			nLeft -= nSent;
			m_Head += nSent;
		}

		Assert( nLeft == 0 );

	} 
	else if( m_Head > m_Tail ) 
	{
		nLeft = m_BufferLen - m_Head;

		while ( nLeft > 0 ) 
		{
			nSent = m_pSocket->send( &m_Buffer[m_Head] , nLeft , flag );
			if (nSent==SOCKET_ERROR_WOULDBLOCK) return 0 ; 
			if (nSent==SOCKET_ERROR) return SOCKET_ERROR;
			if (nSent==0) return 0;

			nFlushed += nSent;
			nLeft -= nSent;
			m_Head += nSent;
		}

		Assert( m_Head == m_BufferLen );

		m_Head = 0;

		nLeft = m_Tail;

		while ( nLeft > 0 ) 
		{
			nSent = m_pSocket->send( &m_Buffer[m_Head] , nLeft , flag );
			if (nSent==SOCKET_ERROR_WOULDBLOCK) return 0 ; 
			if (nSent==SOCKET_ERROR) return SOCKET_ERROR;
			if (nSent==0) return 0;

			nFlushed += nSent;
			nLeft -= nSent;
			m_Head += nSent;
		}

		Assert( nLeft == 0 );
	}

	///???
	////if ( m_Head != m_Tail ) 
	////{
	////	Assert( m_Head == m_Tail );
	////}

	m_Head = m_Tail = 0 ;

	return (INT)nFlushed;

	__LEAVE_FUNCTION_FOXNET

	return 0 ;
}


UINT SocketOutputStream::GetFreeLength()
{
	if (m_Head<=m_Tail)
		return (m_BufferLen-m_Tail+m_Head-1);
	else
		return (m_Head-m_Tail-1);
}





