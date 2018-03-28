

#include "NiceData.h"
#include "TableTool.h"
#include "DataBuffer.h"

#include <stdarg.h>
#include "BaseTable.h"
//----------------------------------------------------------------
const char spaceString[] = "        ";
//----------------------------------------------------------------

bool AData::set(void *obj, const type_info &typeInfo)
{
	FieldInfo mTypeInfo = _getField();
	if (mTypeInfo)
	{
		if (mTypeInfo->set(_dataPtr(), obj, typeInfo))
			return true;
		// 当前类型很可能保存不了很的类型, 所以下面的错误提示可能不需要
		//ERROR_LOG("Error: set data fail: TYPE>[%s], Name>[%s]", mTypeInfo->getTypeString(), mTypeInfo->getName().c_str());
	}

	tFieldInfo *info = FieldInfoManager::getFieldInfo(typeInfo);
	if (info!=NULL)
	{				
		setType(info->getType());
		bool bResult = info->set(_dataPtr(), obj, typeInfo);
		if (!bResult)
			ERROR_LOG("Error: set data fail: TYPE>[%s], Name>[%s]", info->getTypeString(), info->getName().c_str());

		return bResult;
	}

	const char *szName = typeInfo.name();
	if (strlen(szName)>=4 && memcmp(szName, "enum", 4)==0)
		return set(*(int*)obj);

	ERROR_LOG("Error: set data fail: type>[%s]", typeInfo.name());

	return false;
}

bool AData::set( const Data &kVal )
{
	if (kVal.empty()) 
		return false;

	int t = kVal._getFieldInfo()->getType();
	FieldInfo f = setType(t);

	return f->setData(_dataPtr(), &kVal);
}


bool AData::serialize(DataStream *destData) const
{
	FieldInfo mTypeInfo = _getField();
	if (mTypeInfo)
	{
		unsigned char type = (unsigned char)(mTypeInfo->getType() & 0xFF);
		destData->write(type);		
		return mTypeInfo->saveData(_dataPtr(), destData);
	}
	if (getType()==FIELD_NULL)
	{
		destData->write((byte)0);
		return true;
	}
	return false;
}

bool AData::restore(DataStream *scrData)
{
	unsigned char type = 0;
	if (!scrData->read(type) )
	{
		free();
		return false;
	}
	FieldInfo f = setType((int)type);
	if (f)
	{
		return f->restoreData(_dataPtr(), scrData);
	}

	free();
	return type==FIELD_NULL;
}

FieldInfo AData::setType( int type )
{	
	if (type==FIELD_NULL)
	{
		return NULL;
	}

	switch (type)
	{
	case FIELD_CHAR_STRING:
		type = FIELD_STRING;
		break;

	//case FIELD_BINARY:
	//	AssertEx(0, "不支持保存类型 > FIELD_BINARY");
	//	return false;
	}

	FieldInfo typeInfo = _getField();
	if (typeInfo && type==typeInfo->getType())
		return typeInfo;

	free();
	FieldInfo f = FieldInfoManager::getFieldInfo((FIELD_TYPE)type);
	if (f!=NULL)
	{
		mData[0] = (byte)type;
		if (f->getLength()>_getThisSpace())
		{
			*(size_t*)&mData[1] = (size_t)ALLOCT_NEW(f->getLength());
		}
		return f;
	}
	
	return NULL;	
}

//bool AData::_resetTypeLength( size_t length )
//{
//	if (NULL!=mTypeInfo)
//	{
//		if ( mTypeInfo->getLength()>4 )
//		{
//			if (mData!=NULL)
//				Allot::freePtr(mData, mTypeInfo->getLength());
//			mData = NULL;
//		}
//		mTypeInfo->setLength(length);
//		_createData();
//		return true;
//	}
//	return false;
//}


//----------------------------------------------------------------
//----------------------------------------------------------------



Data tNiceData::get(const char* key) const
{
	AData &d = getAData(key);	
	return d.get();
}

bool tNiceData::get( const char* key, void *obj, const type_info &typeInfo ) const
{
	AData &d = getAData(key);
	return d.get(obj, typeInfo);
}

//-------------------------------------------------------------------------*/
void tNiceData::initData()
{
	//mDataMap.clear(false);
	//int sum = count();
	//for (size_t i=0; i<sum; ++i)
	//{
	//	get(i).free();
	//}

	for (NiceIt it=begin(); it->have(); it->next())
	{
		it->get().free();
	}
}


