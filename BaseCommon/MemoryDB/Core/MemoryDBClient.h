
#ifndef _INCLUDE_MEMORYDBCLIENT_H_
#define _INCLUDE_MEMORYDBCLIENT_H_

#include "MemoryDBHead.h"
#include "BaseMemoryDB.h"
#include "IOCPServerNet.h"
#include "DBCallBack.h"

class tDBManager;
//-------------------------------------------------------------------------
class MemoryDB_Export DBClient : public BaseMemoryDB
{
	friend class DB_Request;
	friend class DBNet;
	friend class DB_NotifyDBDistribution_R;

public:
	DBClient();

	virtual tDBManager* GetDBManager(){ return NULL; }

public:
	virtual ABaseTable CreateTable( const char *szIndex );

public:
	virtual bool Start(const char *szDBIp, int port)
	{
		return mDBNet->StartNet(szDBIp, port);
	}

	virtual void Reconnect()
	{
		mTableList.clear(false);
		mDBNet->TryReady();
	}

	virtual void Close()
	{
		mDBNet->StopNet();
		mNetSucceedCallBack.cleanup();
		mNetDisconnectCallBack.cleanup();
		BaseMemoryDB::Close();
	}

	virtual AutoEvent StartOperate(const char *szOperateName);

	virtual void Process()
	{
		BaseMemoryDB::Process();
		mDBNet->Process();
	}

	Logic::tEventCenter* GetEventCenter();
	AutoNet GetConnectNet(){ return mDBNet; }

	virtual bool IsConnected(){ return mDBNet->IsOk(); }

	virtual int GetDBNetSaftCode() const { return DB_SERVER_NET_SAFE_CODE; }

protected:
	AutoNet				mDBNet;
	AutoEvent			mEmptyDBRequest;

public:
	DBCallBack			mNetSucceedCallBack;
	DBCallBack			mNetDisconnectCallBack;

public:
	class DBNet : public IOCPClientNet
	{
		friend class DB_Request;
		friend class DB_NotifyDBDistribution_R;

	public:
		DBNet(DBClient *pDB)
			: mpDBClient(pDB)
			, mSendBufferMax(16*1024*1024)
			, mReceiveBufferMax(32*1024*1024)
		{

		}

		DBClient* GetDBClient(){ return mpDBClient; }

	public:
		virtual Logic::tEventCenter* GetEventCenter(void) const
		{
			return mpDBClient->mEventCenter.getPtr();
		}

		virtual int GetConnectOverTime(){ return 30; }

		virtual void OnConnected(){ mpDBClient->mNetSucceedCallBack.run(NULL, true); }
		virtual void OnConnectFail(){ mpDBClient->mNetSucceedCallBack.run(NULL, false); }
		virtual void OnCloseConnect(tNetConnect *pConnect) { mpDBClient->mNetDisconnectCallBack.run(NULL, true); }

		virtual int GetSafeCode(){ return mpDBClient->GetDBNetSaftCode(); }

		// DB默认网络缓存比较大
		virtual int SendBufferMax() override { return mSendBufferMax; }
		virtual int ReceiveBufferMax() override { return mReceiveBufferMax; }

		virtual HandConnect CreateConnect() override;

	protected:
		DBClient		*mpDBClient;

		int				mSendBufferMax;				// 16M
		int				mReceiveBufferMax;			// 32M
	};
};
//-------------------------------------------------------------------------


#endif //_INCLUDE_MEMORYDBCLIENT_H_