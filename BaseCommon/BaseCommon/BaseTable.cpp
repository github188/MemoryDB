
#include "BaseTable.h"
#include "CSVTableLoader.h"
#include "IndexBaseTable.h"
#include "FileDataStream.h"
#include "TableFieldInfo.h"

#include "TableUpdateLoader.h"

#include "FieldSubTable.h"

#if SAVE_DEBUG_INFO
int tBaseTable::msCount = 0;
#endif

TableCommonConfig tBaseTable::msCommonConfig;

bool tBaseTable::SaveCSV(const char *szCSVFilePathName, bool bTabDelimited)
{ 
	FileDataStream   f(szCSVFilePathName, FILE_CREATE);
	if (!f)
	{
		TABLE_LOG("ERROR: 创建CSV表格文件失败 [%s]", szCSVFilePathName);
		return false;
	}
    if (bTabDelimited)
    {
        TxtTableLoader txtLoader;
        return Save(&txtLoader, &f);
    }
    CSVTableLoader csvLoader;
	return Save(&csvLoader, &f);
}
bool tBaseTable::LoadCSV(const char *szCSVFilePathName, bool bTabDelimited)
{
	FileDataStream f(szCSVFilePathName, FILE_READ);
	if (!f)
	{
		ERROR_LOG("ERROR: 打开CSV文件失败 [%s], 文件不存在", szCSVFilePathName);
		return false;
	}
    SetTableName(szCSVFilePathName);
    if (bTabDelimited)
    {
        TxtTableLoader txtLoader;
        return Load(&txtLoader, &f);
    }
    CSVTableLoader csvLoader;
	return Load(&csvLoader, &f);
}

FieldInfo tBaseTable::SetField(const char *szFieldName, FIELD_TYPE type, int col)
{
	OnFieldChangeBefore();
	FieldInfo info = mOwnerPtr->mField->setField(szFieldName, type, col);	
	if (info!=NULL)
	{		
		if (col==0)
			SetMainIndex(szFieldName, false);

		OnFieldChanged();
	}

	return info;
}

ABaseTable tBaseTable::NewBaseTable(bool bUseRecordPool)
{
	return MEM_NEW SkipBaseTable(bUseRecordPool?eInitPoolField : eInitCreateField);
}

bool tBaseTable::InitField( AutoField scrField )
{
	if (!scrField)
		return false;

	if (mOwnerPtr->mField)
	{
		mOwnerPtr->mpTable = NULL;
		mOwnerPtr = MEM_NEW TablePtr();
		mOwnerPtr->mpTable = this;
	}
	mOwnerPtr->mField = scrField;
	//mField = scrField;
	//*GetField() = *scrField;	
	if (GetField()->getCount()>0)
		return SetMainIndex(GetField()->getFieldInfo(0)->getName().c_str(), false);
	//else
	//{
	//	ERROR_LOG("初始字段为空 at table [%s]", GetTableName());
	//}
	return false;
}

void tBaseTable::Init()
{
	ClearAll();
	if (!mOwnerPtr->mField || mOwnerPtr->mField->getCount()>0)
	{
		if (mOwnerPtr->mField)
		{
			mOwnerPtr->mpTable = NULL;
			mOwnerPtr = MEM_NEW TablePtr();
			mOwnerPtr->mpTable = this;
		}
		mOwnerPtr->mField = MEM_NEW BaseFieldIndex();	
	}
}

bool tBaseTable::SetFieldTable( const char *szField, tBaseTable *pSubTable )
{
	if (pSubTable==NULL || !pSubTable->GetField() || !pSubTable->GetField()->check())
		return false;

	pSubTable->GetField()->_updateInfo();

	if (!pSubTable->GetField()->check())
		return false;

	FieldInfo info = GetField()->getFieldInfo(szField);
	DBTableFieldInfo *p = dynamic_cast<DBTableFieldInfo*>(info);
	if (p!=NULL)
	{
		// NOTE: 一旦被设置在字段类型中, 字段中关联的表格指针被设置为NULL
		p->mSubTableField = pSubTable->GetField();
		//p->mSubTableField->setTable(NULL);
		GetField()->_updateInfo();
		return true;
	}
	return false;
}

int tBaseTable::GetMainIndexCol()
{
	if (GetMainIndex())
		return GetField()->getFieldCol(GetMainIndex()->IndexFieldName());
	return -1;
}

Data tBaseTable::GetValue( const char* recordkey, const char *szFieldName )
{
	ARecord re = GetRecord(recordkey);
	if (re)
		return re->get(szFieldName);
	return Data();
}

Data tBaseTable::GetValue(const char* recordkey, int field)
{
	ARecord re = GetRecord(recordkey);
	if (re)
		return re->get(field);
	return Data();
}

Data tBaseTable::GetValue(int recordkey, int field)
{
	ARecord re = GetRecord(recordkey);
	if (re)
		return re->get(field);
	return Data();
}

bool tBaseTable::SaveData( DataStream *resultStream )
{
	TableDataLoader t;
	return Save(&t, resultStream);
}

bool tBaseTable::LoadFromData( DataStream *scrStream )
{
	TableDataLoader t;
	return Load(&t, scrStream);
}

bool tBaseTable::SortSaveData(DataStream *resultData, bool bSaveTableIndex)
{
	byte dataType = eSaveAll;

	resultData->write(dataType);

	//if (eSaveAll==dataType)
	{
		if (bSaveTableIndex)
			resultData->writeString(GetTableName());
		else
			resultData->writeString("");

		if (!GetField()->saveToData(resultData))
		{
			ERROR_LOG("序列字段信息失败");
			return false;
		}
	}

	uint count = 0;
	EasyMap<AString, ARecord> recordList;
	for (ARecordIt it = GetRecordIt(); *it; ++(*it))
	{
		ARecord re = *it;				
		if (!re)
			continue;

			++count;				
			recordList.insert(re->getIndexData().string(), re);
	}

	resultData->write((int)count);

	for (int i=0; i<recordList.size(); ++i)
	{
		if (!recordList.get(i)->saveData(resultData))
		{
			ERROR_LOG("序列记录数据失败");
			return false;
		}	
	}

	return true;
}

Auto<tBaseTable> tBaseTable::NewFieldTable(bool bCreateField)
{
	return MEM_NEW FieldSubTable(bCreateField ? eInitPoolField : eInitNullField);
}

bool tBaseTable::SetSkipUpdateField( const char *szFieldName, bool bSkip )
{
	if (GetField())
	{		
		int col = GetField()->getFieldCol(szFieldName);
		if (col>=0)		
			return SetSkipUpdateField(col, bSkip); 
	}
	return false;
}

tBaseTable::tBaseTable()
{	
	mOwnerPtr = MEM_NEW TablePtr();
	mOwnerPtr->mpTable = this;
#if SAVE_DEBUG_INFO
	++msCount; 
#endif
}

tBaseTable::~tBaseTable()
{
	mOwnerPtr->mpTable = NULL;
#if SAVE_DEBUG_INFO
	--msCount; WARN_LOG("Now table count>[%d]", msCount); 
#endif
}

