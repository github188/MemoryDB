
#include "MySqlDBTool.h"

#if USE_MYSQL_API

#include <WinSock2.h>
#ifdef _WIN64
#	include "MySql/mysql.h"
#else
#	include "MySqlWin32/mysql.h"
#endif
#include "FieldIndex.h"
//#include "DBTable.h"
#include "BaseTable.h"
#include "LoadDataCallBack.h"
#include "TableUpdateLoader.h"
#include "BaseFieldInfo.h"

#include "CEvent.h"
#include "TimeManager.h"
/*/-------------------------------------------------------------------------
NOTE: 
	1 在存储过程方式返回字符串时，只能够返回最大32字节的字符串
	2 对于大于65KB BLOB 类型，还需要测试存取是否正常 (//!!!)
//-------------------------------------------------------------------------*/
#define DEBUG_LOG_SQL_INFO		0

#define IS_BLOB_TYPE(t) (t==MYSQL_TYPE_MEDIUM_BLOB || t==MYSQL_TYPE_TINY_BLOB || t==MYSQL_TYPE_BLOB || t==MYSQL_TYPE_LONG_BLOB)
//-------------------------------------------------------------------------
MySqlDBTool::MySqlDBTool()
	: mMySql(NULL)
	, mResult(NULL)
{
	mLastCheckTime = TimeManager::Now();
}

MySqlDBTool::~MySqlDBTool()
{
	mWaitTryRunEvent._free();
	clearResult();
	Stop();
}

bool MySqlDBTool::InitStart( NiceData &param )
{
	Log("开始连接 MySQL DB ...\n  %s", param.dump().c_str());

	AString valueDBIP = param.get(DBIP);
	if (valueDBIP.empty())
	{
		Log("未设置>[%s]", DBIP);
		return false;
	}
	Data valueDBPORT = param.get(DBPORT);
	if (valueDBPORT.empty())
	{
		Log("未设置>[%s]", DBPORT);
		return false;
	}
	AString valueDBUSER = param.get(DBUSER);
	if (valueDBUSER.empty())
	{
		Log("未设置>[%s]", DBUSER);
		return false;
	}
	AString valueDBPASSWORD = param.get(DBPASSWORD);
	if (valueDBPASSWORD.empty())
	{
		Log("未设置>[%s]", DBPASSWORD);
		return false;
	}

	AString valueDBBASE = param.get(DBBASE);
	if (valueDBBASE.empty())
	{
		Log("未设置>[%s]", DBBASE);
		return false;
	}

	Stop();

	mMySql = mysql_init((MYSQL*)mMySql);

	if (mMySql==NULL)
	{
		Log("初始MySql DB 失败");
		return false;
	}

	my_bool bParam = 1;
	if (mysql_options((MYSQL*)mMySql, MYSQL_OPT_RECONNECT, &bParam)!=0)
	{
		Log("设置选项失败> [%s]", _getErrorMsg());
		return false;
	}
	// NOTE: 不设置连接超时的时候, 会出现 mysql_real_connect 崩溃
	unsigned int timeout = 6;
	int ret = mysql_options((MYSQL*)mMySql, MYSQL_OPT_CONNECT_TIMEOUT, (const char *)&timeout);
	if (ret!=0) 
	{
		Log("设置连接超时选项失败> [%s]", _getErrorMsg());
		return false;
	}
	
	if (mMySql==mysql_real_connect((MYSQL*)mMySql, valueDBIP.c_str(), valueDBUSER.c_str(), valueDBPASSWORD.c_str(), valueDBBASE.c_str(), (int)valueDBPORT, NULL, 0)!=0)
		Log("MySql DB 连接成功 > [%s: %d]", valueDBIP.c_str(), (int)valueDBPORT);
	else
	{
		Log("MySql 连接错误>[%u] : [%s]", mysql_errno(mMySql), _getErrorMsg());
		return false;
	}

	if (mysql_errno(mMySql)>0)	
		Log("Error: MySql 连接失败>[%s]", _getErrorMsg());		

	// 5.0.19 版本需要放在连接后面
	if (mysql_options((MYSQL*)mMySql, MYSQL_OPT_RECONNECT, &bParam)!=0)
	{
		Log("设置选项失败> [%s]", _getErrorMsg());
		return false;
	}

	// 设置连接默认字符集
	Log("默认字符集> [%s]", mysql_character_set_name(mMySql));

	if (0!=mysql_set_character_set(mMySql, "utf8")) 
	{
		Log("设置字符集失败> [%s]", _getErrorMsg());
		return false;
	}
	Log("当前连接字符集> [%s]", mysql_character_set_name(mMySql));	

	if (0!=mysql_autocommit(mMySql, true))
	{
		Log("设置非自动提交失败", _getErrorMsg());
	}

	if (&param!=&mInitParam)
		mInitParam = param;

	return true;
}

void MySqlDBTool::Stop()
{
	clearResult();

	if (mMySql!=NULL)
	{
		mysql_close((MYSQL*)mMySql);
		mMySql = NULL;
	}
}

void MySqlDBTool::clearResult()
{
	if (mMySql==NULL)
		return;

	if (mResult!=NULL)
	{
		mysql_free_result(mResult);
		mResult = NULL;
	}

	while( 0==mysql_next_result( mMySql ) );    
	{  
		MYSQL_RES *result = mysql_store_result( mMySql );  
		if (result!=NULL)
			mysql_free_result(result);  
	}
}