AString tNiceData::dump( int sub, int code ) const
{	
	AString temp = "\r\n";
    if (sub==0)
    {
        for (int n=0; n<sub; ++n)
        {
            temp += spaceString;
        }
        temp += "_____________________________________________\r\n";
    }
	int i=0;
	for (NiceIt it=begin(); it->have(); it->next(), ++i)
	{
		AData &d = it->get();
		AString key = it->key();
		for (int n=0; n<sub; ++n)
		{
			temp += spaceString;
		}
		temp += "[";
		temp += d.empty() ? "" : d._getField()->getTypeString();
		temp += "]";
		temp += ":[";

		temp += key;

		temp += "] = ";
		if (d.empty())
			temp += "NULL(WARN)";
		else
		{		
			FieldInfo f = d._getField();
			if (f->getType()==FIELD_NICEDATA || f->getType()==FIELD_ARRAYDATA)
			{
				AutoNice subData;
                if (f->getType()==FIELD_NICEDATA)
				    d.get(&subData, typeid(subData));
                else
                {
                    AutoArrayData dV;
                    d.get(&dV, typeid(dV));
                    subData = dV;
                }
				if (subData)
				{
                    if (f->getType()==FIELD_NICEDATA)
					    temp += "SUB Nice\r\n";
                    else
                        temp += "SUB Array\r\n";
                    for (int n=0; n<sub; ++n)
                    {
                        temp += spaceString;
                    }
                    temp +="   {";
					temp += subData->dump(sub+1, (int)i);
                    for (int n=0; n<sub; ++n)
                    {
                        temp += spaceString;
                    }
					temp += "   }";
                    temp += key;
				}
				else
					temp += "NULL";
			}
			else
			{
				AString str;
				f->get(d._dataPtr(), str);
				if (f->getType()==FIELD_STRING)
					temp += AString::getANIS(str.c_str());
				else
					temp += str;
				//if (d.getTypeInfo()->getType()==FIELD_TABLE)
				//{
				//	AutoTable hTable;
				//	d.get(&hTable, typeid(AutoTable));
				//	if (hTable)
				//	{
				//		AString t = "TableLog/CSV/";
				//		t += mName.c_str();
				//		t += "_";
				//		t += szName;
				//		t += ".csv";
				//		if (hTable->SaveCSV(t.c_str()))
				//		{
				//			temp += " This is a table, then saved to [";
				//			temp += t;
				//			temp += "]";
				//		}
				//		else
				//		{
				//			temp += " Warning: This is a table, but save fail.";
				//		}
				//	}
				//	else
				//	{
				//		temp += " Warning: This is a table type, but get table fail.";
				//	}
				//}
			}
		}
		temp += "\r\n";
	}
    for (int n=0; n<sub; ++n)
    {
        temp += spaceString;
    }
    if (sub<=0)
    {
        temp += "_____________________________________________________\r\n";
    }
    else
        temp +="\r\n";
	return temp;
}

bool tNiceData::existData( const char *keyList, AString *resultInfo )
{
	Array<AString>	stringList;
	AString::Split(keyList, stringList, " ", 0xFFFF);

	bool bAllExist = true;
	for (size_t i=0; i<stringList.size(); ++i)
	{
		if (!exist(stringList[i].c_str()))
		{
			if (resultInfo!=NULL)
			{
				if (resultInfo->length()>0)
					(*resultInfo) += ", ";

				(*resultInfo) += stringList[i];
			}
			bAllExist = false;
		}
	}
	return bAllExist;
}

