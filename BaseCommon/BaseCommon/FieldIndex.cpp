
#include "FieldIndex.h"
#include "TableTool.h"
#include "BaseFieldInfo.h"

#include "DataStream.h"
#include "TableFieldInfo.h"
#include "BaseRecord.h"
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------

FieldInfo FieldIndex::createFiledInfo( const char* strType )
{
	tFieldInfoFactory *hF = FieldInfoManager::GetMe().getFieldInfoFactory(strType);
	if (hF)
	{ 
		return hF->createFieldInfo();
	}
	return NULL;
}
//------------------------------------------------------------------------------------

FieldInfo FieldIndex::createFiledInfo( int type )
{
	tFieldInfoFactory *hF = FieldInfoManager::GetMe().getFieldInfoFactory(type);
	if (hF)
	{
		return hF->createFieldInfo();
	}
	return NULL;
}
//------------------------------------------------------------------------------------

void FieldIndex::FreeRecord(BaseRecord *pRecord)
{
	MEM_DELETE pRecord;
}
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

FieldIndex::FieldIndex( tBaseTable *pTable,size_t num ) 
: mDataLength(0)
{
	AssertEx(num <= FIELD_MAX_COUNT, "Field count over max allow count [%d]", FIELD_MAX_COUNT);
	mFieldVec.resize(num, NULL);
}
//------------------------------------------------------------------------------------

FieldIndex::~FieldIndex()
{
	for (int i=0; i<(int)mFieldVec.size(); ++i)
	{
		delete mFieldVec[i];
	}
	mFieldVec.clear();
	mFieldMap.clear();
}
//------------------------------------------------------------------------------------

FieldInfo FieldIndex::getFieldInfo( const char *name ) const
{
	short col = -1;

	if ( mFieldMap.findExBy(name, col) )
		return getFieldInfo( col );

	WARN_LOG("Warning: no exist field [%s] in current table", name);
	return NULL;
}
//------------------------------------------------------------------------------------

FieldInfo FieldIndex::getFieldInfo( int col ) const
{
	if ( col>=0 && col<(int)mFieldVec.size() )
	{
		return mFieldVec[col];
	}
	return NULL;
}
//------------------------------------------------------------------------------------

int FieldIndex::getFieldCol( const char *name ) const
{
	short col = -1;

	if ( mFieldMap.findExBy(name, col) )	
		return col;

	return -1;
}
//------------------------------------------------------------------------------------

bool FieldIndex::existField( const char *name ) const
{
	return mFieldMap.exist(name);
}
//------------------------------------------------------------------------------------

bool FieldIndex::operator==( const FieldIndex &other ) const
{
	if (!check())
		return false;
	if (!other.check())
		return false;
	if (mFieldMap.size()!=other.mFieldMap.size())
		return false;

	for ( size_t i=0;i<mFieldVec.size();++i )
	{
		if (mFieldVec[i]->getType()!=other.mFieldVec[i]->getType() ||
			mFieldVec[i]->getName() != other.mFieldVec[i]->getName()
			)
			{
				return false;
			}	
	}
	return true;

}
//------------------------------------------------------------------------------------

