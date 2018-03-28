#include "UDPNet.h"
#include "SocketAPI.h"
#include "TableTool.h"
#include <mswsock.h>
#include "TimeManager.h"

UDPNet::UDPNet()
	: mSendDataBuffer(1400)
{
	mSocketID = (int)SocketAPI::socket_ex( AF_INET , SOCK_DGRAM , IPPROTO_IP );
	DWORD dw = 0;
	BOOL b = FALSE;
	DWORD status = WSAIoctl(mSocketID, SIO_UDP_CONNRESET, &b, sizeof(b), NULL, 0, &dw, NULL, NULL);
	if (status==SOCKET_ERROR)
	{
		DWORD err = GetLastError();
	}

	//BOOL bBroadcast=TRUE;   
	//setsockopt(mSocketID,SOL_SOCKET,SO_BROADCAST,(const char *)&bBroadcast,sizeof(BOOL));  //设置广播权限   

}

UDPNet::~UDPNet()
{
	if (mSocketID!=INVALID_SOCKET)
		SocketAPI::closesocket_ex(mSocketID);
	mSocketID = INVALID_SOCKET;
}


bool UDPNet::Bind( const char *szIP, int nPort )
{
	sockaddr_in	sAddr;
	memset(&sAddr, 0, sizeof(sAddr));
	sAddr.sin_family = AF_INET;
	sAddr.sin_addr.s_addr = inet_addr(szIP);
	sAddr.sin_port        = htons( nPort);

	BOOL result = SocketAPI::bind_ex( mSocketID , (const struct sockaddr *)&sAddr , sizeof(sAddr) );
	return result==TRUE;
}

int UDPNet::ProcessReceive( DataStream *dataBuffer, UInt64 &scrAddr, bool bBlock )
{
	AssertNote(dataBuffer->data()!=NULL && dataBuffer->size()>0, "接收缓存必须初始");
	sockaddr_in	sAddr;
	int s = sizeof(SOCKADDR_IN);

	if (!bBlock && SocketAPI::availablesocket_ex( mSocketID )<=0)
		return 0;

	int re = recvfrom(mSocketID, (char*)dataBuffer->data(), dataBuffer->size()-1, 0, (sockaddr*)&sAddr, &s);
	if (re==SOCKET_ERROR && GetLastError()==10035) 
		re = 0;

	if (re>0)
	{
		dataBuffer->setDataSize(re);
		dataBuffer->data()[re] = '\0';
		scrAddr = sAddr.sin_addr.s_addr;
		scrAddr = scrAddr<<32;
		scrAddr += ntohs(sAddr.sin_port);
		//TABLE_LOG("----> ip [%s : %u] > R [%d]\n", inet_ntoa(sAddr.sin_addr), ntohs(sAddr.sin_port), re);

		return re;
	}
	else if (re<0)
	{
		//static char szTemp[128]; 
		TABLE_LOG(" >>> receive error > [%d]", ::GetLastError());
		re = 0;
	}
	dataBuffer->setDataSize(0);
	return re;
}

bool UDPNet::Send( DataStream *sendData, UInt64 destAddr )
{
	return Send(destAddr, sendData->data(), sendData->dataSize());
	//sockaddr_in sAddr;
	//static const int s = sizeof(SOCKADDR_IN);
	//sAddr.sin_family = AF_INET;
	//sAddr.sin_addr.s_addr = destAddr>>32; //inet_addr(localIP);
	//uint port = (uint)(destAddr & 0xFFFFFFFF);
	//sAddr.sin_port = htons(port);

	//int re = sendto(mSocketID, (const char*)sendData->data(), sendData->dataSize(), 0, (const sockaddr*)&sAddr, s);
	//if (re==SOCKET_ERROR)
	//{
	//	//char szTemp[128];
	//	TABLE_LOG( "Error: udp send error > %u ", GetLastError());			
	//}
	//return re==sendData->dataSize();
}

bool UDPNet::Send( DataStream *sendData, const char *szIp, int port )
{
	return Send(szIp, port, sendData->data(), sendData->dataSize());
	//sockaddr_in sAddr;
	//static const int s = sizeof(SOCKADDR_IN);
	//sAddr.sin_family = AF_INET;
	//sAddr.sin_addr.s_addr = inet_addr(szIp);		
	//sAddr.sin_port = htons(port);

	//int re = sendto(mSocketID, (const char*)sendData->data(), sendData->dataSize(), 0, (const sockaddr*)&sAddr, s);
	//if (re==SOCKET_ERROR)
	//{
	//	//char szTemp[128];
	//	TABLE_LOG("Error: udp send error > %u ", GetLastError());			
	//}
	//return re==sendData->dataSize();
}

bool UDPNet::Send( UInt64 destAddr, const char *sendData, int sendSize )
{
	if (sendSize<=0)
		return true;

	sockaddr_in sAddr;
	static const int s = sizeof(SOCKADDR_IN);
	sAddr.sin_family = AF_INET;
	sAddr.sin_addr.s_addr = destAddr>>32; 
	uint port = (uint)(destAddr & 0xFFFFFFFF);
	sAddr.sin_port = htons(port);

	int re = sendto(mSocketID, sendData, sendSize, 0, (const sockaddr*)&sAddr, s);
	if (re==SOCKET_ERROR)
	{			
		TABLE_LOG( "Error: udp send error > %u ", GetLastError());			
	}
	return re==sendSize;
}

