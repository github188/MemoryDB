/********************************************************************
	created:	2011/07/11
	created:	11:7:2011   0:06
	filename: 	d:\NewTable__0709\NewTable\Common\DataBase\AutoObjectFieldInfo.h
	file path:	d:\NewTable__0709\NewTable\Common\DataBase
	file base:	AutoObjectFieldInfo
	file ext:	h
	author:		Yang Wenge
	
	purpose:	对象类型，如：表格，记录，DataStream
*********************************************************************/

#ifndef _INCLUDE_AUTOOBJECTFIELDINFO_H_
#define _INCLUDE_AUTOOBJECTFIELDINFO_H_

#include "FieldInfo.h"
#include "FieldTypeDefine.h"
#include <string>
//---------------------------------------------------------------------------------------------------
//保存对象
template<typename T, int TypeIndex, bool saveToBuffer = true>
class AutoObjFieldInfo : public BaseFieldInfo
{
	friend class DataTool;
public:
	virtual ~AutoObjFieldInfo(){}

	virtual int getType() const { return TypeIndex; }
	virtual const char* getTypeString()const{ return "AUTO_DEFAULT"; }
	virtual const type_info& getTypeinfo() const { return typeid(T); }
	virtual DSIZE getLength()const{ return sizeof(T); }

	virtual bool needSaveToBuffer(void) const { return saveToBuffer; }

	virtual bool set(void *data, int nVal) { return false; }

	virtual bool set(void *data, float fVal){ return false; }
	virtual bool set(void *data, bool bVal){ return false; }
	virtual bool set(void *data, const char* szVal){ return false; }
	virtual bool set(void *data, const AString &strVal){ return false; }

	virtual bool get(void *data, int &nVal) const { return false; }
	virtual bool get(void *data, float &fVal) const { return false; }
	virtual bool get(void *data, bool &bVal) const{ return false; }

	virtual bool set(void *data, UInt64 uVal64){ return false; }
	virtual bool get(void *data, UInt64 &uVal64) const{  return false; }

	virtual bool get(void *data, Data &destStringData) const { return false; }
	virtual bool get(void *data, AString &strAString) const { return false; }

	virtual bool set(void *data, const void* obj, const type_info &info) 
	{
		if (typeid(T)!=info)
			return false;
		*_ptr(data) = *((T*)obj);
		return true;
	}
	virtual bool get(void *data, void* obj, const type_info &info) const 
	{
		if (typeid(T)!=info)
			return false;
		*((T*)obj) = *_ptr_const(data);
		return true;
	}

	virtual bool setData(void *data, const Data *dataVal)
	{
		if (dataVal->comType(this))
		{
			T	temp;
			if (dataVal->get(&temp, typeid(T)))
				return set(data, &temp, typeid(T));
		}
		return false; 
	}

	virtual bool saveData(void *data, DataStream *destData) const { return true; }
	virtual bool restoreData(void *data, DataStream *scrData){ return true; }

	virtual void init(void *data)
	{
		__Set(*_ptr(data));
	}
	virtual void free(void *data)
	{
		__Free(*_ptr(data));
	}

	virtual int getIndexValue(void* data) const { return 0; }
	virtual int getIndexValue(int nIndex) const { return 0; }
	virtual int getIndexValue(const char* strIndex) const { return 0; }

protected:
	T* _ptr(void *data){ return (T*)((char*)data + mPosition); }

	const T* _ptr_const(void *data) const { return (T*)((char*)data + mPosition); }

	//void _free(void* data){ (*_ptr(data)).setNull(); }


};
//---------------------------------------------------------------------------------------------------


template<typename T, int TypeIndex, const char* typeName>
class AutoObjFieldInfoFactory : public tFieldInfoFactory
{
public:
	AutoObjFieldInfoFactory(){ }

	virtual int getType() const { return TypeIndex; }
	virtual const char* getTypeName() const { return typeName; }
	virtual tFieldInfo*	createFieldInfo()
	{
		return MEM_NEW AutoObjFieldInfo<T, TypeIndex>();
	}
};
//---------------------------------------------------------------------------------------------------

#endif