bool MySqlDBTool::InitField( AutoField &hField, bool bMustAllField, Array<AString> *selectFieldList/* = NULL*/, Array<AString> *excludeFieldList/* = NULL */)
{
	if (mResult==NULL)
	{
		Log("Need run Select * from table");
		return false;
	}
	if (!hField)
		hField = MEM_NEW FieldIndex(NULL, 0);

	//AString sql = "Show full columns from ";
	//sql += szTableName;

	unsigned int num_fields;
	unsigned int i;
	MYSQL_FIELD *fields;

	num_fields = mysql_num_fields(mResult);
	fields = mysql_fetch_fields(mResult);
	int col = 0;
	for(i = 0; i < num_fields; ++i)
	{
		if (selectFieldList!=NULL && !selectFieldList->empty())
		{
			bool bHave = false;
			for (size_t s=0; s<selectFieldList->size(); ++s)
			{
				if ( (*selectFieldList)[s] == fields[i].name )
				{
					bHave = true;
					(*selectFieldList)[s].setNull();
					break;
				}
			}
			if (!bHave)
				continue;
		}
		if (excludeFieldList!=NULL && !excludeFieldList->empty())
		{
			bool bHave = false;
			for (size_t s=0; s<excludeFieldList->size(); ++s)
			{
				if ( (*excludeFieldList)[s] == fields[i].name )
				{
					bHave = true;
					(*excludeFieldList)[s].setNull();
					break;
				}
			}
			if (bHave)
				continue;
		}
		FIELD_TYPE type = FIELD_NULL;
		int length = 0;

		// NOTE: 排除字符串二进制存储方式
		if (
			(fields[i].type!=MYSQL_TYPE_STRING && fields[i].type!=MYSQL_TYPE_VAR_STRING)
			&& ( (IS_BLOB_TYPE(fields[i].type) && (fields[i].flags & BINARY_FLAG))
			|| (fields[i].flags & BINARY_FLAG) )
			)
		{			
			type = FIELD_DATA;
		}
		else
		{
			switch (fields[i].type)
			{
			case MYSQL_TYPE_TINY:
				type = FIELD_BYTE;
				break;

			case MYSQL_TYPE_SHORT:
				type = FIELD_SHORT;
				break;

			case MYSQL_TYPE_INT24:
			case MYSQL_TYPE_LONG:
				type = FIELD_INT;
				break;

			case MYSQL_TYPE_LONGLONG:
				type = FIELD_UINT64;
				break;

			case MYSQL_TYPE_DECIMAL:
			case MYSQL_TYPE_FLOAT:
			case MYSQL_TYPE_DOUBLE:

				type = FIELD_FLOAT;
				break;

			case MYSQL_TYPE_VAR_STRING:
			case MYSQL_TYPE_STRING:
			case MYSQL_TYPE_BLOB:

				type = FIELD_STRING;
				//length = fields[i].length;
				break;

				//case MYSQL_TYPE_VAR_STRING:
				//	type = FIELD_BINARY;
				//	length = fields[i].length;
				//	break;

			default:
				Log("Warn: 不支持的DB数据类型>[%d]", fields[i].type);
				if (bMustAllField)
					return false;
				continue;
				break;
			}
		}
		FieldInfo info = hField->setField(fields[i].name, type, col++);
		AssertEx(info!=NULL, "设置字段失败, type > [%d]", type);

		if (length>0)
			info->setLength(length);

		if (fields[i].max_length>0)
			info->setLength(fields[i].max_length);
	}
	hField->_updateInfo();
	return hField->check();
}

bool MySqlDBTool::LoadRecord( AutoRecord &hRecord )
{
	if (!hRecord || !hRecord->getField() || mResult==NULL)
		return false;

	bool bNeedByName = false;

	unsigned int num_fields = mysql_num_fields(mResult);
	if (hRecord->getField()->getCount()!=num_fields)
	{
		Log("Warn: 查询到的字段数目与结果记录中的不相符");
		//return false;
		bNeedByName = true;
	}

	MYSQL_ROW row = mysql_fetch_row(mResult);
	if (NULL==row)
		return false;

	unsigned long *lengths;
	lengths = mysql_fetch_lengths(mResult);

	MYSQL_FIELD *fields;
	fields = mysql_fetch_fields(mResult);

	AutoField tableField = hRecord->getField();

	for (int i=0; i<(int)num_fields; ++i)
	{
		// 如果是子表格类型, 取出数据后, 恢复到表格中
		FieldInfo info = NULL;
		if (bNeedByName)
			info = hRecord->getFieldInfo(fields[i].name);
		else
			info = tableField->getFieldInfo(i);

		if (info==NULL)
		{
			ERROR_LOG("记录字段为空 [%d]", i);
			return false;
		}

		if (info->getType()==FIELD_TABLE && IS_BLOB_TYPE(fields[i].type))
		{
			int col = i;
			if (bNeedByName)
				col = hRecord->getFieldCol(fields[i].name);

			AutoTable subTable = hRecord->GetFieldTable(col);

			if (lengths[i]<=0)
			{
				subTable->ClearAll();
				continue;
			}

			if (lengths[i]<=fields[i].length)
            {
                TempDataBuffer d( row[i], lengths[i]);
                d.setDataSize(lengths[i]);
                if (!info->RestoreFromDB(&d, subTable.getPtr()))
                {
                    ERROR_LOG("Sub table %s restore DB fail, table %s", info->getName().c_str(), hRecord->GetTable()->GetTableName());
                    return false;
                }
            }
			continue;
		}
		if (info->getType()==FIELD_NICEDATA && IS_BLOB_TYPE(fields[i].type))
		{
			int col = i;
			if (bNeedByName)
				col = hRecord->getFieldCol(fields[i].name);

			AutoNice t = hRecord->getNice(col);

			if (lengths[i]<=0)
			{
				if (t)
					t->clear();
				continue;
			}

			if (!t)
			{
				t = MEM_NEW NiceData();
				hRecord->set(col, t);
			}

			AutoData d = MEM_NEW DataBuffer();
			if (lengths[i]>0 && lengths[i]<=fields[i].length)
				d->_write( row[i], lengths[i]);

			d->seek(0);			
			
			if (!t->restore(d.getPtr()))
			{
				ERROR_LOG("记录子表恢复还原> 字段[%s] table [%s]", fields[i].name, hRecord->GetTable()->GetTableName());
				t->clear();
				return false;
			}
			continue;
		}

		// 如果是二进制数据, 同 MYSQL_TYPE_BLOB 类型处理
		if (
			 ( (fields[i].flags & BINARY_FLAG) && fields[i].type!=MYSQL_TYPE_STRING && fields[i].type!=MYSQL_TYPE_VAR_STRING )
			|| (IS_BLOB_TYPE(fields[i].type) && (fields[i].flags & BINARY_FLAG)) 
			)
		{
			int col = i;
			if (bNeedByName)
				col = hRecord->getFieldCol(fields[i].name);

			AutoData d = MEM_NEW DataBuffer();
			if (lengths[i]>0 && lengths[i]<=fields[i].length)
				d->_write( row[i], lengths[i]);

			//FieldInfo info = hRecord->getField()->getFieldInfo(col);
			//AssertEx (info!=NULL, "不应该找不到字段信息");
			//int s = 0;
			//if ( !info->set(hRecord->__getData(s), &d, typeid(AutoData)) )
			//	return false;
			if (!hRecord->_set(col, &d, typeid(AutoData)))
			{
				ERROR_LOG("结果记录设置 二进制数据 AutoData 失败, 字段[%s]", fields[i].name);
				return false;
			}
		}
		else
		{
			switch (fields[i].type)
			{
			case MYSQL_TYPE_TINY:
			case MYSQL_TYPE_SHORT:
			case MYSQL_TYPE_INT24:
			case MYSQL_TYPE_LONG:
			case MYSQL_TYPE_LONGLONG:
			case MYSQL_TYPE_DECIMAL:
			case MYSQL_TYPE_FLOAT:
			case MYSQL_TYPE_DOUBLE:				
			case MYSQL_TYPE_BLOB:			// TEXT TYPE
				{
					int col = i;

					if (bNeedByName)
						col = hRecord->getFieldCol(fields[i].name);

					if (row[i]!=NULL && lengths[i]>0)
						hRecord->_set(col, row[i]);
					else
						hRecord->_set(col, "");
					break;
				}

			case MYSQL_TYPE_VAR_STRING:	
			case MYSQL_TYPE_STRING:				
				// 如果是二进制数据, 同 MYSQL_TYPE_BLOB 类型处理
				// if (!(fields[i].flags & BINARY_FLAG))
				{
					int col = i;

					if (bNeedByName)
						col = hRecord->getFieldCol(fields[i].name);

					if (row[i]!=NULL && lengths[i]>0)
					{
						hRecord->_set(col, row[i]);
					}
					else
						hRecord->_set(col, "");

					break;
				}

			//case MYSQL_TYPE_BLOB:
			//	break;

			default:
				Log("Error: 不支持的DB数据类型>[%d]", fields[i].type);
				continue;
				break;
			}
		}
	}
	return true;
}

