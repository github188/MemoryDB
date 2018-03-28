
#include "EventProtocol.h"

#include "TableManager.h"

#include "EventPacket.h"

#include "ResoursePack.h"

#include "NetIndexPacket.h"
#include "EventData.h"
#include "IOCPConnect.h"
#include "TimeManager.h"
//----------------------------------------------------------------------
EventNetProtocol::EventNetProtocol( ) 
	//: mDefaultEventFactory()
	//, mCompressEventFactory()
{
	//mEventPacket = MEM_NEW EventPacket();
	//mCompressPacket = MEM_NEW CompressPacket();
	////mIndexPacket = MEM_NEW NotifyEventIndexPacket();
	////mHeartPacket = MEM_NEW HeartBeatPacket();

	mNetPacketFactoryList.Append(PACKET_EVENT, MEM_NEW DefinePacketFactory<EventPacket, PACKET_EVENT>());
	mNetPacketFactoryList.Append(PACKET_COMPRESS_EVENT, MEM_NEW DefinePacketFactory<CompressPacket, PACKET_COMPRESS_EVENT>());
	mNetPacketFactoryList.Append(PACKET_EVENT_PROCESS, MEM_NEW DefinePacketFactory<EventProcessPacket, PACKET_EVENT_PROCESS>());

	PacketFactory *pFactory = MEM_NEW IndexPacketFactory();
	mNetPacketFactoryList.Append(pFactory->GetPacketID(), pFactory);

	pFactory = MEM_NEW HeartBeatPacketFactory();
	mNetPacketFactoryList.Append(pFactory->GetPacketID(), pFactory);
}

bool EventNetProtocol::RegisterNetPacket(AutoPacketFactory f, bool bReplace)
{
	if (!f)
		return false;
	AutoPacketFactory existFactory = mNetPacketFactoryList.Find(f->GetPacketID());
	if (existFactory)
	{
		if (bReplace)
		{
			NOTE_LOG("WARN: [%d] net packet factory exist, now replace", f->GetPacketID());
		}
		else if (typeid(*existFactory)!=typeid(*f))
		{
			NOTE_LOG("ERROR: Register [%d] fail, net packet factory exist", f->GetPacketID());
			return false;
		}
	}
	mNetPacketFactoryList.Append(f->GetPacketID(), f);
	return true;
}

int EventNetProtocol::AppendNetPacketFrom(tNetProtocol *pOther, bool bReplace)
{
	EventNetProtocol *p = dynamic_cast<EventNetProtocol*>(pOther);
	if (p==NULL)
		return 0;
	int x = 0;
	for (size_t i=0; i<p->mNetPacketFactoryList.size(); ++i)
	{
		if (RegisterNetPacket(p->mNetPacketFactoryList[i], bReplace))
			++x;
	}
	return x;
}

//----------------------------------------------------------------------------------------------

HandPacket EventNetProtocol::CreatePacket( PacketID packetID )
{
	AutoPacketFactory &f = mNetPacketFactoryList.Find(packetID);
	if (f)
		return f->CreatePacket();
	NOTE_LOG("ERROR: [%d] no exist packet factory", packetID);
	return HandPacket();
}

//----------------------------------------------------------------------------------------------

bool EventNetProtocol::WritePacket( const Packet* pPacket, LoopDataStream *mSocketOutputStream )
{
	__ENTER_FUNCTION_FOXNET
		UINT currentTail = mSocketOutputStream->GetTail();
	PacketID_t packetID = pPacket->GetPacketID() ;
	UINT w = mSocketOutputStream->Write( (CHAR*)&packetID , sizeof(PacketID_t) ) ;
	AssertEx( w!=0, "Write packet id fail" ) ;

	UINT packetUINT = 0;

	UINT packetSize = pPacket->GetPacketSize( ) ;
	//UINT packetIndex = pPacket->GetPacketIndex( ) ;
	UINT packetState = pPacket->GetState();
	//if (packetSize<=0)
	//{
	//	OnWritePacketError("Error: packet size is zero, write data fail.");
	//	AssertEx(0, "Error: packet size is zero, write data fail."); 
	//	return false;
	//}

	//printf("Send -->%u\n", packetSize);

	SET_PACKET_INDEX(packetUINT, packetState) ;
	SET_PACKET_LEN(packetUINT, packetSize);

	mSocketOutputStream->Write( (CHAR*)&packetUINT, sizeof(UINT) ) ;
	//Assert( w!=0 ) ;

	UINT currentLen = mSocketOutputStream->Length(); 

	BOOL re = pPacket->Write( *mSocketOutputStream );

	if (re==TRUE && mSocketOutputStream->Length()-currentLen == packetSize)
		return TRUE;
	// lost write data. restore buffer state.
	mSocketOutputStream->_ForceRestoreTail( currentTail );

	AString errorInfo;
	errorInfo.Format("Error: write packet data fail >[%d], size %u", packetID, packetSize);
	AssertEx(0, errorInfo.c_str());
	
	OnWritePacketError(errorInfo);

	return FALSE;

	__LEAVE_FUNCTION_FOXNET

		return FALSE ;
}
//----------------------------------------------------------------------------------------------

