
#include "TableFieldInfo.h"
#include "TableTool.h"
#include "TableManager.h"
#include "IndexBaseTable.h"
#include "TableUpdateLoader.h"
#include "MemoryTableFieldIndex.h"
//bool TableFieldInfo::saveData( void *data, DataStream *destData ) const
//{
//	AutoTable temp;
//	if (base::get(data, &temp, typeid(AutoTable)))
//    {
//        if (temp)
//	    {
//            destData->write(true);
//		    AString type = temp->GetTypeName();
//		    destData->writeString(type.c_str());
//		    //DataBuffer tempData;
//		    size_t size = temp->_SerializeStream(destData);
//		    return size>0; 
//        }
//        else
//            destData->write(false);
//
//        return true;
//	}
//	return false;
//}
//
//bool TableFieldInfo::restoreData( void *data, DataStream *scrData )
//{
//    bool bHave = false;
//    if (scrData->read(bHave))
//    {
//        if (bHave)
//        {
//            AString type;
//            if (scrData->readString(type))
//            {
//                AutoTable temp = mConfigManager.CreateNewObject(type.c_str()) ; 
//                if (temp)		
//                    if (temp->_RestoreFromeStream(scrData))
//                        return base::set(data, &temp, typeid(AutoTable));		
//            }
//        }
//        AutoTable  t;
//        base::set (data, &t, typeid(AutoTable));
//        return true;
//    }
//    return false;
//}
//
//bool TableFieldInfo::set( void *data, const AString &strVal )
//{ 
//	return set(data, strVal.c_str()); 
//}
//
//bool TableFieldInfo::set( void *data, const char* szVal )
//{
//	AutoTable temp;
//	if (!base::get(data, &temp, typeid(AutoTable)) || !temp)
//	{
//		if (!mSubTableField)
//		{
//			TABLE_LOG("ERROR: No exist field when restore form string");
//			return false;
//		}
//		temp = MEM_NEW NiceTable(mSubTableField);
//		base::set(data, &temp, typeid(AutoTable));
//	}
//
//	AString tempScr = szVal;
//
//	while (true)
//	{
//		AString recordData = tempScr.SplitBlock();
//		if (recordData.empty() || recordData=="")
//			break;
//
//		AString key = recordData.SplitBlock('[', ']');
//
//		if (key.empty())
//			return true;
//
//		AutoRecord r = temp->CreateRecord(key);
//
//		if (!r->FullFromString(recordData, false))
//		{
//			TABLE_LOG("WARN: full data from string FAIL, key = [%s]", key.c_str());
//		}
//	}
//	return true;
//}
//
//bool TableFieldInfo::get( void *data, AString &resultString ) const
//{
//	AutoTable temp;
//	if (base::get(data, &temp, typeid(AutoTable)) && temp)
//	{
//		if (mSubTableField && temp->GetField()!=mSubTableField)
//		{
//			TABLE_LOG("WARN: FIELD [%s] table field is not same of now value table", getName().c_str());
//		}
//		for (TableIt hIt(temp); hIt._Have(); hIt._Next())
//		{
//			AutoRecord r = hIt.getCurrRecord();
//			if (r)
//			{
//				AString recordData;
//				if (r->ToString(recordData, true))
//				{
//					resultString += "{";
//					resultString += recordData;
//					resultString += "}";
//				}
//				else
//				{
//					TABLE_LOG("WARN: Record to string fail");
//				}
//			}
//		}
//	}
//	return true;
//}
//
//AString TableFieldInfo::getTypeParam() const
//{
//	if (mSubTableField)
//	{
//		return mSubTableField->ToString();
//	}
//	return AString();
//}
//
//void TableFieldInfo::setTypeParam( const AString &typeParam )
//{
//	if (typeParam.empty() || typeParam=="")
//		return;
//
//	mSubTableField = MEM_NEW FieldIndex(NULL, 0);
//	if (!mSubTableField->FullFromString(typeParam))
//	{
//		TABLE_LOG("ERROR: [%s] Restore field from string fail", getName().c_str());
//	}
//}
//
//void TableFieldInfo::CopyData( const tFieldInfo *pScrFieldInfo )
//{
//	base::CopyData(pScrFieldInfo);
//	const TableFieldInfo *p = dynamic_cast<const TableFieldInfo*>(pScrFieldInfo);
//	if (p!=NULL)
//	{
//		mSubTableField = p->mSubTableField;
//	}
//}


//-------------------------------------------------------------------------
// 用于内存 DB
// 字段信息只保存校验码
//-------------------------------------------------------------------------

AString DBTableFieldInfo::getTypeParam() const
{
	if (mSubTableField)
	{
		return mSubTableField->ToString();
	}
	return AString();
}

void DBTableFieldInfo::setTypeParam( const AString &typeParam )
{
	if (typeParam.empty() || typeParam=="")
		return;

	mSubTableField = MEM_NEW MemoryTableFieldIndex(NULL);
	if (!mSubTableField->FullFromString(typeParam))
	{
		ERROR_LOG("ERROR: [%s] Restore field from string fail", getName().c_str());
	}
}