bool MySqlDBTool::ExeByField(const char *szTable, AutoField &hField, const AString &whereString )
{
	if (!hField /*|| hField->GetTable()==NULL*/)
		return false;

	int colNum = hField->getCount();
	AString sql = "Select ";
	for (int i=0; i<colNum; ++i)
	{
		if (i>0)
			sql += ", ";

		sql += hField->getFieldInfo(i)->getName().c_str();
	}
	sql += " from ";
	sql += szTable;

	if (whereString.length()>0)
	{
		sql += " where ";
		sql += whereString;
	}

	if ( !exeSql(sql, true) )
		return false;
	
	return true;
}

bool MySqlDBTool::exeSql( const AString &sqlString, bool bNeedLoadData, int *affectCount /*= NULL*/, bool bNeedAutoCommit/* = true*/ )
{
	clearResult();

	//Log("DB SQL : [%s]", sqlString.c_str());
	_SetAutoCommit(bNeedAutoCommit);
	if (mysql_real_query(mMySql, sqlString.c_str(), sqlString.length())!=0)
	{
		_LogSqlError(sqlString, NULL);
		return false;
	}		

	mResult = mysql_use_result((MYSQL*)mMySql);
	if (mResult!=NULL)
	{
		//num_fields = mysql_num_fields(result);
		// retrieve rows, then call mysql_free_result(result)
	}
	else  // mysql_store_result() returned nothing; should it have?
	{
		if(mysql_field_count(mMySql) == 0)
		{
			// query does not return data
			// (it was not a SELECT)
			if (affectCount!=0)
				*affectCount = (int)mysql_affected_rows(mMySql);
		}
		else // mysql_store_result() should have returned data
		{
			_LogSqlError(sqlString, NULL);
			return false;
		}
	}

	return true;
}

bool MySqlDBTool::exeSql( const AString &sqlString, Array<AutoData> &paramDataList, int *pAffectCount /*= NULL*/ )
{
	//WARN_LOG("SQL>%s", sqlString.c_str());
	// 绑定参数
	Array<MYSQL_BIND>			mBindList;
	Array<MySQLBindData>		mDataList;

	mBindList.resize(paramDataList.size());
	mDataList.resize(paramDataList.size());

	for (int i=0; i<(int)paramDataList.size(); ++i)
	{
		AutoData data = paramDataList[i];
		if (!data)
		{
			data = MEM_NEW DataBuffer();
			paramDataList[i] = data;

			WARN_LOG("WARN: 提供的参数数据为空, 当前设置为初始值 [%d]", i);
		}

		//mBindList.push_back(MYSQL_BIND());
		MYSQL_BIND &bind = mBindList[i];

		//mDataList.push_back(MySQLBindData());
		MySQLBindData &d = mDataList[i];
		d.mData = data;
		d.mLength = data->dataSize();

		memset(&bind, 0, sizeof(MYSQL_BIND));
		bind.buffer_type = MYSQL_TYPE_BLOB;
		bind.buffer = data->data();
		bind.buffer_length = d.mLength;
		bind.length = &(d.mLength);
	}

	MYSQL_BIND *pBind = NULL;
	if (mBindList.size()>0)
		pBind = &mBindList[0];
	// 执行可返回结果集
	int affectCount = 0;		

	if (!_ExeStmt(sqlString, pBind, affectCount, true, NULL))
	{
		//ERROR_LOG("ERROR: 执行操作失败 [%s]", sqlString.c_str());
		return false;
	}
	if (pAffectCount!=NULL)
		*pAffectCount = affectCount;

	return true;
}

// 存储过程中, 已支持BLOB数据返回
AutoNice MySqlDBTool::ExeSqlFunction(  const AString &sqlString, bool bAutoCommit )
{
	if (!exeSql(sqlString, true, NULL, bAutoCommit))
	{
		return AutoNice();
	}

	if (NULL==mResult)
		return AutoNice();

	MYSQL_ROW row;

	bool bNeedByName = false;

	unsigned int num_fields = mysql_num_fields(mResult);

	row = mysql_fetch_row(mResult);

	AutoNice resultData = MEM_NEW NiceData();
	if (NULL==row)
	{
		resultData->set("result", 1);
		return AutoNice();
	}


	MYSQL_FIELD *fields;
	fields = mysql_fetch_fields(mResult);

	unsigned long *lengths;
	lengths = mysql_fetch_lengths(mResult);

	for (int i=0; i<(int)num_fields; ++i)
	{
		FIELD_TYPE type = FIELD_NULL;
		switch (fields[i].type)
		{
		case MYSQL_TYPE_SHORT:
		case MYSQL_TYPE_INT24:
		case MYSQL_TYPE_LONG:
		case MYSQL_TYPE_LONGLONG:
		case MYSQL_TYPE_DECIMAL:
		case MYSQL_TYPE_FLOAT:
		case MYSQL_TYPE_DOUBLE:
		case MYSQL_TYPE_STRING:
		case MYSQL_TYPE_TINY:
		case MYSQL_TYPE_VAR_STRING:
			{				
				if (row[i]==NULL)
					resultData->set(fields[i].name, "");
				else
					resultData->set(fields[i].name, (const char*)row[i]);
			}

			break;

		case MYSQL_TYPE_BLOB:
		case MYSQL_TYPE_MEDIUM_BLOB:
		case MYSQL_TYPE_LONG_BLOB:
			{
				AutoData d = MEM_NEW DataBuffer(lengths[i]);

				if (row[i]!=NULL && lengths[i]>0)
				{
					d->_write(row[i], lengths[i]);
					//memcpy(d->data(), row[i], d->size());
					//d->setDataSize(lengths[i]);
				}
				if ( !resultData->set(fields[i].name, &d, typeid(AutoData)) )
					return false;
			}

			break;

		default:
			Log("Error: 不支持的DB数据类型>[%d]", fields[i].type);
			continue;
			break;
		}
	}	 

	clearResult();
	return resultData;

}