FieldIndex& FieldIndex::operator = ( const FieldIndex& other )
{
	AssertEx(mFieldVec.empty()&&mFieldMap.empty(), "Field Index muse is new init when copy from other");

	mFieldVec.resize(other.mFieldVec.size());
	for (size_t i=0;i<mFieldVec.size();++i)
	{
		if (other.mFieldVec[i])
		{
			FieldInfo newInfo = setField(other.mFieldVec[i]->getName().c_str(), other.mFieldVec[i]->getType(), (int)i);
			mFieldVec[i]->setLength(other.mFieldVec[i]->getLength());
			mFieldVec[i]->setRelation(other.mFieldVec[i]->getRelation());
			mFieldVec[i]->setAttributeData(other.mFieldVec[i]->getAttributeData());
			newInfo->CopyData(other.mFieldVec[i]);
		}
	}
	_updateInfo();
	return *this;
}
//------------------------------------------------------------------------------------
FieldInfo FieldIndex::setField( const char *name, int type, int col )
{
	FieldInfo info = createFieldInfo(type);
	if (info)
	{
		if (col>=(int)mFieldVec.size())
			mFieldVec.resize(col+1);
		if (mFieldVec[col]!=NULL)
			delete mFieldVec[col];
		mFieldVec[col] = info;
		mFieldVec[col]->setName(name);
		
		_appendField(name, col);
		
		_updateInfo();
		return mFieldVec[col];
	}

	return NULL;
}
//------------------------------------------------------------------------------------
FieldInfo FieldIndex::setField( const char *name, const char* strType, int col )
{
	FieldInfo info = createFieldInfo(strType);
	if (info)
	{
		if (col>=(int)mFieldVec.size())
			mFieldVec.resize(col+1);
		mFieldVec[col] = info;
		mFieldVec[col]->setName(name);
		
		_appendField(name, col);

		_updateInfo();
		return mFieldVec[col];
	}

	return NULL;
}
//------------------------------------------------------------------------------------
void FieldIndex::deleBackField()
{

}
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------

void FieldIndex::_updateInfo()
{
	size_t pos = 0;
	for(size_t i=0; i<mFieldVec.size(); ++i)
	{
		if (mFieldVec[i])
		{
			mFieldVec[i]->setPosition(pos);
			pos += mFieldVec[i]->getLength();
		}
	}
	mDataLength = pos;

	mUpdateState.resize((getCount()+7)/8, (char)0xFF);
	
}
//------------------------------------------------------------------------------------

int FieldIndex::getFieldType( int col )
{
	FieldInfo info = getFieldInfo(col);
	if (info)
		return info->getType();
	return FIELD_NULL;
}
//------------------------------------------------------------------------------------

bool FieldIndex::saveToData( DataStream *destData ) const
{
	unsigned short num = getCount();
	destData->write(num);
	for( unsigned short i=0; i<num; ++i)
	{
		FieldInfo info = getFieldInfo(i);
		if (info)
		{
			unsigned char type = (unsigned char)info->getType();
			destData->write(type);
			if (!info->saveField(destData))
				return false;
		}
		else
			return false;
	}
	return true;
}
//------------------------------------------------------------------------------------

bool FieldIndex::restoreFromData( DataStream *scrData )
{
	AssertNote(mFieldVec.empty() && mFieldMap.empty(), "Must is new init state when restore from data");
	//_init();
	unsigned short num = 0;
	if (!scrData->read(num))
		return false;
	mFieldVec.resize(num);
	for( unsigned short i=0; i<num; ++i)
	{
		unsigned char type;
		if (!scrData->read(type))
			return false;
		FieldInfo info = createFieldInfo(type);
		if (NULL==info)
		{
			ERROR_LOG("读取错误:不存在字段类型[%u]", type);
			return false;
		}
		if (!info->restoreFromData(scrData))
			return false;
		mFieldVec[i] = info;
		_appendField(info->getName().c_str(), i);
	}
	_updateInfo();
	return true;
}

//------------------------------------------------------------------------------------

bool FieldIndex::check() const
{
	if (mFieldVec.size()<=0)
		return false;

	if (mFieldMap.size()!=mFieldVec.size())
	{
#if DEVELOP_MODE && FIELD_USE_HASH_INDEX
		mFieldMap.Dump();
#endif
		return false;	
	}
	((FieldIndex*)this)->_updateInfo();

	int dataLength = 0;
	for( unsigned short i=0; i<getCount(); ++i)
	{
		if (NULL==mFieldVec[i])
			return false;
		FieldInfo info = getFieldInfo(mFieldVec[i]->getName().c_str());
		if (info!=mFieldVec[i])
			return false;

		dataLength += info->getLength();

		if (info->getType()==FIELD_TABLE)
		{
			DBTableFieldInfo *pInfo = dynamic_cast<DBTableFieldInfo*>(info);
			if (pInfo!=NULL)
			{
				pInfo->CheckField();
			}
		}
	}

	AssertEx(dataLength==getDataLength(), "数据长度不相等");
	return dataLength==getDataLength();

}
//------------------------------------------------------------------------------------

