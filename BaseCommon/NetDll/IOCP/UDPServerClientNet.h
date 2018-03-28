#ifndef _INCLUDE_UDPSERVERCLIENTNET_H_
#define _INCLUDE_UDPSERVERCLIENTNET_H_

#include "UDPByTcpNet.h"
#include "IOCPServerNet.h"
#include "IOCPUDPNet.h"
//-------------------------------------------------------------------------*/
// UDP 服务端
class Net_Export UDPByTcpServerNet : public IOCPServerNet
{
public:
	virtual void OnReceiveUdpMsg(UInt64 scrAddr, DataStream *receiveData, int revSize);

	virtual bool sendByUDP(HandPacket pak, UInt64 targetAddr)
	{
		Auto<DataPacket> p = pak;
		return mUDPNet->Send(targetAddr, p->GetData()->data(), p->GetData()->dataSize());
	}

	virtual bool StartReadyUDPConnect(UInt64 checkKey, UInt64 scrAddr);

	virtual void Process() override
	{
		mUDPNet->Process();
		IOCPServerNet::Process();
	}

	virtual bool StartNet( const char *szIP, int nPort )
	{
		bool b = IOCPServerNet::StartNet(szIP, nPort);
		mUDPNet->Start(szIP, nPort);
		return b;
	}

public:
	virtual HandConnect CreateConnect() override;
	virtual bool OnAddConnect(tNetConnect *pConnect);

public:
	UDPByTcpServerNet();
	~UDPByTcpServerNet()
	{
		mUDPNet._free();
	}

public:
	Hand<IOCPUDPNet>	mUDPNet;
	DefinePacketFactory<UDPPartPacket, UDP_PART_PACKET>		mDataPacketFactory;	
	EasyMap<UInt64, Hand<UDPByTcpConnect>>		mWaitUDPResponseList;
	EasyHash<UInt64, Hand<UDPByTcpConnect>>		mUDPConnectList;
};

class _UDPForTcpNet : public IOCPUDPNet
{
public:
	virtual void OnReceive(UInt64 scrAddr, const char *revData, int revSize) override
	{
		//TempDataBuffer t((void*)revData, revSize);
        AutoData d = MEM_NEW DataBuffer(revSize);
        d->_write((void*)revData, revSize);       
		mpOwnerNet->OnReceiveUdpMsg(scrAddr, d.getPtr(), revSize);
	}

public:
	_UDPForTcpNet(UDPByTcpServerNet *p)
		: mpOwnerNet(p){}

protected:
	UDPByTcpServerNet	*mpOwnerNet;
};
//-------------------------------------------------------------------------*/
class Net_Export UDPByTcpServerConnect : public UDPByTcpConnect
{
	friend class UDPByTcpServerNet;
public:
	void StartSendUDPReadyReqest(UInt64 key)
	{
		Auto<DataPacket> p = GetNetHandle()->GetNetProtocol()->CreatePacket(TCP_NOTIFY_CHECK_KEY);
		if (!p)
		{
			UDPByTcpConnect::RegisterMsgPacket(GetNetHandle()->GetNetProtocol());
			p = GetNetHandle()->GetNetProtocol()->CreatePacket(TCP_NOTIFY_CHECK_KEY);
		}
		p->GetData()->seek(0);
		p->GetData()->write(key);
		_sendByTcp(p);
		mWaitResponseCheckKey = GetNetHandle()->GetEventCenter()->WaitTime(10, EventCallBack(&UDPByTcpServerConnect::OnWaitResponseCheckKeyOver, this));
	}

	bool OnWaitResponseCheckKeyOver(Logic::tEvent *)
	{
		ERROR_LOG("WAIT response check key over time");
		return true;
	}

	void SendFirstReadyOkMsg();

public:
	virtual bool _sendByTcp(HandPacket dataPacket){ return IOCPServerConnect::Send(dataPacket.getPtr(), false); }

public:
	virtual HandPacket _createPartPacket()
	{
		Hand<UDPByTcpServerNet> net = GetNetHandle();
		return net->mDataPacketFactory.CreatePacket();
	}
	virtual bool _sendByUDP(HandPacket dataPacket)
	{
		Hand<UDPByTcpServerNet> net = GetNetHandle();
		return net->sendByUDP(dataPacket, mTargetAddr);
	}

protected:
	AutoEvent mWaitResponseCheckKey;
};
//-------------------------------------------------------------------------*/
// UDP客户端部分
//-------------------------------------------------------------------------*/
class Net_Export UDPByTcpClientConnect : public UDPByTcpConnect
{
	friend class _UDPForClientNet;
public:
	virtual HandPacket _createPartPacket()
	{
		return mDataPacketFactory.CreatePacket();
	}

	virtual bool _sendByUDP(HandPacket dataPacket)
	{
		Auto<DataPacket> p = dataPacket;
		return mUDPNet->Send(mTargetAddr, p->GetData()->data(), p->GetData()->dataSize());
		//Hand<UDPByTcpServerNet> net = GetNetHandle();
		//return net->sendByUDP(dataPacket, mTargetAddr);
	}

	virtual bool _sendByTcp(HandPacket dataPacket){ return IOCPConnect::Send(dataPacket.getPtr(), false); }

public:
	virtual void OnConnected() override;
	virtual void OnDisconnect()
	{
		UDPByTcpConnect::OnDisconnect();
		mUDPNet->Close();
	}
	virtual void _ProcessReceiveData() override
	{
		IOCPConnect::_ProcessReceiveData();
	}
	virtual bool Process() override
	{
		mUDPNet->Process();
		bool b = IOCPConnect::Process();
		_udpConnectProcess();
		return b;
	}

public:
	UDPByTcpClientConnect();
	~UDPByTcpClientConnect()
	{
		mUDPNet._free();
	}

public:
	DefinePacketFactory<UDPPartPacket, UDP_PART_PACKET>		mDataPacketFactory;	
	Hand<IOCPUDPNet>	mUDPNet;
};
class _UDPForClientNet : public IOCPUDPNet
{
public:
	virtual void OnReceive(UInt64 scrAddr, const char *revData, int revSize) override
	{
		if (scrAddr==mpOwnerNet->mTargetAddr)
		{
            AutoData d = MEM_NEW DataBuffer(revSize);
            d->_write((void*)revData, revSize);  
			//TempDataBuffer t((void*)revData, revSize);
			mpOwnerNet->OnReceiveUDPData((DataBuffer*)d.getPtr(), revSize, true);
		}
	}

public:
	_UDPForClientNet(UDPByTcpClientConnect *p)
		: mpOwnerNet(p){}

protected:
	UDPByTcpClientConnect	*mpOwnerNet;
};
//-------------------------------------------------------------------------*/
// 客户端
//-------------------------------------------------------------------------*/
class Net_Export UDPByTcpClient : public IOCPClientNet
{
public:
	virtual HandConnect CreateConnect() override { return MEM_NEW UDPByTcpClientConnect(); }

	UDPByTcpClient()
	{
		UDPByTcpConnect::RegisterMsgPacket(GetNetProtocol());
	}

};
//-------------------------------------------------------------------------*/
#endif //_INCLUDE_UDPSERVERCLIENTNET_H_