AutoNice MySqlDBTool::ExeSqlFunction( const AString &sqlString, Array<AutoData> &paramDataList, bool bAutoCommit )
{
	// 绑定参数
	Array<MYSQL_BIND>			mBindList;
	Array<MySQLBindData>		mDataList;

	mBindList.resize(paramDataList.size());
	mDataList.resize(paramDataList.size());

	for (int i=0; i<(int)paramDataList.size(); ++i)
	{
		AutoData data = paramDataList[i];
		if (!data)
		{
			data = MEM_NEW DataBuffer();
			paramDataList[i] = data;

			WARN_LOG("WARN: 提供的参数数据为空, 当前设置为初始值 [%d]", i);
		}

		//mBindList.push_back(MYSQL_BIND());
		MYSQL_BIND &bind = mBindList[i];

		//mDataList.push_back(MySQLBindData());
		MySQLBindData &d = mDataList[i];
		d.mData = data;
		d.mLength = data->dataSize();

		memset(&bind, 0, sizeof(MYSQL_BIND));
		bind.buffer_type = MYSQL_TYPE_BLOB;
		bind.buffer = data->data();
		bind.buffer_length = d.mLength;
		bind.length = &(d.mLength);
	}

	MYSQL_BIND *pBind = NULL;
	if (mBindList.size()>0)
		pBind = &mBindList[0];
	// 执行可返回结果集
	int affectCount = 0;

	AutoNice hResult = MEM_NEW NiceData();
	
	if (!_ExeStmt(sqlString, pBind, affectCount, bAutoCommit, dynamic_cast<NiceData*>(hResult.getPtr())))
	{
		ERROR_LOG("ERROR: 执行操作失败 [%s]", sqlString.c_str());
		return AutoNice();
	}

	return hResult;
}

SQL_RESULT MySqlDBTool::LoadBlobData( const char *szTableName, const char *szFieldName, const char *szWhereCondition, DataStream *resultData )
{
	if (szTableName==NULL || szFieldName==NULL || szWhereCondition==NULL)
		return SQL_ERROR_PARAM_ERROR;

	AString selectString;
	selectString.Format("Select %s from %s where %s", szFieldName, szTableName, szWhereCondition);

	return _LoadBlobData(selectString, resultData);
}

bool MySqlDBTool::SaveBlobData( const char *szTableName, const char *szFieldName, const char *szWhereCondition, DataStream *saveData, size_t fieldLength, bool bTryInsert )
{
	if (szTableName==NULL || szFieldName==NULL || szWhereCondition==NULL)
		return FALSE;
	
	// 先尝试更新
	AString updateString;
	updateString.Format("Update %s set %s = ? where %s", szTableName, szFieldName, szWhereCondition);

	if (TRUE==_SaveBlobData(updateString, saveData, fieldLength))
		return TRUE;

	// 再尝试进行插入
	if (bTryInsert)
	{
		AString insertString;
		insertString.Format("Insert into %s set %s, %s = ?", szTableName, szWhereCondition, szFieldName);

		return _SaveBlobData(insertString, saveData, fieldLength);
	}
	return FALSE;
}

const char* MySqlDBTool::_getErrorMsg()
{
	if (mErrorInfo.empty())
		return mysql_error((MYSQL*)mMySql);

	return mErrorInfo.c_str();
}

void MySqlDBTool::Log( const char *szInfo, ... )
{
#if DEBUG_LOG_SQL_INFO
	va_list	va;
	va_start(va, szInfo);

	LOG_TOOL(va, szInfo);
#endif
}

bool MySqlDBTool::_SaveBlobData( const AString &sqlString, DataStream *saveData, size_t fieldLength )
{
	Log("DB SQL : [%s]", sqlString.c_str());
	
	clearResult();

	MYSQL_BIND bind;
	memset(&bind, 0, sizeof(bind));

	//初始化stmt
	MYSQL_STMT *mStmt = mysql_stmt_init(mMySql);

	//预处理语句
	mysql_stmt_prepare(mStmt, sqlString.c_str(), sqlString.length());

	unsigned long size = saveData->tell();
	//初始化参数
	bind.buffer_type= MYSQL_TYPE_BLOB;
	bind.buffer= (char *)saveData->data();
	bind.buffer_length = saveData->tell();
	bind.is_null= 0;
	bind.length= &size;

	//绑定参数
	mysql_stmt_bind_param(mStmt, &bind);

	_SetAutoCommit(true);
	//执行预处理mysql语句
	if (mysql_stmt_execute(mStmt)!=0)
	{
		_LogSqlError(sqlString, mStmt);
		mysql_stmt_free_result(mStmt);
		mysql_stmt_reset(mStmt);
		mysql_stmt_close(mStmt);
		return false;
	}
	
	my_ulonglong affectCount = mysql_stmt_affected_rows(mStmt);
	mysql_stmt_free_result(mStmt);
	mysql_stmt_reset(mStmt);
	mysql_stmt_close(mStmt);
	return affectCount>0;
}