bool tNiceData::FullJSON(AString scrJsonString)
{
	clear(false);
	if (scrJsonString=="{}")
		return true;
	AString dataString = scrJsonString.SplitBlock('{', '}');
	if (dataString.empty())
		return false;

	if (dataString=="")
		return true;

	while (true)
	{
		if (dataString.empty() || dataString=="")
			break;

		if (*dataString.c_str()!='\"')
			return false;

		AString key = dataString.SplitBlock('\"', '\"');
		if (key.empty())
			return false;

		const char *sz = dataString.c_str();
		if (*sz!=':')
			return false;

		sz += 1;

		if (*sz=='[')
		{
			AString tempValue = dataString.SplitBlock('[', ']', false);
			if (tempValue.empty())
				return false;

			set(key.c_str(), tempValue.c_str());
			// 去掉后面的","
			dataString.SplitLeft(",");	
			continue;
		}
		if (*sz=='{')
		{
			AString tempValue = dataString.SplitBlock('{', '}', true);
			if (tempValue.empty())
				return false;

			AutoNice niceData;
			if (tempValue.length()>1 && tempValue.c_str()[1]=='|')
				niceData = MEM_NEW ArrayNiceData();
			else
				niceData = MEM_NEW NiceData();

			if (!niceData->FullJSON(tempValue))
				return false;

			if (!set(key.c_str(), niceData))
			{
				AssertEx(0, "设置错误");
				return false;
			}
			// 去掉后面的","
			dataString.SplitLeft(",");		
			continue;
		}	

		AString tempValue = dataString.SplitLeft(",");
		if (tempValue.empty())
		{
			tempValue = dataString;
			dataString.setNull();
			if (tempValue.empty())
				return false;
		}

		sz = tempValue.c_str();
		sz += 1;

		if (*sz=='\"')
		{
			AString v = tempValue.SplitBlock('\"', '\"');
			set(key.c_str(), v.c_str());
		}
		else
		{
			if (strstr(tempValue.c_str(), ".")!=NULL)
				set(key.c_str(), (float)atof(sz));
			else if (strlen(tempValue.c_str())>9)
				set(key.c_str(), (UInt64)_atoi64(sz));
			else
				set(key.c_str(), atoi(sz));
		}
	}

	return true;
}

bool tNiceData::FullByStringArray( AString scrStringArray )
{
	AString key;
	while (true)
	{
		if (key.empty())
		{
			key = scrStringArray.SplitBlock('"', '"');
			if (key.empty())
				return true;
		}
		else
		{
			AString val = scrStringArray.SplitBlock('"', '"');
			if (val.empty())
				return false;
			set(key.c_str(), val.c_str());
			key.setNull();
		}
	}

	return true;
}

void tNiceData::setFormat( const char* key, const char* szVal, ... )
{
	AString val;
	va_list va;
	va_start(va, szVal);
	AString tempVal;
	tempVal.Format(va, szVal);
	AData &d = getOrCreate(key);
	d.set(tempVal.c_str());
}

bool tNiceData::SetStringArray( const char *szKey, StringArray &scrArray )
{
	AutoData d = MEM_NEW DataBuffer(128);
	if (d->writeStringArray(scrArray))
		return set(szKey, d);

	return false;
}

bool tNiceData::GetStringArray( const char *szKey, StringArray &resultArray )
{
	AutoData d;
	if (get(szKey, &d, typeid(d)) && d)
	{
		d->seek(0);
		return d->readStringArray(resultArray);
	}
	return false;
}

tBaseTable* tNiceData::getTable( const char *key )
{
	AutoTable t;
	get(key, &t, typeid(t));
	return t.getPtr();
}

//-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*/
#if !USE_IDNICEDATA

AData& NiceData::getOrCreate(const char* key)
{
	AData *p = (AData*)mDataMap.findPtr(key);
	if (p==NULL)
	{
		mDataMap.insert(key, AData());
		return *(AData*)mDataMap.findPtr(key);
	}
	return *p;
}
//----------------------------------------------------------------
#define AUTODATA_COUNT_TYPE		unsigned char
#define AUTODATA_COUNT_MAX		(0xFF)

bool NiceData::serialize(DataStream *destData, bool ) const
{
	AUTODATA_COUNT_TYPE s = (AUTODATA_COUNT_TYPE)count();
	if (count()>AUTODATA_COUNT_MAX)
	{
		ERROR_LOG("NiceData 数量[%llu]超过了 %d\r\n%s"
			, count()
			, (int)AUTODATA_COUNT_MAX
			, dump().c_str() 
			);
		AssertEx(0, "NiceData %d count too most then %d", (int)count(), AUTODATA_COUNT_MAX);
		return false;
	}

	size_t sizePos = destData->tell();
	destData->write(s);

	AUTODATA_COUNT_TYPE saveCount = 0;
	for (auto it = mDataMap.begin(); it.have(); it.next())
	{
		EasyHash<DataKeyType, AData>::Value &val = it.Value();
		FieldInfo f = val.mVal._getField();
		if (!f || !f->needSaveToBuffer() || val.mVal.empty())
		{			
			continue;
		}
		++saveCount;		
		destData->writeString(val.mKey); 
		if (!val.mVal.serialize(destData))
			return false;
	}

	if (saveCount<s)
	{
		size_t nowPos = destData->tell();
		destData->seek(sizePos);
		destData->write(saveCount);
		destData->seek(nowPos);
	}

	return true;
}
//----------------------------------------------------------------

