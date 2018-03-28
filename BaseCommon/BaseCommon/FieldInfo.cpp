#include "FieldInfo.h"
#include "FieldTypeDefine.h"
#include "BaseFieldInfo.h"
#include "TableTool.h"
//#include "NiceTable.h"
#include "AutoObjectFieldInfo.h"
//#include "RelationFieldInfo.h"
#include "NiceData.h"
#include "ExtFieldInfo.h"

#include "TableManager.h"

#include "TableFieldInfo.h"
#include "RecordFieldInfo.h"

#include "Lock.h"

#include <Windows.h>
//-------------------------------------------------------------------------
namespace FieldInfoName
{
	extern const char tableName[]	= TYPE_TABLE_NAME;
	extern const char recordName[]	= TYPE_RECORD_NAME;
	extern const char dataName[]	= TYPE_DATA_NAME;
	extern const char niceName[]	= TYPE_NICEDATA_NAME;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
class AutoDataFieldInfo : public AutoObjFieldInfo<AutoData, FIELD_DATA>
{
protected:
	int mMaxLength;

public:
	AutoDataFieldInfo()
		: mMaxLength(DB_BLOB_FIELD_DEFAULT_LENGTH){}

public:
	virtual const char* getTypeString()const{ return TYPE_DATA_NAME; }

	virtual void setMaxLength(int maxLen) override { mMaxLength = maxLen;  }
	virtual int getMaxLength() const override { return mMaxLength; }
	virtual int getDBDataLength() const override { return getMaxLength()+sizeof(DSIZE); }	// 加4个字节保存长度

	virtual AString getTypeParam() const{ return AString(STRING((int)mMaxLength)); }
	virtual void setTypeParam( const AString &typeParam ){ int len = TOINT(typeParam.c_str()); if (len>0)  mMaxLength = len; }

	virtual bool saveField(DataStream *destData) const override
	{
		BaseFieldInfo::saveField(destData);
		destData->write(getMaxLength());
		return true;
	}

	virtual bool restoreFromData(DataStream *scrData) override
	{
		if (BaseFieldInfo::restoreFromData(scrData))
		{
			if (scrData->read(mMaxLength))
			{				
				return true;
			}
			mMaxLength = DB_BLOB_FIELD_DEFAULT_LENGTH;
		}
		return false;
	}

	virtual bool saveData(void *data, DataStream *destData) const
	{
		AutoData temp;
		if (get(data, &temp, typeid(AutoData)))
		{
			DSIZE size = 0;
			if (temp)
			{
				size = temp->dataSize();
				//if (size==0)
				//	size = temp->size();			
			}

			return destData->writeData(temp.getPtr(), size);		
		}
		return false;
	}

	virtual bool restoreData(void *data, DataStream *scrData)
	{
		AutoData tempData = MEM_NEW DataBuffer();
		if (scrData->readData(tempData.getPtr()))
		{
			return set(data, &tempData, typeid(AutoData));
		}
		return false;
	}

	virtual bool get( void *data, AString &strAString ) const
	{
		// NOTE: 不返回任何字符串数据, 目前不支持与字符串进行自动转换
		return false;
		//AutoData temp;
		//if (get(data, &temp, typeid(AutoData)) && temp)
		//{
		//	DSIZE size = temp->dataSize();
		//	if (size==0)
		//		size = temp->size();			

		//	strAString = TableTool::BinaryToString(temp->data(), size, "0x");
		//	return true;
		//}

		//return false;
	}

	virtual bool get(void *data, void* obj, const type_info &info) const 
	{
		if (typeid(AutoData)!=info)
			return false;
		*((AutoData*)obj) = *_ptr_const(data);
		return true;
	}
};
//----------------------------------------------------------------------------------------------------------------------
class AutoDataFieldInfoFactory : public tFieldInfoFactory
{
public:
	AutoDataFieldInfoFactory(){ }

