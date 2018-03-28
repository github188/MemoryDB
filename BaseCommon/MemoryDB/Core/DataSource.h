/********************************************************************
	created:	2014/07/27
	created:	27:7:2014   1:20
	filename: 	H:\RemoteGame\BaseCommon\ServerBase\DBWork\DataSource.h
	file path:	H:\RemoteGame\BaseCommon\ServerBase\DBWork
	file base:	DataSource
	file ext:	h
	author:		Yang Wenge
	
	purpose:	MemoryDB ����Դ, ���ݳ־ñ���
	NOTE:		ʹ������ʽ�첽��������
*********************************************************************/
#ifndef _INCLUDE_DATASOURCE_H_
#define _INCLUDE_DATASOURCE_H_

#include "TaskManager.h"
#include "LoadDataCallBack.h"
#include "BaseTable.h"
#include "DBCallBack.h"

#include "NiceData.h"
#include "NetHandle.h"
#include "MemoryDBHead.h"
#include "DBResultCallBack.h"
#include "MySqlDBTool.h"

// �Ƿ���DB�����ļ��첽����(����,ɾ��)
#define LOCAL_DB_USE_THREAD		0

#define LOAD_RECORD_CALLBACK LoadDataCallBack

//-------------------------------------------------------------------------
class MemoryDB_Export tDataSource : public AutoBase
{
public:
	virtual ~tDataSource(){}
	virtual bool InitDataSource(NiceData &initParam){ return false; }
	virtual void Process() = 0;

	virtual bool ReadyCreateDBTable(const char* szTableIndex, tBaseTable *table, AString tableInfoData) = 0;

	virtual int GetOperateCount() = 0;
	virtual int GetErrorCount(){ return 0; }

public:
	// WARN: �������,���ɱ�����ʹ��, ֻ����ɺ�, �ſ��Լ���ʹ��
	virtual bool LoadAllRecord(ABaseTable destTable, LOAD_RECORD_CALLBACK callBack) = 0;

	// NOTE: �첽ʱ, ���뱣֤Ͷ��ִ��˳��, ��ɾ�����ٲ���ͬ��KEY��ֵʱ, �Ų��������ɾ�µļ�¼
	virtual void DelectData(const char *szKeyID, UInt64 extValue, LoadDataCallBack callBack) = 0;

	virtual bool SaveRecordData(const char *szKey, BaseRecord *pRe) = 0;

	virtual bool ReloadRecord(const char *szKey, AutoRecord destRecord) = 0;

	virtual bool ReadyReloadTool(NiceData &initParam){ return false; }

};
typedef Auto<tDataSource> AutoSource;

//-------------------------------------------------------------------------
class tDBSaver;
//-------------------------------------------------------------------------


class MemoryDB_Export DataSource : public tDataSource, public TaskManager
{
	friend class DataTask;
public:
	DataSource();
	~DataSource();

	virtual tDBSaver* CreateSaver();

	virtual void ClearAuto() { TaskManager::Close(); }

	virtual int GetOperateCount() { return TaskManager::GetNowTaskCount(); }

	virtual void AddErrorCount(int errorCount)
	{
		mErrorCountLock.lock();
		mErrorCount += errorCount;
		mErrorCountLock.unlock();
	}

	virtual int GetErrorCount()
	{
		int count = 0;
		mErrorCountLock.lock();
		count = mErrorCount;
		mErrorCountLock.unlock();
		return count;
	}

public:
	virtual const char* GetDBName();
	virtual bool InitDataSource(NiceData &initParam);
	virtual bool ReadyCreateDBTable(const char* szTableIndex, tBaseTable *tableInfoData, AString tableData);

	virtual int	OnceProcessOverTime() { return 50; }

	virtual void Process();

public:
	virtual bool SaveRecordData(const char *szKey, BaseRecord *pRe) override
	{ ERROR_LOG("DataSource δʵ��SaveRecordData"); return false; }
	// WARN: �������,���ɱ�����ʹ��, ֻ����ɺ�, �ſ��Լ���ʹ��
	virtual bool LoadAllRecord(ABaseTable destTable, LOAD_RECORD_CALLBACK callBack);

	// NOTE: �첽ʱ, ���뱣֤Ͷ��ִ��˳��, ��ɾ�����ٲ���ͬ��KEY��ֵʱ, �Ų��������ɾ�µļ�¼
	virtual void DelectData(const char *szKeyID, UInt64 extValue, LoadDataCallBack callBack) override;

	virtual bool ReloadRecord(const char *szKey, AutoRecord destRecord) override
	{
		ERROR_LOG("δʵ�� ReloadRecord");
		return false;
	}

protected:
	void ReadyDBSaver(tDBSaver *saver);

protected:
	tDBSaver			*mDBSaver;
	Array<AutoNet>		mBackNet;		// �����ڱ���, ֻ���ڱ���, �����ڶ�ȡ
	
	int					mErrorCount;
	CsLock				mErrorCountLock;
};
//-------------------------------------------------------------------------
// �ַ�����ʽ��������
// NOTE: ������DB�б����ȡ�뱣��, �ɴ�����Դ�����ı��Դ���Ƕ�����(BLOB)��ʽ
//-------------------------------------------------------------------------
class MemoryDB_Export StringDataSource : public DataSource
{
public:
	virtual bool RestoreRecord(BaseRecord *pDestRecord, const char *pData, int size)
	{
		return pDestRecord->FullFromString(pData, false, false);
	}

