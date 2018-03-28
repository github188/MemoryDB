#include "UDPbroadcast.h"
#include "ServerIPInfo.h"

#define _DEBUG_SEND_SELF_	0

//-------------------------------------------------------------------------
void UDPBroadcast::OnReceiveMsg( UInt64 scrAddr, DataStream *revData, int revSize )
{
	if (revSize==8)
	{		
		UInt64 checkCode = 0;
		if (revData->read(checkCode))
		{		
			//auto it = mWaitEventList.find(checkCode);
			AutoEvent existEvt = mWaitEventList.find(checkCode);
			if (existEvt) //(it!=mWaitEventList.end())
			{
				Hand<CS_NotityConnectUDP> evt = existEvt; // it->second;
				if (evt)
				{
					if (evt->OnUDPResponce(scrAddr, checkCode))
					{
						_AppendConnect(scrAddr);
						OnAppendTarget(scrAddr, evt->mNetConnect, evt);
					}
				}
				mWaitEventList.erase(checkCode);
			}
		}
	}
}

void UDPBroadcast::_AppendConnect( UInt64 newAddr )
{
	mListLock.lock();

	WaitConnect conn;
	conn.mbAppend = true;
	conn.mTargetAddr = newAddr;

	mWaitConnectList.push_back(conn);
	mbHaveWaitConnect = true;
	mListLock.unlock();
}

void UDPBroadcast::_RemoveConnect( UInt64 targetAdd )
{
	mListLock.lock();

	WaitConnect conn;
	conn.mbAppend = false;
	conn.mTargetAddr = targetAdd;

	mWaitConnectList.push_back(conn);
	mbHaveWaitConnect = true;
	mListLock.unlock();
}

UDPBroadcast::UDPBroadcast() 
	: mBroadcastReceiveBuffer(2048)
	, mCheckPort(0)
	, mBroadcastPort(0)
	, mRevBroadcastAddr(0)
	, mbHaveWaitConnect(false)
	, mbHaveSourceAddrChanged(false)
	, mDataSourceAddr(0)
	, mSetSourceAddr(0)
{
	//mEventCenter = MEM_NEW BoroadcastEventCenter(this);
	//RegisterNetEvent();

	mpBroadcastUDPReceiveThread = new UDPReceiveThread(this);
}

UDPBroadcast::~UDPBroadcast()
{
	Close();
	delete mpBroadcastUDPReceiveThread;
	mpBroadcastUDPReceiveThread = NULL;
}

void UDPBroadcast::_ProcessNet()
{
	if (mbHaveWaitConnect)
	{
		mListLock.lock();

		while (!mWaitConnectList.empty())
		{
			WaitConnect conn;
			if (mWaitConnectList.pop_front(conn))
			{
				if (conn.mbAppend)
					mTargetAddrList.push_front(conn.mTargetAddr);
				else
				{
					for (PoolList<UInt64>::iterator it = mTargetAddrList.begin(); it!=mTargetAddrList.end();)
					{
						if (*it==conn.mTargetAddr)
						{
							it = mTargetAddrList.erase(it);								
						}
						else
							++it;
					}
				}
			}
		}

		mbHaveWaitConnect = false;
		mListLock.unlock();
	}
	short revLen = (short)mBroadcastUDP.ProcessReceive(&mBroadcastReceiveBuffer, mRevAddr, true);
	if (revLen>0)
	{
		printf("Rev broadcast >[%d]\n", revLen);
		if (revLen<=8)
		{
			// 可能是探测端口数据
			return;
		}
		if (mbHaveSourceAddrChanged)
		{
			mSourceAddLock.lock();
			mDataSourceAddr = mSetSourceAddr;
			mbHaveSourceAddrChanged = false;
			mSourceAddLock.unlock();
		}
#if !_DEBUG_SEND_SELF_
		// 屏蔽转发数据源地址之外的所有UDP数据
		if (mRevAddr!=mDataSourceAddr)
			return;
#endif
#if _DEBUG_SEND_SELF_
		//mBroadcastUDP.Send("192.168.1.155", 4256, mBroadcastReceiveBuffer.data(), revLen);
		AString sendString;
		for (PoolList<UInt64>::iterator it = mTargetAddrList.begin(); it!=mTargetAddrList.end(); ++it)
		{
			int p, p2;
			AString ip = ServerIPInfo::Num2IP(*it, p, p2);
			//printf(">>>>>>>>>Send to [%s:%d]\n", ip.c_str(), p2);
			//mBroadcastUDP.Send(ip.c_str(), p2, mBroadcastReceiveBuffer.data(), revLen);
			//if (*it!=mRevAddr)
			{
				//sendString += ", ";
				//sendString += *it;
				mBroadcastUDP.Send(*it, mBroadcastReceiveBuffer.data(), revLen);
			}
		}
		//printf("Send >[%s]\n", sendString.c_str());
#else
		for (PoolList<UInt64>::iterator it = mTargetAddrList.begin(); it!=mTargetAddrList.end(); ++it)
		{
			if (*it!=mRevAddr)
			{
				printf("*** send [%d] to >%s\n", revLen, ServerIPInfo::GetAddrInfo(*it).c_str());
				mBroadcastUDP.Send(*it, mBroadcastReceiveBuffer.data(), mBroadcastReceiveBuffer.dataSize());
			}
		}
#endif
	}
}

