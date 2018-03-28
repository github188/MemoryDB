
#include "BaseNetManager.h"
#include "SocketOutputStream.h"
#include "SocketInputStream.h"
#include "Packet.h"
#include "BasePacketFactoryManager.h"
#include "Socket.h"
#include "ServerSocket.h"
#include "BaseType.h"
#include <WinSock2.h>
#include "Assertx.h"

#include "NetSafeCheck.h"
#include "NetServerTool.h"

#include "EventCore/Core/NetTool.h"
//----------------------------------------------------------------------------------------------

//#define Assert(a) assert(a)



//bool tNetManager::__RemoveConnect( tConnect *pConnect )
//{
//	for (EasyList<tConnect*>::iterator it=mConnectList.begin(); it!=mConnectList.end(); ++it)
//	{
//		if (*it==pConnect)
//		{
//			mConnectList.erase(it);
//			break;
//		}
//	}
//	return _ReleaseConnect(pConnect); 
//	//return false;
//}



//----------------------------------------------------------------------------------------------

//bool tNetManager::_ReleaseConnect(tConnect *pConnect)
//{
//#if USE_SELECT_MODE
//	Socket *p = pConnect->GetSocket();
//	if (NULL==p)
//	{
//		ReleaseConnect(pConnect);
//		return TRUE;
//	}
//	SOCKET fd = pConnect->GetSocket()->getSOCKET() ;
//	Assert( m_MinFD!=INVALID_SOCKET ) ;
//	Assert( m_MaxFD!=INVALID_SOCKET ) ;
//
//	if( fd==INVALID_SOCKET ) 
//	{
//		ReleaseConnect(pConnect);
//		return TRUE;
//	}
//	if( fd==m_MinFD ) 
//	{
//		if( m_MinFD == m_MaxFD )
//		{
//			m_MinFD = m_MaxFD = INVALID_SOCKET ;
//		}
//		else
//		{
//			SOCKET s = m_MaxFD ;
//			for (EasyList<tConnect*>::iterator it=mConnectList.begin(); it!=mConnectList.end(); ++it)
//			{			
//				SOCKET temp = (*it)->GetSocket()->getSOCKET() ;
//				if( temp == fd )
//					continue ;
//				if( temp==INVALID_SOCKET )
//					continue ;
//
//				if( temp < s )
//				{
//					s = temp ;
//				}
//			}
//			if( s > m_SocketID )
//			{
//				m_MinFD = m_SocketID ;
//			}
//			else
//			{
//				m_MinFD = s ;
//			}
//		}
//	} 
//	else if( fd==m_MaxFD ) 
//	{
//		if( m_MaxFD == m_MinFD )
//		{
//			m_MinFD = m_MaxFD = INVALID_SOCKET ;
//		}
//		else
//		{
//			SOCKET s = m_MinFD ;
//			for (EasyList<tConnect*>::iterator it=mConnectList.begin(); it!=mConnectList.end(); ++it)
//			{			
//				SOCKET temp = (*it)->GetSocket()->getSOCKET() ;
//				if( temp == fd )
//					continue ;
//				if( temp==INVALID_SOCKET )
//					continue ;
//
//				if( temp > s )
//				{
//					s = temp ;
//				}
//			}
//			if( s < m_SocketID )
//			{
//				m_MaxFD = m_SocketID ;
//			}
//			else
//			{
//				m_MaxFD = s ;
//			}
//		}
//	}
//
//	FD_CLR(fd , m_ReadFDs[SELECT_BAK]);
//	FD_CLR(fd , m_ReadFDs[SELECT_USE]);
//	FD_CLR(fd , m_WriteFDs[SELECT_BAK]);
//	FD_CLR(fd , m_WriteFDs[SELECT_USE]);
//	FD_CLR(fd , m_ExceptFDs[SELECT_BAK]);
//	FD_CLR(fd , m_ExceptFDs[SELECT_USE]);
//
//	m_nFDSize-- ;
//	Assert( m_nFDSize>=0 ) ;
//#endif
//	
//	//Log( "Remove [%s : %u] success.", pConnect->GetSocket()->getHost(), pConnect->GetSocket()->getPort() ) ;
//	delete pConnect; 
//
//	return TRUE;
//}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------
//void tNetManager::Broadcast(Packet *pPacket)
//{
//	for (size_t it = 0; it<mConnectList.size(); ++it)
//	{
//		tConnect *pConnect = mConnectList[it];
//		if (NULL==pConnect)
//			break;
//
//		if (!pConnect->IsUse() || pConnect==m_pWaitConnect)
//			continue;
//
//		pConnect->Send(pPacket);
//	}
//}

//BOOL tNetManager::AddConnect( tConnect* pConnect )
//{
//#if USE_SELECT_MODE
//	if( m_nFDSize>=FD_SETSIZE )
//	{
//		//已经超出能够检测的网络句柄最大数；
//		return FALSE ;
//	}
//	SOCKET fd = pConnect->GetSocket()->getSOCKET() ;
//	Assert( fd != INVALID_SOCKET ) ;
//
//	if( FD_ISSET(fd, m_ReadFDs[SELECT_BAK]) )
//	{
//		return FALSE ;
//	}
//
//	m_MinFD = min(fd , m_MinFD);
//	m_MaxFD = max(fd , m_MaxFD);
//
//	FD_SET(fd , m_ReadFDs[SELECT_BAK]);
//	FD_SET(fd , m_WriteFDs[SELECT_BAK]);
//	FD_SET(fd , m_ExceptFDs[SELECT_BAK]);
//
//	m_nFDSize++ ;
//#endif
//	mConnectList.insert(pConnect);
//	Log(" Append connect, now connect count %u \n", mConnectList.size() );
//	OnAddConnect(pConnect);
//
//	return TRUE ;
//}



//----------------------------------------------------------------------------------------------