bool UDPNet::Send( const char *szIp, int port, const char *sendData, int sendSize )
{
	sockaddr_in sAddr;
	static const int s = sizeof(SOCKADDR_IN);
	sAddr.sin_family = AF_INET;
	sAddr.sin_addr.s_addr = inet_addr(szIp);		
	sAddr.sin_port = htons(port);

	int re = sendto(mSocketID, sendData, sendSize, 0, (const sockaddr*)&sAddr, s);
	if (re==SOCKET_ERROR)
	{			
		TABLE_LOG( "Error: udp send error > %u ", GetLastError());			
	}
	return re==sendSize;
}

bool UDPNet::Send(DataStream *sendData, int startPos, int sendSize, UInt64 targetAddr)
{
	if (sendSize<=0)
		return true;

	if (startPos<0)
	{
		ERROR_LOG("Send buffer startPos less 0");
		return false;
	}

	sockaddr_in sAddr;
	static const int s = sizeof(SOCKADDR_IN);
	sAddr.sin_family = AF_INET;
	sAddr.sin_addr.s_addr = targetAddr>>32; 
	uint port = (uint)(targetAddr & 0xFFFFFFFF);
	sAddr.sin_port = htons(port);

	int re = sendto(mSocketID, sendData->data()+startPos, sendSize, 0, (const sockaddr*)&sAddr, s);
	if (re==SOCKET_ERROR)
	{			
		TABLE_LOG( "Error: udp send error > %u ", GetLastError());			
	}
	return re==sendSize;
}

bool UDPNet::SendPacket(UInt64 packedCode, DataStream *sendData, int sendSize, Array<UInt64> &targetList, int nSleepMilSec)
{
	if (sendSize<=0)
		return true;

	if (sendSize>sendData->size())
		return false;

	int msgCount = (sendSize + (MAX_UDP_MSG_SIZE-1))/MAX_UDP_MSG_SIZE;
	if (msgCount>0xFF)
	{
		ERROR_LOG("发送消息包大小大于限制[%d]", (int)(MAX_UDP_MSG_SIZE*0xFF));
		return false;
	}

	int	nowTime = (int)TimeManager::NowTick();

	int alreadySize = 0;
	for (int i=0; i<msgCount; ++i)
	{
		mSendDataBuffer.clear();		

		alreadySize = i*MAX_UDP_MSG_SIZE;
		int lastSize = sendSize - alreadySize;
		if (lastSize>MAX_UDP_MSG_SIZE)
			lastSize = MAX_UDP_MSG_SIZE;
		mSendDataBuffer._write(sendData->data()+(alreadySize), lastSize );		

		if (i==0)
			mSendDataBuffer.write((byte)msgCount);

		// 写入发包时间
		//mSendDataBuffer.write(nowTime);
		mSendDataBuffer.write(packedCode);
		mSendDataBuffer.write((byte)i);

		for (size_t m=0; m<targetList.size(); ++m)
		{
			UInt64 addr = targetList[m];				
			Send (&mSendDataBuffer, addr);
		}			
		//if (nSleepMilSec>0)
		//	TimeManager::Sleep(nSleepMilSec);
	}
	return true;
}

bool UDPNet::SetBlock( bool bBlock )
{
	return SocketAPI::setsocketnonblocking_ex( mSocketID , bBlock ? FALSE:TRUE )==TRUE;
}

void UDPNet::Close()
{
	if (mSocketID!=INVALID_SOCKET)
		SocketAPI::closesocket_ex(mSocketID);
	mSocketID = INVALID_SOCKET;
}

AString UDPNet::getIpPort(int &port)
{
	struct sockaddr addr;
	struct sockaddr_in* addr_v4;
	int addr_len = sizeof(addr);

	//获取local ip and port
	ZeroMemory(&addr, sizeof(addr));

	if (0 == getsockname((SOCKET)mSocketID, &addr, &addr_len))
	{
		if (addr.sa_family == AF_INET)
		{
			addr_v4 = (sockaddr_in*)&addr;
			port = ntohs(addr_v4->sin_port);
			return inet_ntoa(addr_v4->sin_addr);
		}
	}

	return AString();

	//获取remote ip and port
	//ZeroMemory(&addr, sizeof(addr));
	//if (0 == getpeername(s, &addr, &addr_len))
	//{
	//	if (addr.sa_family == AF_INET)
	//	{
	//		addr_v4 = (sockaddr_in*)&addr;
	//		cMonitorInf.nDesIP = addr_v4->sin_addr.S_un.S_addr;
	//		cMonitorInf.nDesPort = ntohs(addr_v4->sin_port);
	//	}
	//}
}

bool UDPNet::SetReceiveBufferSize(int size)
{
	return SocketAPI::setsockopt_ex( (SOCKET)mSocketID , SOL_SOCKET , SO_RCVBUF , &size, sizeof(UINT) )==TRUE;
}

bool UDPNet::SetSendBufferSize(int size)
{
	return SocketAPI::setsockopt_ex( (SOCKET)mSocketID , SOL_SOCKET , SO_SNDBUF , &size, sizeof(UINT) )==TRUE;
}

