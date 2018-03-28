
#include "IndexDBRecord.h"
#include "IndexBaseTable.h"
#include "TableFieldInfo.h"

bool IndexDBRecord::saveUpdateData( DataStream *destData )
{
#if RECORD_ONLY_CALL_RESTOREDATA
	// 写入完全保存标签, 如果是UPDATA，restoreData 时， 直接调用restoreUpdate, 这样只需要 restoreData 来恢复就可以了
	destData->write(false);	// 更新数据标签
#endif
	// 保存字段校验
	destData->write(getField()->GetCheckCode());

	bool bHaveChanged = NeedUpdate();
	destData->write(bHaveChanged);

	if (!bHaveChanged)
		return false;

	// 保存修改字段状态
	destData->_write(_getStateData(), _getStateDataSize()) ;

	void *dataPtr = _getData();
	for (int i=0; i<getField()->getCount(); ++i)
	{
		if (_hadChanged(i))
		{
			FieldInfo info = getFieldInfo(i);
			bool b = info->saveData(dataPtr, destData);

			if (!b)
			{
				const char *szTableName = "Unknow";
				if (GetTable())
					szTableName = GetTable()->GetTableName();

				NOTE_LOG( "Error: 严重错误>字段数据保存失败>[%s] type[%s] at table[%s]", info->getName().c_str(), info->getTypeString(), szTableName );
				return false;
			}
		}
	}
	return true;
}


#if RECORD_ONLY_CALL_RESTOREDATA	
bool IndexDBRecord::_updateFromData( DataStream *scrData )
#else
bool IndexDBRecord::updateFromData( DataStream *scrData )
#endif
{
	// 恢复校验
	CheckType checkCode;
	READ(scrData, checkCode);

	if (checkCode==0 || (!getField()->CheckSame(checkCode)) )
	{
		ERROR_LOG("记录字段校验失败 [%d] in table [%d]", checkCode, getField()->GetCheckCode());
		return false;
	}

	bool bHave = false;
	READ(scrData, bHave);

	if (!bHave)
		return true;

	Buffer  tempState(_getStateDataSize());
	bool b = scrData->_read(tempState.data(), _getStateDataSize());
	if (!b)
	{
		ERROR_LOG("恢复字段状态信息失败");
		return false;
	}

	void *dataPtr = _getData();
	bool bNoExist = false;

	tBaseTable *pOwnTable = GetTable();
	if (pOwnTable==NULL)
	{
		ERROR_LOG("记录数据修改时, 必须提供所在的表格");
		return false;
	}

	for (int i=0; i<getField()->getCount(); ++i)
	{
		if ((tempState.isOpenState(i)))
		{
			FieldInfo info = getFieldInfo(i);
			AssertEx (info!=NULL, "应该存在字段信息");

			bool b = false;

			if (info->getType()==FIELD_INT || info->getType()==FIELD_UINT64 || info->getType()==FIELD_SHORT || info->getType()==FIELD_BYTE)
			{
				Int64 scrValue = get(i);
				b = info->restoreData(dataPtr, scrData);	
				UInt64 x = 0;
				info->get(dataPtr, x);
				if (scrValue!=(Int64)x)
					pOwnTable->OnRecordDataChanged(this, i, scrValue);
			}
			else if (info->getType()==FIELD_STRING)
			{
				EasyString scrValue = get(i).string();
				b = info->restoreData(dataPtr, scrData);				
				if (scrValue!=get(i).string())
					pOwnTable->OnRecordDataChanged(this, i, scrValue.c_str());
			}
			else
				b = info->restoreData(dataPtr, scrData);		
			
			//AssertEx(b, "字段数据保存到数据失败");
			if (!b)
			{
				const char *szTableName = "Unknow";
				
				szTableName = pOwnTable->GetTableName();

				ERROR_LOG( "严重错误>字段数据恢复失败>[%s] type[%s] at table[%s]", info->getName().c_str(), info->getTypeString(), szTableName );
				return false;
			}
			else
				_setNeedUpdate(i);

			bNoExist = true;
		}
	}
	if (!bNoExist)
	{
		ERROR_LOG("未发现变化的数据");
		return false;
	}
	return true;
}