SQL_RESULT MySqlDBTool::_LoadBlobData( const AString &sqlString, DataStream *resultData )
{
	//Log("DB SQL : [%s]", sqlString.c_str());
	
	clearResult();

	MYSQL_BIND bind;
	memset(&bind, 0, sizeof(bind));
	
	//初始化stmt
	MYSQL_STMT *mStmt = mysql_stmt_init(mMySql);

	//预处理语句
	mysql_stmt_prepare(mStmt, sqlString.c_str(), sqlString.length());

	unsigned long size = 0;
	//初始化参数
	bind.buffer_type= MYSQL_TYPE_BLOB;
	bind.buffer= (char *)resultData->data();
	bind.buffer_length = resultData->size();
	bind.is_null= 0;
	bind.length= &size;
	_SetAutoCommit(true);
	//执行预处理mysql语句
	if (mysql_stmt_execute(mStmt)!=0)
	{
		_LogSqlError(sqlString, mStmt);
		mysql_stmt_close(mStmt);
		return SQL_EXE_ERROR;
	}
	
	//绑定结果集缓冲
	mysql_stmt_bind_result(mStmt, &bind);
	
	int re = mysql_stmt_fetch(mStmt);
	SQL_RESULT resultInfo = SQL_SUCCEED;
	if (re==0)
	{
		resultData->seek(size);
	}
	else if (re==1)
	{
		resultInfo = SQL_EXE_ERROR;
		_LogSqlError(sqlString, mStmt);
	}
	else if (re==MYSQL_NO_DATA)
	{
		resultInfo = SQL_ERROR_NO_DATA;
	}
	else if (re = MYSQL_DATA_TRUNCATED)
	{
		resultInfo = SQL_DATA_TRUNCATED;
	}

	mysql_stmt_free_result(mStmt);
	mysql_stmt_reset(mStmt);
	mysql_stmt_close(mStmt);

	return resultInfo;

}

void _LinkValueAtCol(const char *szFieldName, const char *val, AString &valueString)
{
	valueString += szFieldName;
	valueString += "=";
	valueString += val;
}

bool MySqlDBTool::SaveRecord( BaseRecord *scrRecord, bool bUpdate, bool bTryInsert )
{
	AString sql;

	Array<AutoData>	dataList;

	if (bUpdate)
	{
		_MakeSaveSqlData(sql, dataList, scrRecord->GetSelf(), false);
		int affect = 0;
		exeSql(sql, dataList, &affect);
		if (affect>0)
			return true;
	}
	if (bTryInsert)
	{
		_MakeSaveSqlData(sql, dataList, scrRecord->GetSelf(), true);
		int affect = 0;
		exeSql(sql, dataList, &affect);
		if (affect>0)
			return true;
	}
	return false;
	//AString valueString;

	//Array<MYSQL_BIND>			mBindList;
	//Array<MySQLBindData>		mDataList;

	//for (int i=0; i<scrRecord->GetFieldNum(); ++i)
	//{
	//	if (scrRecord->HadChanged(i))
	//	{
	//		FieldInfo info = scrRecord->getFieldInfo(i);
	//		if (valueString.length()>0)
	//			valueString += ", ";

	//		valueString += info->getName().c_str();
	//		valueString += " = ";
	//		if (info->getType()==FIELD_BINARY || info->getType()==FIELD_DATA)
	//		{
	//			valueString += "?";

	//			AutoData data;
	//			bool b = scrRecord->get(i, &data, typeid(data));
	//			if (!b || !data)
	//			{
	//				data = MEM_NEW DataBuffer();
	//				Log("WARN: 提供的参数数据为空, 当前使用初始值");
	//			}
	//			//AssertEx(b&&data, "获取数据为空");

	//			mBindList.push_back(MYSQL_BIND());
	//			MYSQL_BIND &bind = mBindList.back();

	//			mDataList.push_back(MySQLBindData());
	//			MySQLBindData &d = mDataList.back();
	//			d.mData = data;
	//			d.mLength = data->dataSize();

	//			memset(&bind, 0, sizeof(MYSQL_BIND));
	//			bind.buffer_type = MYSQL_TYPE_BLOB;
	//			bind.buffer = data->data();
	//			bind.buffer_length = d.mLength;
	//			bind.length = &(d.mLength);
	//		}
	//		else
	//		{
	//			_LinkValueAtCol(info->getName(), scrRecord->get(i).c_str(), valueString);
	//			//bool b = scrRecord->_LinkValueAtCol(info, i, valueString);
	//			//AssertEx(b, "连接记录数据值失败> [%s]", info->getName().c_str());
	//			//AssertEx(0, "未实现完成");
	//		}
	//	}
	//}

	//MYSQL_BIND *pBind = NULL;
	//if (mBindList.size()>0)
	//	pBind = &mBindList[0];

	//if (bUpdate)
	//{
	//	AString sqlString;
	//	sqlString.Format("Update %s set ",
	//		scrRecord->GetTable()->GetTableName() );
	//
	//	sqlString += valueString;
	//	sqlString += " where ";
	//	sqlString += scrRecord->GetTable()->GetMainIndex()->IndexFieldName();
	//	sqlString += " = ";
	//	int nIndexCol = scrRecord->GetTable()->GetMainIndexCol();
	//	_LinkValueAtCol(scrRecord->getFieldInfo(nIndexCol)->getName(), scrRecord->get(nIndexCol).c_str(), sqlString);
	//	//bool b = scrRecord->_LinkValueAtCol(scrRecord->getFieldInfo(nIndexCol), nIndexCol, sqlString);
	//	//AssertEx(b, "链接 Update 语句条件值时失败");
	//	AssertEx(0, "未实现完成");
	//	int affectCount = 0;
	//	
	//	if (_ExeStmt(sqlString, pBind, affectCount, true))
	//	{
	//		if (affectCount>=1)
	//			return true;
	//	}
	//}

	//if (bTryInsert)
	//{
	//	AString sqlString;
	//	sqlString.Format("Insert into %s set ",
	//		scrRecord->GetTable()->GetTableName());

	//	if (!scrRecord->HadChanged(scrRecord->GetTable()->GetMainIndexCol()))
	//	{
	//		sqlString += scrRecord->GetTable()->GetMainIndex()->IndexFieldName();
	//		sqlString += " = ";
	//		int nIndexCol = scrRecord->GetTable()->GetMainIndexCol();
	//		_LinkValueAtCol(scrRecord->getFieldInfo(nIndexCol)->getName(), scrRecord->get(nIndexCol).c_str(), sqlString);
	//		//bool b = scrRecord->_LinkValueAtCol(scrRecord->getFieldInfo(nIndexCol), nIndexCol, sqlString);
	//		AssertEx(0, "未实现完成");
	//		if (valueString.length()>0)
	//			sqlString += ", ";
	//	}

	//	sqlString += valueString;

	//	int affectCount = 0;
	//	
	//	if (_ExeStmt(sqlString, pBind, affectCount, true))
	//	{
	//		if (affectCount==1)
	//			return true;
	//	}
	//}

	return false;
}