bool NiceData::restore(DataStream *scrData)
{
	mDataMap.clear(false);

	AUTODATA_COUNT_TYPE s = 0;

	if (!scrData->read(s))
		return false;

	for (size_t i=0; i<s; ++i)
	{
		EasyString str;
		if (!scrData->readString(str))
			return false;
		mDataMap.insert(str, AData());
		AData *p = (AData*)mDataMap.findPtr(str);
		if (!p->restore(scrData))
			return false;
	}

	return true;
}

size_t NiceData::append(const tNiceData &scrData, bool bReplace )
{
	if (dynamic_cast<const NiceData*>(&scrData)==NULL)
	{
		ERROR_LOG("NiceData append必须追加的源数据为 NiceData, now [%d]", scrData.getType());
		return 0;
	}
	size_t resultCount = 0;
	for (auto it=scrData.begin(); it->have(); it->next())
	{		
		const AString &key = it->key();
		const AData &d = it->get();

		// 如果替换, 先直接销毁元素, 不替换但如果存在,则直接处理下一个
		if (bReplace)
			remove(key.c_str());
		else if (!getAData(key.c_str()).empty())
			continue;

		mDataMap.insert(key, d);
		++resultCount;
	}

	return resultCount;
}
//-------------------------------------------------------------------------*/
bool NiceData::append( int index, const AData &data, bool bReplace /*= false*/ )
{
	ERROR_LOG(0, "XXX ERROR: NiceData Can not use append int");
	return false;
}

bool NiceData::append( const EasyString &index, const AData &data, bool bReplace /*= false*/ )
{
	if (mDataMap.exist(index))
	{
		if (bReplace)
		{
			mDataMap.insert(index, data);
			return true;
		}		
	}
	else
	{
		mDataMap.insert(index, data);
		return true;
	}
	return false;
}


void NiceData::initData()
{
	for (auto it=mDataMap.begin(); it.have(); it.next())
	{
		auto &kV = it.Value();
		kV.mVal.free();
	}
}


//-------------------------------------------------------------------------
//{"key":"char","key2":10}
AString NiceData::ToJSON()
{
	AString result = "{";
	bool bFrist = true;
	for (auto it=mDataMap.begin(); it.have(); it.next())
	{
		const auto &kV = it.Value();
		const char *szKey = kV.mKey.c_str();
		const AData &d = kV.mVal;

		if (!d.empty())
		{
			Data val = d.get();
			AString valueString;
			FIELD_TYPE type = (FIELD_TYPE)d.getType();
			if (type==FIELD_INT
				|| type==FIELD_FLOAT
				|| type==FIELD_UINT64
				|| type==FIELD_SHORT
				)
			{
				if (bFrist)
					valueString.Format("\"%s\":%s", szKey, val.string().c_str());
				else
					valueString.Format(",\"%s\":%s", szKey, val.string().c_str());
			}
			else if (type==FIELD_NICEDATA)
			{
				AutoNice nice;
				d.get(&nice, typeid(AutoNice));
				AutoNice niceValue = nice;
				if (niceValue)
				{
					AString strVal = niceValue->ToJSON();
					if (bFrist)
						valueString.Format("\"%s\":%s", szKey, strVal.c_str());
					else
						valueString.Format(",\"%s\":%s", szKey, strVal.c_str());
				}
				else
					valueString.Format(",\"%s\":{}", szKey);
			}
			else
			{
				if (bFrist)
					valueString.Format("\"%s\":\"%s\"", szKey, val.string().c_str());
				else
					valueString.Format(",\"%s\":\"%s\"", szKey, val.string().c_str());
			}
			bFrist = false;

			result += valueString;
		}						
	}
	result += "}";

	return result;
}

#endif //USE_IDNICEDATA

//-------------------------------------------------------------------------

//----------------------------------------------------------------