void DBTableFieldInfo::CopyData( const tFieldInfo *pScrFieldInfo )
{
	base::CopyData(pScrFieldInfo);
	const DBTableFieldInfo *p = dynamic_cast<const DBTableFieldInfo*>(pScrFieldInfo);
	if (p!=NULL)
	{
		mSubTableField = p->mSubTableField;
	}
}

bool DBTableFieldInfo::saveData( void *data, DataStream *destData ) const
{
	ABaseTable temp;
	if (base::get(data, &temp, typeid(ABaseTable)))
	{
		if (temp)
		{
			destData->write((byte)_SubTableDefault);
			destData->write(temp->GetField()->GetCheckCode());
			TableDataLoader loader;
			if ( !temp->Save(&loader, destData) )
				return false;			
		}
		else
			destData->write((byte)_SubTableNull);

		return true;
	}
	return false;
}

bool DBTableFieldInfo::restoreData( void *data, DataStream *scrData )
{
	byte bHave = 0;
	if (scrData->read(bHave))
	{
		if (bHave==_SubTableDefault)
		{
			int checkCode = 0;
			if (!scrData->read(checkCode))
			{
				ERROR_LOG("读取字段验证失败");
				return false;
			}
			if (mSubTableField && !mSubTableField->CheckSame(checkCode))
			{
				TABLE_LOG("WARN: 字段校验失败 now [%d] of [%d]", checkCode, mSubTableField->GetCheckCode());
				//return false;
			}

			ABaseTable temp;
			if (!base::get(data, &temp, typeid(ABaseTable)))
				return false;

			if (!temp)
			{
				temp = CreateFieldTable(); //tBaseTable::NewFieldTable(false); 
				//temp->InitField(mSubTableField);

				if (!base::set(data, &temp, typeid(ABaseTable) ))
					return false;
			}
			TableDataLoader loader;
			if (!temp->Load(&loader, scrData))
				return false;			
			//else if (!mSubTableField)
			//{
			//	mSubTableField = temp->GetField();
			//	mSubTableField->_updateInfo();
			//}
		}
		else if (bHave!=_SubTableNull)
		{
			ERROR_LOG("Restore type error >%d", (int)bHave);
			return false;
		}
		return true;
	}
	return false;
}

bool DBTableFieldInfo::set( void *data, const char* szVal )
{
	ABaseTable temp;
	if (!base::get(data, &temp, typeid(ABaseTable)) || !temp)
	{
		if (!mSubTableField)
		{
			TABLE_LOG("ERROR: No exist field when restore form string");
			return false;
		}
		temp = tBaseTable::NewBaseTable();
		temp->InitField(mSubTableField);
		base::set(data, &temp, typeid(ABaseTable));
	}

	AString tempScr = szVal;

	while (true)
	{
		AString recordData = tempScr.SplitBlock();
		if (recordData.empty() || recordData=="")
			break;

		AString key = recordData.SplitBlock('[', ']');

		if (key.empty())
			return true;

		ARecord r = temp->CreateRecord(key.c_str(), true);

		if (!r->FullFromString(recordData.c_str(), false))
		{
			TABLE_LOG("WARN: full data from string FAIL, key = [%s]", key.c_str());
		}
	}
	return true;
}

bool DBTableFieldInfo::get( void *data, AString &resultString ) const
{
	ABaseTable temp;
	if (base::get(data, &temp, typeid(ABaseTable)) && temp)
	{
		if (mSubTableField && temp->GetField()!=mSubTableField)
		{
			TABLE_LOG("WARN: FIELD [%s] table field is not same of now value table", getName().c_str());
		}
		for (ARecordIt hIt = temp->GetRecordIt(); *hIt; ++(*hIt))
		{
			ARecord r = *hIt;
			if (r)
			{
				AString recordData;
				if (r->ToString(recordData, true))
				{
					resultString += "{";
					resultString += recordData;
					resultString += "}";
				}
				else
				{
					TABLE_LOG("WARN: Record to string fail");
				}
			}
		}
	}
	return true;
}

bool DBTableFieldInfo::saveField( DataStream *destData ) const
{
	base::saveField(destData);
	if (mSubTableField)
	{
		destData->write(true);
		return mSubTableField->saveToData(destData);
	}
	else
		return destData->write(false);

	return true;
}

bool DBTableFieldInfo::restoreFromData( DataStream *scrData )
{
	if (base::restoreFromData(scrData))
	{
		bool bHave = false;
		if (!scrData->read(bHave))
			return false;

		if (bHave)
		{
			AutoField newField = MEM_NEW BaseFieldIndex();
			if (newField->restoreFromData(scrData))
			{
				//if (!mSubTableField || !(*mSubTableField==*newField))
				mSubTableField = newField;
				newField->_updateInfo();
				return true;
			}
			ERROR_LOG("ERROR: restore sub field fail");
			return false;
		}
		return true;
	}
	return false;
}

AutoTable DBTableFieldInfo::CreateFieldTable()
{
	AutoTable t = tBaseTable::NewFieldTable(false);
	t->InitField(mSubTableField);
	return t;
}
