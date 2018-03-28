
#include "IOCPConnect.h"
#include "IOCPCommon.h"
#include "EventProtocol.h"
#include "SocketAPI.h"
#include "TimeManager.h"
#include "EventData.h"
#include "ServerIPInfo.h"

#if _DEBUG_NET_
#   include "IOCPServerNet.h"
#endif

void IOCPConnect::_SendTo()
{
	if (mSocketID!=0 && !mNeedRemove)
	{		
		if (mSendOverlapped->mbWaiting)
		{
			if (TimeManager::Now()-mLastSendTime>_IOCP_SEND_OVERTIME)
			{
				WARN_LOG("[%d][%s:%d]连接发送超时>[%d] Sec, 强制关闭连接", GetNetID(), GetIp(), GetPort(), _IOCP_SEND_OVERTIME);
				SetRemove(true);
				return;
			}
		}
		else
		{
			_IOCP_AUTO_LOCK_SEND
				int nFreeSize = 0;
				while (!mWaitSendPacketList.empty())
				{
					HandPacket p =  *mWaitSendPacketList.begin();
					if (mSendData.lastSize()<(int)p->GetPacketSize()+PACKET_HEADER_SIZE)
					{
						if (mSendData.Length()<=0)
						{
							if (mSendData.Resize((int)p->GetPacketSize()+PACKET_HEADER_SIZE*2)==FALSE)
							{
								ERROR_LOG("Resize send buffer size %d fail", (int)p->GetPacketSize()+PACKET_HEADER_SIZE*2);
								SetRemove(true);
								return;
							}
						}
						else
							break;
					}
					if (!GetNetHandle()->GetNetProtocol()->WritePacket(p.getPtr(), &mSendData))
					{
						ERROR_LOG("Save send data fail, then remove");
						SetRemove(true);
						return;
					}
				}

				UINT s = mSendData.Length();
			if (s<=0)
			{			
				return;
			}
			//UINT len = mSendData.Read(mSendOverlapped->buffer, s<MAX_PACKAGE_SIZE ? s : MAX_PACKAGE_SIZE );
			//if (len <= 0)
			//{			
			//	return;			
			//}
			mSendOverlapped->mbWaiting = true;		
			mLastSendTime = TimeManager::Now();
			// 创建一个发送完成事件
			SendOverlapped* pSendOverlapped = mSendOverlapped;

			int dataSize = 0;
			pSendOverlapped->WsaBuf.buf = mSendData.GetDataPtr(dataSize); // pSendOverlapped->buffer;
			AssertNote(dataSize>0, "Must has send data");				
			pSendOverlapped->WsaBuf.len = dataSize;
			//pSendOverlapped->dwSentBytes = 0;
			//pSendOverlapped->dwTotalBytes = dataSize;

			// 发送事件
			DWORD SendBytes = 0;
            int result = WSASend(mSocketID, &pSendOverlapped->WsaBuf, 1, &SendBytes, 0,
                (LPOVERLAPPED)pSendOverlapped, NULL);
			if (   result ==   SOCKET_ERROR )
			{   
				int iError = WSAGetLastError();
				if ( iError != ERROR_IO_PENDING )
				{   
					if (iError==10054)
					{
						// WSAECONNRESET(10054) 远程客户端强制关闭连接
						TABLE_LOG("[%d] [%s:%d] Other part connect close", iError, GetIp(), GetPort());
					}
					else
						ERROR_LOG("IOCPServer::SendData - WSASend() 发生了如下错误：   %d\n",   iError);
					// 异常错误事件
					//OnError(nClient, iError);
					// 关闭SOCKET
					mSendOverlapped->mbWaiting = false;
					Close();
					//Disconnect(nClient);
					return;
				}   
			}   
//#if _IOCP_THREAD_RECEIVE_SEND
//            else if (result!=WSA_IO_PENDING)
//            {
//            }
//#endif
		}
	}
}