HandPacket EventNetProtocol::ReadPacket( tNetConnect *pConnect, LoopDataStream *mSocketInputStream )
{
	__ENTER_FUNCTION_FOXNET

		byte uID = -1;
	if (mSocketInputStream->Peek( (CHAR*)&uID, 1 )==FALSE)
		return NULL;

	static __declspec(thread)	char szBuf[PACKET_HEADER_SIZE];

	// step 1: try get packet head info
	if (mSocketInputStream->Peek( szBuf, PACKET_HEADER_SIZE )==FALSE)
		return NULL;

	UINT *packetUINT = (UINT *)(szBuf + sizeof(PacketID_t));
	UINT packetSize = GET_PACKET_LEN(*packetUINT);
	byte packetStateData = (byte)GET_PACKET_INDEX(*packetUINT);

	//PacketID_t *packetID = (PacketID_t *)szBuf;
	if (!CheckPacketInfo(pConnect, uID, packetSize))
	{
		return NULL;
	}
	// step 2: check data length over need packet size.
	if ( mSocketInputStream->Length() < (PACKET_HEADER_SIZE + packetSize) )
	{
		return NULL;
	}

	// step 3 : now ok, must had a packet
	//PacketID_t *packetID = (PacketID_t *)szBuf;

	// step 4: move to packet data pos
	mSocketInputStream->Skip(PACKET_HEADER_SIZE);

	// step 5: create packet
	UINT currentHead = mSocketInputStream->GetHead();
	UINT currentLen = mSocketInputStream->Length();

	HandPacket p;
	if (uID==PACKET_EVENT && (packetStateData & ePacketStateEncrypt)==0)
	{
		//NOTE: 这里直接将一般事件包，转换成直读处理的消息包处理	
		uID = PACKET_EVENT_PROCESS;
		Auto<EventProcessPacket> packet = CreatePacket(uID);
		if (packet)
		{			
			if (packet->ReadEvent(pConnect, *mSocketInputStream, packetSize))
			{
				p = packet;
				p->SetState(packetStateData);
			}
		}
	}
	else
	{
		p = CreatePacket(uID); 
		if (!p)
		{
			// 考虑网络安全, 一般需要作断开连接处理, 且记录黑名单
			//AssertEx(0, "Error : no exist packet");
			AString errorInfo = "Error : no exist packet >>>";
			errorInfo += (int)uID;

			OnReadPacketError(errorInfo);

			mSocketInputStream->Skip(packetSize);

			AssertEx(0, errorInfo.c_str());

			return NULL;
		}
		// 根据状态确定是否已经是加密消息
		p->SetState(packetStateData);

		// step 5: read packet data
		p->Read(*mSocketInputStream, packetSize);
	}

	if (currentLen-mSocketInputStream->Length()!=packetSize)
	{
		// 程序代码错误
		//AssertEx(0, "Error : read packet fail, size is not right");
		AString errorInfo = "Error : code logic bug >>> read packet fail, size is not right >>>";
		errorInfo += (int)uID;

		OnReadPacketError(errorInfo);

		mSocketInputStream->_ForceRestoreHead(currentHead);
		mSocketInputStream->Skip(packetSize);		


		//#ifdef __WINDOWS__
		//		throw std::exception(errorInfo.c_str());
		//#else
		//        throw (1);
		//#endif
		return NULL;
	}
	//printf("Receive-->%u\n", packetSize);
	return p;

	__LEAVE_FUNCTION_FOXNET

		return NULL;
}

