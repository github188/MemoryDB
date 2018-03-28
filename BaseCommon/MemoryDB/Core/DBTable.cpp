#include "DBTable.h"
#include "MySqlDBTool.h"

void DBRecord::Update()
{
	_Update();
	if (NeedUpdate())
	{
		DBTable *pTable =dynamic_cast<DBTable*>( GetTable() );
		if (pTable==NULL)
		{
			ERROR_LOG("Owner table is null, or is not DBTable");
			return;
		}
		if (pTable->mDBTool->SaveRecord(this, true, false))
			FullAllUpdate(false);
	}
}

DBTable::DBTable() : SkipBaseTable(eInitCreateField)
{

}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
void DBTable::Init( AutoDBTool dbTool, const char* szDBTableName )
{
	ClearAll();
	if (mOwnerPtr->mField)
	{
		mOwnerPtr->mpTable = NULL;
		mOwnerPtr = MEM_NEW TablePtr();
		mOwnerPtr->mpTable = this;
	}	
	SetTableName(szDBTableName);
	mDBTool = dbTool;	
}

bool DBTable::CreateToDB()
{
	if (mDBTool && GetField() && GetField()->check())
	{
		Auto<MySqlDBTool> t = mDBTool;
		if (mDBTool)
			return t->CreateDBTable(GetTableName(), GetField());
		else
			ERROR_LOG("Now only mysql tool can create table to DB");
	}
	return false;
}

bool DBTable::LoadDB( bool bLoadAllRecord /*= false*/, bool bLoadField )
{
	if (bLoadField)
	{	
		if (mOwnerPtr->mField)
		{
			mOwnerPtr->mpTable = NULL;
			mOwnerPtr = MEM_NEW TablePtr();
			mOwnerPtr->mpTable = this;
		}	
		mOwnerPtr->mField = MEM_NEW FieldIndex(this, 0);
	}
	if (mDBTool && GetField())
	{
		AString sql;
		if (bLoadAllRecord)
			sql.Format("SELECT * from `%s`", GetTableName());
		else
			sql.Format("SELECT * from `%s` LIMIT 0", GetTableName());

		if (!mDBTool->exeSql(sql, true))
			return false;

		if (bLoadField)
		{		
			if ( !mDBTool->InitField(mOwnerPtr->mField, true) )
				return false;
		}
		SetMainIndex(0, true);
		if (bLoadAllRecord)
		{		
			while (true)
			{
				ARecord r = NewRecord();
				if (mDBTool->LoadRecord(r))
				{			
					AppendRecord(r, true);
					r->FullAllUpdate(false);
				}
				else
					break;
			}
		}
		return true;
	}
	return false;
}

bool DBTable::SaveTable(bool bTryInsert)
{
	if (!mDBTool)
	{
		TABLE_LOG("DB tool is null >[%s]", GetTableName());
		return false;
	}
	bool bResout = true;
	for (ARecordIt it = GetRecordIt(); *it; ++(*it) )
	{
		ARecord r = *it;
		if (r && r->NeedUpdate())
		{
			bool bRe = mDBTool->SaveRecord(r.getPtr(), true, bTryInsert);
			if (!bRe)
			{
				ERROR_LOG("Update record fail >[%s]", r->getIndexData().string());
				//return false;
				bResout = false;
			}
			else
				r->FullAllUpdate(false);
		}
	}
	return bResout;
}

ARecord DBTable::CreateRecord( float fIndex, bool bReplace )
{
	ARecord r = NewRecord();
	int col = GetField()->getFieldCol(mMainKeyIndex->IndexFieldName());
	r->_set(col, fIndex);
	if (AppendRecord(r, bReplace))
	{
		if (mDBTool)
		{
			r->FullAllUpdate(true);
			mDBTool->SaveRecord(r.getPtr(), false, true);
		}
		else
			ERROR_LOG("WARN: DBTool is null, then can not insert to DB");
		return r;
	}
	TABLE_LOG("ERROR: 加入到主索引失败");
	return ARecord();
}

