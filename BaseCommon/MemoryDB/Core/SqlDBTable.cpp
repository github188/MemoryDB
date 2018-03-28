#include "SqlDBTable.h"
#include "DataSource.h"
#include "DataSourceOperate.h"


void SqlDBTable::LoadRecordData( const char *szKey, AString fieldInfo, DBResultCallBack callBack )
{
	if (mDataSource==NULL)
	{
		ERROR_LOG("DB [%s] 数据源为空", GetTableName());
		callBack(NULL, false);
		return;
	}
	AString sql;
	if (fieldInfo=="")
	{
		if (mMainKeyIndex->IsStringHashKey())
			sql.Format("Select * from %s WHERE ID = `%s`", szKey);
		else
			sql.Format("Select * from %s WHERE ID = %s", szKey);
	}
	else
	{
		Array<AString> strList;
		AString::Split(fieldInfo.c_str(), strList, " ", 100);

		AutoField field = GetField();

		AString fieldString;
		for (size_t i=0; i<strList.size(); ++i)
		{
			if (field->getFieldInfo(strList[i].c_str())==NULL)
			{
				ERROR_LOG("Field [%s] no exist at table [%s]", strList[i].c_str(), GetTableName());
				callBack(NULL, false);
				return;
			}

			if (!fieldString.empty())
				fieldString += ", ";
			fieldString += strList[i].c_str();
		}

		if (mMainKeyIndex->IsStringHashKey()) 
			sql.Format("Select %s from %s WHERE ID = `%s`", fieldString.c_str(), GetTableName(), szKey);
		else
			sql.Format("Select %s from %s WHERE ID = %s", fieldString.c_str(), GetTableName(), szKey);
	}
	DataSource *pSource = dynamic_cast<DataSource*>(mDataSource);
	Auto<DB_RunSqlGetData_SQL> runSql = pSource->StartTask(eRunSqlGetData);
	runSql->mSqlString = sql;
	runSql->mKeyID = szKey;
	runSql->mFinishCall = callBack;
}

void SqlDBTable::SaveRecordData( const char *szKey, AutoNice reData, bool bNewInsert, DBResultCallBack callBack )
{
	tNiceData &recordData = *reData;

	if (mDataSource==NULL || !mTempRecord)
	{
		ERROR_LOG("DB [%s] 数据源为空", GetTableName());
		callBack(NULL, false);
		return;
	}
	Auto<MemoryDBRecord> r = mTempRecord;
	r->SetNewInsert(bNewInsert);
	AutoField field = GetField(); 
	if (szKey==NULL)
	{
		int nLast = 0;
		
		ARecord lastRe = mMainKeyIndex->GetLastRecord();
		if (lastRe)
			nLast = lastRe->getIndexData();

		mTempRecord->InitData();
		mTempRecord->FullAllUpdate(false);
		mTempRecord->_set(0, (Int64)nLast+1);
		r->_setNeedUpdate(0, true);
		r->SetNewInsert(true);

		ARecord newRe = CreateRecord((Int64)nLast+1, false);
		if (!newRe)
		{
			ERROR_LOG("应该不存在重复");
		}
	}
	else
	{
		mTempRecord->set(0, szKey);
		mTempRecord->FullAllUpdate(false);
	}

	for (auto it=recordData.begin(); it->have(); it->next())
	{
		const EasyString &key = it->key();
		if (key==mMainKeyIndex->IndexFieldName())
		{
			ERROR_LOG("不可以修改记录KEY值");
			continue;
		}
		FieldInfo info = field->getFieldInfo(key.c_str());
		if (info==NULL)
		{
			ERROR_LOG("Field [%s] no exist at table [%s]", key.c_str(), GetTableName());
			callBack(NULL, false);
			return;
		}

		if (mTempRecord->set(key.c_str(), it->get().get()))
			mTempRecord->NotifyChanged(key.c_str());
		else
		{
			ERROR_LOG("Set record data fail [%s], type [%s]", key.c_str(), info->getTypeString());
			callBack(NULL, false);
			return;
		}		
	}

	if (mTempRecord->NeedUpdate())
	{			
		// 同步到DB
		AString key = mTempRecord->get(mMainIndexCol);
		if (!mDataSource->SaveRecordData(key.c_str(), mTempRecord.getPtr()))				
		{
			ERROR_LOG("记录[%s]保存到数据源失败", mTempRecord->getIndexData().string().c_str());			
			callBack(NULL, false);
		}	
		else
		{
			if (szKey==NULL)
				recordData.set(mMainKeyIndex->IndexFieldName(), (int)mTempRecord->get(0));

			callBack(reData, true);
		}

		//mTempRecord->FullAllUpdate(false);
	}
}