void UDPBroadcast::RegisterNetEvent(AutoEventCenter center)
{
	center->RegisterEvent("CS_NotityConnectUDP", MEM_NEW EventFactory<CS_NotityConnectUDP>());
}

AutoEvent UDPBroadcast::StartNotifyClient( HandConnect tcpConnect )
{
	Hand<CS_NotityConnectUDP> evt = tcpConnect->StartEvent("CS_NotityConnectUDP");
	//evt->_OnBindNet(tcpConnect.getPtr());
	evt->mUDP = GetSelf();
	evt->DoEvent();

	return evt;
}

void UDPBroadcast::SetSourceAddr( UInt64 sourceAddr )
{
	mSourceAddLock.lock();
	mSetSourceAddr = sourceAddr;
	mbHaveSourceAddrChanged = true;
	mSourceAddLock.unlock();
}

UInt64 UDPBroadcast::GetSourceAddr() 
{
	CsLockTool t(mSourceAddLock);

	return mSetSourceAddr;
}

bool UDPBroadcast::StartNet( const char *szIP, int lowPort, int highPort )
{
	bool bOk = false;
	if (highPort==0)
		bOk = mBroadcastUDP.Bind(szIP, lowPort);
	else
	{
		for (int i=lowPort; i<highPort; ++i)
		{
			if (mBroadcastUDP.Bind(szIP, i))
			{
				bOk = true;
				break;
			}
		}
	}
	if (!bOk)
		return false;

	bool b = UDPThreadNet::StartNet(szIP, lowPort, highPort);
	if (b)
	{
		mpBroadcastUDPReceiveThread->InitThread();
		mServerIP = getIpPort(mCheckPort, mBroadcastPort);
		printf("Check UDP Init [%s:%d], Broadcast port [%d]", mServerIP.c_str(), mCheckPort, mBroadcastPort);
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------
bool CS_NotityConnectUDP::_DoEvent()
{
	//Hand<BoroadcastEventCenter> center = GetEventCenter();

	mCheckCode = GetEventCenter()->AutoAlloctKey();

	set("CHECK_IP", mUDP->mServerIP.c_str());
	set("CHECK_PORT", mUDP->mCheckPort);
	set("CHECK_CODE", mCheckCode);	

	WaitTime(10);
	Send();

	mUDP->AppendWaitEvent(mCheckCode, GetSelf());

	return true;
}

bool CS_NotityConnectUDP::OnUDPResponce( UInt64 targetAdd, UInt64 checkCode )
{
	if (checkCode==mCheckCode)
	{
		//Hand<BoroadcastEventCenter> center = GetEventCenter();
		AutoEvent evt = GetEventCenter()->StartDefaultEvent("CS_NotifyStartBroadcast");
		evt->set("SERVER_IP", mUDP->mServerIP.c_str());
		evt->set("SERVER_PORT", mUDP->mBroadcastPort);
		mNetConnect->SendEvent(evt.getPtr());

		Finish();
		return true;
	}
	return false;
}
//-------------------------------------------------------------------------