bool MySqlDBTool::_ExeStmt( const AString &sqlString, st_mysql_bind *pBindList, int &affectCount, bool bNeedAutoCommit, NiceData *pResultData )
{
	
	//Log("DB SQL : [%s]", sqlString.c_str());

	clearResult();

	//初始化stmt
	MYSQL_STMT *mStmt = mysql_stmt_init(mMySql);

	//预处理语句
	if ( 0!=mysql_stmt_prepare(mStmt, sqlString.c_str(), sqlString.length()) )
	{
		_LogSqlError(sqlString, mStmt);

		mysql_stmt_free_result(mStmt);
		mysql_stmt_reset(mStmt);
		mysql_stmt_close(mStmt);
		return false;
	}

	//绑定参数
	if (pBindList!=NULL)
		mysql_stmt_bind_param(mStmt, pBindList);

	_SetAutoCommit(bNeedAutoCommit);
	//执行预处理mysql语句
	if (mysql_stmt_execute(mStmt)!=0)
	{
		_LogSqlError(sqlString, mStmt);
		mysql_stmt_free_result(mStmt);
		mysql_stmt_reset(mStmt);
		mysql_stmt_close(mStmt);
		return false;
	}

	affectCount = (int)mysql_stmt_affected_rows(mStmt);

	bool bResult = true;
	if (pResultData)
	{
		bResult = _getDataResult(mStmt, pResultData);
		if (!bResult)
		{
			ERROR_LOG("ERROR: 获取执行结果失败");				
		}		
	}
	else
	{
		_ClearMysqlResult(mysql_stmt_result_metadata(mStmt), mStmt);
	}

	mysql_stmt_free_result(mStmt);
	mysql_stmt_reset(mStmt);
	mysql_stmt_close(mStmt);

	return bResult;
}

void MySqlDBTool::_LogSqlError( const AString &sqlString, st_mysql_stmt *pStamt )
{
	if (pStamt!=NULL)
		mErrorInfo.Format("MySql SQL Error>[%s]>[%u][%s]", sqlString.c_str(), mysql_stmt_errno(pStamt), mysql_stmt_error(pStamt));
	else
		mErrorInfo.Format("MySql SQL Error>[%s]>[%u][%s]", sqlString.c_str(), mysql_errno(mMySql), mysql_error(mMySql));

	ERROR_LOG(mErrorInfo.c_str());
}

struct TempData
{
	char mData[33];
	unsigned long mLength;
};

bool MySqlDBTool::_getDataResult( st_mysql_stmt *mStmt, NiceData *resultData )
{
	if (resultData==NULL)
		return false;

	st_mysql_res *pResult = mysql_stmt_result_metadata(mStmt);
	if (pResult==NULL)
		return false;

	unsigned int num_fields = mysql_num_fields(pResult);

	if (num_fields<=0)
	{
		resultData->set("result", 1);
		
		_ClearMysqlResult(pResult, mStmt);

		return true;
	}

	MYSQL_FIELD *fields;
	fields = mysql_fetch_fields(pResult);

	//unsigned long *lengths;
	//lengths = mysql_fetch_lengths(pResult);

	Array<MYSQL_BIND>			mBindList;
	Array<TempData>				mDataList;

	mBindList.resize(num_fields);
	mDataList.resize(num_fields);

	memset(&mBindList[0], 0, sizeof(MYSQL_BIND)*mBindList.size());

	// 绑定结果
	for (int i=0; i<(int)num_fields; ++i)
	{
		if (IS_BLOB_TYPE(fields[i].type))
		{
			AData &d = resultData->getAData(fields[i].name);
			AutoData data = MEM_NEW DataBuffer(fields[i].length);
			d.set(&data, typeid(data));
			mBindList[i].buffer_type = MYSQL_TYPE_BLOB;
			mBindList[i].buffer = (char *)data->data();
			mBindList[i].buffer_length = fields[i].length;

			mDataList[i].mLength = fields[i].length;
		}
		else
		{
			mBindList[i].buffer_type = MYSQL_TYPE_STRING;
			mBindList[i].buffer= mDataList[i].mData;
			mBindList[i].buffer_length = 32;

			mDataList[i].mLength = 32;
			mDataList[i].mData[32] = '\0';
		}
		mBindList[i].length= &(mDataList[i].mLength);
	}

	if (mysql_stmt_bind_result(mStmt, &mBindList[0])!=0)
	{
		_LogSqlError("STMT", mStmt);
	}
	/* Now buffer all results to client */
	else if (mysql_stmt_store_result(mStmt)!=0)
	{
		_LogSqlError("STMT", mStmt);
	}	
	else if (mysql_stmt_fetch(mStmt)==0)
	{
		for (int i=0; i<(int)num_fields; ++i)
		{
			//FIELD_TYPE type = FIELD_NULL;
			if (!IS_BLOB_TYPE(fields[i].type))
			{
				resultData->set(fields[i].name, mDataList[i].mData);
			}	
		}
	}
	else
	{
		_LogSqlError("STMT", mStmt);
	}

	_ClearMysqlResult(pResult, mStmt);

	return true;
}

void MySqlDBTool::_ClearMysqlResult( st_mysql_res *pResult, st_mysql_stmt *mStmt )
{
	if (pResult!=NULL)
		mysql_free_result(pResult);
	while( 0==mysql_next_result( mMySql ) );    
	{
		pResult = mysql_stmt_result_metadata(mStmt);
		mysql_stmt_store_result(mStmt);
		if (pResult!=NULL)
			mysql_free_result(pResult);
	}
}

void MySqlDBTool::_SetAutoCommit( bool bAuto )
{
	if (mMySql!=NULL)
	{
		if (0!=mysql_autocommit(mMySql, bAuto))
		{
			mErrorInfo = "";
			Log("设置非自动提交失败", _getErrorMsg());
		}
	}
}