void EventNetProtocol::OnReadPacketError( AString strErrorInfo )
{
	ERROR_LOG(strErrorInfo.c_str());
}

void EventNetProtocol::OnBeforeSendEvent(tNetConnect *pConnect, Logic::tEvent *pEvent)
{
    AutoEventFactory f = pEvent->GetEventFactory();
    if (UseNiceDataProtocol() && !f->HasState(eFactoryNoProtocolSave))
    {
        if (f->GetID()>0 && pEvent->GetData().getType()==NICEDATA && pEvent->GetData().count()>0 )
        {
            pEvent->setState(STATE_EVENT_USE_PROTOCOL_SAVE, true);
            int key = f->GetNiceDataProtocolKey();
            pEvent->setState(STATE_EVENT_NEED_SAVE_PROTOCOL, key==0 || key!=pConnect->mInfoData.Find(f->GetID()));
        }
    }
}

void EventNetProtocol::OnWritePacketError( AString strErrorInfo )
{
	ERROR_LOG(strErrorInfo.c_str());
}

HandPacket EventNetProtocol::GenerateEventPacket(tNetConnect *pConnect,  Logic::tEvent *sendEvent, bool bNeedZip )
{
	Auto<EventPacket> sendPacket = CreatePacket(PACKET_EVENT);
	AssertEx(sendPacket, "消息包获取失败");
    OnBeforeSendEvent(pConnect, sendEvent);
	int eventDataSize = sendPacket->SetEvent(sendEvent->GetSelf());
	if (eventDataSize<=0)
	{
		ERROR_LOG("[%s]事件数据流化失败", sendEvent->GetEventName());
		return NULL;
	}
    if (sendEvent->hasState(STATE_EVENT_NEED_SAVE_PROTOCOL))
        pConnect->mInfoData.Append(sendEvent->GetEventFactory()->GetID(), sendEvent->GetEventFactory()->GetNiceDataProtocolKey());
	//??? 使用自动压缩后，ZIP压缩耗时较大
	if (bNeedZip)
	{
#if DEVELOP_MODE
		int scrSize = 0;
		UInt64 now = 0;
#endif
		// 实际测试, 1KB左右,压缩为几百个字节,压缩比很小,效率反而较低, 约4~5K,压缩为1~2K, 压缩比较高, 所以只有超过2K才进行压缩
		if (sendEvent->NeedAutoCompress() && eventDataSize>=MIN_NEED_COMPRESS_SIZE)
		{
			Auto<EventPacket> p2 = CreatePacket(PACKET_COMPRESS_EVENT);

			p2->SwapEventData(sendPacket->GetData());
			sendPacket = p2;
#if DEVELOP_MODE
			//sendEvent->Log("自动调整为 [压缩] 发送 >>> [%d]", eventDataSize);
			scrSize = eventDataSize;
			now = TimeManager::NowTick();
#endif
		}					

		if (!sendPacket->ReadySendData())
		{
			sendEvent->Log("消息包准备发送数据失败, 压缩数据失败.");
			ERROR_LOG("消息包准备发送数据失败, 压缩数据失败");
			return NULL;
		}
#if DEVELOP_MODE
		if (scrSize>0)
			NOTE_LOG("Zip [%d]~[%u], use time [%llu]", scrSize, sendPacket->GetPacketSize(), TimeManager::NowTick()-now);
#endif
	}
	return sendPacket;
}

// 根据MSG消息数据定义表格, 获取或创建消息中的数据记录
//AutoRecord EventNetProtocol::GetStructRecord( AutoEvent evt, const AString &structDefineIndex, AutoEvent parentEvent, int nIndex, bool bCreate /*= true */ )
//{
//	AutoTable hTable;
//	if (evt->get(structDefineIndex.c_str(), hTable) && hTable)
//	{
//		return hTable->CreateRecord(nIndex, false);
//	}
//
//	AString tableIndex;
//	if (parentEvent)
//	{
//		tableIndex = evt->GetEventName();
//		tableIndex += ".";
//	}
//	tableIndex += evt->GetEventName();
//
//	AutoTable msgDefineTable = TableManager::getSingleton().GetTable(tableIndex.c_str());
//
//	for (TableIt  tIt(msgDefineTable); tIt._Have(); tIt._End())
//	{
//		AutoRecord hRecond = tIt.getCurrRecord();
//		Data name = hRecond->get(VALUENAME_COL);
//		Data type = hRecond->get(DEFAULT_COL);
//		if (name.mFieldInfo != NULL
//			&& structDefineIndex == name.mFieldInfo->getTypeString()				
//			)
//		{
//			if ( strcmp(type.c_str(), TABLE_TYPE_STRING_FLAT)==0 )
//			{
//				hTable = TableManager::getSingleton().CreateNewObject("NICE");
//				evt->set(structDefineIndex.c_str(), hTable);
//			}
//			else if (hTable)
//			{
//
//			}
//		}
//	}
//
//	return AutoRecord();
//}


