/********************************************************************
            	created:	2014/07/23
            	created:	23:7:2014   1:53
            	filename: 	H:\RemoteGame\Server\WorldServer\WorldThread.cpp
            	file path:	H:\RemoteGame\Server\WorldServer
            	file base:	WorldThread
            	file ext:	cpp
            	author:		Yang Wenge
            	
            	purpose:	基于内存运行的DB库
				原则:		1 快速SQL操作
							2 内存占用少
							3 安全落地，多备份MySQL 安全落地方案
							4 使用专用协议, 省去解析过程
				实现:		1 表格列表
							2 集群节点网络
							3 服务网络
							4 保存DB列表信息的存储器
*********************************************************************/

#ifndef _INCLUDE_MEMORYSQL_H_
#define _INCLUDE_MEMORYSQL_H_

#include "NiceData.h"
#include "BaseTable.h"
#include "NetHandle.h"
#include "EventCenter.h"
#include "IOCPServerNet.h"
#include "IOCPConnect.h"
#include "ServerIPInfo.h"
#include "Hand.h"
#include "MemoryDBHead.h"

#include "BaseMemoryDB.h"
#include "DBSaver.h"
#include "MemoryDBTable.h"

#include "DBOperate.h"

#include "DBNodeOperate.h"

typedef EasyHash<AString, AutoDBTable>	DBTableMap;
//-------------------------------------------------------------------------
// 节点间创建记录错误信息
namespace Logic
{
	class tEventCenter;
}
class BaseThread;
//-------------------------------------------------------------------------
class MemoryDB_Export MemoryDB : public BaseMemoryDB
{
	friend class DB_Responce;
	friend class MemoryDBTable;
public:
	MemoryDB();
	virtual ~MemoryDB()
	{
	}

public:
	static void RegisterDBServerEvent(AutoEventCenter serverNetEventCenter);
	static void RegisterDBClientEvent(AutoEventCenter clientDBEventCenter);

	static bool GetDBTableComment(NiceData &dbParam, const char *szTableName, AString &resultComment);
	static bool SetDBTableComment(NiceData &dbParam, const char *szTableName, const AString &comment);

	static AString GetTableInfoString(ARecord tableInfoRecord);

public:
	// 可以在此功能内设定冷却DB加载限定字段及时间范围
	virtual void OnLoadDBTableBefore(AutoTable dbTable){}
	virtual void OnLoadDBTableFinished(AutoTable dbTable) {}
	virtual void OnSucceedStart();
	virtual void LoadAllDBTable();

public:
	virtual bool ReadyTableDBDataSource(AutoDBTable t, int checkCode);

	virtual ABaseTable CreateTable( const char *szIndex )
	{
		ABaseTable t = MEM_NEW MemoryDBTable((MemoryDB*)this); //tBaseTable::NewBaseTable();
		t->SetTableName(szIndex);
		mTableList.erase(szIndex);
		mTableList.insert(szIndex, t);
		return t;
	}

	virtual ABaseTable CreateIndexTable(const char *szTableIndex);
	virtual AutoTable CreateNewDBTable(const char *szTableIndex, const char *szType );

public:
	virtual ABaseTable NewCreateDBTable(const char *szTableType, const char *szTable, const char *fieldData, int checkCode, AString indexFields, AutoNice extParam, AString &errorInfo)
	{
		AssertEx(0, "目前只实现MemoryDBNote 功能");
		return ABaseTable();
	}
	virtual eDBResultType SaveRecord(const char *szIndex, const char *szKey, DataStream *scrRecordData, bool bReplace, bool bUpdate, AutoEvent waitEvt);
	virtual eDBResultType FindRecord(const char *szTable, const char *szKey, AutoEvent waitEvt)
	{ 
		AssertEx(0, "目前只实现MemoryDBNote 功能");
		return eSourceCodeNotFinish;
	}
	virtual eDBResultType DeleteRecord(const char *szTable, const char *szKey, AutoEvent waitEvt)
	{ 
		AssertEx(0, "目前只实现MemoryDBNote 功能");
		return eSourceCodeNotFinish;
	}
	virtual eDBResultType InsertGrowthRecord(const char *szIndex, DataStream *scrRecordData, AutoEvent waitEvt)
	{
		AssertEx(0, "目前只实现MemoryDBNote 功能");
		return eSourceCodeNotFinish;
	}

	ARecord FindRecord(const char *szTable, const char *szKey);

public:
	// 同步表格字段结构数据到数据源
	AString SaveDBTableFieldToDB(const char *szTableIndex, ABaseTable table, AString *pDBTableInfoData, AutoNice extParam);
	void InitReadyMemoryDBTable(const char *szTableIndex, AString fieldData, bool bStartDB = true);
	
	void OnNotifyFinishLoadAllRecord(AutoDBTable t);