bool MySqlDBTool::LoadData( int keyCol, int dataCol, LoadDataCallBack *callBack, DataStream *pDestData  )
{
	LoadDataCallBack &loadCallBack = *callBack;

	unsigned int num_fields = mysql_num_fields(mResult);
	if (dataCol>=(int)num_fields || keyCol>=(int)num_fields)
	{
		Log("Warn: 查询到的字段数目与结果记录中的不相符");
	}

	MYSQL_ROW row = mysql_fetch_row(mResult);
	if (NULL==row)
		return false;

	unsigned long *lengths;
	lengths = mysql_fetch_lengths(mResult);

	MYSQL_FIELD *fields;
	fields = mysql_fetch_fields(mResult);

	int i = dataCol;
	// 如果是二进制数据, 同 MYSQL_TYPE_BLOB 类型处理
	if ((fields[i].flags & BINARY_FLAG) 
		|| (IS_BLOB_TYPE(fields[i].type) && (fields[i].flags & BINARY_FLAG)) )
	{			
		loadCallBack.run(row[keyCol], row[i], lengths[i]);				
		if (pDestData!=NULL)
		{
			pDestData->resize(lengths[i]);
			pDestData->seek(0);
			pDestData->_write(row[i], lengths[i]);
		}
	}
	else
	{
		switch (fields[i].type)
		{
		case MYSQL_TYPE_TINY:
		case MYSQL_TYPE_SHORT:
		case MYSQL_TYPE_INT24:
		case MYSQL_TYPE_LONG:
		case MYSQL_TYPE_LONGLONG:
		case MYSQL_TYPE_DECIMAL:
		case MYSQL_TYPE_FLOAT:
		case MYSQL_TYPE_DOUBLE:
		case MYSQL_TYPE_VAR_STRING:		
		case MYSQL_TYPE_BLOB:			// TEXT TYPE
			{						
				if (row[i]!=NULL && lengths[i]>0)
					loadCallBack.run(row[keyCol], row[i], lengths[i]);
				else
					loadCallBack.run(row[keyCol], "", 0);

				if (pDestData!=NULL)
				{
					pDestData->resize(lengths[i]);
					pDestData->seek(0);
					pDestData->_write(row[i], lengths[i]);
				}
				break;
			}

		case MYSQL_TYPE_STRING:
			// 如果是二进制数据, 同 MYSQL_TYPE_BLOB 类型处理
			if (!(fields[i].flags & BINARY_FLAG))
			{						
				if (row[i]!=NULL && lengths[i]>0)
				{
					loadCallBack.run(row[keyCol], row[i], lengths[i]);
				}
				else
					loadCallBack.run(row[keyCol], "", 0);
				if (pDestData!=NULL)
				{
					pDestData->resize(lengths[i]+1);
					pDestData->seek(0);
					pDestData->_write(row[i], lengths[i]);
					pDestData->write('\0');
				}
				break;
			}

		default:
			ERROR_LOG("Error: 不支持的DB数据类型>[%d]", fields[i].type);
			return false;
		}
	}

	return true;
}

AutoTable MySqlDBTool::LoadDBTable( const char *szTableName, bool bLoadAllRecord )
{
	AutoTable resultTable;

	AString sql;
	if (bLoadAllRecord)
		sql.Format("SELECT * from `%s`", szTableName);
	else
		sql.Format("SELECT * from `%s` LIMIT 0", szTableName);
	if (exeSql(sql, true))
	{
		AutoField field;
		if (InitField(field, true))
		{
			resultTable = tBaseTable::NewBaseTable();
			resultTable->InitField(field);

			if (bLoadAllRecord)
			{			
				ARecord newRe = resultTable->NewRecord();
				while (LoadRecord(newRe))
				{
					resultTable->AppendRecord(newRe, true);
					newRe = resultTable->NewRecord();
				}
			}
			return resultTable;
		}
	}

	return resultTable;
}

bool MySqlDBTool::CreateDBTable( const char *szTableName, AutoField tableField, const char *szInfoData )
{
	AString sql;

	int count = tableField->getCount();
	if (count<=0)
	{
		ERROR_LOG("字段数量为零");
		return false;
	}
	for (int i=0; i<count; ++i)
	{
		if (i>0)
			sql += ", ";
			
		FieldInfo info = tableField->getFieldInfo(i);
		sql += "`";
		sql += info->getName().c_str(); 
		sql += "` ";

		const char *szType = "int";
		int nLen = 4;
		switch (info->getType())
		{
		case FIELD_BOOL:
		case FIELD_BYTE:
			szType = "tinyint";
			nLen = 4;
			break;

		case FIELD_SHORT:
			szType = "smallint";
			nLen = 6;
			break;

		case FIELD_INT:
			szType = "int";
			nLen = 11;
			break;

		case FIELD_UINT64:
			szType = "bigint";
			nLen = 20;
			break;

		case FIELD_FLOAT:
			szType = "float";
			nLen = 16;
			break;

		case FIELD_STRING:
			{			
				szType = "char";
				StringFieldInfo *p = dynamic_cast<StringFieldInfo*>(info);
				AssertEx(p!=NULL, "Must is StringFieldInfo");
				nLen = p->getMaxLength();
				if (nLen>0xFF)
					szType = "varchar";
			}
			break;

		case FIELD_DATA:
		case FIELD_TABLE:
//		case FIELD_BINARY:
		case FIELD_NICEDATA:
			szType = "blob";
			nLen = info->getMaxLength();

			break;

		default:
			ERROR_LOG("不支持MYSQL DB 表格字段类型 >[%s]", info->getTypeString());
			return false;
		}

		sql += szType;
		sql += "(";
		sql += nLen;
		sql += ") ";

		// 使用大小写区分比对
		if (info->getType()==FIELD_STRING)
			sql += " binary";

		if (i==0)
		{			
			sql += " NOT NULL ";
		}
		sql += " comment '";		
		sql += info->getTypeString();
		AString ext = info->getTypeParam();
		if (ext.length()>0)
		{
			sql +="-{";
			sql += ext;
			sql += "}'";
		}
		else		
			sql += "'";
	}
	// 表的注释只保存基本信息，不包括字段信息
	// NOTE :  ENGINE=MyISAM 必须此选项时, 才不会出现 错误: Row size too large (> 8126)
	AString sqlString;
	sqlString.Format("CREATE TABLE `%s` ( %s, PRIMARY KEY (%s) ) ENGINE=MyISAM DEFAULT CHARSET=utf8 comment  = '%s'", 
		szTableName, 
		sql.c_str(), 
		tableField->getFieldInfo(0)->getName(),
		szInfoData
		);

	//sql += ", PRIMARY KEY (";
	//sql += tableField->getFieldInfo(0)->getName();
	//sql += ") ) DEFAULT CHARSET=utf8 comment = ''";
	
	if (exeSql(sqlString, false))
	{
		return true;
		//sqlString.Format("alter table `%s` comment  = '%s'", szTableName, szInfoData);
		//int count = 0;
		//return exeSql(sqlString, false, &count);
	}
	return false;
}

void MySqlDBTool::Process()
{
	if (TimeManager::Now()-mLastCheckTime>MYSQL_TRY_RUN_TIME)
	{
		mLastCheckTime = TimeManager::Now();
		CheckConnect();
	}
}