bool EventNetProtocol::CheckPacketInfo( tNetConnect *pConnect, PacketID_t packetID, UINT packetSize )
{
	if (packetID<=0 || packetID>=mNetPacketFactoryList.size())
	{
		NOTE_LOG("WARN: [%s : %d] Read packet data error, packet id != [%d] and id >= [%d], then now set remove", 
			pConnect->GetIp(), pConnect->GetPort(), PACKET_EVENT, (int)mNetPacketFactoryList.size());

		pConnect->SetRemove(true);
		return false;
	}

	if (packetSize>1280000)
	{
		NOTE_LOG("WARN: [%s : %d] Read packet data error, more 1280000, or packet id > [%d], then now set remove", pConnect->GetIp(), pConnect->GetPort(), packetID);
		pConnect->SetRemove(true);
		return false;
	}
	return true;
}


bool EventNetProtocol::SendEvent( tNetConnect *connect, Logic::tEvent *sendEvent )
{
	AssertEx(connect!=NULL, "发送事件时, 连接为NULL");
	
	tNetHandle * net = connect->GetNetHandle();
	AssertEx(net!=NULL, "NetHandle is NULL of connect");

	EventNetProtocol  *pEventProtocol = dynamic_cast<EventNetProtocol*>(net->GetNetProtocol());
	AssertEx(pEventProtocol!=NULL, "发送的事件为空");
    
	Auto<EventPacket> pPacket = pEventProtocol->GenerateEventPacket(connect, sendEvent, net->NeedZipSendPacket());
	if (!pPacket)
	{
		ERROR_LOG("生成消息包失败>[%s]", sendEvent->GetEventName());
		return false;
	}

	bool bEncrypt = net->NeedEncryptPacket() && sendEvent->GetEventFactory()->HasState(ePacketStateEncrypt);

	if ( connect->Send(pPacket.getPtr(), bEncrypt) )
	{
		connect->OnSucceedSendEvent(sendEvent, pPacket.getPtr());		
		return true;
	}
	else
	{
		sendEvent->Log("发送失败 X");
	}
	return false;
}

