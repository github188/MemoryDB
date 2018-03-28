
#include "MemoryDBIndexTable.h"


ARecord MemoryDBIndexTable::NewRecord()
{
	AssertEx(GetField(), "Now table must exist FieldIndex");
	ARecord re = GetField()->TryGetFreeRecord();
	MemoryDBIndexRecord *p = dynamic_cast<MemoryDBIndexRecord*>(re.getPtr());
	if (p!=NULL)	
		p->InitTablePtr(mOwnerPtr);	
	else
	{
		re = MEM_NEW MemoryDBIndexRecord(GetSelf());
		re->_alloctData(0);
	}
	return re;
}

bool MemoryDBIndexTable::SetIndexField( const char *szFieldName, bool bHash )
{
	FieldInfo info = mOwnerPtr->mField->getFieldInfo(szFieldName);
	if (info!=NULL)
	{
		int col = mOwnerPtr->mField->getFieldCol(szFieldName);
		AutoIndex index = NewRecordIndex((FIELD_TYPE)info->getType(), bHash, true);
		if (index)
		{
			index->SetIndexField(szFieldName);
			NOTE_LOG("%s Create index >%s, type %s", GetTableName(), szFieldName, info->getTypeString());
			if (col>=mRecordIndexList.size())
				mRecordIndexList.resize(col+1);
			
			mRecordIndexList[col] = index;

			for (ARecordIt it=GetRecordIt(); *it; ++(*it) )
			{
				ARecord re = *it;
				if (re)
					index->InsertRecord(re);
			}

			return true;
		}		
		else
			ERROR_LOG("%s创建索引对象失败>%s", GetTableName(), szFieldName);
	}
	else
	{
		ERROR_LOG("ERROR: 不存在字段 [%s] at table [%s]", szFieldName, GetTableName());
	}

	return false;
}

bool MemoryDBIndexTable::AppendRecord( ARecord scrRecord, bool bReplace )
{
	if (MemoryDBTable::AppendRecord(scrRecord, bReplace))
	{
		for (size_t i=0; i<mRecordIndexList.size(); ++i)
		{
			if (mRecordIndexList[i])
				mRecordIndexList[i]->InsertRecord(scrRecord);
		}
		return true;
	}

	return false;
}


void MemoryDBIndexTable::OnRecordDataChanged( ARecord re, int col, Int64 scrValue )
{
	MemoryDBTable::OnRecordDataChanged(re, col, scrValue);

	AutoIndex index = GetIndex(col);
	if (index)
	{
		//!!! NOTE:支持多键时,不判断获取出来的记录,此时第一个可能不为re
		if (/*index->GetRecord(scrValue)==re && */index->RemoveRecord(scrValue, re))
			index->InsertRecord(re);
	}
}

void MemoryDBIndexTable::OnRecordDataChanged( ARecord re, int col, const char *scrValue )
{
	MemoryDBTable::OnRecordDataChanged(re, col, scrValue);

	AutoIndex index = GetIndex(col);
	if (index)
	{
		//!!! NOTE:支持多键时,不判断获取出来的记录,此时第一个可能不为re
		if (/*index->GetRecord(scrValue)==re && */index->RemoveRecord(scrValue, re))
			index->InsertRecord(re);
	}
}

AString MemoryDBIndexTable::GetIndexFieldsInfo()
{
	AString indexInfo;
	for (size_t i=0; i<mRecordIndexList.size(); ++i)
	{
		if (!mRecordIndexList[i])
			continue;

		if (!indexInfo.empty())
			indexInfo += " ";
		indexInfo += mRecordIndexList[i]->IndexFieldName();
	}
	return indexInfo;
}

bool MemoryDBIndexTable::RemoveRecord( ARecord record )
{
	if (!mRecordIndexList.empty())
	{		
		for (size_t i=0; i<mRecordIndexList.size(); ++i)
		{
			if (!mRecordIndexList[i])
				continue;
			mRecordIndexList[i]->RemoveRecord(record);
		}
	}
	return MemoryDBTable::RemoveRecord(record);
}

//ARecord MemoryDBIndexTable::GrowthNewRecord( DataStream *recordData )
//{
//	ARecord newRe = MemoryDBTable::GrowthNewRecord(recordData);
//	if (newRe)
//	{
//		for (size_t i=0; i<mRecordIndexList.size(); ++i)
//		{
//			if (!mRecordIndexList[i])
//				continue;
//			mRecordIndexList[i]->InsertRecord(newRe);
//		}
//	}
//	return newRe;
//}

bool MemoryDBIndexRecord::restoreData( DataStream *scrData )
{
	EasyString key = getIndexData().string();

	bool bRe = MemoryDBRecord::restoreData(scrData);
	if (!bRe)
	{
		ERROR_LOG("严重错误: 恢复记录数据失败>", key.c_str());
		return false;
	}

	return true;
}