#define  SQL_CHAR_BUFFER	(4096)

bool MySqlDBTool::_MakeSaveSqlData( AString &resultSQLString, Array<AutoData> &resultData, AutoRecord scrRecord, bool bInsert, bool bGrownInsert /*= false*/, const char *szTableName )
{	
	resultSQLString = "";
	if (!scrRecord->NeedUpdate())
		return false;

	AString indexString;
	AString dataSQLString;

	AutoField field = scrRecord->getField();
	int count = field->getCount();

	char	szBuffer[SQL_CHAR_BUFFER+1];
	szBuffer[0] = '\0';

	for (int i=0; i<count; ++i)
	{
		if (i>0 && !scrRecord->HadChanged(i))
			continue;

		int nowBufferPos = 0;

		AString *pStr = &dataSQLString;
		if (i==0)
		{
			pStr = &indexString;
		}

		AString &dataSQL = *pStr;

		if (!dataSQL.empty())		
		{
			//dataSQL += ", ";
			szBuffer[0] = ',';
			++nowBufferPos;
		}

		FieldInfo info = field->getFieldInfo(i);
		int len = sprintf_s(szBuffer+nowBufferPos, SQL_CHAR_BUFFER-nowBufferPos, "`%s`=", info->getName().c_str());
		nowBufferPos += len;
		//dataSQL += "`";
  //      dataSQL += info->getName().c_str();
  //      dataSQL += "`";

		switch (info->getType())
		{
		case FIELD_STRING:
			{			
				//dataSQL += "='";
				//dataSQL += scrRecord->get(i).string();
				//dataSQL += "'";
				AString d = scrRecord->get(i).string();
				len = sprintf_s(szBuffer+nowBufferPos, SQL_CHAR_BUFFER-nowBufferPos, "'%s'", d.c_str());
				nowBufferPos += len;
			}
			break;

		case FIELD_DATA:
			//		case FIELD_BINARY:
			{					
				//dataSQL += "=?";
				szBuffer[nowBufferPos] = '?';
				++nowBufferPos;

				AutoData d;
				scrRecord->get(i, &d, typeid(d));
				if (!d)
				{
					d = MEM_NEW DataBuffer(1);
					//ERROR_LOG("获取数据为空, 不支持空数据保存");
					//return false;
				}

				resultData.push_back(d);
			}
			break;

		case FIELD_TABLE:
			{					
				//dataSQL += "=?";
				szBuffer[nowBufferPos] = '?';
				++nowBufferPos;

				AutoData d;
				AutoTable t = scrRecord->getTable(i);
				if (t)
				{
					d = MEM_NEW SaveRecordDataBuffer(1024);					
					if (!info->SaveDataToDB(t.getPtr(), d.getPtr()))
					{
						ERROR_LOG("记录中的表格保存到数据失败");
						return false;
					}
					if (d->dataSize()>=64*1024)
					{
						ERROR_LOG("[%s] Field table data over 64k, can not save to blom field", info->getName().c_str());
						return false;
					}
				}
				else
					d = MEM_NEW DataBuffer(1);
				resultData.push_back(d);
			}
			break;

		case FIELD_NICEDATA:
			{					
				//dataSQL += "=?";
				szBuffer[nowBufferPos] = '?';
				++nowBufferPos;

				AutoData d;
				AutoNice niceData = scrRecord->getNice(i);
				if (niceData)
				{
					d = MEM_NEW DataBuffer(512);
					
					if (!niceData->serialize(d.getPtr(), true))
					{
						ERROR_LOG("记录中的NiceData序列化数据失败");
						return false;
					}
				}
				else
					d = MEM_NEW DataBuffer(1);
				resultData.push_back(d);
			}
			break;
		case FIELD_BYTE:
			{
				char d = (char)(byte)scrRecord->get(i);
				//const char *s = dString.c_str();
				//NOTE_LOG("[%s] >%s == %d", info->getName().c_str(), s, d);
				len = sprintf_s(szBuffer+nowBufferPos, SQL_CHAR_BUFFER-nowBufferPos, "%d", d);
				nowBufferPos += len;
			}
			break;
		case FIELD_UINT64:
			{			
				Int64 x = scrRecord->get(i);			
				AString dString = STRING(x);
				len = sprintf_s(szBuffer+nowBufferPos, SQL_CHAR_BUFFER-nowBufferPos, "%s", dString.c_str());
				nowBufferPos += len;
			}
			break;
		default:
			{			
				//dataSQL += "=";
				Data d = scrRecord->get(i);
				//dataSQL += d.string();
				AString dString = d.string();
				len = sprintf_s(szBuffer+nowBufferPos, SQL_CHAR_BUFFER-nowBufferPos, "%s", dString.c_str());
				nowBufferPos += len;
			}
		}
		szBuffer[nowBufferPos] = '\0';
		dataSQL += szBuffer;
	}

	if (dataSQLString=="")
	{
		if (scrRecord->HadChanged(0))
		{
			resultSQLString.Format("INSERT INTO `%s` SET %s", 
				szTableName!=NULL ? szTableName:scrRecord->GetTable()->GetTableName(),
				indexString.c_str()				
				);
			return true;
		}
		return false;
	}

	if (!bInsert)
	{
		resultSQLString.Format("UPDATE `%s` SET %s WHERE %s", 
			szTableName!=NULL ? szTableName:scrRecord->GetTable()->GetTableName(),
			dataSQLString.c_str(),
			indexString.c_str()
			);
	}
	else
	{
		if (bGrownInsert)
			indexString.setNull();
		else if (!indexString.empty())
			indexString += ", ";

		resultSQLString.Format("INSERT INTO `%s` SET %s %s", 
			szTableName!=NULL ? szTableName:scrRecord->GetTable()->GetTableName(),
			indexString.c_str(),
			dataSQLString.c_str()
			);
	}
	return true;
}

//-------------------------------------------------------------------------*/


//-------------------------------------------------------------------------*/


void MySqlDBTool::CheckConnect()
{
	if (mMySql!=NULL)
	{
		clearResult();
		if (mysql_ping(mMySql)!=0)
		{
			_LogSqlError("检查MYSQL连接错误", NULL);
			TableTool::red();
			ERROR_LOG("严重错误: 检查MYSQL连接错误 [%s]", getErrorMsg());
			TableTool::yellow();
			NOTE_LOG("尝试进行一次重新连接");
			Reconnect();
			//TableTool::white();
		}
	}
}

AString MySqlDBTool::GetFieldData()
{
	return AString();
}

//-------------------------------------------------------------------------*/

#endif