	ABaseTable GetDBTableListTable(){ return mTableListDBTable; }
	virtual bool GetmemoryState() const { return true; }
	bool IsLocalDB(){ return mbLocalModeSaveDB; }

	virtual void OnNewCreateDBTable(const char *szTableName, AutoTable table, const AString &tableInfoData, AutoNice extParam, AutoRecord &tableInfoRecord){}

public:
	virtual bool InitDB(NiceData &initParam);
	virtual bool Start(const char *szIp, int nPort);

	virtual void Close();

	virtual void Process();

	// 如果当前无任何待处理的操作返回true
	virtual bool NowDBState();

	virtual void Log(const char *szInfo, ...);

	virtual Logic::tEventCenter* GetEventCenter();
	virtual AutoNet GetDBServerNet(){ return mDBServerNet; }
	virtual HandConnect FindGSConnect(GSKEY gskey);

	virtual HandConnect CreateNewUserConnect();
	virtual int GetDBNetSaftCode() const { return DB_SERVER_NET_SAFE_CODE; }
	NiceData& GetDBConfig() { return mDataSourceConfig; }

	virtual void _OnUserConnected(tNetConnect *pConnect) {}
	virtual void OnUserConnected(tNetConnect *pConnect) {}
	virtual void OnUserDisconnect(tNetConnect *pConnect) {}

public:
	virtual bool RegisterDBOperate(const char *type, AutoDBNodeOpreateFactory factory);
	virtual AutoOpereate StartOperate(const char *type);

protected:
	// Net
	AutoNet			mDBServerNet;			

	NiceData		mDataSourceConfig;		// 磁盘数据库DB,　MySql或者MsSql		

	// 用于表格列表(数据表同样处理)信息的存储器, 通常与数据存储放在一起
	AutoTable		mTableListDBTable;		
	int				mConfigCoolDBTime;		// 秒 冷表处理时间，默认 DB_COOL_TIME
	bool			mbLocalModeSaveDB;		// 是否使用本地DB文件落地, 配置 DBServer STRING3 为LOCAL_DB

public:
	//-------------------------------------------------------------------------*/
	class DBServerNet : public IOCPServerNet
	{
	public:
		DBServerNet(MemoryDB *pDB)
			: mpServerDB(pDB)
			, mReceiveCount(0)
			, mSendCount(0)
			, mReceiveSize(0)
			, mSendSize(0)
			, mSendBufferMax(16*1024*1024)
			, mReceiveBufferMax(32*1024*1024)
		{

		}

	public:
		virtual HandConnect CreateConnect();
		virtual Logic::tEventCenter* GetEventCenter(void) const
		{
			return mpServerDB->mEventCenter.getPtr();
		}

		virtual bool OnAddConnect(tNetConnect *pConnect) override;
		virtual void OnCloseConnect(tNetConnect *pConnect) override { mpServerDB->OnUserDisconnect(pConnect); }

		virtual int GetSafeCode() { return mpServerDB->GetDBNetSaftCode(); }		

		virtual bool NeedZipSendPacket(){ return false; }
		virtual bool NeedEncryptPacket() const { return false; }

		virtual void AddReceiveTotalSize(int receiveDataSize){ mReceiveSize += receiveDataSize; }
		virtual void AddSendTotalSize(int sendDataSize){ ++mSendCount;  mSendSize+= sendDataSize; }

		// DB默认网络缓存比较大
		virtual int SendBufferMax() override { return mSendBufferMax; }
		virtual int ReceiveBufferMax() override { return mReceiveBufferMax; }

	protected:
		int				mSendBufferMax;				// 16M
		int				mReceiveBufferMax;			// 32M

	public:
		MemoryDB		*mpServerDB;

		int				mReceiveCount;
		int				mSendCount;
		size_t			mReceiveSize;
		size_t			mSendSize;
	};

	//-------------------------------------------------------------------------*/
	class DBUserConnect : public IOCPServerConnect
	{
	public:
		DBUserConnect()
			: IOCPServerConnect(DB_NET_INIT_BUFFER_SIZE, DB_NET_INIT_BUFFER_SIZE)
			, mIpKey(0)
		{

		}

	public:
		virtual UInt64 GetIPKey(){ return mIpKey; }
		virtual void OnConnected(){ mIpKey = ServerIPInfo::IP2Num(GetIp(), GetPort(), 0); }
		virtual void OnDisconnect() override
		{
			NOTE_LOG("[%s:%d] Disconnect", GetIp(), GetPort());
		}

		virtual void OnReceivePacket(Packet *pPacket) override
		{
			Hand<MemoryDB::DBServerNet> net = GetNetHandle();
			++net->mReceiveCount;
			IOCPServerConnect::OnReceivePacket(pPacket);
		}

	protected:
		GSKEY					mIpKey;
	};
	//-------------------------------------------------------------------------*/
};
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
#endif //_INCLUDE_MEMORYSQL_H_