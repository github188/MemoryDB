/********************************************************************
	created:	2011/07/10
	created:	10:7:2011   0:51
	filename: 	d:\NewTable__0709\NewTable\Common\DataBase\BaseFieldInfo.h
	file path:	d:\NewTable__0709\NewTable\Common\DataBase
	file base:	BaseFieldInfo
	file ext:	h
	author:		Yang Wenge
	
	purpose:	
*********************************************************************/
#ifndef _INCLUDE_BASEFIELDINFO_H_
#define _INCLUDE_BASEFIELDINFO_H_

#include "FieldInfo.h"
#include "DataStream.h"
#include "EasyString.h"
#include "TableTool.h"
#include "FieldTypeDefine.h"

#include "Data.h"

//整数
class IntFieldInfo : public BaseFieldInfo
{
public:
	virtual ~IntFieldInfo(){}

	virtual int getType() const { return FIELD_INT; }
	virtual const char* getTypeString() const{ return TYPE_INT_NAME; }
	virtual const type_info& getTypeinfo() const { return typeid(int); }
	virtual DSIZE getLength() const { return sizeof(int); }

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
		*_ptr(data) = atoi(szVal);
		return true;
	}
	virtual bool set(void *data, const AString &strVal)
	{
		*_ptr(data) = atoi(strVal.c_str());
		return true;
	}

	virtual bool get(void *data, int &nVal) const
	{
		nVal = *_ptr_const(data);
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


	virtual bool get(void *data, AString &strAString) const
	{
		strAString.setNull();
		strAString += *_ptr_const(data);
		return true;
	}

	virtual bool set(void *data, UInt64 uVal64)
	{
		*_ptr(data) =  (int)uVal64 & 0xffffffff;
		return true;
	}
	virtual bool get(void *data, UInt64 &uVal64) const
	{
		uVal64 = *_ptr_const(data);
		return true;
	}

	typedef int FIELD_VALUE_TYPE;

	virtual bool set(void *data, const void* obj, const type_info &info)
	{
		if (info==typeid(FIELD_VALUE_TYPE))
		{
			*_ptr(data) = *((FIELD_VALUE_TYPE*)obj);
			return true;
		}
		return false;
	}
	virtual bool get(void *data, void* obj, const type_info &info) const
	{
		if (info==typeid(FIELD_VALUE_TYPE))
		{
			*((FIELD_VALUE_TYPE*)obj) = *_ptr_const(data);
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

	virtual void init(void *data){ *_ptr(data)= 0;  }

	virtual void free(void *data)
	{
		*_ptr(data)= 0;
	}

	virtual int getIndexValue(void* data) const { return *_ptr_const(data); }
	virtual int getIndexValue(int nIndex) const { return nIndex; }
	virtual int getIndexValue(const char* strIndex) const { return atoi(strIndex); }

protected:
	int* _ptr(void *data){ return (int*)((char*)data + mPosition); }

	const int* _ptr_const(void *data) const { return (int*)((char*)data + mPosition); }
};
class IntFieldInfoFactory : public tFieldInfoFactory
{
public:
	virtual int getType() const { return FIELD_INT; }
	virtual const char* getTypeName() const { return TYPE_INT_NAME; }
	virtual tFieldInfo*	createFieldInfo()
	{
		return MEM_NEW IntFieldInfo();
	}
};
//---------------------------------------------------------------------------------------------------
//浮点数
class FloatFieldInfo : public BaseFieldInfo
{
public:
	virtual ~FloatFieldInfo(){}

	virtual int getType() const { return FIELD_FLOAT; }
	virtual const char* getTypeString() const { return TYPE_FLOAT_NAME; }
	virtual const type_info& getTypeinfo() const { return typeid(float); }
	virtual DSIZE getLength() const{ return sizeof(float); }

	virtual bool set(void *data, int nVal) 
	{
		*_ptr(data) = (float)nVal;
		return true;
	}
	virtual bool set(void *data, float fVal)
	{
		*_ptr(data) = fVal;
		return true;
	}
	virtual bool set(void *data, bool bVal)
	{
		*_ptr(data) = (float)(bVal ? 1 : 0);
		return true;
	}
	virtual bool set(void *data, const char* szVal)
	{
		*_ptr(data) = (float)atof(szVal);
		return true;
	}
	virtual bool set(void *data, const AString &strVal)
	{
		*_ptr(data) = (float)atof(strVal.c_str());
		return true;
	}

	virtual bool setData(void *data, const Data *dataVal)
	{
		*_ptr(data) = *dataVal;
		return true;
	}

	virtual bool get(void* data, int &nVal) const
	{
		nVal = (int)*_ptr_const(data);
		return true;
	}
	virtual bool get(void* data, float &fVal) const
	{
		fVal = *_ptr_const(data);
		return true;
	}
	virtual bool get(void *data, bool &bVal) const
	{
		bVal = *_ptr_const(data) > 0.0000001 || *_ptr_const(data) < -0.0000001;
		return true;
	}

	virtual bool set(void *data, UInt64 uVal64)
	{
		*_ptr(data) =  (float)uVal64;
		return true;
	}
	virtual bool get(void *data, UInt64 &uVal64) const
	{
		uVal64 = (UInt64)(*_ptr_const(data));
		return true;
	}

	virtual bool get(void *data, AString &strAString) const
	{
		strAString.setNull();
		strAString += *_ptr_const(data);
		return true;
	}

	typedef float FIELD_VALUE_TYPE;

	virtual bool set(void *data, const void* obj, const type_info &info)
	{
		if (info==typeid(FIELD_VALUE_TYPE))
		{
			*_ptr(data) = *((FIELD_VALUE_TYPE*)obj);
			return true;
		}
		return false;
	}
	virtual bool get(void *data, void* obj, const type_info &info) const
	{
		if (info==typeid(FIELD_VALUE_TYPE))
		{
			*((FIELD_VALUE_TYPE*)obj) = *_ptr_const(data);
			return true;
		}
		return false;
	}

	virtual bool saveData(void* data, DataStream *destData) const 
	{
		destData->write(*_ptr_const(data));
		return true;
	}
	virtual bool restoreData(void *data, DataStream *scrData)
	{
		return scrData->read(*_ptr(data));
	}

	virtual void init(void *data){ *_ptr(data)= 0;  }
	virtual void free(void *data)
	{
		*_ptr(data)= 0.0f;
	}

	virtual int getIndexValue(void* data) const
	{
		AString temp;
		if (get(data, temp))
		{
			return MAKE_INDEX_ID(temp.c_str());
		}
		return 0; 
	}

	virtual int getIndexValue(int nIndex) const { return nIndex; }
	virtual int getIndexValue(const char* strIndex) const { return atoi(strIndex); }

protected:
	float* _ptr(void *data){ return (float*)((char*)data + mPosition); }

	const float* _ptr_const(void *data) const { return (float*)((char*)data + mPosition); }
};
class FloatFieldInfoFactory : public tFieldInfoFactory
{
public:
	virtual int getType() const { return FIELD_FLOAT; }
	virtual const char* getTypeName() const { return TYPE_FLOAT_NAME; }
	virtual tFieldInfo*	createFieldInfo()
	{
		return MEM_NEW FloatFieldInfo();
	}
};

//---------------------------------------------------------------------------------------------------
//浮点数
class DoubleFieldInfo : public BaseFieldInfo
{
public:
	virtual ~DoubleFieldInfo(){}

	virtual int getType() const { return FIELD_DOUBLE; }
	virtual const char* getTypeString() const { return TYPE_DOUBLE_NAME; }
	virtual const type_info& getTypeinfo() const { return typeid(double); }
	virtual DSIZE getLength() const{ return sizeof(double); }

	virtual bool set(void *data, int nVal) 
	{
		*_ptr(data) = (double)nVal;
		return true;
	}
	virtual bool set(void *data, float fVal)
	{
		*_ptr(data) = fVal;
		return true;
	}
	virtual bool set(void *data, bool bVal)
	{
		*_ptr(data) = (float)(bVal ? 1 : 0);
		return true;
	}
	virtual bool set(void *data, const char* szVal)
	{
		*_ptr(data) = (float)atof(szVal);
		return true;
	}
	virtual bool set(void *data, const AString &strVal)
	{
		*_ptr(data) = (float)atof(strVal.c_str());
		return true;
	}

	virtual bool setData(void *data, const Data *dataVal)
	{
		*_ptr(data) = (float)*dataVal;
		return true;
	}

	virtual bool get(void* data, int &nVal) const
	{
		nVal = (int)*_ptr_const(data);
		return true;
	}
	virtual bool get(void* data, float &fVal) const
	{
		fVal = (float)*_ptr_const(data);
		return true;
	}
	virtual bool get(void *data, bool &bVal) const
	{
		bVal = *_ptr_const(data) > 0.0000001 || *_ptr_const(data) < -0.0000001;
		return true;
	}

	virtual bool set(void *data, UInt64 uVal64)
	{
		*_ptr(data) =  (float)uVal64;
		return true;
	}
	virtual bool get(void *data, UInt64 &uVal64) const
	{
		uVal64 = (UInt64)(*_ptr_const(data));
		return true;
	}

	virtual bool get(void *data, AString &strAString) const
	{
		strAString.setNull();
		strAString += (float)*_ptr_const(data);
		return true;
	}

	typedef double FIELD_VALUE_TYPE;

	virtual bool set(void *data, const void* obj, const type_info &info)
	{
		if (info==typeid(FIELD_VALUE_TYPE))
		{
			*_ptr(data) = *((FIELD_VALUE_TYPE*)obj);
			return true;
		}
		return false;
	}
	virtual bool get(void *data, void* obj, const type_info &info) const
	{
		if (info==typeid(FIELD_VALUE_TYPE))
		{
			*((FIELD_VALUE_TYPE*)obj) = *_ptr_const(data);
			return true;
		}
		return false;
	}

	virtual bool saveData(void* data, DataStream *destData) const 
	{
		destData->write(*_ptr_const(data));
		return true;
	}
	virtual bool restoreData(void *data, DataStream *scrData)
	{
		return scrData->read(*_ptr(data));
	}
	virtual void init(void *data){ *_ptr(data)= 0.0f;   }
	virtual void free(void *data)
	{
		*_ptr(data)= 0.0f;
	}

	virtual int getIndexValue(void* data) const
	{
		AString temp;
		if (get(data, temp))
		{
			return MAKE_INDEX_ID(temp.c_str());
		}
		return 0; 
	}

	virtual int getIndexValue(int nIndex) const { return nIndex; }
	virtual int getIndexValue(const char* strIndex) const { return atoi(strIndex); }

protected:
	double* _ptr(void *data){ return (double*)((char*)data + mPosition); }

	const double* _ptr_const(void *data) const { return (double*)((char*)data + mPosition); }
};
class DoubleFieldInfoFactory : public tFieldInfoFactory
{
public:
	virtual int getType() const { return FIELD_DOUBLE; }
	virtual const char* getTypeName() const { return TYPE_DOUBLE_NAME; }
	virtual tFieldInfo*	createFieldInfo()
	{
		return MEM_NEW DoubleFieldInfo();
	}
};
//---------------------------------------------------------------------------------------------------
//字符串
// NOTE: 在DB记录中, 禁止使用此字段, 原因是, 在后台线程与主线转递数据时, 使用直接内存数据处理
//-------------------------------------------------------------------------
class StringFieldInfo : public BaseFieldInfo
{
protected:
	int		mMaxLength;

public:
	StringFieldInfo()
		: mMaxLength(32){}
	virtual ~StringFieldInfo(){}

public:
	virtual int getType() const { return FIELD_STRING; }
	virtual const char* getTypeString() const { return TYPE_STRING_NAME; }
	virtual const type_info& getTypeinfo() const { return typeid(EasyString); }
	virtual DSIZE getLength() const { return sizeof(EasyString); }

	virtual void setMaxLength(int maxLen) override { mMaxLength = maxLen; }
	virtual int getMaxLength()const override { return mMaxLength; }
	virtual int getDBDataLength() const override { return mMaxLength+sizeof(StrLenType); }	// 加2个字节保存长度

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
			mMaxLength = 32;
		}
		return false;
	}

	virtual void CopyData( const tFieldInfo *pScrFieldInfo )
	{ 
		if (pScrFieldInfo->getType()==getType())  
		{
			const StringFieldInfo *p = dynamic_cast<const StringFieldInfo*>(pScrFieldInfo);
			mMaxLength = p->mMaxLength;
		}
	}

	virtual void init(void *data)
	{
		__Set(*_ptr(data));
		//EasyString	temp;
		//memcpy(_ptr(data), &temp, sizeof(EasyString));
	}

	virtual bool set(void *data, int nVal) 
	{
		_free(data);
		StringTool str;
	
		_ptr(data)->set(str.ToString(nVal));
		return true;
	}

	virtual bool set(void *data, float fVal)
	{
		_free(data);
		StringTool str;	
		_ptr(data)->set(str.ToString(fVal));
		return true;
	}
	virtual bool set(void *data, bool bVal)
	{
		set(data, (bVal ? "1" : "0"));
		return true;
	}
	virtual bool set(void *data, const char* szVal)
	{
		_free(data);
		if (szVal!=NULL)
		{
			EasyString *str = _ptr(data);
			str->set(szVal);
		}
		return true;
	}

	virtual bool set(void *data, const AString &strVal)
	{
		_free(data);		
		EasyString *str = _ptr(data);
		*str = strVal;		
		return true;
	}

	virtual bool setData(void *data, const Data *dataVal)
	{
		return set(data, dataVal->string());
	}

	virtual bool get(void *data, int &nVal) const
	{
		nVal = atoi(_ptr_const(data)->c_str());
		return true;
	}

	virtual bool get(void *data, float &fVal) const
	{
		fVal = (float)atof(_ptr_const(data)->c_str());
		return true;
	}
	virtual bool get(void *data, bool &bVal) const
	{
		bVal = atoi(_ptr_const(data)->c_str())!=0;
		return true;
	}

	virtual bool set(void *data, UInt64 uVal64)
	{
		char temp[48];
		sprintf(temp, "%llu", uVal64);
		*_ptr(data) = temp;
		return true;
	}
	virtual bool get(void *data, UInt64 &uVal64) const
	{
		const EasyString &p = (*_ptr_const(data));
		return sscanf(p.c_str(), "%llu", &uVal64 )==1;
		return true;
	}	

	virtual bool get(void *data, AString &strAString) const
	{
		strAString = *_ptr_const(data);
		return true;
	}

	virtual bool set(void *data, const void* obj, const type_info &info) 
	{
		if (info==getTypeinfo())
		{
			*_ptr(data) = *((EasyString*)obj);
			return true;
		}
		return false; 
	}
	virtual bool get(void *data, void* obj, const type_info &info) const 
	{
		if (info==getTypeinfo())
		{
			*((EasyString*)obj) = *_ptr_const(data);
			return true;
		}
		return false; 
	}

	virtual const char* getString(void *data) const { return _ptr_const(data)->c_str(); }

	virtual bool saveData(void *data, DataStream *destData) const 
	{
		return destData->writeString(*_ptr_const(data));
		//return true;
	}
	virtual bool restoreData(void *data, DataStream *scrData)
	{
		_free(data);
		
		return scrData->readString(*_ptr(data));		
		
		//return false;
	}

	virtual void free(void *data)
	{
		__Free(*_ptr(data));
		//_free(data);
	}

	virtual int getIndexValue(void* data) const
	{
		return MAKE_INDEX_ID(_ptr_const(data)->c_str());
	}

	virtual int getIndexValue(int nIndex) const { StringTool t; return MAKE_INDEX_ID( t.ToString(nIndex) ); }
	virtual int getIndexValue(const char* strIndex) const { return MAKE_INDEX_ID(strIndex); }

protected:
	EasyString* _ptr(void *data){ return (EasyString*)((char*)data + mPosition); }

	const EasyString* _ptr_const(void *data) const { return (EasyString*)((char*)data + mPosition); }

	void _free(void* data){ __Free(*_ptr(data)); }
};
class StringFieldInfoFactory : public tFieldInfoFactory
{
public:
	virtual int getType() const{ return FIELD_STRING; }
	virtual const char* getTypeName() const { return TYPE_STRING_NAME; }
	virtual tFieldInfo*	createFieldInfo()
	{
		return MEM_NEW StringFieldInfo();
	}
};
//---------------------------------------------------------------------------------------------------
//布尔类型
class BoolFieldInfo : public BaseFieldInfo
{
public:
	virtual ~BoolFieldInfo(){}