void IOCPConnect::_PostReceive( )
{
	if (!mNeedRemove && mSocketID!=0)
	{		
        if (mRecvOverlapped->mbWaiting)
        {
            ERROR_LOG("[%s:%d]Now wait receive state, May be code error", GetIp(), GetPort());
            return;
        }
		_IOCP_AUTO_LOCK_RECEIVE
		DWORD RecvBytes = 0;
		DWORD Flags = 0; 
		RecvOverlapped* pRecvOverlapped = mRecvOverlapped;

		int nFreeSize = 0;	
		pRecvOverlapped->WsaBuf.buf = mReceiveData.GetFreeSpacePtr(nFreeSize);
		if (nFreeSize<1024)
		{
			if (!mReceiveData.Resize(mReceiveData.size()+1024*8))
			{
				ERROR_LOG("[%s:%d] rev data size %d more big, resize %d fail, now remove connect", (int)mReceiveData.size(), (int)mReceiveData.size()+1024);
				SetRemove(true);
				return;
			}
			pRecvOverlapped->WsaBuf.buf = mReceiveData.GetFreeSpacePtr(nFreeSize);
		}
        mRecvOverlapped->mbWaiting = true;
		pRecvOverlapped->WsaBuf.len = nFreeSize;
        int result = WSARecv(mSocketID, &pRecvOverlapped->WsaBuf, 1, &RecvBytes, &Flags,
            (LPOVERLAPPED)pRecvOverlapped, NULL);
		if ( result   ==   SOCKET_ERROR )   
		{   
			int iError = WSAGetLastError();
			if ( iError != ERROR_IO_PENDING )
			{   
                mRecvOverlapped->mbWaiting = false;
				// 异常错误
				ERROR_LOG("NET ERROR: [%d], [%u]", GetNetID(), iError);
				SetRemove(true);
				return;
			}   
		}
//#if _IOCP_THREAD_RECEIVE_SEND
//        else if (result!=WSA_IO_PENDING)
//        {
//            TABLE_LOG("****** [%s:%d] Immediately rev data >%d", GetIp(), GetPort(), RecvBytes);
//        }
//#endif
	}
}

int IOCPConnect::GetLocalPort()
{
	std::string ip;
	unsigned short port = 0;

	struct sockaddr_storage sa;  
	int salen = sizeof(sa);  

	if (::getsockname((SOCKET)mSocketID, (struct sockaddr*)&sa, &salen) == -1) {  
		ip = "?";  
		port = 0;  
		return port;  
	}  

	if (sa.ss_family == AF_INET) {  
		struct sockaddr_in *s = (struct sockaddr_in*)&sa;  
		ip = ::inet_ntoa(s->sin_addr);  
		port = ::ntohs(s->sin_port);  
		return port;  
	}  
	return 0;  	
}

AString IOCPConnect::GetRemoteAddr(int &port)
{
	struct sockaddr addr;
	struct sockaddr_in* addr_v4;
	int addr_len = sizeof(addr);
	//获取remote ip and port
	ZeroMemory(&addr, sizeof(addr));
	if (0 == getpeername(mSocketID, &addr, &addr_len))
	{
		if (addr.sa_family == AF_INET)
		{
			addr_v4 = (sockaddr_in*)&addr;
			port = ntohs(addr_v4->sin_port);
			return ServerIPInfo::Num2IP(addr_v4->sin_addr.S_un.S_addr);			
		}
	}
	return AString();
}

bool IOCPConnect::SendEvent( Logic::tEvent *sendEvent )
{
	if (sendEvent!=NULL)
	{
		return EventNetProtocol::SendEvent(this, sendEvent);
	}

	return false;
}