AString ArrayNiceData::dump( int sub, int code ) const
{
	bool bHave = false;
	AString info = ""; //tNiceData::dump(sub, code);
	for (size_t i=0; i<mDataList.size(); ++i)
	{
		//if (!mDataList[i].empty())
		{
			bHave = true;
			info += "\r\n";
			
			if (sub>0)
			{
				for (int n = 0; n<sub; ++n)
				{
					info += spaceString;
				}				
			}
			if (code>0)
			{
				info += code;
				info += "_";
			}
			info += (int)i;
			if (mDataList[i].empty())
			{
				info += ">[NULL]";
				continue;
			}
			info += ">[";
			info += mDataList[i]._getField()->getTypeString();
			info += "] = ";
			if (mDataList[i].getType()==FIELD_NICEDATA ||mDataList[i].getType()==FIELD_ARRAYDATA)
			{
				AutoNice nice;
                if (mDataList[i].getType()==FIELD_NICEDATA)
				    mDataList[i].get(&nice, typeid(AutoNice));
                else
                {
                    AutoArrayData dV;
                    mDataList[i].get(&dV, typeid(dV));
                    nice = dV;
                }
				if (nice)
                {
                     if (mDataList[i].getType()==FIELD_NICEDATA)
                        info += "SUB Nice\r\n";
                     else
                         info += "SUB Array\r\n";
                    for (int n=0; n<sub; ++n)
                    {
                        info += spaceString;
                    }
                    info +="   {";
					info += nice->dump(sub+1, i);		
                    for (int n=0; n<sub; ++n)
                    {
                        info += spaceString;
                    }
                    info += "   }";
                    info += (int)i;
				}
				else
					info += "NULL";
			}
			else
			{
				info += (AString)mDataList[i];
			}
		}
		//else
		//	info += "[NULL]:NULL";
	}
	//if (bHave && sub>0 )
	//	info += "\r\n}\r\n";
    for (int n=0; n<sub; ++n)
    {
        info += spaceString;
    }
    if (sub==0)
        info +="-------------------------------------------------------------------------\r\n";
    else
        info +="\r\n";
	return info;
}

bool ArrayNiceData::append( int index, const AData &data, bool bReplace /*= false*/ )
{
	if (index>0xFFFF)
	{
		ERROR_LOG("数据下标过大, 超过 65535，[%s] 追加失败", ((AString)data).c_str());
		return false;
	}
	if (index>=mDataList.size())
	{
		mDataList.resize(index+1);
	}
	else if (!bReplace)
	{
		if (!mDataList[index].empty())
			return false;
	}

	mDataList[index] = data;
	return true;
}

bool ArrayNiceData::FullJSON(AString scrJsonString)
{
	// 为了支持逗号, 使用  | 符号隔开
	clear(true);
	AString data = scrJsonString.SplitBlock('{', '}');
	if (data.length()>0)
	{		
		if (data.c_str()[0] == '|')
		{
			if (data.length()>1)
			{			
				Array<AString> strlist;
				AString::Split(data.c_str()+1, strlist, "|", 1024);
				mDataList.resize(strlist.size());
				for (int i=0; i<strlist.size(); ++i)
				{
					mDataList[i] = strlist[i];
				}
			}
			return true;
		}
	}
	return false;
}

AString ArrayNiceData::ToJSON()
{
	AString result = "{";	
	for (size_t i=0; i<mDataList.size(); ++i)
	{
		result += "|";
		result += mDataList[i].get().string();
	}
	result += "}";
	return result;
}

bool ArrayNiceData::serialize( DataStream *destData, bool bKeyIndex ) const
{
	if (mDataList.size()>0xFFFF)
	{
		ERROR_LOG("数组下标[%d]超过 65535 >\r\n%s", (int)mDataList.size(), dump().c_str());
		AssertEx(0, "数组下标[%d]超过 65535 >\r\n%s", (int)mDataList.size(), dump().c_str());
		return false;
	}
	destData->write((ushort)mDataList.size());
	for (size_t i=0; i<mDataList.size(); ++i)
	{			
		if (!mDataList[i].serialize(destData))
		{
			ERROR_LOG("数据 [%u] 序列保存失败", i);
			return false;
		}
	}
	return true;
}

bool ArrayNiceData::restore( DataStream *scrData )
{
	ushort num = 0;
	if (!scrData->read(num))
		return false;

	if (mDataList.size()<num)
		mDataList.resize(num);

	for (size_t i=0; i<num; ++i)
	{
		if (!mDataList[i].restore(scrData))
		{
			ERROR_LOG("数据 [%u] type >[%s] 恢复失败", i, mDataList[i]._getField()!=NULL ? mDataList[i]._getField()->getTypeString():"NULL");
			return false;
		}
	}
	return true;
}
