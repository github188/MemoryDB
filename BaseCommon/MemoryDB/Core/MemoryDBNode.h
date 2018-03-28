
#ifndef _INCLUDE_MEMORYDBNODE_H_
#define _INCLUDE_MEMORYDBNODE_H_

#include "MemoryDB.h"
#include "MeshedNodeNet.h"
#include "MemoryDBHead.h"

#include "KeyDistributionInfo.h"
#include "DBResultCallBack.h"
#include "DBManager.h"
//-------------------------------------------------------------------------
#define DB_NODE_NET_CODE	-10166

//-------------------------------------------------------------------------
class MemoryDB_Export MemoryDBNode : public MemoryDB
{
	friend class DBMeshedNodeNet;
public:
	MemoryDBNode(const char *szNodeIp, int nodePort);
	virtual ~MemoryDBNode();

public:
	// 新建DB表格, 且同步到存储器, 初始数据源
	virtual ABaseTable NewCreateDBTable(const char *szTableType, const char *szTable, const char *fieldData, int checkCode, AString indexFields, AutoNice extParam, AString &errorInfo);

	// 数据操作, 创建新记录, NOTE: 不进行替换创建, 如果记录存在, 返回否
	ARecord _CreateRecord(const char *szTable, const char *recordKey, DataStream *recordData, bool bReplace, eDBResultType &result);
	ARecord _FindRecord( const char *szTable, const char *recordKey);

	// NOTE: 且只有调用_SaveRecord 才可以在主表DB插入新记录
	eDBResultType _SaveRecord( const char *szIndex, const char *recordKey, DataStream *scrRecordData, bool bReplace, bool bUpdate);
	eDBResultType _DeleteRecord( const char *szTable, const char *recordKey, ARecord &resultRe);

	// 通用记录操作, 操作任何子表中的记录 [子表字段:子表记录KEY] ... 通过递归找到目标记录
	eDBResultType _SaveTargetRecord( const char *szIndex, const char *recordKey, const char *subFieldTargetKey, DataStream *scrRecordData, eRecordOperateMode saveMode, ARecord &resultRecord, AString &errorInfo, int recordCountLimit);
	eDBResultType _DeleteTargetRecord( const char *szTable, const char *recordKey, const char *subFieldTargetKey, ARecord &resultRe, AString &errorInfo);

	// 本地自增长记录
	eDBResultType _InsertGrowthRecord( const char *szIndex, DataStream *scrRecordData, ARecord &re );
	eDBResultType _GetRecordData(const char *szIndex, const char *recordkey, const char *szTargetFieldInfo, Array<EasyString> &fieldList, AutoNice &resultData);
	eDBResultType ModifyData( const char *szTable, const char *szRecordKey, const char *targetFieldInfo, const char *destField, const Data &limitValue, eModifyDataMode modifyDataMode, const char *szDestValue, AString &resultValue, AString &originalValue, AString &errorInfo );
	bool ModifyData(DBResultCallBack callBack, const char *szTable, const char *szRecordKey, const char *targetFieldInfo, const char *destField, const Data &limitValue, eModifyDataMode modifyDataMode, const char *szDestValue, AString &resultValue, AString &originalValue, AString &errorInfo );

	//eDBResultType ModifyData( const char *szTable, const char *szRecordKey, const char *targetFieldInfo, const char *destField, DataStream *pDestStream, AString &errorInfo );
	//bool ModifyData(DBResultCallBack callBack, const char *szTable, const char *szRecordKey, const char *targetFieldInfo, const char *destField, DataStream *pDestStream, AString &errorInfo );

	// 智能保存数据, 如果存在, 修改, 如果不存在, 保存进行创建, 更新时返回不存在错误
	eDBResultType SaveRecord(const char *szIndex, const char *recordKey, DataStream *scrRecordData, bool bReplace, bool bUpdate, AutoEvent waitEvt);

	eDBResultType FindRecord(const char *szTable, const char *recordKey, AutoEvent waitEvt);

	eDBResultType DeleteRecord(const char *szTable, const char *recordKey, AutoEvent waitEvt);

	eDBResultType InsertGrowthRecord(const char *szIndex, DataStream *scrRecordData, AutoEvent waitEvt);

	eDBResultType  LoadRecordData(const char *szIndex, const char *szKey, AString fieldInfo, DBResultCallBack callBack);

	eDBResultType  SaveRecordData(const char *szIndex, const char *szKey, AutoNice recordData, DBResultCallBack callBack);

public:
	// subFieldKey: [子表字段名:子表记录KEY] ... 通过递归找到目标记录及子表
	// resultTable 结果子表
	// targetRecord 目标记录
	// mainRecordField : 目标所在的主表记录字段
	eDBResultType _FindTargetRecord( const char *szTable, const char *mainRecordKey, const char *subFieldKey, ARecord &mainRecord, ABaseTable &targetTable, ARecord &targetRecord, AString &mainRecordField, AString &errorInfo );

public:
	// 获取当前数据库中所有表格的字段分布信息
	ABaseTable	GetNowDataTableKeyInfo();
	// 获取数据库集群内所有的DB表格分布信息
	ABaseTable GetAllDBTableDistributionData();
	virtual void OnLoadDBTableFinished(AutoTable dbTable) override {}

