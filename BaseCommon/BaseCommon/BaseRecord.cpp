
#include "BaseRecord.h"
#include "BaseTable.h"

#include "TableFieldInfo.h"

#include "FieldSubTable.h"

void BaseRecord::Release()
{
	// 有可能放入FieldIndex的缓存中
	if (getField())
		getField()->FreeRecord(this);
	else
		MEM_DELETE this;
}

void BaseRecord::InitData()
{
	if (getField() && _getData()!=NULL)
	{
		//AssertEx(_getDataSize()>=(DSIZE)getField()->getDataLength(), "Record data allot length muse more then field info length");

		for ( int i=0;i<(int)getField()->getCount();++i )
		{
			FieldInfo info = getField()->getFieldInfo(i);
			if ( info )
			{
				info->free(_getData());
#if DEVELOP_MODE		// DEBUG_RECORD_DATA
				//检验
				char *space = _getData() + info->getPosition();
				for (DSIZE s = 0; s<info->getLength(); ++s)
				{
					if ( *(space+s) !=0 )
					{
						AString str;
						info->get(_getData(), str);
						ERROR_LOG("严重错误: [%s]->[%d]数据使用后,未清理为零!", 
							info->getName().c_str(), info->getType());
						AssertEx(0,  "校验失败: 数据使用后,没有清理为零,校验使用!");
					}
				}
#endif
			}
			else
			{
				ERROR_LOG("严重警告: 在记录释放时,没有找到对应的字段信息, 有可能会造成内存泄漏!");
				AssertEx(0, "没有找到字符信息!");
			}			
		}

#if DEVELOP_MODE
		//内存使用校验, 规则是全部清零
		DSIZE dataSize = (DSIZE)getField()->getDataLength();
		char *szData = (char*)_getData();
		if (szData!=NULL)
		{
			for(DSIZE i=0; i<dataSize; ++i)
			{
				if ( (*(szData+i)) != 0 )
				{
					ERROR_LOG("数据使用后,没有清理!");
					AssertEx(0,  "校验失败: 数据使用后,没有清理为零,校验使用!");
				}
			}
		}
#endif
	}
}

void BaseRecord::_alloctData( int extData )
{
	AutoField f = getField();
	if (f)
	{
		InitData();

		_freeData();
		mRecordData = (char*)_allotMemory(f->getDataLength()+extData);
		memset(mRecordData, 0, f->getDataLength()+extData);
		int count = f->getCount();
		for (int i=0; i<count; ++i)
		{
			FieldInfo info = f->getFieldInfo(i);
			if (info!=NULL)
				info->init(_getData());
			else
				ERROR_LOG("Field col [%d] is NULL", i);
		}
	}
}

bool BaseRecord::_set( int col, Int64 nVal )
{
	FieldInfo info = getFieldInfo(col);		
	if (info)
	{
		return info->set(_getData(), (UInt64)nVal);
	}
	else
		ERROR_LOG("字段信息不存在 [%d]", col);
	return false;
}

bool BaseRecord::_set( int col, float nVal )
{
	FieldInfo info = getFieldInfo(col);		
	if (info)
	{
		return info->set(_getData(), nVal);
	}
	else
		ERROR_LOG("字段信息不存在 [%d]", col);
	return false;
}

bool BaseRecord::_set( int col, const char *szVal )
{
	FieldInfo info = getFieldInfo(col);		
	if (info)
	{
		return info->set(_getData(), szVal);
	}
	else
		ERROR_LOG("字段信息不存在 [%d]", col);
	return false;
}

bool BaseRecord::_set( int col, const AString &strVal )
{
	FieldInfo info = getFieldInfo(col);		
	if (info)
	{
		return info->set(_getData(), strVal);
	}
	else
		ERROR_LOG("字段信息不存在 [%d]", col);
	return false;
}

