
#ifndef _INCLUDE_MYSQLDBSAVER_H_
#define _INCLUDE_MYSQLDBSAVER_H_

#include "DBSaver.h"
#include "MySqlDBTool.h"
#include "MemoryDBHead.h"
#include "BaseTable.h"

class MemoryDB_Export  MySqlDBSaver : public tDBSaver
{
public:
	virtual ~MySqlDBSaver();

public:
	virtual bool Start(NiceData &initParam);

	virtual void Log(const char *szInfo, ...);
	virtual const char* GetDBName() { return mDBName.c_str(); }
	virtual bool IsStringKey();
	// �½�DB��
	virtual bool InitReadyNewDB(const char *szDBName, tBaseTable *memoryTable, AString tableData);

	// ���뱣֤��ʹ��ͬһ�߳�ִ��
	virtual void Process() override 
	{
		mMySqlDB.Process();
	}

public:
	virtual bool SaveData(Int64 keyID, void *pData, DSIZE size)
	{
		return SaveData(STRING(keyID), pData, size);
	}
	virtual bool SaveData(const char *szKeyID, void *pData, DSIZE size);

	virtual bool LoadData(Int64 keyID, DataStream *destData)
	{
		return LoadData(STRING(keyID), destData);
	}
	virtual bool LoadData(const char *szKeyID, DataStream *destData);

	// ����MemoryDB ��ʼ�����������ݼ�¼
	virtual bool LoadAllData(LoadDataCallBack &loadCallBack, AString &errorInfo);

	virtual bool DelectData(const char *szKeyID, UInt64 extValue) override;

public:
	MySqlDBTool		mMySqlDB;

	AString			mDBName;		// ����Ӧ�ı������
	AString			mMainKeyName; // Ĭ�ϵ�1�������ֶ���
	AutoRecord		mDBRecord;
	ABaseTable		mDBTempTable;
};
//-------------------------------------------------------------------------
// ʹ��MYSQL DB �ṹ��ʽ�洢
// NOTE: //!!! ��δ֧�ֱ���
//-------------------------------------------------------------------------
class DBTableSaver : public MySqlDBSaver
{
public:
	virtual bool InitReadyNewDB( const char *szDBName, tBaseTable *memoryTable, AString tableData )
	{
		FieldInfo info = memoryTable->GetField()->getFieldInfo(memoryTable->GetMainIndex()->IndexFieldName());
		if (info==NULL)
		{
			ERROR_LOG("[%s]�����ֶ�Ϊ��", szDBName);
			return false;
		}

		if (!mMySqlDB.CreateDBTable(szDBName, memoryTable->GetField(), tableData.c_str()))
		{
			ERROR_LOG("�������ݱ��ʧ��>[%s], ���ܱ���Ѿ�����, ��ȷ����պ����� SQL ERROR:[%s]", szDBName, mMySqlDB.getErrorMsg());
			return false;
		}
		return true;
	}


	virtual bool Start( NiceData &pInitParam )
	{
		mDBName = pInitParam.get(DBNAME).string();
		AssertNote(!mDBName.empty(), "DB����Ϊ��, ��Ҫ�ṩ���� [DBNAME]");
		if (mMySqlDB.InitStart(pInitParam))
		{
			AString sql;
			sql.Format("SELECT * from `%s` LIMIT 0", mDBName.c_str());
			if (mMySqlDB.exeSql(sql, true, NULL) )
			{
				AutoField hField;
				if (mMySqlDB.InitField(hField, true))
				{		
					mDBTempTable = tBaseTable::NewBaseTable();
					mDBTempTable->InitField(hField);
					mDBRecord = mDBTempTable->NewRecord();//MEM_NEW DataRecord(hField);
					//mDBRecord->_alloctData();
					mDBTempTable->AppendRecord(mDBRecord, true);
					Log("�ɹ�׼�� �洢 [%s]", mDBName.c_str());
					if (hField->getCount()>0)
						mMainKeyName = hField->getFieldInfo(0)->getName();
					return true;
				}
			}
		}

		Log("׼�� �洢 [%s]ʧ��, MySql ����ʧ��, �� ��ȡ�洢���ʧ��, ����SQL���Ƿ���� [%s]��", mDBName.c_str(), mDBName.c_str());
		return false;
	}
};
//-------------------------------------------------------------------------
// ��������DB�б���
class DBListMySqlSaver : public DBTableSaver
{
	// ��ҪDATA ʹ��text ������MYSQL��������ֱ�Ӳ鿴�޸�	
	virtual bool InitReadyNewDB( const char *szDBName, tBaseTable *memoryTable, AString tableData ) override
	{
		FieldInfo info = memoryTable->GetField()->getFieldInfo(memoryTable->GetMainIndex()->IndexFieldName());
		if (info==NULL)
		{
			ERROR_LOG("[%s]�����ֶ�Ϊ��", szDBName);
			return false;
		}
		int keyType = info->getType();
		// �ȴ������ݱ�
		AString sql;
		if (keyType==FIELD_STRING)
			sql.Format("CREATE TABLE `%s` ( ID char(%d) NOT NULL, DATA text(0), PRIMARY KEY (ID) ) comment = '%s'", szDBName, 64, tableData.c_str());
		else
			sql.Format("CREATE TABLE `%s` ( ID bigint(%d) NOT NULL, DATA text(0), PRIMARY KEY (ID) ) comment = '%s'", szDBName, 20, tableData.c_str());

		if (mMySqlDB.exeSql(sql, false))
		{
			//// �������ݱ���Ϣ����
			//SaveData(szDBName, pData, size);
		}
		else
		{
			ERROR_LOG("�������ݱ��ʧ��>[%s], ���ܱ���Ѿ�����, ��ȷ����պ����� SQL ERROR:[%s]", szDBName, mMySqlDB.getErrorMsg());
			return false;
		}
		return true;
	}
};
//-------------------------------------------------------------------------*/

#endif //_INCLUDE_MYSQLDBSAVER_H_