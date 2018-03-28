
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
	// �½�DB���, ��ͬ�����洢��, ��ʼ����Դ
	virtual ABaseTable NewCreateDBTable(const char *szTableType, const char *szTable, const char *fieldData, int checkCode, AString indexFields, AutoNice extParam, AString &errorInfo);

	// ���ݲ���, �����¼�¼, NOTE: �������滻����, �����¼����, ���ط�
	ARecord _CreateRecord(const char *szTable, const char *recordKey, DataStream *recordData, bool bReplace, eDBResultType &result);
	ARecord _FindRecord( const char *szTable, const char *recordKey);

	// NOTE: ��ֻ�е���_SaveRecord �ſ���������DB�����¼�¼
	eDBResultType _SaveRecord( const char *szIndex, const char *recordKey, DataStream *scrRecordData, bool bReplace, bool bUpdate);
	eDBResultType _DeleteRecord( const char *szTable, const char *recordKey, ARecord &resultRe);

	// ͨ�ü�¼����, �����κ��ӱ��еļ�¼ [�ӱ��ֶ�:�ӱ��¼KEY] ... ͨ���ݹ��ҵ�Ŀ���¼
	eDBResultType _SaveTargetRecord( const char *szIndex, const char *recordKey, const char *subFieldTargetKey, DataStream *scrRecordData, eRecordOperateMode saveMode, ARecord &resultRecord, AString &errorInfo, int recordCountLimit);
	eDBResultType _DeleteTargetRecord( const char *szTable, const char *recordKey, const char *subFieldTargetKey, ARecord &resultRe, AString &errorInfo);

	// ������������¼
	eDBResultType _InsertGrowthRecord( const char *szIndex, DataStream *scrRecordData, ARecord &re );
	eDBResultType _GetRecordData(const char *szIndex, const char *recordkey, const char *szTargetFieldInfo, Array<EasyString> &fieldList, AutoNice &resultData);
	eDBResultType ModifyData( const char *szTable, const char *szRecordKey, const char *targetFieldInfo, const char *destField, const Data &limitValue, eModifyDataMode modifyDataMode, const char *szDestValue, AString &resultValue, AString &originalValue, AString &errorInfo );
	bool ModifyData(DBResultCallBack callBack, const char *szTable, const char *szRecordKey, const char *targetFieldInfo, const char *destField, const Data &limitValue, eModifyDataMode modifyDataMode, const char *szDestValue, AString &resultValue, AString &originalValue, AString &errorInfo );

	//eDBResultType ModifyData( const char *szTable, const char *szRecordKey, const char *targetFieldInfo, const char *destField, DataStream *pDestStream, AString &errorInfo );
	//bool ModifyData(DBResultCallBack callBack, const char *szTable, const char *szRecordKey, const char *targetFieldInfo, const char *destField, DataStream *pDestStream, AString &errorInfo );

	// ���ܱ�������, �������, �޸�, ���������, ������д���, ����ʱ���ز����ڴ���
	eDBResultType SaveRecord(const char *szIndex, const char *recordKey, DataStream *scrRecordData, bool bReplace, bool bUpdate, AutoEvent waitEvt);

	eDBResultType FindRecord(const char *szTable, const char *recordKey, AutoEvent waitEvt);

	eDBResultType DeleteRecord(const char *szTable, const char *recordKey, AutoEvent waitEvt);

	eDBResultType InsertGrowthRecord(const char *szIndex, DataStream *scrRecordData, AutoEvent waitEvt);

	eDBResultType  LoadRecordData(const char *szIndex, const char *szKey, AString fieldInfo, DBResultCallBack callBack);

	eDBResultType  SaveRecordData(const char *szIndex, const char *szKey, AutoNice recordData, DBResultCallBack callBack);

public:
	// subFieldKey: [�ӱ��ֶ���:�ӱ��¼KEY] ... ͨ���ݹ��ҵ�Ŀ���¼���ӱ�
	// resultTable ����ӱ�
	// targetRecord Ŀ���¼
	// mainRecordField : Ŀ�����ڵ������¼�ֶ�
	eDBResultType _FindTargetRecord( const char *szTable, const char *mainRecordKey, const char *subFieldKey, ARecord &mainRecord, ABaseTable &targetTable, ARecord &targetRecord, AString &mainRecordField, AString &errorInfo );

public:
	// ��ȡ��ǰ���ݿ������б����ֶηֲ���Ϣ
	ABaseTable	GetNowDataTableKeyInfo();
	// ��ȡ���ݿ⼯Ⱥ�����е�DB���ֲ���Ϣ
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

	// ͳ�Ƶ�ǰ��̨���ڴ�������������ܸ����������ݷ�ӳDB�洢����
	virtual int TotalDataTaskCount();

	virtual void TotalNetInfo(float &frames, int &revCount, int &sendCount, UInt64 &revSizeBySec, UInt64 &sendSizeBySec);

protected:
	AutoNet				mDBNodeNet;					// ��Ⱥ����
	bool				mMainDBNode;
	bool				mMemoryOk;					// �ڴ�����, ϵͳ�ڴ治����ʱ, ��ֹ������¼���׳��쳣
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
	//??? �����Ӻ�, �����������ȡ�����������������״���¼�
	virtual void OnAppendNetNode(NetNodeConnectData *nodeData);

public:
	// ���������ӶԷ�Node���ɹ���, ֪ͨ�Է��¼�����֮ǰ����Я����������
	// ֪ͨ��ǰ��ʲô���͵ķ�����, ����������
	virtual void OnRequestNode(NetNodeConnectData *nodeData, AutoEvent &requestEvt) override
	{

	}

	// ��Ϊ�������ߣ����յ����������Ӻ�������¼�, ��Ҫ���������ߵ�ǰ�����ڵ�Ⱥ����Ϣ
	// ʶ����������ͣ��������ͻظ������Ϣ
	virtual void OnResponseNode(GSKEY otherPartyGSKey, AutoEvent &requstEvt, AutoEvent &respEvt) override;

	// �Է��ظ�����
	virtual void OnReceiveResponse(NetNodeConnectData *nodeData, AutoEvent &requestEvt, AutoEvent &respEvt) override;

	void OnTestFinish(DBOperate *op, bool bSu);

public:
	MemoryDBNode	*mpDBNode;
};
//-------------------------------------------------------------------------

#endif //_INCLUDE_MEMORYDBNODE_H_