bool BaseRecord::_set( int col, void *obj, const type_info &typeInfo )
{
	FieldInfo info = getFieldInfo(col);		
	if (info)
	{
		if (info->set(_getData(), obj, typeInfo))
			return true;
		else
		{
			const char *szName = typeInfo.name();
			if (strlen(szName)>=4 && memcmp(szName, "enum", 4)==0)
				return info->set(_getData(), *(int*)obj);
			return false;
		}		
	}
	else
		ERROR_LOG("字段信息不存在 [%d]", col);
	return false;
}

bool BaseRecord::set( int col, tBaseTable *data )
{
	ABaseTable t;
	if (data!=NULL)
		t = data->GetSelf();
	return set(col, &t, typeid(t));
}

bool BaseRecord::set( int col, const Data &scrData )
{
	FieldInfo info = getFieldInfo(col);
	if (info!=NULL)
	{
		return info->setData(_getData(), &scrData);
	}
	return false;
}

tBaseTable* BaseRecord::getTable( int col ) const
{
	ABaseTable t;
	get(col, &t, typeid(t));
	return t.getPtr();
}

void BaseRecord::Remove()
{
	if (GetTable()!=NULL)
		GetTable()->RemoveRecord(GetSelf());
}

bool BaseRecord::get( int col, void *resultValue, const type_info &typeInfo ) const
{
	FieldInfo info = getFieldInfo(col);		
	if (info)
	{
		return info->get(_getData(), resultValue, typeInfo);
	}
	else
		ERROR_LOG("字段信息不存在 [%d]", col);
	return false;
}

Data BaseRecord::getIndexData()
{
	return get(GetTable()->GetMainIndexCol());
}

bool BaseRecord::saveData( DataStream *destData )
{
#if RECORD_ONLY_CALL_RESTOREDATA
	// 写入完全保存标签, 如果是UPDATA，restoreData 时， 直接调用restoreUpdate, 这样只需要 restoreData 来恢复就可以了
	destData->write(true);	
#endif
	//destData->write(getField()->GetCheckCode());
	for (int i=0; i<getField()->getCount(); ++i)
	{
		FieldInfo info = getFieldInfo(i);
		if (!info->saveData(_getData(), destData))
		{
			ERROR_LOG("记录数据序列化失败");
			return false; 
		}
	}
	return true;
}

bool BaseRecord::restoreData( DataStream *scrData )
{
#if RECORD_ONLY_CALL_RESTOREDATA
	// 如果全部使用 restoreData 来恢复, 打开以下注释
	bool bAll = true;
	if (!scrData->read(bAll))
		return false;

	if (!bAll)
		return _updateFromData(scrData);
#endif	

	//int checkCode = 0;
	//READ(scrData, checkCode);
	//if (!getField()->CheckSame(checkCode))
	//{
	//	ERROR_LOG("字段校验失败 now [%d] of [%d]", checkCode, getField()->GetCheckCode());
	//	return false;
	//}
	for (int i=0; i<getField()->getCount(); ++i)
	{
		FieldInfo info = getFieldInfo(i);

		bool b = false;
		if (info->getType()==FIELD_INT || info->getType()==FIELD_UINT64 || info->getType()==FIELD_SHORT || info->getType()==FIELD_BYTE)
		{
			Int64 scrValue = get(i);
			b = info->restoreData(_getData(), scrData);			
			UInt64 x = 0;
			info->get(_getData(), x);
			if (scrValue!=(Int64)x)
				GetTable()->OnRecordDataChanged(this, i, scrValue);
		}
		else if (info->getType()==FIELD_STRING)
		{
			EasyString scrValue = get(i).string();
			b = info->restoreData(_getData(), scrData);	
			if (scrValue!=get(i).string())
				GetTable()->OnRecordDataChanged(this, i, scrValue.c_str());
		}		
		else
			b = info->restoreData(_getData(), scrData);	

		if (!b)
		{
			ERROR_LOG("记录数据恢复失败");
 			return false;
		}
	}
	return true;
}