	virtual int getType() const { return FIELD_BOOL; }
	virtual const char* getTypeString() const { return TYPE_BOOL_NAME; }
	virtual const type_info& getTypeinfo() const { return typeid(bool); }
	virtual DSIZE getLength() const{ return sizeof(bool); }

	virtual bool set(void *data, int nVal) 
	{
		*_ptr(data) = (nVal!=0);
		return true;
	}
	virtual bool set(void *data, float fVal)
	{
		*_ptr(data) = fVal>0.0000001 || fVal<-0.0000001;
		return true;
	}
	virtual bool set(void *data, bool bVal)
	{
		*_ptr(data) = bVal;
		return true;
	}
	virtual bool set(void *data, const char* szVal)
	{
		*_ptr(data) = boolFromString(szVal);
		return true;
	}
	virtual bool set(void *data, const AString &strVal)
	{
		*_ptr(data) = boolFromString(strVal.c_str());
		return true;
	}
	virtual bool setData(void *data, const Data *dataVal)
	{
		*_ptr(data) = *dataVal;
		return true;
	}
	virtual bool get(void* data, int &nVal) const
	{
		nVal = (*_ptr_const(data)) ? 1 : 0;
		return true;
	}
	virtual bool get(void* data, float &fVal) const
	{
		fVal = (*_ptr_const(data)) ? 1.0f : 0.0f;
		return true;
	}
	virtual bool get(void *data, bool &bVal) const
	{
		bVal = *_ptr_const(data);
		return true;
	}