bool IndexDBRecord::set( int col, Int64 nVal )
{
	UInt64 nowVal = get(col);
	if ((Int64)nowVal==nVal)
		return true;
	// 当改变之前如主索引不存在此记录, 则不执行改变之后, 加入到主索引, 避免新建记录NewRecord()不通过 AppendRecord加入	
	bool b = BaseRecord::_set(col, nVal);
	if (b)
	{			
		_setNeedUpdate(col, true);	
		if (GetTable()!=NULL && InTable())
			GetTable()->OnRecordDataChanged(this, col, nowVal);		
	}
	return b;
}

bool IndexDBRecord::set( int col, float nVal )
{
	float nowVal = get(col);
	if (nowVal==nVal)
		return true;
	
	bool b = BaseRecord::_set(col, nVal);
	if (b)
		_setNeedUpdate(col, true);

	return b;
}

bool IndexDBRecord::set( int col, const char *szVal )
{
	EasyString scrValue = get(col).string();
	if (strcmp(scrValue.c_str(), szVal)==0)
		return true;

	bool b = BaseRecord::_set(col, szVal);
	if (b)
	{
		_setNeedUpdate(col, true);
		if (GetTable()!=NULL && InTable())
			GetTable()->OnRecordDataChanged(this, col, scrValue.c_str());
	}
	return b;
}

bool IndexDBRecord::set( int col, const AString &strVal )
{
	EasyString scrValue = get(col).string();
	if (scrValue==strVal)
		return true;

	bool b = BaseRecord::_set(col, strVal);
	if (b)
	{
		_setNeedUpdate(col, true);
		if (GetTable()!=NULL && InTable())
			GetTable()->OnRecordDataChanged(this, col, scrValue.c_str());
	}
	return b;
}

bool IndexDBRecord::set( int col, void *obj, const type_info &typeInfo )
{	
	bool b = BaseRecord::_set(col, obj, typeInfo);
	if (b)
		_setNeedUpdate(col, true);

	return b;
}


void IndexDBRecord::FullAllUpdate( bool bNeed )
{
	char *pData = _getStateData();
	if (pData==NULL)
		return;

	mbNeedUpdate = false;

	if (bNeed)
	{
		//mDataState.initData(0xFF);
		memset(pData, 0xFF, _getStateDataSize());	
	}
	else
	{
		//mDataState.initData(0);
		memset(pData, 0, _getStateDataSize());	
	}
	OnNeedUpdate(bNeed);

	for (int col = 0; col<getField()->getCount(); ++col)
	{
		if (dynamic_cast<DBTableFieldInfo*>(getFieldInfo(col))!=NULL)
		{
			ABaseTable t = GetFieldTable(col);
			if (t)
			{
				if (!bNeed)
					t->ClearDeleteRecordList();
                if (t->NeedCheckRecordUpdateState())
                {
                    for (ARecordIt it=t->GetRecordIt(); *it; ++(*it))
                    {
                        ARecord r = (*it);
                        if (r)
                            r->FullAllUpdate(bNeed);
                    }
                }
			}
		}
	}
}

void IndexDBRecord::_Update()
{
	for (size_t i=0; i<(size_t)getField()->getCount(); ++i)
	{			
		if (HadChanged(i))	
			continue;	

		FieldInfo info = getField()->getFieldInfo(i);
		if (info->getType()==FIELD_TABLE) 
		{
			ABaseTable t;
			t = getTable(i);
			if (t)
			{
				if (t->HasDeleteRecord())
				{
					NotifyChanged(i);
				}
				else
				{				
                    if (t->NeedCheckRecordUpdateState())
                    {
                        for (ARecordIt it=t->GetRecordIt(); *it; ++(*it))
                        {
                            ARecord re = *it;
                            if (re)
                            {
                                re->Update();
                                if (re->NeedUpdate())
                                {
                                    NotifyChanged(i);
                                    break;
                                }
                            }					
                        }
                    }
				}
			}
		}
	}	
}
//-------------------------------------------------------------------------
#define FIRST_UPDATE_FIELD	(1)
#define DEFAULT_INDEX_FILED	(0)

