
#ifndef _INCLUDE_TABLEUPDATELOADER_H_
#define _INCLUDE_TABLEUPDATELOADER_H_

#include "BaseTable.h"
#include "IndexBaseTable.h"

//-------------------------------------------------------------------------
// 三种保存情况
// 1 保存全部数据, 包括字段信息, 全部记录数据
// 2 只保存完整记录数据, 不包含字段信息
// 3 只保存更新记录数据, 不包含字段信息
enum TABLE_SAVE_TYPE
{
	eSaveAll = 0,
	eOnlySaveRecordData = 1,
	eOnlyUpdateRecordData = 2,
};
//-------------------------------------------------------------------------
// 用与识别是序列完整数据，还是只序列修改的数据
class UpdateDataBuffer : public DataBuffer
{
public:
	UpdateDataBuffer()
	{}

	UpdateDataBuffer(DSIZE initSize)
		: DataBuffer(initSize)
	{}
};
// 识别只保存记录完整数据
class SaveRecordDataBuffer : public DataBuffer
{
public:
	SaveRecordDataBuffer()
	{}

	SaveRecordDataBuffer(DSIZE initSize)
		: DataBuffer(initSize)
	{}
};

//-------------------------------------------------------------------------

class TableDataLoader : public TableLoader
{	
public:
	virtual bool LoadTable(tBaseTable *pTable, DataStream *scrData) override
	{
		byte dataType = eSaveAll;

		READ(scrData, dataType);		
		if (dataType<eSaveAll || dataType>eOnlyUpdateRecordData)
		{
			ERROR_LOG("恢复表格数据保存类型错误 [%d]", dataType);
			return false;
		}
		if (eSaveAll==dataType)
		{
			EasyString tableName;
			if (!scrData->readString(tableName))
				return false;
			pTable->SetTableName(tableName.c_str());
			pTable->ClearAll();
			AutoField newField = MEM_NEW BaseFieldIndex();
			if (!newField->restoreFromData(scrData))
			{
				ERROR_LOG("恢复字段信息失败 [%s]", tableName.c_str());
				return false;
			}
			newField->_updateInfo();

			if (pTable->GetField())
			{
				if (pTable->GetField()->GetCheckCode()!=newField->GetCheckCode())
					pTable->InitField(newField);
				else
				{
					LOG_RED
					TABLE_LOG("!!! WARN: [%s] Already exist same field, May be not need eSaveAll, Can use SaveRecordDataBuffer save data!!!\r\n%s", 
						tableName.c_str(), pTable->GetField()->ToString().c_str());
					LOG_WHITE
				}
			}
			else 
				pTable->InitField(newField);
			pTable->SetMainIndex(0, false);
		}
		// 在记录更新时, 删除记录
		if (eOnlyUpdateRecordData==dataType)
		{
			if (!pTable->ApplyDeleteInfo(scrData))
			{
				ERROR_LOG("恢复使用删除记录信息失败 [%s]", pTable->GetTableName());
				return false;
			}
		}
		if (pTable->GetField()->getCount()<=0)
		{
			ERROR_LOG("[%s]恢复表格记录前, 表格字段为空", pTable->GetTableName());
			return false;
		}
		// 恢复记录
		int count;
		READ(scrData, count);
		for (int i =0; i<count; ++i)
		{						
			if (eOnlyUpdateRecordData==dataType)
			{
				AString key;
				if (!scrData->readString(key))
					return false;

				ARecord re = pTable->GetRecord(key.c_str());
				if (!re)
				{
					//TABLE_LOG("WARN: 恢复更新记录数据时, 获取不到需要更新的记录[%s] at table [%s]", key.c_str(), pTable->GetTableName());
					re = pTable->CreateRecord(key.c_str(), true);
				}

				if (!re->updateFromData(scrData))
				{
					ERROR_LOG("恢复记录更新数据失败");
					return false;
				}
			}
			else
			{
				ARecord re = pTable->NewRecord();				
				if (!re->restoreData(scrData))
				{
					ERROR_LOG("恢复记录数据失败");
					return false;
				}
				pTable->AppendRecord(re, true, true);
			}
		}
		return true;
	}

	virtual bool SaveTable(tBaseTable *pTable, DataStream *resultData, const char *szIndexField = NULL)
	{
		bool bUpdate = dynamic_cast<UpdateDataBuffer*>(resultData)!=NULL;
		bool bOnlySaveRecord = dynamic_cast<SaveRecordDataBuffer*>(resultData)!=NULL;

		byte dataType = eSaveAll;
		if (bUpdate)
			dataType = eOnlyUpdateRecordData;
		else if (bOnlySaveRecord)
			dataType = eOnlySaveRecordData;

		resultData->write(dataType);

		if (eSaveAll==dataType)
		{
			resultData->writeString(pTable->GetTableName());
			if (!pTable->GetField()->saveToData(resultData))
			{
				ERROR_LOG("序列字段信息失败");
				return false;
			}
		}

		// 保存删除记录信息
		if (eOnlyUpdateRecordData==dataType)
		{
			if (!pTable->SaveDeleteInfo(resultData, true))
			{
				ERROR_LOG("保存删除记录信息失败 [%s]", pTable->GetTableName());
				return false;
			}
		}

		DSIZE nowPos = resultData->tell();
		resultData->write((int)0);
		uint count = 0;
		for (ARecordIt it = pTable->GetRecordIt(); *it; ++(*it))
		{
			ARecord re = *it;				
			if (!re)
				continue;

			if (eOnlyUpdateRecordData==dataType)
			{
				if (re->NeedUpdate()) 
				{
					++count;
					Data key = re->getIndexData();
					resultData->writeString(key.string());
					if (!re->saveUpdateData(resultData))
					{
						ERROR_LOG("序列记录更新数据失败");
						return false;
					}
				}
			}
			else
			{
				++count;				
				if (!re->saveData(resultData))
				{
					ERROR_LOG("序列记录数据失败");
					return false;
				}
			}

		}
		DSIZE last = resultData->tell();
		resultData->seek(nowPos);
		resultData->write(count);
		resultData->seek(last);

		return true;
	}
};

//class OnlySaveRecordLoader : public TableDataLoader
//{	
//public:
//	virtual bool NeedSaveAll(){ return false; }
//};

//-------------------------------------------------------------------------

#endif //_INCLUDE_TABLEUPDATELOADER_H_