bool IOCPConnect::Send( const Packet *msgPacket, bool bEncrypt )
{
	tNetHandle *p = GetNetHandle();
	if (p!=NULL)
	{
		_IOCP_LOCK_SEND
			DSIZE nowSize = mSendData.Length();
		msgPacket->SetNeedEncrypt(bEncrypt);
		int msgSize = msgPacket->GetPacketSize()+PACKET_HEADER_SIZE;
		if (msgSize>=mSendData.size())
		{
			ERROR_LOG("%d 消息包%d大于发送缓存空间%d, 发送失败", msgPacket->GetPacketID(), msgSize, (int)mSendData.size() );
			_IOCP_UNLOCK_SEND
			return false;
		}
		GetNetHandle()->AddSendTotalSize( msgSize);		
		if (mWaitSendPacketList.empty() && mSendData.lastSize()>=msgSize)
		{
			bool re = p->GetNetProtocol()->WritePacket(msgPacket, &mSendData);
			if (!re)				
			{
				AString err;
				err.Format("%d [%s:%d] write packet fail to send [%d] size %u, then remove", GetNetID(), GetIp(), GetPort(), msgPacket->GetPacketID(), msgPacket->GetPacketSize());
				ERROR_LOG(err.c_str());
				SetRemove(true);
			}
		}
		else
		{
			if (mWaitSendPacketList.size()>10000)
			{
				NOTE_LOG("ERROR: [%s:%d] connect wait send packet more then 10000, then now remove", GetIp(), GetPort());
				SetRemove(true);
			}
            else
			    mWaitSendPacketList.insert((Packet *)msgPacket);
		}
		_IOCP_UNLOCK_SEND
			_SendTo();
		return true;
	}
	return false;
}

bool IOCPConnect::Process()
{
	if (mNeedRemove)
	{
		if (!mRecvOverlapped->mbWaiting && !mSendOverlapped->mbWaiting)
		{
			// 如果返回false, 会进行释放
				return false;
		}
#if		_DEBUG_NET_
		else if (TimeManager::Now()-mRemoveTime>_DEBUG_REMOVE_OVER_TIME)
		{
			WARN_LOG("连接移除状态超过 [%d] S, 应及时清理", _DEBUG_REMOVE_OVER_TIME);
			mRemoveTime = TimeManager::Now();
		}
#endif
	}
	else
	{
#if	!_IOCP_THREAD_RECEIVE_SEND
		if (mReceiveData.IsEmpty()==FALSE)
#endif
			_ProcessReceiveData();
	}
	return true;
}

void IOCPConnect::Log( const char *szInfo, ... )
{
#if DEVELOP_MODE
	va_list va;
	va_start(va, szInfo);
	LOG_TOOL(va, szInfo);
#endif
}

void IOCPConnect::_Free()
{
	_IOCP_LOCK_RECEIVE
#if DEVELOP_MODE
		if (!mReceiveData.IsEmpty())
			WARN_LOG("移除连接时，接收缓存尚存在 [%u] 数据未处理", mReceiveData.Length());
#endif
	mReceiveData.CleanUp();
	if (mSocketID!=0)
	{		
		if (SocketAPI::closesocket_ex(mSocketID)==FALSE)
			//if (closesocket(mSocketID) == SOCKET_ERROR) 
		{   
			Log("Close socket error >[%u]", GetLastError());				
		}				
		mSocketID = 0;
	}	
	_IOCP_UNLOCK_RECEIVE
}

IOCPConnect::~IOCPConnect()
{
	//if (mSocketID!=0)
	//{
	//	shutdown(mSocketID, SD_BOTH);
	//	mSocketID = 0;
	//}
	if (mSocketID!=0 || mRecvOverlapped->mbWaiting || mSendOverlapped->mbWaiting)
	{
		AssertEx(0, "Socket close 逻辑顺序错误");
	}

	delete mSendOverlapped;
	delete mRecvOverlapped;
	mSendOverlapped = NULL;
	mRecvOverlapped = NULL;

	// 只需要关闭Socket即可，如果关闭Iocp Handle, 则整个IOCP都无效，其它连接也都不能正常工作
	//if (mIocpHandle!=NULL)
	//	CloseHandle((HANDLE)mIocpHandle);
	mIocpHandle = NULL;
}

IOCPConnect::IOCPConnect(int revBufferLen, int sendBufferLen) 
	: mSocketID(0)
	, mNeedRemove(false)
	, mPort(0)
	, mLastSendTime(0)