	virtual int getType() const { return FIELD_DATA; }
	virtual const char* getTypeName() const { return TYPE_DATA_NAME; }
	virtual tFieldInfo*	createFieldInfo()
	{
		return MEM_NEW AutoDataFieldInfo();
	}
};
//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
class AutoNiceFieldInfo : public AutoObjFieldInfo<AutoNice, FIELD_NICEDATA>
{
protected:
    AString     mTypeParam;

public:
	virtual const char* getTypeString()const{ return TYPE_NICEDATA_NAME; }
	virtual int getMaxLength() const override { return DB_BLOB_FIELD_DEFAULT_LENGTH; }
	virtual int getDBDataLength() const override { return getMaxLength()+sizeof(DSIZE); }	// 加4个字节保存长度

    virtual AString getTypeParam() const  { return mTypeParam; }
    virtual void setTypeParam(const AString &paramData) { mTypeParam = paramData; }

    virtual bool saveField(DataStream *destData) const override
    {
        AutoObjFieldInfo<AutoNice, FIELD_NICEDATA>::saveField(destData);
        destData->writeString(mTypeParam);
        return true;
    }
    virtual bool restoreFromData(DataStream *scrData) override
    {
        if (AutoObjFieldInfo<AutoNice, FIELD_NICEDATA>::restoreFromData(scrData))
            return scrData->readString(mTypeParam);
        return false;
    }

	virtual bool saveData(void *data, DataStream *destData) const
	{
		AutoNice temp;
		if (AutoObjFieldInfo<AutoNice, FIELD_NICEDATA>::get(data, &temp, typeid(AutoNice)))
		{
			byte dataType = (byte)(temp ? temp->getType() : NULL_NICEDATA);
			//!!! 使用ID方式序列速度要慢于NiceData，所以不在转换成ID存储
			//if (dataType==NICEDATA)
			//	dataType = (byte)ID_NICEDATA;
			destData->write(dataType);
			if (temp)
				return temp->serialize(destData, false);
			return true;
		}
		return false;
	}

	virtual bool restoreData(void *data, DataStream *scrData)
	{
		byte dataType = NULL_NICEDATA;
		if (scrData->read(dataType))
		{
			if (dataType!=NULL_NICEDATA)
			{
				AutoNice tempNice;
				//if (dataType==ID_NICEDATA)
				//	tempNice = MEM_NEW IDNiceData();
				//else 
					if (dataType==ARRAY_NICEDATA)
					tempNice = MEM_NEW ArrayNiceData();
				else
					tempNice = MEM_NEW NiceData();
				bool b = tempNice->restore(scrData);
				if (b)
					AutoObjFieldInfo<AutoNice, FIELD_NICEDATA>::set(data, &tempNice, typeid(tempNice));
				return b;
			}
			return true;
		}
		return false;
	}


	virtual bool set(void *data, const char* szVal)
	{ 
		AutoNice d;
		AutoObjFieldInfo<AutoNice, FIELD_NICEDATA>::get(data, &d, typeid(AutoNice));
		if (!d)
			d = MEM_NEW NiceData();
		if (d->FullJSON(szVal))
			return AutoObjFieldInfo<AutoNice, FIELD_NICEDATA>::set(data, &d, typeid(AutoNice));
		return false; 
	}
	virtual bool get(void *data, AString &strAString) const 
	{ 
		AutoNice dV;
		AutoObjFieldInfo<AutoNice, FIELD_NICEDATA>::get(data, &dV, typeid(AutoNice));
		if (dV)
			strAString = dV->ToJSON();
		else
			strAString = "";
		return true; 
	}
};
//----------------------------------------------------------------------------------------------------------------------
class AutoNiceFieldInfoFactory : public tFieldInfoFactory
{
public:
	AutoNiceFieldInfoFactory(){ }