bool FieldIndex::checkFieldName(const AString &fieldNameList) const
{
	if (!check())
		return false;

	Array<AString> strList;
	AString::Split(fieldNameList.c_str(), strList, " ", FIELD_MAX_COUNT);

	if (strList.size() != mFieldVec.size())
		return false;

	bool re = true;
	for (size_t i=0;i<mFieldVec.size();++i)
	{
		if (mFieldVec[i]->getName()!=strList[i])
		{
			ERROR_LOG( "错误字段配置:字段[%s]与要求[%s]不一至!",mFieldVec[i]->getName().c_str(), strList[i].c_str() );
			return false;
		}
	}
	return true;
}
//------------------------------------------------------------------------------------

void FieldIndex::_appendField( const char* fieldName, int col )
{
	if (mFieldMap.erase(fieldName))
		WARN_LOG("已经存在字段 [%s]", fieldName);
	mFieldMap.insert(fieldName, col);
}

FieldInfo FieldIndex::createFieldInfo( int type )
{
	return FieldInfoManager::GetMe().createFieldInfo(type);
}

FieldInfo FieldIndex::createFieldInfo( const char* typeName )
{
	return FieldInfoManager::GetMe().createFieldInfo(typeName);
}

AString FieldIndex::ToString() const
{
	AString result;
    result.Format("%d\t", getCount());
    
	for (int i=0; i<getCount(); ++i)
	{
        FieldInfo info = getFieldInfo(i);
        if (info==NULL)
            return AString();
		AString fieldString;
        AString param = info->getTypeParam();
        if (param.length()>0)        
            fieldString.Format("[%s %s %d<%s>]", info->getName().c_str(), info->getTypeString(), (int)info->getAttributeData(), param.c_str());
        else
            fieldString.Format("[%s %s %d]", info->getName().c_str(), info->getTypeString(), (int)info->getAttributeData() );
        result += fieldString;
	}

	return result;
}

bool FieldIndex::FullFromString( const AString &scrString )
{
    AssertNote(mFieldVec.empty() && mFieldMap.empty(), "Must is new init state when restore from data");

	AString temp = scrString.c_str();
    AString countString = temp.SplitLeft("\t");
    if (countString.length()<=0)
        return false;
    int fieldCount = TOINT(countString.c_str());
    if (fieldCount<=0)
        return true;
    
	for (int col=0; col<fieldCount; ++col)
	{
		AString fieldString = temp.SplitBlock('[', ']');
		if (fieldString.length()<=0)
        {
            ERROR_LOG("Restore field form string fail >%s", scrString.c_str());
            return false;
        }        
        AString name = fieldString.SplitLeft(" ");
        AString type = fieldString.SplitLeft(" ");
        if (name.length()<=0 || type.length()<=0 || fieldString.length()<=0)
        {
            ERROR_LOG("Restore field info form string fail >Name %s, type %s >%s", name.c_str(), type.c_str(), fieldString.c_str());
            return false;
        }
        FieldInfo info = setField(name.c_str(), type.c_str(), col);
        if (info==NULL)
        {
            ERROR_LOG("Set field info form string fail >Name %s, type %s >%s", name.c_str(), type.c_str(), fieldString.c_str());
            return false;
        }
        info->setAttributeData( (StateDataType)TOINT(fieldString.c_str()) );		
        AString paramString = fieldString.SplitBlock('<', '>');
        if (paramString.length()>0)
            info->setTypeParam(paramString);
	}
    _updateInfo();
    if (getCount()!=fieldCount)
    {
        ERROR_LOG("Restore field count is error now %d != %d", getCount(), fieldCount);
        return false;
    }
	return check();
}

bool FieldIndex::_replaceField( int col, FieldInfo newField )
{
	if (col>=0 || col<mFieldVec.size())
	{
		FieldInfo info = mFieldVec[col];
		if (info!=NULL)
		{
			newField->setName(info->getName().c_str());
			mFieldVec[col] = newField;
			delete info;
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