#if		_DEBUG_NET_
	, mRemoveTime(0)
#endif
	, mIocpHandle(NULL)
	, mbHaveReceiveEventMsg(false)
    , mbHaveRevData(false)
	, mReceiveData(revBufferLen>0?revBufferLen:DEFAULTSOCKETINPUTBUFFERSIZE, revBufferLen>DISCONNECTSOCKETINPUTSIZE?revBufferLen:DISCONNECTSOCKETINPUTSIZE)
	, mSendData(sendBufferLen>0?sendBufferLen:DEFAULTSOCKETINPUTBUFFERSIZE, sendBufferLen>DISCONNECTSOCKETINPUTSIZE?sendBufferLen:DISCONNECTSOCKETINPUTSIZE)
{
	mSendOverlapped = new SendOverlapped();
	mRecvOverlapped = new RecvOverlapped();
}

void IOCPConnect::_ProcessReceiveData()
{
//#if _IOCP_THREAD_RECEIVE_SEND
//	_ProcessMsgData();
//	return; 
//#endif

	if (!mbHaveRevData)
	{
		return;
	}

    //tNetHandle *pNet = GetNetHandle();
    //AssertEx(pNet!=NULL && pNet->GetNetProtocol()!=NULL, "网络管理不存在或协议为空");	
	// NOTE: 如果每个包都加锁，可能会在执行事件的同时会继续接收数据，这样其他连接得到执行的机会就会减少
	// 所以一次处理接收，只需要加锁一次，效率也相对高些
	while (true)
	{
        _IOCP_LOCK_RECEIVE
		HandPacket packet = mNet->GetNetProtocol()->ReadPacket(this, &mReceiveData);
        mbHaveRevData = false;
        _IOCP_UNLOCK_RECEIVE
		if (packet)
		{
			try{
				packet->Execute(this);		
			}
			catch (std::exception &e)
			{
				ERROR_LOG("Net message execute error >%s", e.what());

			}
			catch (...)
			{
				ERROR_LOG("Net message [%d] execute error", packet->GetPacketID());
			}
		}
		else
			break;

		//if (++count>_ONCE_EXECUTE_MSG_COUNT)
		//	break;
	}
}

void IOCPConnect::_ProcessMsgData()
{
#if _IOCP_THREAD_RECEIVE_SEND
	// 处理接收线程接收到的消息数据
	if (mbHaveReceiveEventMsg)
	{
		mRevLock.lock();
		mProcessDataList.swap(mReceiveDataList);
		mbHaveReceiveEventMsg = false;
		mRevLock.unlock();

		for (int i=0; i<mProcessDataList.size(); ++i)
		{
			MsgData &data = mProcessDataList[i];

			if (data.mpEventData!=NULL)
			{					
				EventNetProtocol::ProcessEventData(this, data.mpEventData);
				EventDataPool::Push(data.mpEventData);
			}
			if (data.mpPacketData!=NULL)
			{
				// 处理其他消息包
				EventNetProtocol::ProcessPacketData(this, data.mpPacketData);
				PacketDataPool::Push(data.mpPacketData);
			}
		}
		mProcessDataList.clear(false);
	}
#endif 
}

void IOCPConnect::_OnThreadSendFinish(SendOverlapped* pSendOverlapped, DWORD size)
{
    _IOCP_LOCK_SEND
        pSendOverlapped->mbWaiting = false;
    mSendData._ForceRestoreHead(mSendData.GetHead()+size);
    _SendTo();				
    _IOCP_UNLOCK_SEND
}

void IOCPConnect::_OnThreadRevFinish(RecvOverlapped* pRecvOverlapped, DWORD size)
{
    if (!IsRemove())
    {        
            mReceiveData._ForceRestoreTail(mReceiveData.GetTail()+size);
        _PostReceive();
        mbHaveRevData = true;        
//#if _IOCP_THREAD_RECEIVE_SEND
//            // 直接在接收线程解析接收到的消息数据
//            if (_checkOk())
//                EventNetProtocol::ReadAnalyzeEventData(this, &mReceiveData);
//#endif
    }
}

