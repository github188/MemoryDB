/********************************************************************
	created:	2011/09/21
	created:	21:9:2011   16:45
	filename: 	e:\01_Langrisser\third\DataTable\DataBase\ExtFieldInfo.h
	file path:	e:\01_Langrisser\third\DataTable\DataBase
	file base:	ExtFieldInfo
	file ext:	h
	author:		Wenge Yang
	description:
	purpose:	Ext type info, sample unsigned 64 bit int
*********************************************************************/

#ifndef _INCLUDE_EXTFIELD_H_
#define _INCLUDE_EXTFIELD_H_

#include "FieldInfo.h"
#include "FieldTypeDefine.h"


//---------------------------------------------------------------------------------------------------
//用于共享内存表格的字符串字段信息, 特点时,动态指定长度, 且使用固定长度进行保存, 创建后不能再改变
class CharStringFieldInfo : public BaseFieldInfo
{
public:
	CharStringFieldInfo()
		: mLength(8)
	{

	}
	virtual ~CharStringFieldInfo(){}

	virtual int getType() const { return FIELD_CHAR_STRING; }
	virtual const char* getTypeString() const { return TYPE_CHARSTRING_NAME; }
	virtual const type_info& getTypeinfo() const { return typeid(char*); }
	virtual DSIZE getLength() const { return mLength; }
	virtual void setLength(DSIZE length){ mLength = length + 1; /* last char is '\0' end */  }

	bool saveField( DataStream *destData ) const
	{
		BaseFieldInfo::saveField(destData);
		destData->write(mLength);
		return true;
	}

	bool restoreFromData( DataStream *scrData )
	{
		if (BaseFieldInfo::restoreFromData(scrData))
			return scrData->read(mLength);
		return false;
	}

	virtual void init(void *data)
	{
		_free(data);
	}

	virtual bool set(void *data, int nVal) 
	{
		_free(data);
		sprintf(_ptr(data), "%d", nVal );
		return true;
	}

	virtual bool set(void *data, float fVal)
	{
		_free(data);
		sprintf(_ptr(data), "%f", fVal );
		return true;
	}
	virtual bool set(void *data, bool bVal)
	{
		sprintf(_ptr(data), "%d", bVal);
		return true;
	}
	virtual bool set(void *data, const char* szVal)
	{
		_free(data);
		if (szVal)
		{
			size_t len = strlen(szVal);
			if ((DSIZE)len<getLength())
			{
				sprintf(_ptr(data), "%s", szVal);
			}
			else
				return false;

		}
		return true;
	}

	virtual bool set(void *data, const AString &strVal)
	{
#ifdef __WINDOWS__
        sprintf_s(_ptr(data), getLength(), "%s", strVal.c_str());
#else
		sprintf(_ptr(data), "%s", strVal.c_str());
#endif
		return true;
	}

	virtual bool setData(void *data, const Data *dataVal)
	{
		return set(data, dataVal->string());
	}

	virtual bool get(void *data, int &nVal) const
	{
		nVal = atoi(_ptr_const(data));
		return true;
	}

	virtual bool get(void *data, float &fVal) const
	{
		fVal = (float)atof(_ptr_const(data));
		return true;
	}
	virtual bool get(void *data, bool &bVal) const
	{
		bVal = atoi(_ptr_const(data))!=0;
		return true;
	}
	
	virtual const char* getString(void* data) const { return _ptr_const(data); }

	virtual bool get(void *data, AString &strAString) const
	{
		strAString.setNull();
		strAString += _ptr_const(data);
		return true;
	}

	virtual bool set(void *data, const void* obj, const type_info &info) 
	{
		return false; 
	}
	virtual bool get(void *data, void* obj, const type_info &info) const 
	{
		return false; 
	}

	virtual bool set(void *data, UInt64 uVal64)
	{
		sprintf(_ptr(data), "%llu", uVal64);
		return true;
	}
	virtual bool get(void *data, UInt64 &uVal64) const
	{
		return sscanf(_ptr_const(data), "%llu", &uVal64 )==1;
		return sscanf(_ptr_const(data), "%llu", &uVal64 )==1;
		return true;
	}

	virtual bool saveData(void *data, DataStream *destData) const 
	{
		destData->writeString(_ptr_const(data));
		return true;
	}
	virtual bool restoreData(void *data, DataStream *scrData)
	{
		_free(data);
		AString temp;
		if (scrData->readString(temp))
		{
			set(data, temp.c_str());
			return true;
		}
		return false;
	}

	virtual void free(void *data)
	{
		_free(data);
	}

	virtual int getIndexValue(void* data) const;

