#include "UDPByTcpNet.h"
#include "TimeManager.h"
#include "ServerIPInfo.h"

void UDPByTcpConnect::RegisterMsgPacket(tNetProtocol *pPro)
{
	pPro->RegisterNetPacket(MEM_NEW DefinePacketFactory<UDP_RequestResendPart, UDP_REQUEST_RESEND_PACKET>());
	pPro->RegisterNetPacket(MEM_NEW DefinePacketFactory<TCP_NotifyUDPCheckKeyMsg, TCP_NOTIFY_CHECK_KEY>());
	pPro->RegisterNetPacket(MEM_NEW DefinePacketFactory<TCP_NotifyUDPReadyOkMsg, TCP_NOTIFY_READY_OK>());
	pPro->RegisterNetPacket(MEM_NEW DefinePacketFactory<UDPPartPacket, UDP_PART_PACKET>());
}

void UDPByTcpConnect::StartSendUDPCheck(UInt64 checkKey)
{
	// 间隔2秒发送一次, 直到接收到准备OK
	Hand<TM_WaitSendUDPCheckEvent> evt = GetNetHandle()->GetEventCenter()->StartEvent("TM_WaitSendUDPCheckEvent");
	if (!evt)
	{
		GetNetHandle()->GetEventCenter()->RegisterEvent("TM_WaitSendUDPCheckEvent", MEM_NEW Logic::EventFactory<TM_WaitSendUDPCheckEvent>());
		evt = GetNetHandle()->GetEventCenter()->StartEvent("TM_WaitSendUDPCheckEvent");
	}
	mWaitResponeOkEvent = evt;
	evt->mpConnect = this;
	evt->mCheckKey = checkKey;
	evt->DoEvent();
}

void UDPByTcpConnect::_sendAllData()
{
	UInt64 nowTick = TimeManager::NowTick();
	while (mUDPSendData.IsEmpty()==FALSE)
	{		
		Auto<DataPacket> partPacket = _createPartPacket();
		AutoData partData = partPacket->GetData();
		partData->clear(false);
		partData->write((byte)UDP_PART_PACKET);
		partData->write(++mSendCode);
		partData->write((uint)TimeManager::NowTick());
		int size = mUDPSendData.dataSize();
		if (size>MAX_UDP_MSG_SIZE)
		{
			size = MAX_UDP_MSG_SIZE;
		}
		partData->write((ushort)size);
		mUDPSendData.Read(partData->data()+partData->tell(), size);
		partData->setDataSize(partData->tell()+size);
		partData->seek(partData->tell()+size);
		//NOTE_LOG("++ UDP send part >%u", mSendCode);
		//!!! 测试丢包 if (rand()%6!=1)
		_sendByUDP(partPacket);
		SendPartData d;
		d.mSendTime = nowTick;
		d.mPartPacket = partPacket;
		d.mCode = mSendCode;
		mSendPacketList.push_back(d);
	}

}