void IOCPConnect::_Start()
{
	mSendOverlapped->mbWaiting = false;
	mRecvOverlapped->mbWaiting = false;
	_SendTo();
	_PostReceive();
}

void IOCPConnect::SetRemove(bool bNeedRemove)
{
	mNeedRemove = bNeedRemove;  
	_Free(); 
#if _DEBUG_NET_
	mRemoveTime = TimeManager::Now();
#endif
}
//-------------------------------------------------------------------------*/
//-------------------------------------------------------------------------*/
void IOCPServerConnect::OnReceivePacket(Packet *pPacket)
{
	pPacket->Execute(this);
}

//-------------------------------------------------------------------------

void IOCPServerConnect::_ProcessReceiveData()
{
    if (!mbHaveRevData)
        return;

    if (!mbReceiveData)
	{
		_IOCP_AUTO_LOCK_RECEIVE;
		if (mReceiveData.IsEmpty()==TRUE)
		{
			return;
		}
		// NOTE: 默认验证码为0时, 不进行安全码验证
		// 只有验证OK后才执行OnConnected()
		if (mNet->GetSafeCode()==0)
		{
			mbReceiveData = true;
			mNet->OnAddConnect(this);
			OnConnected();
		}
		else
		{	
			int safeCode;
			if (mReceiveData.Read((CHAR*)&safeCode, sizeof(int))==sizeof(int))
			{
				if (safeCode==mNet->GetSafeCode())
				{
					mbReceiveData = true;
					mNet->OnAddConnect(this);
					OnConnected();
					return;
				}
			}
#if DEVELOP_MODE
			ERROR_LOG("WARN: [%s : %d] Connect safe check fail, then now remove", GetIp(), GetPort());
#else
			TABLE_LOG("WARN: [%s : %d] Connect safe check fail, then now remove", GetIp(), GetPort());
#endif
			OnConnectSafeCodeError(GetIp(), GetPort(), safeCode);
			SetRemove(true);
		}
	}
	else
	{
//#if _IOCP_THREAD_RECEIVE_SEND
//		_ProcessMsgData();
//#else
		// NOTE: 如果每个包都加锁，可能会在执行事件的同时会继续接收数据，这样其他连接得到执行的机会就会减少
		// 所以一次处理接收，只需要加锁一次，效率也相对高些
		int count = 0;
		while (count++<_ONCE_EXECUTE_MSG_COUNT) 
		{
            _IOCP_LOCK_RECEIVE
			HandPacket packet = mNet->GetNetProtocol()->ReadPacket(this, &mReceiveData);
            if (!packet)
                mbHaveRevData = false;
            _IOCP_UNLOCK_RECEIVE
			if (packet)
			{
				//if (++count>MAX_PROCESS_PACKET_NUM)
				//{
				//	NOTE_LOG("[%s:%d] 处理接收包数量超时最大数量，移除处理", GetIp(), GetPort());
				//	SetRemove(true);
				//	break;
				//}
#if _DEBUG_NET_
				Hand<IOCPBaseNet> net = GetNetHandle();
				++net->mRevPacketCount;
#endif
				try{
					OnReceivePacket(packet.getPtr());			
				}
				catch (std::exception &e)
				{
					ERROR_LOG("Net message execute error >%s", e.what());
				}
				catch (...)
				{
					ERROR_LOG("Net message [%d] execute error", packet->GetPacketID());
				}
			}
			else
				break;
		}
//#endif
	}
}

bool IOCPServerConnect::Process()
{
	bool re = IOCPConnect::Process();

	if (!mbReceiveData && re)
	{
		if (TimeManager::Now()-mStartTime>mNet->GetSafeCheckOverTime())
		{
			Log("NET ERROR: [%s:%d] Over time for connect wait safe check or reveice data, then remove", GetIp(), GetPort());
			mbReceiveData = true;
			Close();
		}
	}		

	return re;
}

//-------------------------------------------------------------------------