	virtual int getIndexValue(int nIndex) const;
	virtual int getIndexValue(const char* strIndex) const;

protected:
	char* _ptr(void *data){ return ((char*)data + mPosition); }

	const char* _ptr_const(void *data) const { return ((char*)data + mPosition); }

	void _free(void* data){ memset(_ptr(data), 0, getLength()); }

protected:
	DSIZE		mLength;
};

class CharStringFieldInfoFactory : public tFieldInfoFactory
{
public:
	virtual int getType() const{ return FIELD_CHAR_STRING; }
	virtual const char* getTypeName() const { return TYPE_CHARSTRING_NAME; }
	virtual tFieldInfo*	createFieldInfo()
	{
		return MEM_NEW CharStringFieldInfo();
	}
};
//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------

//无符号64位整数
class UINT64FieldInfo : public BaseFieldInfo
{
public:
	virtual ~UINT64FieldInfo(){}

	virtual int getType() const { return FIELD_UINT64; }
	virtual const char* getTypeString() const{ return TYPE_UINT64_NAME; }
	virtual const type_info& getTypeinfo() const { return typeid(UInt64); }
	virtual DSIZE getLength() const { return sizeof(UInt64); }

	virtual bool set(void *data, int nVal) 
	{
		*_ptr(data) = nVal;
		return true;
	}
	virtual bool set(void *data, float fVal)
	{
		*_ptr(data) = (int)fVal;
		return true;
	}
	virtual bool set(void *data, bool bVal)
	{
		*_ptr(data) = (int)(bVal ? 1 : 0);
		return true;
	}
	virtual bool set(void *data, const char* szVal)
	{
		return (sscanf(szVal, ("%llu"), _ptr(data))==1);
			return true;
		return false;
	}
	virtual bool set(void *data, const AString &strVal)
	{		
		return set(data, strVal.c_str());
		return true;
	}

	virtual bool get(void *data, int &nVal) const
	{
		nVal = (int)((*_ptr_const(data)) & 0XFFFFFFFF);
		return true;
	}
	virtual bool get(void *data, float &fVal) const
	{
		fVal = (float)(*_ptr_const(data));
		return true;
	}
	virtual bool get(void *data, bool &bVal) const
	{
		bVal = *_ptr_const(data) != 0;
		return true;
	}

	virtual bool set(void *data, UInt64 uVal64)
	{
		*_ptr(data) = uVal64;
		return true;
	}

	virtual bool get(void *data, UInt64 &uVal64) const
	{
		uVal64 = (*_ptr_const(data));
		return true;
	}

	virtual bool get(void *data, AString &strAString) const
	{
		strAString.setNull();
		strAString += *_ptr_const(data);
		return true;
	}

	virtual bool set(void *data, const void* obj, const type_info &info)
	{
		if (info==typeid(UInt64))
		{
			*_ptr(data) = *((UInt64*)obj);
			return true;
		}
		return false;
	}
	virtual bool get(void *data, void* obj, const type_info &info) const
	{
		if (info==typeid(UInt64))
		{
			*((UInt64*)obj) = *_ptr_const(data);
			return true;
		}
		return false;
	}

	virtual bool setData(void *data, const Data *dataVal)
	{
		(*_ptr(data)) = (*dataVal);
		return true;
	}

	virtual bool saveData(void *data, DataStream *destData) const 
	{
		destData->write(*_ptr_const(data));
		return true;
	}
	virtual bool restoreData(void *data, DataStream *scrData)
	{
		return scrData->read(*_ptr(data));
	}		

	virtual void init(void *data){ *_ptr(data)= 0; }

	virtual void free(void *data)
	{
		*_ptr(data)= 0;
	}

	virtual int getIndexValue(void* data) const { return (int)(*_ptr_const(data)); }
	virtual int getIndexValue(int nIndex) const { return nIndex; }
	virtual int getIndexValue(const char* strIndex) const { return atoi(strIndex); }

protected:
	UInt64* _ptr(void *data){ return (UInt64*)((char*)data + mPosition); }

	const UInt64* _ptr_const(void *data) const { return (UInt64*)((char*)data + mPosition); }
};
//---------------------------------------------------------------------------------------------------
class UINT64FieldInfoFactory : public tFieldInfoFactory
{
public:
	virtual int getType() const { return FIELD_UINT64; }
	virtual const char* getTypeName() const { return TYPE_UINT64_NAME; }
	virtual tFieldInfo*	createFieldInfo()
	{
		return MEM_NEW UINT64FieldInfo();
	}
};
//---------------------------------------------------------------------------------------------------
// 二进制数据字段, 固定长度
//---------------------------------------------------------------------------------------------------