void EventNetProtocol::ReadAnalyzeEventData(tNetConnect *pConnect, LoopDataStream *pReceiveData )
{
	__ENTER_FUNCTION_FOXNET
#if _IOCP_THREAD_RECEIVE_SEND
		byte uID = -1;
	if (pReceiveData->Peek( (CHAR*)&uID, 1 )==FALSE)
		return;

	static __declspec(thread)	char szBuf[PACKET_HEADER_SIZE];

	// step 1: try get packet head info
	if (pReceiveData->Peek( szBuf, PACKET_HEADER_SIZE )==FALSE)
		return;

	UINT *packetUINT = (UINT *)(szBuf + sizeof(PacketID_t));
	UINT packetSize = GET_PACKET_LEN(*packetUINT);
	byte packetStateData = (byte)GET_PACKET_INDEX(*packetUINT);

	//PacketID_t *packetID = (PacketID_t *)szBuf;
	//if (!CheckPacketInfo(pConnect, uID, packetSize))
	//{
	//	return NULL;
	//}
	// step 2: check data length over need packet size.
	if ( pReceiveData->Length() < (PACKET_HEADER_SIZE + packetSize) )
	{
		return;
	}

	// step 3 : now ok, must had a packet
	//PacketID_t *packetID = (PacketID_t *)szBuf;

	// step 4: move to packet data pos
	pReceiveData->Skip(PACKET_HEADER_SIZE);

	// step 5: create packet

	//p->SetState(packetStateData);

	// step 5: read packet data
	UINT currentHead = pReceiveData->GetHead();
	UINT currentLen = pReceiveData->Length();

	EventData *pData = NULL;
	PacketData *pPacketData = NULL;

	if ((packetStateData&eEventDataPacket)==0)
	{
		pPacketData = PacketDataPool::Pop();
		if (pPacketData->mData.size()<(int)packetSize)
			pPacketData->mData.InitSize(packetSize);
		
		UINT readSize = pReceiveData->Read(pPacketData->mData.GetBuffer(), packetSize);
		if (readSize==packetSize)
		{
			pPacketData->mPacketID = uID;
			pPacketData->mState = packetStateData;
			pPacketData->mData._ForceRestoreTail(packetSize);
		}
		else
		{
			PacketDataPool::Push(pPacketData);
			pPacketData = NULL;
		}
	}
	else
	{	
		DataStream *pRevData =pReceiveData;
		pData = EventDataPool::Pop();
		bool bOk = false;
		int eventTypeIndex = 0;
		if (pRevData->read(eventTypeIndex))
		{
			ushort id = 0;
			if (pRevData->read(id))
			{		
				byte niceType = 0;
				if (pRevData->read(niceType))		
				{					
					pRevData->read(pData->mExtValue);
					int target = 0;
					if (pRevData->read(target))
						if (pData->mData.restore(pRevData))
						{
							//if (currentLen-pData->Length()==packetSize)
							{					
								pData->_setTarget(target);
								pData->setEventIndex(eventTypeIndex);
								bOk = true;
							}
						}
				}
			}
		}
		if (!bOk)
		{
			EventDataPool::Push(pData);
			pData = NULL;
		}
	}

	if (currentLen-pReceiveData->Length()!=packetSize)
	{
		if (pData!=NULL)
			EventDataPool::Push(pData);
		if (pPacketData!=NULL)
			PacketDataPool::Push(pPacketData);
		// 程序代码错误
		//AssertEx(0, "Error : read packet fail, size is not right");
		AString errorInfo = "Error : code logic bug >>> read packet fail, size is not right >>>";
		errorInfo += (int)uID;

		ERROR_LOG(errorInfo.c_str());

		pReceiveData->_ForceRestoreHead(currentHead);
		pReceiveData->Skip(packetSize);		

		return;
	}

	if (pData!=NULL || pPacketData!=NULL)
	{
		IOCPConnect *pConn = dynamic_cast<IOCPConnect*>(pConnect);
		AssertEx(pConn, "必须是IOCPConnect");
		pConn->mRevLock.lock();
		pConn->mReceiveDataList.push_back(IOCPConnect::MsgData(pData, pPacketData));
		pConn->mbHaveReceiveEventMsg = true;
		pConn->mRevLock.unlock();
	}

#endif
	return;

	__LEAVE_FUNCTION_FOXNET

		return;
}



void EventNetProtocol::ProcessEventData(tNetConnect *pConnect, EventData *pData)
{
	Hand<tNetEvent> evt = pConnect->GetNetHandle()->GetEventCenter()->StartEvent(pData->getEventIndex());
	if (evt)
	{
		Hand<tNetEventFactory> f = evt->GetEventFactory();
		if (!f)
		{
			ERROR_LOG("[%s] 必须使用 tNetEventFactory 注册事件", evt->GetEventName());
			return;
		}
		f->SetEventData(pData, evt.getPtr());
		evt->_OnBindNet(pConnect);
		try{
			evt->DoEvent(true);
		}
		catch (...)
		{
			ERROR_LOG("[%d] 事件执行异常", evt->GetEventName());
		}
	}
	else
		ERROR_LOG("No exist net msg evet [%d], or no register  tNetEvent type", pData->getEventIndex());
}

void EventNetProtocol::ProcessPacketData(tNetConnect *pConnect, PacketData *pData)
{
	EventNetProtocol *p = dynamic_cast<EventNetProtocol*>(pConnect->GetNetHandle()->GetNetProtocol());
	AssertEx(p!=NULL, "必须是 EventNetProtocol");
	HandPacket packet = p->CreatePacket(pData->mPacketID);
	packet->SetState(pData->mState);
	pData->mData.seek(0);

	if (packet->Read(pData->mData, pData->mData.dataSize()))
		packet->Execute(pConnect);
}
