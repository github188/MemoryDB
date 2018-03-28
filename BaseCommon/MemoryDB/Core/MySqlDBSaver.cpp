
#include "MySqlDBSaver.h"
#include <stdarg.h>
#include "TimeManager.h"
#include "BaseTable.h"

#include <WinSock2.h>
#include "mysql/MySql.h"

bool MySqlDBSaver::Start( NiceData &pInitParam )
{
	mDBName = pInitParam.get(DBNAME).string();
	AssertEx(!mDBName.empty(), "DB����Ϊ��, ��Ҫ�ṩ���� [DBNAME]");
	if (mMySqlDB.InitStart(pInitParam))
	{
		AString sql;
		sql.Format("SELECT * from `%s` LIMIT 0", mDBName.c_str());
		if (mMySqlDB.exeSql(sql, true, NULL) )
		{
			AutoField hField;
			if (mMySqlDB.InitField(hField, true))
			{
				if (hField->getFieldCol("ID")!=0 || hField->getFieldCol("DATA")!=1)
				{
					Log("׼�� �洢 [%s]ʧ��, �洢����ֶ��� ��1�б���Ϊ [ID], ��2�б���Ϊ [DATA]", mDBName.c_str());
					return false;
				}				
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

void MySqlDBSaver::Log( const char *szInfo, ... )
{
#if DEVELOP_MODE
	va_list va;
	va_start(va, szInfo);
	LOG_TOOL(va, szInfo);
#endif
}

bool MySqlDBSaver::SaveData( const char *szKeyID, void *pData, DSIZE size )
{
#if DEVELOP_MODE
	static __declspec(thread) int sTotal = 0;
	if (++sTotal % 1000 ==0)
	{
		TABLE_LOG("Now save [%d]", sTotal);
	}
#endif
	if (size>=0xFFFF)
	{
		ERROR_LOG("�����ڳ��� 65535 ��¼���� > [%s], size [%d], table [%s]", szKeyID, size, mDBName.c_str());
		return false;
	}
	
	MYSQL_BIND bind;

	unsigned long ulLength = size;

	memset(&bind, 0, sizeof(MYSQL_BIND));
	bind.buffer_type = MYSQL_TYPE_BLOB;
	bind.buffer = pData;
	bind.buffer_length = size;
	bind.length = &(ulLength);


	AString sqlString;
	sqlString.Format("UPDATE `%s` SET DATA=? WHERE ID='%s'", mDBName.c_str(), szKeyID);
	int affectCount = 0;
	if (mMySqlDB._ExeStmt(sqlString, &bind, affectCount, true))
		if (affectCount>0)
			return true;

	sqlString.Format("INSERT INTO `%s` SET ID='%s', DATA=?", mDBName.c_str(), szKeyID);
	affectCount = 0;
	if (mMySqlDB._ExeStmt(sqlString, &bind, affectCount, true))
		if (affectCount>0)
			return true;

	return false;
}

bool MySqlDBSaver::LoadData( const char *szKeyID, DataStream *destData )
{
	AString sqlString;
	sqlString.Format("SELECT ID, DATA FROM `%s` WHERE ID='%s'", mDBName.c_str(), szKeyID);

	if (mMySqlDB.exeSql(sqlString, true))
	{
		//if (mMySqlDB.LoadRecord(mDBRecord))
		//{
		//	Data d = mDBRecord->get(1);
		//	int len = strlen(d.c_str());
		//	destData->seek(0);
		//	destData->_write((void*)d.c_str(), len+1);
		//	return true;
		//}
		//else
		LoadDataCallBack loadCallBack;
		if (mMySqlDB.LoadData(0, 1, &loadCallBack, destData))
		{
			return true;
		}
		else
			Log("ERROR: ��ȡ��¼����ʧ�� [%s]", szKeyID);
	}
	else
		Log("ERROR: ִ��SQLʧ�� > [%s], ERROR:[%s]", szKeyID, mMySqlDB.getErrorMsg());

	return false;
}

bool MySqlDBSaver::LoadAllData( LoadDataCallBack &loadCallBack, AString &errorInfo )
{
	DataBuffer	tempBuffer(1024);
	AString sqlString;
	sqlString.Format("SELECT ID, DATA FROM `%s`", mDBName.c_str());

	if (mMySqlDB.exeSql(sqlString, true))
	{
		UInt64 nowTime = TimeManager::NowTick();
		int count = 0;
		while (true)
		{			
			if (mMySqlDB.LoadData(0, 1, &loadCallBack))
			{
				++count;
				// show load info may be wait long time
				if (TimeManager::NowTick()-nowTime >= 10000)
				{
					Log("load %d in 10 sec", count);
					count = 0;
					nowTime = TimeManager::NowTick();
				}
			}
			else
				break;
		}
	}
	else
	{
		errorInfo.Format("����: ִ��SQLʧ��>[%s]", mMySqlDB.getErrorMsg());
		return false;
	}
	return true;
}

bool MySqlDBSaver::InitReadyNewDB( const char *szDBName, tBaseTable *memoryTable, AString tableData )
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
		sql.Format("CREATE TABLE `%s` ( ID char(%d) NOT NULL, DATA blob(0), PRIMARY KEY (ID) ) comment = '%s'", szDBName, 64, tableData.c_str());
	else
		sql.Format("CREATE TABLE `%s` ( ID bigint(%d) NOT NULL, DATA blob(0), PRIMARY KEY (ID) ) comment = '%s'", szDBName, 20, tableData.c_str());

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

bool MySqlDBSaver::DelectData( const char *szKeyID, UInt64 extValue )
{
	AString sql;
	sql.Format("DELETE FROM `%s` WHERE `%s` = '%s'", mDBName.c_str(), mMainKeyName.c_str(), szKeyID);
	int affect = 0;
	if (mMySqlDB.exeSql(sql, false, &affect))
	{
		if (affect<=0)
		{
			TABLE_LOG("ɾ����KEY���ܲ�����[%s] in [%s]", szKeyID, mDBName.c_str());
		}
		return affect>0;
	}
	ERROR_LOG("SQL ִ�д���>[%s]", mMySqlDB.getErrorMsg());
	return false;
}

MySqlDBSaver::~MySqlDBSaver()
{
	mDBRecord.setNull();
	if (mDBTempTable)
		mDBTempTable->ClearAll();
}

bool MySqlDBSaver::IsStringKey()
{
	return mDBTempTable->GetMainIndex()->IsStringHashKey();
}