//class BaseCommon_Export BinaryDataFieldInfo : public BaseFieldInfo
//{
//protected:
//	DSIZE	mLength;
//
//public:
//	BinaryDataFieldInfo()		
//		: mLength(8)
//	{
//
//	}
//	virtual ~BinaryDataFieldInfo(){}
//
//	virtual int getType() const { return FIELD_BINARY; }
//	virtual const char* getTypeString()const{ return FIELD_BINARY_NAME; }
//	virtual const type_info& getTypeinfo() const { return typeid(BinaryDataFieldInfo); }
//
//	virtual DSIZE getLength() const { return mLength; }
//	virtual void setLength(DSIZE length){ mLength = length; }
//
//	bool saveField( DataStream *destData ) const
//	{
//		BaseFieldInfo::saveField(destData);
//		destData->write(mLength);
//		return true;
//	}
//
//	bool restoreFromData( DataStream *scrData )
//	{
//		if (BaseFieldInfo::restoreFromData(scrData))
//			return scrData->read(mLength);
//		return false;
//	}
//
//
//	virtual bool set(void *data, int nVal) { return false; }
//
//	virtual bool set(void *data, float fVal){ return false; }
//	virtual bool set(void *data, bool bVal){ return false; }
//	virtual bool set(void *data, const char* szVal){ return false; }
//	virtual bool set(void *data, const AString &strVal){ return false; }
//
//	virtual bool get(void *data, int &nVal) const { return false; }
//	virtual bool get(void *data, float &fVal) const { return false; }
//	virtual bool get(void *data, bool &bVal) const{ return false; }
//
//	virtual bool set(void *data, UInt64 uVal64){ return false; }
//	virtual bool get(void *data, UInt64 &uVal64) const{  return false; }
//
//	virtual bool get(void *data, Data &destStringData) const { return false; }
//	virtual bool get(void *data, AString &strAString) const;
//
//	// only use function to set data
//	virtual bool set(void *data, const void* obj, const type_info &info);
//
//	virtual bool get(void *data, void* obj, const type_info &info) const ;
//
//
//	virtual bool setData(void *data, const Data *dataVal);
//
//	virtual bool saveData(void *data, DataStream *destData) const 
//	{		
//		destData->write(mLength);
//		destData->_write((void*)_ptr_const(data), getLength());
//		return true; 
//	}
//	virtual bool restoreData(void *data, DataStream *scrData)
//	{
//		DSIZE len = 0;
//		if (scrData->read(len) && len==getLength())
//		{
//			return scrData->_read(_ptr(data), getLength());
//		}
//		return false; 
//	}
//
//	virtual void free(void *data)
//	{
//		_free(data);
//	}
//
//	virtual int getIndexValue(void* data) const { return 0; }
//	virtual int getIndexValue(int nIndex) const { return 0; }
//	virtual int getIndexValue(const char* strIndex) const { return 0; }
//
//protected:
//	char* _ptr(void *data){ return ((char*)data + mPosition); }
//
//	const char* _ptr_const(void *data) const { return ((char*)data + mPosition); }
//
//	bool _saveData(void *thisData, void* scrData, size_t len)
//	{
//		if (scrData!=NULL && (DSIZE)len<=getLength())
//		{
//			memcpy(_ptr(thisData), scrData, len);
//			memset(_ptr(thisData)+len, 0, getLength()-len);
//			return true;
//		}
//		return false;
//	}
//
//	bool _loadData(void *thisData, void *destData, size_t destLen)
//	{
//		if (destData!=NULL && (DSIZE)destLen>=getLength())
//		{
//			memcpy(destData, _ptr(thisData), getLength());
//			memset( (char*)destData+getLength(), 0, destLen-getLength() );
//			return true;
//		}
//		return false;
//	}
//
//	virtual void init(void *data){ memset(_ptr(data), 0, getLength());   }
//
//	void _free(void* data)
//	{ 
//#if DEVELOP_MODE
//		// 配合错误检测, 可以不进行处理
//		memset(_ptr(data), 0, getLength()); 
//#endif
//	}
//
//
//};
////---------------------------------------------------------------------------------------------------
//class BinaryDataFieldInfoFactory : public tFieldInfoFactory
//{
//public:
//	virtual int getType() const { return FIELD_BINARY; }
//	virtual const char* getTypeName() const { return FIELD_BINARY_NAME; }
//	virtual tFieldInfo*	createFieldInfo()
//	{
//		return MEM_NEW BinaryDataFieldInfo();
//	}
//};
////---------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------

#endif //_INCLUDE_EXTFIELD_H_