	virtual bool set(void *data, UInt64 uVal64){ *_ptr(data) = uVal64!=0;return true; }
	virtual bool get(void *data, UInt64 &uVal64) const{  uVal64 = *_ptr_const(data) ? 1 : 0;return true; }


	virtual bool get(void *data, AString &strAString) const
	{
		strAString =  (*_ptr_const(data)) ? "1" : "0";
		return false;
	}

	typedef bool FIELD_VALUE_TYPE;

	virtual bool set(void *data, const void* obj, const type_info &info)
	{
		if (info==typeid(FIELD_VALUE_TYPE))
		{
			*_ptr(data) = *((FIELD_VALUE_TYPE*)obj);
			return true;
		}
		return false;
	}
	virtual bool get(void *data, void* obj, const type_info &info) const
	{
		if (info==typeid(FIELD_VALUE_TYPE))
		{
			*((FIELD_VALUE_TYPE*)obj) = *_ptr_const(data);
			return true;
		}
		return false;
	}


	virtual bool saveData(void* data, DataStream *destData) const 
	{
		char val = (*_ptr_const(data)) ? 1 : 0;
		destData->write(val);
		return true;
	}
	virtual bool restoreData(void *data, DataStream *scrData)
	{
		char val;
		bool re = scrData->read(val);
		if (re)
		{
			*_ptr(data) = (val==1 ? true : false);
			return true;
		}
		return false;
	}
	virtual void init(void *data){ *_ptr(data)= false;  }
	virtual void free(void *data)
	{
		*_ptr(data)= 0;
	}