tBaseTable* BaseRecord::CreateFieldTable( int col )
{
	//int nCol = getFieldCol(szFieldName);
	if (col<1)
	{
		TABLE_LOG("ERROR: FIELD [%d] is not exist, or is not first COL", col);
		return NULL;
	}
	DBTableFieldInfo *p = dynamic_cast<DBTableFieldInfo*>(getFieldInfo(col));
	if (p!=NULL)
	{
		ABaseTable t = p->CreateFieldTable();// tBaseTable::NewFieldTable(false);
		//t->SetTableName(p->getName().c_str());
		//t->InitField(p->mSubTableField);
		
		_set(col, &t, typeid(ABaseTable));
		return t.getPtr();
	}

	TABLE_LOG("ERROR: FIELD [%d] is not TABLE type, OR no exist", col);
	return NULL;
}

bool BaseRecord::SetFieldTable( int col, tBaseTable *pSubTable )
{
	AssertEx( dynamic_cast<FieldSubTable*>(pSubTable)!=NULL, "字段子表必须使用 [FieldSubTable]");
	//int nCol = getFieldCol(szFieldName);
	if (col<1)
	{
		TABLE_LOG("ERROR: FIELD [%d] is not exist, or is not first COL", col);
		return false;
	}
	DBTableFieldInfo *p = dynamic_cast<DBTableFieldInfo*>(getFieldInfo(col));
	if (p!=NULL)
	{
		if ( p->mSubTableField->CheckSame(pSubTable->GetField()->GetCheckCode()) )			
			return set(col, &(pSubTable->GetSelf()), typeid(ABaseTable));	
	}

	TABLE_LOG("ERROR: FIELD [%d] is not TABLE type", col);
	return false;
}

tBaseTable* BaseRecord::GetFieldTable( int nCol )
{
	//int nCol = getFieldCol(szFieldName);
	if (nCol<1)
	{
		TABLE_LOG("ERROR: FIELD [%d] is not exist, or is not first COL", nCol);
		return false;
	}
	DBTableFieldInfo *p = dynamic_cast<DBTableFieldInfo*>(getFieldInfo(nCol));
	if (p==NULL)
	{
		TABLE_LOG("ERROR: FIELD [%d] is not TABLE type", nCol);
		return NULL;
	}
	ABaseTable t;
	get(nCol, &t, typeid(ABaseTable));
	if (!t)
		t = CreateFieldTable(nCol);

	return t.getPtr();
}

bool BaseRecord::FullFromString( const char *scrData, bool bHaveKey, bool bNeedCheck )
{
	AString strData = scrData;
	if (bNeedCheck)
	{
		AString checkCode = strData.SplitLeft(":");
		if (!getField()->CheckSame( TOINT(checkCode.c_str()) ))
		{
			ERROR_LOG("字段校验失败> data [%s] != now [%d]", checkCode.c_str(), getField()->GetCheckCode());
			return false;
		}
	}
	AString strVal;
	if (bHaveKey)
		strVal = strData.SplitBlock('[', ']');

	for (int i=1; i<getField()->getCount(); ++i)
	{		
		strVal = strData.SplitBlock('[', ']');
		if (strVal.empty())
		{
			TABLE_LOG("ERROR: String data restore fail, num less");
			return false;
		}
		_set(i, strVal.c_str());
	}

	return true;
}

bool BaseRecord::ToString( AString &resultData, bool bHaveKey, bool bNeedCheck )
{
	if (bNeedCheck)
	{
		resultData += getField()->GetCheckCode();
		resultData += ":";
	}
	int beginCol = bHaveKey ? 0 : 1;
	for (int i=beginCol; i<getField()->getCount(); ++i)
	{
		resultData += "[";			
		resultData += get(i).string();
		resultData += "]";
	}
	return true;
}

AutoNice BaseRecord::getNice( int col ) const
{
	AutoNice result;
	get(col, &result, typeid(AutoNice));
	return result;
}

AutoData BaseRecord::getDataBuffer( int col ) const
{
	AutoData data;
	get(col, &data, typeid(AutoData));
	return data;
}

