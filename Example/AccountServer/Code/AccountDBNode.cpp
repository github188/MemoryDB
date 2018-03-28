

#include "AccountDBNode.h"

#include "AccountServer.h"
#include "CommonDefine.h"

#include <Windows.h>
//#define  PSAPI_VERSION 2
#include "psapi.h"

//void WinMemoryDBNode::OnSucceedStart()
//{
//	MemoryDBNode::OnSucceedStart();
//
//	if (GetTable(GAME_PLAYER_PRIVATE_TABLE) && GetTable(GAME_PLAYER_PUBLIC_TABLE))
//	{		
//		// 到此DB才真正完成准备好, 就是集群准备完成
//		GameLogic::Init(this);		
//	}
//
//	WorldBoss::GetMe().Init();
//}

void AccountDBNode::OnLoadDBTableFinished(AutoTable dbTable)
{

}

size_t AccountDBNode::AvailMemory()
{
	static MEMORYSTATUS sysMemory;
	::GlobalMemoryStatus(&sysMemory);	
	return sysMemory.dwAvailPhys;
}

size_t AccountDBNode::NowUseMemory()
{
	static PROCESS_MEMORY_COUNTERS memoryUseInfo;

	GetProcessMemoryInfo(
		GetCurrentProcess(),
		&memoryUseInfo,
		sizeof(memoryUseInfo)
		);

	return memoryUseInfo.WorkingSetSize;
}


bool AccountDBNode::NowDBState()
{
	bool b = MemoryDBNode::NowDBState();
	mpLogicDBThread->SyncServerInfoToLG();
	return b;
}

void AccountDBNode::OnUserConnected(tNetConnect *pConnect)
{

}

void AccountDBNode::OnUserDisconnect(tNetConnect *pConnect)
{
	// 移除并同步到其他LG
	Hand<DBConnect> conn = pConnect;
	ARecord re = mpLogicDBThread->mServerInfoTable->GetRecord(conn->mServerID);
	if (re)
	{
		AutoNice lgIpList = (tNiceData*)re["LG_IP"];
		if (lgIpList && lgIpList->remove(conn->mLoginServerIP.c_str()))
		{
			if (lgIpList->empty())
				mpLogicDBThread->mServerInfoTable->RemoveRecord(conn->mServerID);
			NOTE_LOG("移除登陆服务器 [%d] %s", conn->mServerID, ServerIPInfo::GetAddrInfo(TOUINT64(conn->mLoginServerIP.c_str())).c_str());
			mpLogicDBThread->SyncServerInfoToLG();
		}
	}
}

int AccountDBNode::GetDBNetSaftCode() const 
{
	return  ACCOUNT_NET_SAFECODE;
}

//-------------------------------------------------------------------------



//-------------------------------------------------------------------------