	virtual int getIndexValue(void* data) const
	{
		//StringData temp;
		//if (get(data, temp))
		//{
		//	return MAKE_INDEX_ID(temp.getString());
		//}
		return 0; 
	}

	virtual int getIndexValue(int nIndex) const { return nIndex; }
	virtual int getIndexValue(const char* strIndex) const { return atoi(strIndex); }

protected:
	bool* _ptr(void *data){ return (bool*)((char*)data + mPosition); }

	const bool* _ptr_const(void *data) const { return (bool*)((char*)data + mPosition); }

	bool boolFromString(const char* szString) const
	{
		return atoi(szString)!=0 ||
			//strcmp(szString, "Yes")==0 ||
			//strcmp(szString, "YES")==0 ||
			//strcmp(szString, "true")==0 ||
			strcmp(szString, "True")==0 ||
			strcmp(szString, "TRUE")==0;

	}
};
class BoolFieldInfoFactory : public tFieldInfoFactory
{
public:
	virtual int getType() const { return FIELD_BOOL; }
	virtual const char* getTypeName() const { return TYPE_BOOL_NAME; }
	virtual tFieldInfo*	createFieldInfo()
	{
		return MEM_NEW BoolFieldInfo();
	}
};
//---------------------------------------------------------------------------------------------------
//单字节类型
class ByteFieldInfo : public BaseFieldInfo
{
public:
	virtual ~ByteFieldInfo(){}