void UDPByTcpConnect::OnReceiveUDPData(DataBuffer *receiveData, int nSize, bool bUdp)
{
	tNetConnect *pConnect = this;
	receiveData->setDataSize(nSize);
	receiveData->seek(0);
	byte msgType = 0;
	bool bHaveMsg = false;
	while (true)
	{
		if (!receiveData->read(msgType))
			break;

		switch (msgType)
		{
		case UDP_PART_PACKET:
			{
				uint code = 0;
				uint time = 0;
				if (!receiveData->read(code))
					goto _ErrorRead;

				//if (code!=mReceiveCode+1)
				//	_requestResend(mReceiveCode+1);

				if (!receiveData->read(time))
					goto _ErrorRead;
				ushort partSize = 0;
				if (!receiveData->read(partSize))
					goto _ErrorRead;
				if (partSize>MAX_UDP_MSG_SIZE)
				{
					ERROR_LOG("%u 包数据大小错误 >%d", code, (int)partSize);
					goto _ErrorRead;
				}
			
				if (receiveData->lastDataSize()<partSize)
					goto _ErrorRead;

				if (code==mReceiveCode+1)
				{
					mUDPReceiveData._write(receiveData->data()+receiveData->tell(), partSize);
					receiveData->seek(receiveData->tell()+partSize);
					mReceiveCode = code;
					bHaveMsg = true;
				}
				else if (code>mReceiveCode)
				{	// 缓存
					Auto<DataPacket> pak = _createPartPacket();
					if (pak->GetData()->size()<partSize)
						pak->GetData()->resize(partSize);
					receiveData->_read(pak->GetData()->data(), partSize);
					pak->GetData()->setDataSize(partSize);

					if (mReceivePartDataList.exist(code))
					{
						ERROR_LOG("严重错误, 已经存在分包 >%d", (int)code);
						mReceivePartDataList.erase(code);
					}
					RevPartData d;
					d.mPartPacket = pak;
					mReceivePartDataList.insert(code, d);
				}
				else
				{
					// 如果是不需要的包,直接丢弃数据
					receiveData->seek(receiveData->tell()+partSize);
				}

			}
			break;

		case UDP_REQUEST_RESEND_PACKET:
			{
				uint code = 0;
				if (!receiveData->read(code))
					goto _ErrorRead;

				HandPacket pak =  _findSendPacket(code);
				if (pak)
				{
					//NOTE_LOG(" == UDP send part >%u", code);
					_sendByUDP(pak);
				}
			}
			break;

		case UDP_RESPONSE_PACKET:
			{
                //NOTE_LOG("*** Rev udp UDP_RESPONSE_PACKET");
				uint code = 0;
				if (!receiveData->read(code))
					goto _ErrorRead;						

				while (!mSendPacketList.empty())
				{
					auto it = mSendPacketList.begin();
					if ((*it).mCode<=code)
					{
						mSendPacketList.erase(it);
					}
					else
						break;
				}

				byte nCount = 0;
				if (!receiveData->read(nCount))
					goto _ErrorRead;			

				for (int i=0; i<nCount; ++i)
				{
					uint nCode = 0;
					if (!receiveData->read(nCode))
						goto _ErrorRead;

					for (auto it=mSendPacketList.begin(); it!=mSendPacketList.end(); )
					{
						if ((*it).mCode==nCode)
						{
							mSendPacketList.erase(it);
							break;
						}
						else
							++it;
					}
				}
			
			}
			break;

		default:
			goto _ErrorRead;
		}
	}

_ErrorRead:
	// 将当前接收到的序号发回
	if (bHaveMsg)
	{	
		_CheckProcessWaitReceivePartList();
		_ProcessUDPReceive();

		while (true)
		{
			Auto<DataPacket> pak = _createPartPacket();
			pak->GetData()->clear(false);
			pak->GetData()->write((byte)UDP_RESPONSE_PACKET);
			pak->GetData()->write(mReceiveCode);
			
			Array<uint> responseList;
			bool bHave = false;
			for (int i=0; i<mReceivePartDataList.size(); ++i)
			{			
				auto &v = mReceivePartDataList.getValue(i);
				if (!v.mVal.mbResponse)
				{
					v.mVal.mbResponse = true;
					responseList.push_back(v.mKey);
					if (mReceivePartDataList.size()>=200)
					{
						bHave = i<mReceivePartDataList.size()-1;
						break;
					}
				}
			}
			pak->GetData()->write((byte)responseList.size());
			for (int i=0; i<responseList.size(); ++i)
			{
				pak->GetData()->write(responseList[i]);
			}
			_sendByUDP(pak);

			if (!bHave)
				break;
		}

		_checkResend();
	}
}

void UDPByTcpConnect::_CheckProcessWaitReceivePartList()
{
	//for (int i=0; i<mReceivePartDataList.size(); ++i)
	while (!mReceivePartDataList.empty())
	{
		if (mReceivePartDataList.getKey(0)==mReceiveCode+1)
		{
			Auto<DataPacket> partPart = mReceivePartDataList.get(0).mPartPacket;
			AutoData d = partPart->GetData();
			mUDPReceiveData._write(d->data(), d->dataSize());
			mReceivePartDataList._remove(0);
			++mReceiveCode;
		}
		else
			break;
	}
}

