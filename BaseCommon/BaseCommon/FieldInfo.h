/********************************************************************
	created:	2011/07/10
	created:	10:7:2011   0:36
	filename: 	d:\NewTable__0709\NewTable\Common\DataBase\FieldInfo.h
	file path:	d:\NewTable__0709\NewTable\Common\DataBase
	file base:	FieldInfo
	file ext:	h
	author:		Yang Wenge
	
	purpose:	表格字段信息
*********************************************************************/

#ifndef _INCLUDE_FIELDINFO_H_
#define _INCLUDE_FIELDINFO_H_
#include "BaseCommon.h"
#include "AutoString.h"

#include "Array.h"
#include "EasyHash.h"
#include "EasyString.h"
#include "DataBuffer.h"
#include "Auto.h"
#include "Data.h"

#if __LINUX__
#include <typeinfo>
using namespace std;
#endif
//------------------------------------------------------------------------------------
enum FIELD_ATTRIBUTE
{
    _REPEATED_ARRAY_FLAG_  = 1,
    _SAVE_FLAG_  = 1<<1,
    _DB_FIELD_TABLE = 1<<2,         //使用DB字段子表
};
//------------------------------------------------------------------------------------
class DataStream;
class DataTableManager;
//------------------------------------------------------------------------------------
#define INVALLIDATION_POSITION		(-1)
// 字符描述, 根据不同类型进行保存,读取
class BaseCommon_Export tFieldInfo : public AutoBase
{
public:
	virtual ~tFieldInfo(){}

	void Release(void){ delete this; }

public:
	virtual int getType() const						= 0;
	virtual const char* getTypeString() const		= 0;
	virtual const type_info& getTypeinfo() const	= 0;
	virtual void setLength(DSIZE length)			= 0;
	virtual DSIZE getLength() const					= 0;
	virtual void setPosition(size_t pos)			= 0;
	virtual size_t getPosition(void) const			= 0;
	virtual bool availability() const				= 0;
	//virtual bool onlyRead() const					= 0;
	virtual void setAttribute(int attributeFlag)	= 0;
	virtual bool hasAttribute(int attributeFlag)const= 0;
    virtual void removeAttribute(int attributeFlag) = 0;
	virtual void setAttributeData(StateDataType attributeData)	= 0;
	virtual StateDataType getAttributeData() const				= 0;

	virtual void setName(const char *aName)			= 0;
	virtual const EasyString& getName()const		= 0;

	//virtual void setAttribute(int attributeFlag)	= 0;
	virtual bool needSaveToBuffer(void) const		= 0;

	virtual void init(void *data)					= 0;

	virtual bool set(void *data, int nVal)			= 0;
	virtual bool set(void *data, float fVal)		= 0;
	virtual bool set(void *data, bool bVal)			= 0;
	virtual bool set(void *data, const char* szVal) = 0;
	virtual bool set(void *data, const AString &strVal)					= 0;
	virtual bool set(void *data, UInt64 uVal64)							= 0;

	virtual bool get(void *data, int &nVal) const						= 0;
	virtual bool get(void *data, UInt64 &uVal64) const					= 0;
	virtual bool get(void *data, float &fVal) const						= 0;
	virtual bool get(void *data, bool &bVal) const						= 0;
	virtual bool get(void *data, AString &strAString) const				= 0;

	virtual const char* getString(void *data)	const					= 0;

	virtual bool set(void *data, const void* obj, const type_info &info) = 0;
	virtual bool get(void *data, void* obj, const type_info &info) const = 0;

	virtual bool setData(void *data, const Data *dataVal) = 0;
	// 释放数据, 注意: 数据使用完成后,必须通过这个接口来进行保证内存正确回收
	virtual void free(void *data) = 0;

public:
	virtual bool saveData(void *data, DataStream *destData) const		= 0;
	virtual bool restoreData(void *data, DataStream *scrData)			= 0;

	virtual bool saveField(DataStream *destData) const					= 0;
	virtual bool restoreFromData(DataStream *scrData)					= 0;

	virtual int getIndexValue(void* data) const							= 0;
	virtual int getIndexValue(int nIndex) const							= 0;
	virtual int getIndexValue(const char* strIndex) const				= 0;
	
public:	
	virtual void _ready(void *data)										= 0;
	virtual void setRelation(const char* relationTableIndex)			= 0;
	virtual const char* getRelation(void)								= 0;
	virtual bool readyRelationTable(DataTableManager *tableMgr)			= 0;

	virtual const EasyString& getRelationTableIndex() const				= 0;

	virtual void CopyData(const tFieldInfo *pScrFieldInfo)				= 0;

    virtual bool SaveDataToDB(tBaseTable *pScrTable, DataStream *pDBData){ ERROR_LOG("%s not can use SaveDataToDB", getName().c_str()); return false; }
    virtual bool RestoreFromDB(DataStream *pScrDBData, tBaseTable *destTable)  { ERROR_LOG("%s not can use RestoreFromDB", getName().c_str()); return false; }

public:
	// For redis DB
	virtual AString getTypeParam() const  = 0;
	virtual void setTypeParam(const AString &paramData) = 0;

	virtual void setMaxLength(int maxLen){  }
	virtual int getMaxLength()const { return getLength(); }

	virtual int getDBDataLength() const { return getLength(); }
};
//------------------------------------------------------------------------------------

#ifndef __LINUX__
template<>
void Auto<tFieldInfo>::freeClass()
{
	getPtr()->Release();
}
#endif
//typedef Auto<tFieldInfo>			AutoFieldInfo;
typedef tFieldInfo*					FieldInfo;
//------------------------------------------------------------------------------------