	virtual int getType() const { return FIELD_BYTE; }
	virtual const char* getTypeString() const{ return TYPE_BYTE_NAME; }
	virtual const type_info& getTypeinfo() const { return typeid(unsigned char); }
	virtual DSIZE getLength() const { return sizeof(char); }

	virtual bool set(void *data, int nVal) 
	{
		*_ptr(data) = (char)nVal&0xff;
		return true;
	}
	virtual bool set(void *data, float fVal)
	{
		*_ptr(data) = (char)((int)fVal)&0xFF;
		return true;
	}
	virtual bool set(void *data, bool bVal)
	{
		*_ptr(data) = (char)(bVal ? 1 : 0);
		return true;
	}
	virtual bool set(void *data, const char* szVal)
	{
		*_ptr(data) = (char)atoi(szVal)&0xFF;
		return true;
	}
	virtual bool set(void *data, const AString &strVal)
	{
		*_ptr(data) = (char)((int)atoi(strVal.c_str())) && 0xFF;
		return true;
	}

	virtual bool get(void *data, int &nVal) const
	{
		nVal = *_ptr_const(data);
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
	
	virtual bool set(void *data, UInt64 uVal64){ *_ptr(data) = (char)uVal64&0xff;return true; }
	virtual bool get(void *data, UInt64 &uVal64) const{  uVal64 = *_ptr_const(data);return true; }


	virtual bool get(void *data, AString &strAString) const
	{
		strAString.setNull();
		strAString += (int)*_ptr_const(data);
		return true;
	}

	typedef unsigned char FIELD_VALUE_TYPE;

	virtual bool set(void *data, const void* obj, const type_info &info)
	{
		if (info==typeid(FIELD_VALUE_TYPE))
		{
			*_ptr(data) = *((FIELD_VALUE_TYPE*)obj);
			return true;
		}
		return false;
	}
	virtual bool get(void *data, void* obj, const type_info &info) const
	{
		if (info==typeid(FIELD_VALUE_TYPE))
		{
			*((FIELD_VALUE_TYPE*)obj) = *_ptr_const(data);
			return true;
		}
		return false;
	}

	virtual bool setData(void *data, const Data *dataVal)
	{
		int n = (*dataVal);
		(*_ptr(data)) = (char) n & 0xFF;
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
	virtual void init(void *data){ *_ptr(data)= 0;  }
	virtual void free(void *data)
	{
		*_ptr(data)= 0;
	}

	virtual int getIndexValue(void* data) const { return *_ptr_const(data); }
	virtual int getIndexValue(int nIndex) const { return nIndex; }
	virtual int getIndexValue(const char* strIndex) const { return atoi(strIndex); }

protected:
	unsigned char* _ptr(void *data){ return ((unsigned char*)data + mPosition); }
	const unsigned char* _ptr_const(void *data) const { return ((unsigned char*)data + mPosition); }
};
class ByteFieldInfoFactory : public tFieldInfoFactory
{
public:
	virtual int getType() const { return FIELD_BYTE; }
	virtual const char* getTypeName() const { return TYPE_BYTE_NAME; }
	virtual tFieldInfo*	createFieldInfo()
	{
		return MEM_NEW ByteFieldInfo();
	}
};
//---------------------------------------------------------------------------------------------------
//16位整数
class ShortFieldInfo : public BaseFieldInfo
{
public:
	virtual ~ShortFieldInfo(){}

	virtual int getType() const { return FIELD_SHORT; }
	virtual const char* getTypeString() const{ return TYPE_SHORT_NAME; }
	virtual const type_info& getTypeinfo() const { return typeid(short); }
	virtual DSIZE getLength() const { return sizeof(short); }

	virtual bool set(void *data, int nVal) 
	{
		*_ptr(data) = (short)nVal;
		return true;
	}
	virtual bool set(void *data, float fVal)
	{
		*_ptr(data) = (short)fVal;
		return true;
	}
	virtual bool set(void *data, bool bVal)
	{
		*_ptr(data) = (short)(bVal ? 1 : 0);
		return true;
	}
	virtual bool set(void *data, const char* szVal)
	{
		*_ptr(data) = (short)atoi(szVal) & 0xFFFF;
		return true;
	}
	virtual bool set(void *data, const AString &strVal)
	{
		*_ptr(data) = (short)atoi(strVal.c_str()) & 0xFFFF;
		return true;
	}

	virtual bool get(void *data, int &nVal) const
	{
		nVal = (int)*_ptr_const(data);
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
	
	virtual bool set(void *data, UInt64 uVal64){ *_ptr(data) = (short)uVal64&0xffff; return true; }
	virtual bool get(void *data, UInt64 &uVal64) const{  uVal64 = *_ptr_const(data); return true; }


	virtual bool get(void *data, AString &strAString) const
	{
		strAString.setNull();
		strAString += (int)*_ptr_const(data);
		return true;
	}

	typedef short FIELD_VALUE_TYPE;

	virtual bool set(void *data, const void* obj, const type_info &info)
	{
		if (info==typeid(FIELD_VALUE_TYPE))
		{
			*_ptr(data) = *((FIELD_VALUE_TYPE*)obj);
			return true;
		}
		return false;
	}
	virtual bool get(void *data, void* obj, const type_info &info) const
	{
		if (info==typeid(FIELD_VALUE_TYPE))
		{
			*((FIELD_VALUE_TYPE*)obj) = *_ptr_const(data);
			return true;
		}
		return false;
	}

	virtual bool setData(void *data, const Data *dataVal)
	{
		(*_ptr(data)) = (int)(*dataVal);
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
	virtual void init(void *data){ *_ptr(data)= 0;  }
	virtual void free(void *data)
	{
		*_ptr(data) = 0;
	}

	virtual int getIndexValue(void* data) const { return *_ptr_const(data); }
	virtual int getIndexValue(int nIndex) const { return nIndex; }
	virtual int getIndexValue(const char* strIndex) const { return atoi(strIndex); }

protected:
	short* _ptr(void *data){ return (short*)((char*)data + mPosition); }

	const short* _ptr_const(void *data) const { return (short*)((char*)data + mPosition); }
};
class ShortFieldInfoFactory : public tFieldInfoFactory
{
public:
	virtual int getType() const { return FIELD_SHORT; }
	virtual const char* getTypeName() const { return TYPE_SHORT_NAME; }
	virtual tFieldInfo*	createFieldInfo()
	{
		return MEM_NEW ShortFieldInfo();
	}
};
//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------
#endif //_INCLUDE_BASEFIELDINFO_H_