	virtual int getType() const { return FIELD_NICEDATA; }
	virtual const char* getTypeName() const { return TYPE_NICEDATA_NAME; }
	virtual tFieldInfo*	createFieldInfo()
	{
		return MEM_NEW AutoNiceFieldInfo();
	}
};


//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
class AutoArrayDataFieldInfo : public AutoObjFieldInfo<AutoArrayData, FIELD_ARRAYDATA>
{
    typedef AutoObjFieldInfo<AutoArrayData, FIELD_ARRAYDATA> base;
protected:
    AString     mTypeParam;

public:
    virtual const char* getTypeString()const{ return TYPE_ARRAYDATA_NAME; }
    virtual int getMaxLength() const override { return DB_BLOB_FIELD_DEFAULT_LENGTH; }
    virtual int getDBDataLength() const override { return getMaxLength()+sizeof(DSIZE); }	// 加4个字节保存长度

    virtual AString getTypeParam() const  { return mTypeParam; }
    virtual void setTypeParam(const AString &paramData) { mTypeParam = paramData; }

    virtual bool saveField(DataStream *destData) const override
    {
        base::saveField(destData);
        destData->writeString(mTypeParam);
        return true;
    }
    virtual bool restoreFromData(DataStream *scrData) override
    {
        if (base::restoreFromData(scrData))
            return scrData->readString(mTypeParam);
        return false;
    }

    virtual bool saveData(void *data, DataStream *destData) const
    {
        AutoArrayData temp;
        if (base::get(data, &temp, typeid(AutoArrayData)))
        {
            byte dataType = (byte)(temp ? temp->getType() : NULL_NICEDATA);
            //!!! 使用ID方式序列速度要慢于NiceData，所以不在转换成ID存储
            //if (dataType==NICEDATA)
            //	dataType = (byte)ID_NICEDATA;
            destData->write(dataType);
            if (temp)
                return temp->serialize(destData, false);
            return true;
        }
        return false;
    }

    virtual bool restoreData(void *data, DataStream *scrData)
    {
        byte dataType = NULL_NICEDATA;
        if (scrData->read(dataType))
        {
            if (dataType==ARRAY_NICEDATA)
            {
                AutoArrayData tempNice;
                    tempNice = MEM_NEW ArrayNiceData();
                bool b = tempNice->restore(scrData);
                if (b)
                    base::set(data, &tempNice, typeid(tempNice));
                return b;
            }
            return true;
        }
        return false;
    }


    virtual bool set(void *data, const char* szVal)
    { 
        AutoArrayData d;
        base::get(data, &d, typeid(AutoArrayData));
        if (!d)
            d = MEM_NEW ArrayNiceData();
        if (d->FullJSON(szVal))
            return base::set(data, &d, typeid(AutoArrayData));
        return false; 
    }
    virtual bool get(void *data, AString &strAString) const 
    { 
        AutoArrayData dV;
        base::get(data, &dV, typeid(AutoArrayData));
        if (dV)
            strAString = dV->ToJSON();
        else
            strAString = "";
        return true; 
    }
};
//----------------------------------------------------------------------------------------------------------------------
class AutoArrayFieldInfoFactory : public tFieldInfoFactory
{
public:
    AutoArrayFieldInfoFactory(){ }