	virtual bool SaveRecordData(const char *szKey, BaseRecord *pRe);

};

//-------------------------------------------------------------------------
// ʹ��MYSQL DB �ṹ��ʽ�洢
//-------------------------------------------------------------------------
class DBTableSaver;

class MemoryDB_Export MySqlDataSource : public DataSource
{
public:
	virtual tDBSaver* CreateSaver();
	virtual bool SaveRecordData(const char *szKey, BaseRecord *pRe);

	virtual bool LoadAllRecord(ABaseTable destTable, LOAD_RECORD_CALLBACK callBack);

public:
	virtual bool ReloadRecord(const char *szKey, AutoRecord destRecord) override;

	virtual bool ReadyReloadTool(NiceData &initParam) override;

public:
	virtual void Process() override
	{
		DataSource::Process();
		if (mReloadRecordTool)
			mReloadRecordTool->Process();
	}

	//NOTE: �����߳�ִ��, �˵���һ��� 6�����һ�� event_timedwait(mWaitEvent, 6000)
	virtual void backProcess() override;

public:
	DBTableSaver* GetMySqlSaver();

public:
	// �����������µ�ȡ��¼��DB
	Auto<MySqlDBTool>	mReloadRecordTool;
};

class MemoryDB_Export MySqlStringDataSource : public StringDataSource
{
public:
	virtual tDBSaver* CreateSaver();

	//NOTE: �����߳�ִ��, �˵���һ��� 6�����һ�� event_timedwait(mWaitEvent, 6000)
	virtual void backProcess () override;
};
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// ���ݼ���ض���һ ����Դ
// �ڲ��趨һ��������Զ����ؽ���
// ֱ�������߳�Ͷ����Ϣ, ��ʡ�߳�����, ���������ݱ仯����ֱ��ת�Ƶ�Ŀ�ĵ�
////class BackDataSource : public tDataSource
////{
////public:
////	virtual void Process();
////
////	virtual void ReadyCreateDBTable(const char* szTableIndex, tBaseTable *tableInfoData)
////	{
////		
////	}
////
////public:
////	virtual void SaveData(Int64 keyID, void *pData, DSIZE size)
////	{
////		SaveData(STRING(keyID), pData, size);
////	}
////	virtual void SaveData(const char *szKeyID, void *pData, DSIZE size);
////
////	virtual void LoadData(Int64 keyID, LoadDataCallBack callBack)
////	{
////		LoadData(STRING(keyID), callBack);
////	}
////
////	virtual void LoadData(const char *szKeyID, LoadDataCallBack callBack);
////
////	// WARN: �������,���ɱ�����ʹ��, ֻ����ɺ�, �ſ��Լ���ʹ��
////	virtual void LoadAllRecord(ABaseTable destTable, DBCallBack callBack);
////
////	// NOTE: �첽ʱ, ���뱣֤Ͷ��ִ��˳��, ��ɾ�����ٲ���ͬ��KEY��ֵʱ, �Ų��������ɾ�µļ�¼
////	virtual void DelectData(const char *szKeyID, LoadDataCallBack callBack);
////
////
////public:
////	Array<AutoNet>		mBackNet;
////	AString				mDBName;
////};
//-------------------------------------------------------------------------
// �Ա����ļ���ط�ʽ
// DB�ļ��첽�����ɾ��������Ҫ���´���
// DB�ļ��洢���ڱ���Hash������¼KEY��Ӧ�ļ�¼λ��
// ��Ϊ�½���¼�޷���ͬ���������б���ļ�¼λ��
//-------------------------------------------------------------------------

class LocalFileDBSaver;
class LocalDataSource : public DataSource
{
public:
	virtual bool InitDataSource(NiceData &initParam) override;

	virtual bool ReadyCreateDBTable(const char* szTableIndex, tBaseTable *tableInfoData, AString tableData) override;

	// WARN: �������,���ɱ�����ʹ��, ֻ����ɺ�, �ſ��Լ���ʹ��
	virtual bool LoadAllRecord(ABaseTable destTable, LOAD_RECORD_CALLBACK callBack) override;

	// NOTE: �첽ʱ, ���뱣֤Ͷ��ִ��˳��, ��ɾ�����ٲ���ͬ��KEY��ֵʱ, �Ų��������ɾ�µļ�¼
	virtual void DelectData(const char *szKeyID, UInt64 extValue, LoadDataCallBack callBack) override;
	virtual bool SaveRecordData(const char *szKey, BaseRecord *pRe) override;

	// ��Ҫ�������ͬ������, �򵥷�����, ֻ��DB�ļ�
	// Ŀǰȫ����ͬһ�߳�, ֱ�Ӷ�ȡ
	virtual bool ReloadRecord(const char *szKey, AutoRecord destRecord) override;

	virtual bool ReadyReloadTool(NiceData &initParam);

public:
	virtual tDBSaver* CreateSaver() override;

	LocalFileDBSaver* GetFileDBSaver();

public:
	LocalDataSource()
#if LOCAL_DB_USE_THREAD
		: mReloadTool(NULL)
#endif
	{
		CreateSaver();
	}
	~LocalDataSource();

#if LOCAL_DB_USE_THREAD
protected:
	LocalFileDBSaver	*mReloadTool;		// Ϊ�˶���ͬ��, ����һ�����¼��ؼ�¼�ĸ���, ����ֻ��ʹ�� ReloadRecord
#endif
};
//-------------------------------------------------------------------------

#endif //_INCLUDE_DATASOURCE_H_