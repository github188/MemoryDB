
#ifndef _INCLUDE_LOGICDBNODE_H_
#define _INCLUDE_LOGICDBNODE_H_

#include "IOCPServerNet.h"
#include "IOCPConnect.h"
#include "MemoryDBNode.h"
#include "TableManager.h"
#include "AccountServer.h"
//-------------------------------------------------------------------------
//class GSUserConnect : public IOCPServerConnect
//{
//public:
//	GSUserConnect()
//		: IOCPServerConnect()
//		, mGSKey(0)
//	{
//	}
//
//public:
//	virtual GSKEY GetIPKey() override { return mGSKey; }
//
//	virtual void OnReceivePacket(Packet *pPacket) override
//	{
//		Hand<MemoryDB::DBServerNet> net = GetNetHandle();
//		++net->mReceiveCount;
//		IOCPServerConnect::OnReceivePacket(pPacket);
//	}
//
//	virtual void OnDisconnect() override
//	{
//		NOTE_LOG("[%s:%d] Disconnect, Now connect count [%d]", GetIp(), GetPort(), GetNetHandle()->GetConnectCount()-1);
//	}
//
//	GSKEY			mGSKey;
//};
//-------------------------------------------------------------------------
enum CONNECT_TYPE
{
	eDefaultDB,
	eWebDB,
	eLGConnect,
};

class AccountDBNode : public MemoryDBNode
{
public:
	//virtual AutoTable CreateNewDBTable(const char *szTable, const char *szTypex);
	//virtual void OnSucceedStart() override;
	virtual void OnLoadDBTableFinished(AutoTable dbTable) override;

public:
	virtual size_t AvailMemory();

	virtual size_t NowUseMemory();

	virtual void OnNotifyMemoryWarn(size_t availSize) 
	{

	}

	// 大约1分钟1次, 同步一次在线人数信息
	virtual bool NowDBState() override;

public:
	AccountDBNode(AccountThread *pDBThread, const char *szNodeIp, int nodePort)
		: MemoryDBNode(szNodeIp, nodePort)
		, mpLogicDBThread(pDBThread)
	{
		//TableManager::getSingleton().LoadTable(SERVER_GAME_CONFIG_FILE);
	}

	virtual void Process()
	{
		MemoryDBNode::Process();
	}

	// GS 连接成功
	virtual void OnUserConnected(tNetConnect *pConnect) override;
	virtual void OnUserDisconnect(tNetConnect *pConnect) override;

	virtual HandConnect CreateNewUserConnect()
	{
		return MEM_NEW DBConnect();
	}

	virtual int GetDBNetSaftCode() const override;

public:
	AccountThread	*mpLogicDBThread;

public:
	class DBConnect : public MemoryDB::DBUserConnect
	{
	public:
		DBConnect() 
			: mServerID(0) 
			, mConnectType(eDefaultDB)
			, mLgMainNodeNetKey(0)
		{}

	public:
		int mServerID;
		AString mLoginServerIP;
		AString mLGVersionFlag;
		UInt64		mLgMainNodeNetKey;

		CONNECT_TYPE mConnectType;
	};
};


#endif //_INCLUDE_LOGICDBNODE_H_