	virtual void OnNewCreateDBTable(const char *szTableName, AutoTable table, const AString &tableInfoData, AutoNice extParam, AutoRecord &tableInfoRecord);

	virtual void _OnUserConnected(tNetConnect *pConnect) override;

    HandConnect GetUserConnect(int userNetID);

public:
	virtual bool InitDB(NiceData &initParam);
	virtual void Process();

	virtual HandConnect FindNodeByKey(const char *szTable, Int64 key);
	virtual HandConnect FindNodeByKey(const char *szTable, const char *szKey);

	virtual bool AppendNodeDistributionData( Hand<NetNodeConnectData> node, ABaseTable keyInfoTable );

	virtual bool CheckKeyInThisNodeRange(const char *szTalbe, const char *recordKey);

	//virtual void RunOperate( const char *operateType, const char *szTable, const char *recordKey, AutoNice &paramData, Hand<tClientEvent> requestEvent );
	AutoOpereate RunOperate(tNetConnect *pRequestConnect, const char *operateType, const char *szTable, const char *recordKey, AutoNice &paramDatae, eDBResultType &result);
	virtual bool RunOperate(tNetConnect *pRequestConnect, DBResultCallBack callBack,  const char *operateType, const char *szTable, const char *recordKey, AutoNice &paramData );

	bool RecordOperate( DBResultCallBack callBack, eRecordOperateMode mode, const char *szTable, const char *szRecordKey, AutoRecord dataRecord, const char *subTableFieldName, ... );
	bool RecordOperate(DBResultCallBack callBack, eRecordOperateMode mode, const char *szTable, const char *szRecordKey, AutoRecord dataRecord, int recordCountLimit, const char *subTableFieldName, ... );

public:
	virtual AutoNet GetNodeNet(){ return mDBNodeNet; }
	virtual HandDB GetDBOperateMgr(){ return mDBOperateManager; }

	virtual void Close();

	virtual size_t AvailMemory() = 0;
	virtual size_t NowUseMemory() = 0;
	virtual void OnNotifyMemoryWarn(size_t availSize) = 0;

	virtual void SetMemoryState(bool bOk){ mMemoryOk = bOk; }
	virtual bool GetmemoryState() const { return mMemoryOk; }

	// 统计当前后台正在处理的数据请求总个数，此数据反映DB存储性能
	virtual int TotalDataTaskCount();

	virtual void TotalNetInfo(float &frames, int &revCount, int &sendCount, UInt64 &revSizeBySec, UInt64 &sendSizeBySec);

protected:
	AutoNet				mDBNodeNet;					// 集群功能
	bool				mMainDBNode;
	bool				mMemoryOk;					// 内存正常, 系统内存不够用时, 中止创建记录，抛出异常
	EasyHash<EasyString, AutoDistributionData>		mTableDistributionDataList;
	HandDB				mDBOperateManager;

public:
	AutoEvent			mMemoryCheckEvent;
	int					mFrameCount;
	Array<HandConnect>	mUserConnectList;
};

typedef Hand<MemoryDBNode>		HandDBNode;
//-------------------------------------------------------------------------

class DBMeshedNodeNet : public MeshedNodeNet
{

public:
	DBMeshedNodeNet(MemoryDBNode *pDBNode, const char *szServerIp, int nServerPort, int safeCheckCode);

	virtual void RegisterDBNodeNetEvent();

	virtual const char* NodeInfo() override { return "DB"; }

public:
	//??? 当连接后, 即启动请求获取在线人数即检查网络状况事件
	virtual void OnAppendNetNode(NetNodeConnectData *nodeData);

public:
	// 当主动连接对方Node，成功后, 通知对方事件发送之前，可携带其他数据
	// 通知当前是什么类型的服务器, 及在线人数
	virtual void OnRequestNode(NetNodeConnectData *nodeData, AutoEvent &requestEvt) override
	{

	}

	// 作为被连接者，接收到连接者连接后的请求事件, 主要告诉连接者当前整个节点群的信息
	// 识别服务器类型，根据类型回复相关信息
	virtual void OnResponseNode(GSKEY otherPartyGSKey, AutoEvent &requstEvt, AutoEvent &respEvt) override;

	// 对方回复数据
	virtual void OnReceiveResponse(NetNodeConnectData *nodeData, AutoEvent &requestEvt, AutoEvent &respEvt) override;

	void OnTestFinish(DBOperate *op, bool bSu);

public:
	MemoryDBNode	*mpDBNode;
};
//-------------------------------------------------------------------------

#endif //_INCLUDE_MEMORYDBNODE_H_