class tFieldInfoFactory : public MemBase
{
public:
	tFieldInfoFactory()
	{
	}
	virtual ~tFieldInfoFactory(){}

	void Release(void){ delete this; }

	virtual int getType() const = 0;
	virtual const char* getTypeName() const = 0;
	virtual tFieldInfo*	createFieldInfo() = 0;

};
//------------------------------------------------------------------------------------
#ifndef __LINUX__
template<>
void AutoPtr<tFieldInfoFactory>::FreeClass()
{
	getPtr()->Release();
}
#endif

typedef AutoPtr<tFieldInfoFactory>		AutoFieldInfoFactory;
//------------------------------------------------------------------------------------

class BaseCommon_Export FieldInfoManager
{
public:
	static FieldInfoManager& GetMe();
	static void FreeAllFieldInfo(void);

	static tFieldInfo* getFieldInfo(int type)
	{
		if (type>0 && type<GetMe().mFieldList.size())
			return GetMe().mFieldList[type];
		return NULL;
	}

	static tFieldInfo* getFieldInfo(const char *szType)
	{
		return getFieldInfo(GetMe().toFieldType(szType));
	}

	static tFieldInfo* getFieldInfo(const type_info &typeInfo)
	{
		int type = GetMe().mTypeIndex.find(typeInfo.name());
		if (type>0)
			return getFieldInfo(type);
		return NULL;
	}

private:    
    void InitFieldInfo();

public:
	FieldInfoManager(){ InitFieldInfo(); };
	~FieldInfoManager()
	{
		ClearAllFieldInfo();
	}

	tFieldInfo* createFieldInfo(int typeIndex);
	tFieldInfo* createFieldInfo(const char* typeName);

	int toFieldType(const char* typeName);

	const char* toFieldTypeName(int typeIndex);

	void addFieldInfoFactory(tFieldInfoFactory *hFact);

	tFieldInfoFactory* getFieldInfoFactory(int typeIndex);
	tFieldInfoFactory* getFieldInfoFactory(const char* typeName);

	void removeFieldInfoFactory(int typeIndex);
	void removeFieldInfoFactory(const char* typeName);

	void ClearAllFieldInfo(void);

	template<typename T>
	void appendDataFieldOnTypeInfo(FIELD_TYPE fieldType)
	{
		mTypeIndex.insert(typeid(T).name(), fieldType);		
	}

private:
	Array<tFieldInfoFactory*>			mFieldInfoFactoryArray;
	EasyHash<AString, tFieldInfoFactory*>	mFieldInfoFactoryMapOnName;
	EasyHash<AString, int> mTypeIndex;
	Array<tFieldInfo*> mFieldList;
};
//------------------------------------------------------------------------------------
//enum FIELD_ATTRIBUTE
//{
//	FIELD_ATTRIBUTE_ONLYREAD = 1,
//};
//------------------------------------------------------------------------------------
class BaseCommon_Export BaseFieldInfo : public tFieldInfo
{
public:
	BaseFieldInfo()
		: mPosition(INVALLIDATION_POSITION)
	{
		
	}
	virtual ~BaseFieldInfo(){}

public:
	virtual void setName(const char *aName){ mName = aName; }
	virtual const EasyString& getName() const{ return mName; }
	virtual void setPosition(size_t pos){ mPosition = (unsigned short)(pos&0xffff); }
	virtual size_t getPosition(void) const { return mPosition; }
	virtual void setLength(DSIZE length){ }

	virtual bool availability() const { return mPosition!=INVALLIDATION_POSITION; }
	//virtual bool onlyRead() const { return mAttrState.isOpen(FIELD_ATTRIBUTE_ONLYREAD); }
	

	virtual bool saveField(DataStream *destData) const;
	virtual bool restoreFromData(DataStream *scrData);

	virtual void setAttribute(int attributeFlag) { mAttrState.open(attributeFlag); }
	virtual bool hasAttribute(int attributeFlag) const { return mAttrState.isOpen(attributeFlag);	}
    virtual void removeAttribute(int attributeFlag) { mAttrState.close(attributeFlag); }
	virtual void setAttributeData(StateDataType attributeData) { mAttrState = attributeData;		}
	virtual StateDataType getAttributeData() const { return mAttrState; }

	virtual bool needSaveToBuffer(void) const { return true; }

	//virtual void init(void *data) = 0;

public:
	virtual void _ready(void *data){}
	virtual void setRelation(const char* relationTableIndex) {}
	virtual const char* getRelation(void)	{ return NULL; }
	virtual bool readyRelationTable(DataTableManager *tableMgr) { return false; }

	virtual const EasyString& getRelationTableIndex() const { static EasyString temp; return temp; }

	virtual bool set(void *data, const void* obj, const type_info &info) { return false; }
	virtual bool get(void *data, void* obj, const type_info &info) const { return false; }

	virtual const char* getString(void *data) const { return NULL; }

	virtual AString getTypeParam() const  { return AString(); }
	virtual void setTypeParam(const AString &paramData) {}

	virtual void CopyData(const tFieldInfo *pScrFieldInfo){ }

public:
	template<typename T>
	static void __Set(T &o) { new (&o) T; }

	template<typename T>
	static void __Free(T &o) { (&o)->~T(); }

protected:
	EasyString			mName;
	unsigned short 		mPosition;
	StateData			mAttrState;
};
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

#endif