void UDPByTcpConnect::_ProcessUDPReceive()
{
	tNetConnect *pConnect = this;
	tNetHandle *pNet = GetNetHandle();
	int count = 0;
	while (count++<_ONCE_EXECUTE_MSG_COUNT) 
	{
		HandPacket packet = pNet->GetNetProtocol()->ReadPacket(pConnect, &mUDPReceiveData);
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
				packet->Execute(pConnect);
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
}

void UDPByTcpConnect::_requestResend( uint nPartCode )
{
	if (mWaitResendList.exist(nPartCode))
		return;

	Auto<DataPacket> pak = GetNetHandle()->GetNetProtocol()->CreatePacket(UDP_REQUEST_RESEND_PACKET);
	pak->GetData()->clear(false);
	pak->GetData()->write((byte)UDP_REQUEST_RESEND_PACKET);
	pak->GetData()->write(nPartCode);

	_sendByUDP(pak);

	// 请求TCP, 使用请求事件方式, 如果未回复, 分析断线
	Hand<TM_WaitTcpResponseResendPart> evt = GetNetHandle()->GetEventCenter()->StartEvent("TM_WaitTcpResponseResendPart");
	if (!evt)
	{
		GetNetHandle()->GetEventCenter()->RegisterEvent("TM_WaitTcpResponseResendPart", MEM_NEW Logic::EventFactory<TM_WaitTcpResponseResendPart>());
		evt = GetNetHandle()->GetEventCenter()->StartEvent("TM_WaitTcpResponseResendPart");
	}

	evt->mConnect = this;
	evt->mWaitCode = nPartCode;
	mWaitResendList.insert(nPartCode, evt);
	evt->WaitTime(10);

	bool b = _sendByTcp(pak);
	//NOTE_LOG("Tcp send request >%u %s", nPartCode, b?"OK":"FAIL");
}

void UDPByTcpConnect::_udpConnectProcess()
{
	if (!mSendPacketList.empty())
	{		
		UInt64 nowTick = TimeManager::NowTick();
		for (auto it=mSendPacketList.begin(); it!=mSendPacketList.end(); ++it)
		{
			SendPartData &d = *it;
			if (nowTick-d.mSendTime>_WAIT_RESEND_TIME)
			{
				if (!d.mbResend)
				{
					d.mbResend = true;
					_sendByUDP(d.mPartPacket);
					_sendByTcp(d.mPartPacket);
					NOTE_LOG("Over time resend >%u", d.mCode);
				}
			}
			else
				break;
		}
	}
}

UINT UDP_RequestResendPart::Execute( tNetConnect* pConnect )
{
	byte type = 0;
	uint nCode = 0;
	mData->seek(0);
	mData->read(type);
	if (!mData->read(nCode))
	{
		ERROR_LOG("UDP_RequestResendPart read code fail");
		return 0;
	}
	Hand<UDPByTcpConnect> conn = pConnect;
	Auto<DataPacket> pak = conn->_findSendPacket(nCode);
	if (pak)
	{
		Auto<DataPacket> p = pak;
		bool b = conn->_sendByTcp(p);

		byte partType = 0;
		uint partCode = 0;
		p->GetData()->seek(0);
		p->GetData()->read(partType);
		p->GetData()->read(partCode);

		//NOTE_LOG("Exe Send -> Tcp resend part >%u = %d, type %d, part code >%u", nCode, p->GetData()->dataSize(), (int)partType, partCode);
	}
	else
	{
		Auto<DataPacket> d = conn->_createPartPacket();
		d->GetData()->clear(false);
		d->GetData()->write((byte)TCP_RESPONSE_EMPTY_PART);
		d->GetData()->write(nCode);
		conn->_sendByTcp(d);
		//ERROR_LOG("Part Data not exist >%u", nCode);
		//NOTE_LOG("Exe Send Empty response -> Tcp resend part >%u", nCode);
	}
	return 0;
}

UINT TCP_NotifyUDPReadyOkMsg::Execute(tNetConnect* pConnect)
{
	Hand<UDPByTcpConnect> conn = pConnect;
	conn->SetUDPReadyOk(true);
	NOTE_LOG("接收到回复 %s 准备OK", ServerIPInfo::GetAddrInfo(conn->mTargetAddr).c_str());
	return 0;
}

UINT UDPPartPacket::Execute( tNetConnect* pConnect )
{
	byte type = 0;
	uint nCode = 0;
	mData->seek(0);
	mData->read(type);
	if (!mData->read(nCode))
		return 0;

	Hand<UDPByTcpConnect> conn = pConnect;
	//NOTE_LOG("Rev Tcp response resend part >%u, Now receive last code >%u, type %d", nCode, conn->mReceiveCode, (int)type);
	if (type==UDP_PART_PACKET)
	{
		conn->OnReceiveUDPData((DataBuffer*)mData.getPtr(), mData->dataSize(), false);
	}
	
	if (type==UDP_PART_PACKET || type==TCP_RESPONSE_EMPTY_PART)
	{
		AutoEvent evt = conn->mWaitResendList.find(nCode);
		evt._free();
		conn->mWaitResendList.erase(nCode);
	}

	return 0;
}
