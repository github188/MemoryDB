
#include "NetConnect.h"

#include "Socket.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

#include "NetHandle.h"
#include "TableTool.h"

#include "EventProtocol.h"
#include "Packet.h"

// 发送数据需要进行压缩的最小大小, 大于或等于此值才进行压缩. 测试 32, 发布可以考虑为256
#define  NEED_ZIP_MIN_SIZE	(256)
//----------------------------------------------------------------------------------------------
tConnect::tConnect()
: mSocketInputStream(NULL)
, mSocketOutputStream(NULL)
, mSocket(NULL)
, mNetState(NET_INIT_CONNECT)
, mOutputZipBuffer(NULL)
, mInputUNZipBuffer(NULL)
, mConnectID(-1)
, mSendTotalSize(0)
{

}
//----------------------------------------------------------------------------------------------

tConnect::~tConnect()
{	
	SetNeedRemove("释放连接资源");
	SAFE_DELETE(mSocket);
	SAFE_DELETE(mSocketInputStream);
	SAFE_DELETE(mSocketOutputStream);

	SAFE_DELETE(mOutputZipBuffer);
	SAFE_DELETE(mInputUNZipBuffer);
}
//----------------------------------------------------------------------------------------------

void tConnect::Close()
{
	if (GetNetHandle()!=NULL)
		GetNetHandle()->OnCloseConnect(this);

	SetNeedRemove("连接被关闭");
}
//----------------------------------------------------------------------------------------------

void tConnect::SetNeedRemove( const char *szInfo )
{
	OnClose();
	
	if (mSocket)
	{			
		mRemoveInfo = szInfo;
		AString temp = GetIpPort();
		Log("Now remove connect [%d] >>>[%s], Info [%s]", GetNetID(), temp.c_str(), mRemoveInfo.c_str() ); 
		
		mSocket->close();	

		SAFE_DELETE(mSocket);
	} 
	else
		Log(szInfo);
	
	SetState(NET_INIT_CONNECT);
	SetUse(false);
}

//----------------------------------------------------------------------------------------------

bool tConnect::Connect( const char *ip, unsigned int port, bool bWait /*= false*/ )
{
	SetState(NET_CONNECT_BEGIN);
	
	return true;
}

//----------------------------------------------------------------------------------------------

bool tConnect::TryConnect( void )
{
	AString strIp = GetHost();
	int nPort = GetPort();
	if (strIp.length()>0 && nPort>0)
	{
		CreateSocket();
		return Connect(strIp.c_str(), nPort); 
	}
	return false;
}

//----------------------------------------------------------------------------------------------

bool tConnect::Send(const char* szData, size_t length )
{
	if (mSocketOutputStream)
		return mSocketOutputStream->Write(szData, (UINT)length)==(UINT)length;
	return false;
}

bool tConnect::Send( const Packet* pPacket, bool )
{
	tNetHandle *p = GetNetHandle();
	if (p)
		return p->GetNetProtocol()->WritePacket(pPacket, mSocketOutputStream);
	return false;
}

bool tConnect::SendEvent( Logic::tEvent *pEvent )
{
	if (GetNetHandle()!=NULL)
		return EventNetProtocol::SendEvent(this, pEvent);

	return false;
}

//----------------------------------------------------------------------------------------------

bool tConnect::ProcessOutput()
{
	if (mNetState!=NET_CONNECT_OK)
		return true;

	UINT result = mSocketOutputStream->Flush(mSocket);	

	if  (result!=SOCKET_ERROR && result!=SOCKET_ERROR-1)
	{
		mSendTotalSize += result;
		return true;
	}
	SetNeedRemove("发送数据时出错,设置为移除");
	
	return false;
		
}
//----------------------------------------------------------------------------------------------

bool tConnect::ProcessInput()
{
	if (mNetState==NET_INIT_CONNECT)
		return true;

	bool bRe = _ReceiveData();

	OnReceiveData();

	return bRe;
}
//----------------------------------------------------------------------------------------------

bool tConnect::Process(void)
{
	if( ProcessInput() )
		return ProcessOutput();

	return false;
}
//----------------------------------------------------------------------------------------------
void tConnect::CreateSocket(void)
{	
	SAFE_DELETE(mOutputZipBuffer);
	SAFE_DELETE(mInputUNZipBuffer);
	if (mSocket)
	{
		mSocket->close();
		delete mSocket;		
	}
	mSocket = NULL;
	{	
		_CreateSocket();

		if (mSocket->create())
			Log("创建网络成功!");
		else
			Log("创建网络失败!");

		mSocket->setNonBlocking(TRUE);
		mSocket->setLinger(0);
	}

	if (mSocketInputStream==NULL)
		mSocketInputStream = MEM_NEW SocketInputStream();

	if (mSocketOutputStream==NULL)
		mSocketOutputStream = MEM_NEW SocketOutputStream();

	SetState(NET_INIT_CONNECT);
}
//----------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------

bool tConnect::HasNetError()
{
	if (GetSocket())
		return GetSocket()->isSockError()==TRUE;
	return false;
}
//----------------------------------------------------------------------------------------------

Socket* tConnect::GetSocket( void )
{
	return mSocket;
}
//----------------------------------------------------------------------------------------------


AString tConnect::GetHost( void )
{
	if (GetSocket())
		return GetSocket()->getHost();
	return "";
}

//----------------------------------------------------------------------------------------------

int tConnect::GetPort( void )
{
	if (GetSocket())
		return GetSocket()->getPort();
	return 0;
}

//----------------------------------------------------------------------------------------------
void tConnect::OnReceiveData()
{		
	SocketInputStream *receiveDataStream = mSocketInputStream;

	tNetHandle *pNetTool = GetNetHandle();
	if (pNetTool)
	{
		while (receiveDataStream)
		{
			HandPacket packet = pNetTool->GetNetProtocol()->ReadPacket(this, receiveDataStream);
			if (packet)
				packet->Execute(this);				
			else
				break;
			//No thing ... look up
		}
	}
}


AString tConnect::GetIpPort( void )
{
	AString info = "[";
	info += GetHost(); 
	info += ":"; 
	info += GetPort(); 
	info += "]";
	return info;
}



int tConnect::SendSize( void )
{
	return (int)mSocketOutputStream->Length();
}

void tConnect::Log( const char *szInfo, ... )
{
#if DEVELOP_MODE
	va_list va;
	va_start(va, szInfo);
	LOG_TOOL(va, szInfo); 
#endif
}


void tConnect::_CreateSocket( void )
{
	mSocket = new Socket();
}



UInt64 tConnect::GetAddrIndexKey( void )
{
	UInt64 key = 0;
	if (GetSocket())
	{
#if __LINUX__
        key = GetSocket()->getSocketAddr().sin_addr.s_addr;
#else
		key = GetSocket()->getSocketAddr().sin_addr.S_un.S_addr;
#endif
		key = (key<<32)+GetSocket()->getSocketAddr().sin_port;
	}
	return key;
}


bool tConnect::_ReceiveData()
{
	UINT result = mSocketInputStream->Fill(mSocket);
	if (result==SOCKET_ERROR || result==SOCKET_ERROR-3)
	{
		SetNeedRemove("接收数据时出错,设置为移除");
		return false;
	}
	return true;
}

UINT tConnect::GetSendBufferDataSize()
{
	return mSocketOutputStream->Length();
}



//----------------------------------------------------------------------------------------------