bool IndexDBRecord::MakeUpdateSqlString( AString &sqlString, const char* indexFieldName )
{
	if (GetTable()==NULL)
	{
		ERROR_LOG("[%s]记录表格为空，生成SQL失败", get(0).string().c_str());
		return false;
	}

	if (!NeedUpdate())
		return true;
	//"update %s set charm=%d,nimbus=%d,hotvalue=%d,\
	//farmdata=\'%s\' where charguid = %d";
	sqlString = "update ";
	if  (strcmp(GetTable()->GetTableName(), "")==0)
	{
		AssertEx( 0,  "数据表名称为空,需要设置后才能更新" );
		return false;
	}
	sqlString += GetTable()->GetTableName();
	sqlString += " set ";
	bool first = true;
	int count = getField()->getCount();
	AutoField field = getField();

	for (int col=FIRST_UPDATE_FIELD; col<count; ++col)
	{
		if (_hadChanged(col))
		{
			if (col>0)
			{
				if (!first)
					sqlString += ", ";
				else
					first = false;
			}
			FieldInfo pInfo = field->getFieldInfo(col);
			if (NULL==pInfo)
			{
				AssertEx(0,  "字段信息获取错误");
				return false;
			}
			sqlString += pInfo->getName();
			sqlString += "=";
			if(!_LinkValueAtCol(pInfo, col, sqlString))
			{
				AssertEx(0,  "连接数值失败");
				return false;
			}
		}
	}
	if (first)
	{
		// 如果first的值没变动说明只有索引进行了更新
		//Assert(0,  "还未实现记录的更新 索引KEY 字符串");
		return false;
	}

	FieldInfo pInfo = field->getFieldInfo(0);
	if (NULL==pInfo)
	{
		AssertEx(0,  "字段信息获取错误");
		return false;
	}
	sqlString += " where ";
	sqlString +=  pInfo->getName();
	sqlString += "=";
	if(!_LinkValueAtCol(pInfo, 0, sqlString))
	{
		AssertEx(0,  "连接数值失败");
		return false;
	}

	return true;
}
//----------------------------------------------------------------------------
bool IndexDBRecord::MakeInsertSqlString( AString &sqlString )
{
	AutoField field = getField();
	if (GetTable()==NULL || !field || field->getCount()<=0)
		return false;

	sqlString = "insert into ";
	sqlString += GetTable()->GetTableName();
	sqlString += " set ";

	bool first = true;
	int count = field->getCount();

	for (int col=0; col<count; ++col)
	{
		if (col==DEFAULT_INDEX_FILED || _hadChanged(col))
		{
			//if (col>0)
			{
				if (!first)
					sqlString += ", ";
				else
					first = false;
			}
			FieldInfo pInfo = field->getFieldInfo(col);
			if (NULL==pInfo)
			{
				AssertEx(0,  "字段信息获取错误");
				return false;
			}
			sqlString += pInfo->getName();
			sqlString += "=";
			if(!_LinkValueAtCol(pInfo, col, sqlString))
			{
				AssertEx(0,  "连接数值失败");
				return false;
			}
		}
	}
	return true;
}
//-------------------------------------------------------------------------
bool IndexDBRecord::_LinkValueAtCol( FieldInfo filedInfo, int col, AString &sqlString )
{
	switch (filedInfo->getType())
	{
	case FIELD_STRING:
		{
			Data result = get(col);
			if (result.empty())
				return false;	
			sqlString += "'";
			sqlString += result.string();
			sqlString += "' ";	
		}
		break;

	case FIELD_CHAR_STRING:
		{
			Data dataValue = get(col);
			if (!dataValue.empty())
			{
				sqlString += "'";
				sqlString += dataValue.string();
				sqlString += "' ";	
			}
			else
				return false;
		}
		break;

	case FIELD_BYTE:
	case FIELD_SHORT:
	case FIELD_INT:		
	case FIELD_UINT64:
	case FIELD_FLOAT:
		{
			Data dataValue = get(col);
			if (!dataValue.empty())
			{						
				sqlString += dataValue.string();
			}
			else
				return false;
		}
		break;

	default:
		AString info = "Error: can`t save to db sql field type ->";
		info += filedInfo->getType();
		info += " field name ->";
		info += filedInfo->getName().c_str();
		ERROR_LOG(info.c_str());
		AssertEx(0,  info.c_str());
		throw std::exception(info.c_str());
	}
	return true;
}

bool IndexDBRecord::NeedUpdate( void )
{
	if (mbNeedUpdate)
	{
		_Update();
		mbNeedUpdate = false;
	}

	byte *pData = (byte*)_getStateData();
	if (pData==NULL)
		return false;

	if (!mOwnerTable->mField)
		return false;

	Buffer &updateState = mOwnerTable->mField->mUpdateState;
	if (updateState.size()<_getStateDataSize())
		return false;

	for (size_t i=0; i<(size_t)_getStateDataSize(); ++i)
	{
		byte s = * ((byte*)updateState.data()+i);
		if ((pData[i] & s)!=0)
			return true;
	}
	return false;
}

//-------------------------------------------------------------------------