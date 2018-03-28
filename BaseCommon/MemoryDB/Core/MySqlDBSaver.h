
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
	// 新建DB表
	virtual bool InitReadyNewDB(const char *szDBName, tBaseTable *memoryTable, AString tableData);

	// 必须保证在使用同一线程执行
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

	// 用于MemoryDB 初始加载所有数据记录
	virtual bool LoadAllData(LoadDataCallBack &loadCallBack, AString &errorInfo);

	virtual bool DelectData(const char *szKeyID, UInt64 extValue) override;

public:
	MySqlDBTool		mMySqlDB;

	AString			mDBName;		// 即对应的表格名称
	AString			mMainKeyName; // 默认第1列主键字段名
	AutoRecord		mDBRecord;
	ABaseTable		mDBTempTable;
};
//-------------------------------------------------------------------------
// 使用MYSQL DB 结构方式存储
// NOTE: //!!! 尚未支持备份
//-------------------------------------------------------------------------
class DBTableSaver : public MySqlDBSaver
{
public:
	virtual bool InitReadyNewDB( const char *szDBName, tBaseTable *memoryTable, AString tableData )
	{
		FieldInfo info = memoryTable->GetField()->getFieldInfo(memoryTable->GetMainIndex()->IndexFieldName());
		if (info==NULL)
		{
			ERROR_LOG("[%s]索引字段为空", szDBName);
			return false;
		}

		if (!mMySqlDB.CreateDBTable(szDBName, memoryTable->GetField(), tableData.c_str()))
		{
			ERROR_LOG("创建数据表格失败>[%s], 可能表格已经存在, 请确认清空后重试 SQL ERROR:[%s]", szDBName, mMySqlDB.getErrorMsg());
			return false;
		}
		return true;
	}


	virtual bool Start( NiceData &pInitParam )
	{
		mDBName = pInitParam.get(DBNAME).string();
		AssertNote(!mDBName.empty(), "DB名称为空, 需要提供参数 [DBNAME]");
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
					Log("成功准备 存储 [%s]", mDBName.c_str());
					if (hField->getCount()>0)
						mMainKeyName = hField->getFieldInfo(0)->getName();
					return true;
				}
			}
		}

		Log("准备 存储 [%s]失败, MySql 连接失败, 或 调取存储表格失败, 请检查SQL中是否存在 [%s]表", mDBName.c_str(), mDBName.c_str());
		return false;
	}
};
//-------------------------------------------------------------------------
// 用来保存DB列表表格
class DBListMySqlSaver : public DBTableSaver
{
	// 主要DATA 使用text 可以在MYSQL管理工具中直接查看修改	
	virtual bool InitReadyNewDB( const char *szDBName, tBaseTable *memoryTable, AString tableData ) override
	{
		FieldInfo info = memoryTable->GetField()->getFieldInfo(memoryTable->GetMainIndex()->IndexFieldName());
		if (info==NULL)
		{
			ERROR_LOG("[%s]索引字段为空", szDBName);
			return false;
		}
		int keyType = info->getType();
		// 先创建数据表
		AString sql;
		if (keyType==FIELD_STRING)
			sql.Format("CREATE TABLE `%s` ( ID char(%d) NOT NULL, DATA text(0), PRIMARY KEY (ID) ) comment = '%s'", szDBName, 64, tableData.c_str());
		else
			sql.Format("CREATE TABLE `%s` ( ID bigint(%d) NOT NULL, DATA text(0), PRIMARY KEY (ID) ) comment = '%s'", szDBName, 20, tableData.c_str());

		if (mMySqlDB.exeSql(sql, false))
		{
			//// 更新数据表信息数据
			//SaveData(szDBName, pData, size);
		}
		else
		{
			ERROR_LOG("创建数据表格失败>[%s], 可能表格已经存在, 请确认清空后重试 SQL ERROR:[%s]", szDBName, mMySqlDB.getErrorMsg());
			return false;
		}
		return true;
	}
};
//-------------------------------------------------------------------------*/

#endif //_INCLUDE_MYSQLDBSAVER_H_