ARecord DBTable::CreateRecord( const char* szIndex, bool bReplace )
{
	ARecord r = NewRecord();
	int col = GetField()->getFieldCol(mMainKeyIndex->IndexFieldName());

	r->_set(col, szIndex);
	if (AppendRecord(r, bReplace))
	{
		if (mDBTool)
		{
			r->FullAllUpdate(true);
			mDBTool->SaveRecord(r.getPtr(), false, true);
		}
		else
			ERROR_LOG("WARN: DBTool is null, then can not insert to DB");
		return r;
	}
	TABLE_LOG("ERROR: 加入到主索引失败"); 
	return ARecord();
}

ARecord DBTable::CreateRecord( Int64 nIndex, bool bReplace )
{
	ARecord r = NewRecord();
	int col = GetField()->getFieldCol(mMainKeyIndex->IndexFieldName());
	r->_set(col, nIndex);
	if (AppendRecord(r, bReplace))
	{
		if (mDBTool)
		{
			r->FullAllUpdate(true);
			mDBTool->SaveRecord(r.getPtr(), false, true);
		}
		else
			ERROR_LOG("WARN: DBTool is null, then can not insert to DB");
		return r;
	}
	TABLE_LOG("ERROR: 加入到主索引失败");
	return ARecord();
}

ARecord DBTable::NewRecord()
{
	AssertEx(GetField(), "Now table must exist FieldIndex");
	ARecord freeRe = GetField()->TryGetFreeRecord();
	DBRecord *p = dynamic_cast<DBRecord*>(freeRe.getPtr());
	if (p!=NULL)	
		p->InitTablePtr(mOwnerPtr);	
	else
	{
		p = MEM_NEW DBRecord(this);
		p->_alloctData(0);
	}
	// 终端新建记录, 需要全部同步到DB
	//p->FullAllUpdate(true);
	return p;
}

bool DBTable::DeleteRecord( ARecord record )
{
	if (SkipBaseTable::DeleteRecord(record))
	{
		if (mDBTool)
		{
			AString sql;
			if (mMainKeyIndex->IsStringHashKey())
				sql.Format("Delete from `%s` WHERE `%s`='%s'", GetTableName(), mMainKeyIndex->IndexFieldName(), record->getIndexData().string());
			else
				sql.Format("Delete from `%s` WHERE `%s`=%s", GetTableName(), mMainKeyIndex->IndexFieldName(), record->getIndexData().string());
			int count = 0;
			mDBTool->exeSql(sql, false, &count, true);
			return count>0;
		}
		else
			ERROR_LOG("DBTool is null, delete record fail from DB");
	}

	return false;
}

ARecord DBTable::GetRecord( Int64 nIndex )
{
	if (mMainKeyIndex)
	{
		ARecord r = mMainKeyIndex->GetRecord(nIndex);
		if (r)
			return r;
	}

	return _LoadDBRecord(STRING(nIndex));

	return ARecord();
}

ARecord DBTable::GetRecord( float fIndex )
{
	if (mMainKeyIndex)
	{
		ARecord r = mMainKeyIndex->GetRecord(fIndex);
		if (r)
			return r;
	}

	return _LoadDBRecord(STRING(fIndex));

	return ARecord();
}

ARecord DBTable::GetRecord( const char* szIndex )
{
	if (mMainKeyIndex)
	{
		ARecord r = mMainKeyIndex->GetRecord(szIndex);
		if (r)
			return r;
	}

	return _LoadDBRecord(szIndex);

	return ARecord();
}

ARecord DBTable::_LoadDBRecord( const char *szKey )
{
	if (!mMainKeyIndex || !mDBTool)
		return ARecord();

	AString sql;
	if (mMainKeyIndex->IsStringHashKey())
		sql.Format("Select * from `%s` WHERE `%s`='%s'", GetTableName(), mMainKeyIndex->IndexFieldName(), szKey);
	else
		sql.Format("Select * from `%s` WHERE `%s`=%s", GetTableName(), mMainKeyIndex->IndexFieldName(), szKey);
	mDBTool->exeSql(sql, true);
	ARecord r = NewRecord();
	if (mDBTool->LoadRecord(r))
	{			
		r->FullAllUpdate(false);
		AppendRecord(r, true);
		return r;
	}
	return ARecord();
}