//Hand<BaseRecord> BaseRecord::getRecord(int col) const
//{
//	Hand<BaseRecord> record;
//	get(col, &record, typeid(Hand<BaseRecord>));
//	return record;
//}

AutoNice BaseRecord::ToNiceData()
{
	AutoNice niceData = MEM_NEW NiceData();
	for (int i=0; i<GetFieldNum(); ++i)
	{
		FieldInfo info = getFieldInfo(i);
		if (!niceData->getOrCreate(info->getName().c_str()).set(get(i)))
		{
			ERROR_LOG("未能将字段数据保存到目标 NiceData中>[%s], type [%s]", info->getName().c_str(), info->getTypeString());
			return AutoNice();
		}
	}
	return niceData;
}

bool BaseRecord::SaveToArrayData(DataStream *pDestData, EasySet<AString> *pSelect, EasySet<AString> *pExclude)
{
	AutoField field = getField();
	pDestData->write((ushort)field->getCount());
	for (int i=0; i<field->getCount(); ++i)
	{
		FieldInfo f = field->getFieldInfo(i);
		if (pSelect!=NULL)
			if (!pSelect->exist(f->getName()))
			{
				pDestData->write((byte)FIELD_NULL);
				continue;
			}

		if (pExclude!=NULL && pExclude->exist(f->getName()))
		{
			pDestData->write((byte)FIELD_NULL);
			continue;
		}

		pDestData->write((byte)f->getType());
		if (!f->saveData(_getData(), pDestData))
		{
			ERROR_LOG("序列保存数据失败 [%s]", f->getTypeString());
			return false;
		}
	}
	return true;
}

bool BaseRecord::SaveSelectData(DataStream *pDestData, EasyMap<int, FieldInfo> &selectField)
{
	FieldInfo info = getFieldInfo(0);
	if (info==NULL)
		return false;
	if (!info->saveData(_getData(), pDestData))
		return false;
	for (int i=0; i<selectField.size(); ++i)
	{
		FieldInfo p = selectField.get(i);
		// NOTE_LOG("[%d] [%s] >[%d]", selectField.getKey(i), p->getName().c_str(), p->getPosition());
		if (!p->saveData(_getData(), pDestData))			
			return false;							
	}
	return true;
}

bool BaseRecord::SaveSelectData(DataStream *pDestData, Buffer &excludeFieldState)
{
    AutoField f = getField();
    if (!f)
        return false;
    for (int i=0; i<f->getCount(); ++i)
    {
        if (excludeFieldState.isOpenState(i))
            continue;

        if (!f->getFieldInfo(i)->saveData(_getData(), pDestData))
        {
            ERROR_LOG("Field %s save fail", f->getFieldInfo(i)->getName().c_str());
            return false;
        }
    }
    return true;
}

bool BaseRecord::restoreSelectData(DataStream *pScrData, EasyMap<int, FieldInfo> &selectField )
{
	FieldInfo info = getFieldInfo(0);
	if (info==NULL)
		return false;
	if (!info->restoreData(_getData(), pScrData))
		return false;
	for (int i=0; i<selectField.size(); ++i)
	{
		FieldInfo p = selectField.get(i);
		// NOTE_LOG("[%d] [%s] >[%d]", selectField.getKey(i), p->getName().c_str(), p->getPosition())
		if (!p->restoreData(_getData(), pScrData))
			return false;							
	}
	return true;
}

int BaseRecord::getFieldCol( const char *szField ) const
{
	int col = getField()->getFieldCol(szField);
	if (col<0)
	{
		ERROR_LOG("No exist field [%s], at table>[%s]", szField, ((BaseRecord*)this)->GetTable()->GetTableName());
	}
	return col;
}

const char* BaseRecord::getString( const char* szField ) const
{
	FieldInfo f = getFieldInfo(szField);
	if (f!=NULL)
		return f->getString(_getData());
	return "";
}