    virtual int getType() const { return FIELD_ARRAYDATA; }
    virtual const char* getTypeName() const { return TYPE_ARRAYDATA_NAME; }
    virtual tFieldInfo*	createFieldInfo()
    {
        return MEM_NEW AutoArrayDataFieldInfo();
    }
};


//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
void FieldInfoManager::InitFieldInfo()
{
	ClearAllFieldInfo();

		appendDataFieldOnTypeInfo<AString>(FIELD_STRING);
	
		appendDataFieldOnTypeInfo<AutoData>(FIELD_DATA);
		appendDataFieldOnTypeInfo<AutoTable>(FIELD_TABLE);
		appendDataFieldOnTypeInfo<AutoRecord>(FIELD_RECORD);
		appendDataFieldOnTypeInfo<AutoNice>(FIELD_NICEDATA);
	    appendDataFieldOnTypeInfo<AutoArrayData>(FIELD_ARRAYDATA);
		//appendDataFieldOnTypeInfo<ABaseTable>(FIELD_DB_TABLE);
	
		appendDataFieldOnTypeInfo<int>(FIELD_INT);
		appendDataFieldOnTypeInfo<float>(FIELD_FLOAT);
		appendDataFieldOnTypeInfo<bool>(FIELD_BOOL);
		appendDataFieldOnTypeInfo<byte>(FIELD_BYTE);
		appendDataFieldOnTypeInfo<short>(FIELD_SHORT);
		appendDataFieldOnTypeInfo<UInt64>(FIELD_UINT64);
		appendDataFieldOnTypeInfo<double>(FIELD_DOUBLE);

	mFieldInfoFactoryArray.resize(FIELD_TYPE_MAX);

	addFieldInfoFactory(MEM_NEW IntFieldInfoFactory);
	addFieldInfoFactory(MEM_NEW FloatFieldInfoFactory);
	addFieldInfoFactory(MEM_NEW DoubleFieldInfoFactory);
	addFieldInfoFactory(MEM_NEW StringFieldInfoFactory);
	addFieldInfoFactory(MEM_NEW BoolFieldInfoFactory);
	addFieldInfoFactory(MEM_NEW ByteFieldInfoFactory);
	addFieldInfoFactory(MEM_NEW ShortFieldInfoFactory);
	//addFieldInfoFactory(MEM_NEW IntRelationFieldInfoFactory);
	//addFieldInfoFactory(MEM_NEW StringRelationFieldInfoFactory);
	addFieldInfoFactory(MEM_NEW CharStringFieldInfoFactory);
	//addFieldInfoFactory(MEM_NEW BinaryDataFieldInfoFactory);
	addFieldInfoFactory(MEM_NEW UINT64FieldInfoFactory);

	addFieldInfoFactory(MEM_NEW DB_TableFieldInfoFactory());
	//addFieldInfoFactory(MEM_NEW TableFieldInfoFactory());
	addFieldInfoFactory(MEM_NEW AutoDataFieldInfoFactory());
	
	addFieldInfoFactory(MEM_NEW AutoNiceFieldInfoFactory());
	addFieldInfoFactory(MEM_NEW AutoRecordFieldInfoFactory());
    addFieldInfoFactory(MEM_NEW AutoArrayFieldInfoFactory());

	//addFieldInfoFactory(MEM_NEW AutoObjFieldInfoFactory<AutoTable, FIELD_TABLE, FieldInfoName::tableName>);
	//addFieldInfoFactory(MEM_NEW AutoObjFieldInfoFactory<AutoRecord, FIELD_RECORD, FieldInfoName::recordName>);
	//addFieldInfoFactory(MEM_NEW AutoObjFieldInfoFactory<AutoData, FIELD_DATA, FieldInfoName::dataName>);
	//addFieldInfoFactory(MEM_NEW AutoObjFieldInfoFactory<AutoNice, FIELD_NICEDATA, FieldInfoName::dataName>);

	mFieldList.resize(mFieldInfoFactoryArray.size());
	for (int i=0; i<mFieldInfoFactoryArray.size(); ++i)
	{
		if (mFieldInfoFactoryArray[i]!=NULL)
		{
			mFieldList[i] = mFieldInfoFactoryArray[i]->createFieldInfo();
			if (mFieldList[i])
				mFieldList[i]->setPosition(0);
		}
	}

	// 优化HASH查寻
	mFieldInfoFactoryMapOnName.OptimizeSpeed(10.5f);
}
CsLock gLock;
void FieldInfoManager::addFieldInfoFactory(tFieldInfoFactory *hFact )
{
	CsLockTool l(gLock);
	if (mFieldInfoFactoryArray[hFact->getType()]!=NULL)
		delete mFieldInfoFactoryArray[hFact->getType()];

	mFieldInfoFactoryArray[hFact->getType()] = hFact;
	mFieldInfoFactoryMapOnName.insert(hFact->getTypeName(), hFact);
}

tFieldInfoFactory* FieldInfoManager::getFieldInfoFactory( const char* typeName )
{
	return mFieldInfoFactoryMapOnName.find(typeName);
}

tFieldInfoFactory* FieldInfoManager::getFieldInfoFactory( int typeIndex )
{
	if (typeIndex>FIELD_NULL && typeIndex<FIELD_TYPE_MAX)
		return mFieldInfoFactoryArray[typeIndex];

	return NULL;
}

tFieldInfo* FieldInfoManager::createFieldInfo( int typeIndex )
{
	tFieldInfoFactory *hF = getFieldInfoFactory(typeIndex);
	if (hF)
		return hF->createFieldInfo();
	return NULL;
}

tFieldInfo* FieldInfoManager::createFieldInfo( const char* typeName )
{
	tFieldInfoFactory *hF = getFieldInfoFactory(typeName);
	if (hF)
		return hF->createFieldInfo();
	return NULL;
}

void FieldInfoManager::FreeAllFieldInfo( void )
{
	GetMe().ClearAllFieldInfo();
}

void FieldInfoManager::removeFieldInfoFactory( int typeIndex )
{
	tFieldInfoFactory *hF = getFieldInfoFactory(typeIndex);
	if (hF)
	{		
		mFieldInfoFactoryMapOnName.erase(hF->getTypeName());
		mFieldInfoFactoryArray[typeIndex] = NULL;
		delete hF;
	}
}

void FieldInfoManager::removeFieldInfoFactory( const char* typeName )
{
	tFieldInfoFactory *hF = getFieldInfoFactory(typeName);
	if (hF)
	{
		mFieldInfoFactoryArray[hF->getType()] = NULL;
		mFieldInfoFactoryMapOnName.erase(hF->getTypeName());
		delete hF;
	}
}

int FieldInfoManager::toFieldType( const char* typeName )
{
	tFieldInfoFactory *hF = getFieldInfoFactory(typeName);
	if (hF)
		return hF->getType();
	return FIELD_NULL;
}

const char* FieldInfoManager::toFieldTypeName( int typeIndex )
{
	tFieldInfoFactory *hF = getFieldInfoFactory(typeIndex);
	if (hF)
		return hF->getTypeName();
	return NULL;
}

void FieldInfoManager::ClearAllFieldInfo( void )
{
	for (int i=0; i<mFieldInfoFactoryArray.size(); ++i)
	{
		if (mFieldInfoFactoryArray[i])
			delete mFieldInfoFactoryArray[i];
	}
	mFieldInfoFactoryArray.clear();
	mFieldInfoFactoryMapOnName.clear();

	for (int i=0; i<mFieldList.size(); ++i)
	{
		if (mFieldList[i]!=NULL)
			delete mFieldList[i];
	}
	mFieldList.clear();
}

FieldInfoManager& FieldInfoManager::GetMe()
{
#if FIELD_INFO_THREAD_SAFE
	FieldInfoManager *pMgr = FieldThreadTool::ThreadFieldMgr();
	if (pMgr!=NULL)
		return *pMgr;
#endif
	static  FieldInfoManager temp;
	return temp;
}

bool BaseFieldInfo::saveField( DataStream *destData ) const
{
	destData->writeString(mName.c_str());
    destData->write((StateDataType)mAttrState);
	return true;
}

bool BaseFieldInfo::restoreFromData( DataStream *scrData )
{
	if (!scrData->readString(mName))
        return false;
    StateDataType sData;
    READ(scrData, sData);
    mAttrState = sData;
    return true;
}