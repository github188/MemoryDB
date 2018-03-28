/********************************************************************
created:	2013/02/26
created:	26:2:2013   21:21
filename: 	C:\NewGame\Common\DataBase\tNiceData.h
file path:	C:\NewGame\Common\DataBase
file base:	tNiceData
file ext:	h
author:		Wenge Yang

purpose:	数据容器
支持保存自身指针对象, 可以保存结构数据
*********************************************************************/
#ifndef _INCLUDE_ADATA_H_
#define _INCLUDE_ADATA_H_


#include "FieldTypeDefine.h"
#include "FieldInfo.h"
#include "Data.h"
#include "Array.h"
#include "Auto.h" 
#include "NiceDataFieldInfo.h"
#include "AutoObjectFieldInfo.h"
#include "EasyHash.h"
//-------------------------------------------------------------------------*/

typedef EasyString			DataKeyType;

//-------------------------------------------------------------------------*/
enum NICEDATA_TYPE
{
	NULL_NICEDATA,
	//ID_NICEDATA,
	NICEDATA,
	ARRAY_NICEDATA,
};

class DataStream;
//----------------------------------------------------------------
class BaseCommon_Export AData : public MemBase
{
	friend class tNiceData;
	friend class NiceData;
	//friend class IDNiceData;
	friend class Array<AData>;

	typedef tFieldInfo*					FieldInfo;

public:	
	AData()
	{
		memset(mData, 0, sizeof(mData));
	}


	~AData()
	{
		free();
	}
	//----------------------------------------------------------------
	AData(const AData &other)
	{
		memset(mData, 0, sizeof(mData));
		*this = other;
	}
	//-------------------------------------------------------------------------
	bool empty() const { return mData[0]==FIELD_NULL; }
	//-------------------------------------------------------------------------
	AData& operator = (const AData &other)
	{		
		int otherInfo = (int)other.mData[0];
		if (otherInfo!=FIELD_NULL)
		{
			FieldInfo f = setType(otherInfo);
			Data d = other.get();
			f->setData(_dataPtr(), &d);
			return *this;
		}
		else
			free();
		return *this;
	}
	//-------------------------------------------------------------------------
	AData& operator = (int nVal)
	{
		set(nVal);
		return *this;
	}

	AData& operator = (float fVal)
	{
		set(fVal);
		return *this;
	}

	AData& operator = (const char* szVal)
	{
		set(szVal);
		return *this;
	}

	AData& operator = (UInt64 IVal)
	{
		set(IVal);
		return *this;
	}

	AData& operator = (Int64 nVal)
	{
		set((UInt64)nVal);
		return *this;
	}

	AData& operator = (bool bVal)
	{
		set(bVal);
		return *this;
	}

	AData& operator = (const Data &dVal)
	{
		set(dVal);
		return *this;
	}

	AData& operator = (const AString &dVal)
	{
		set(dVal);
		return *this;
	}

	template<typename T>
	AData& operator = (const T &niceData)
	{
		set((void*)&niceData, typeid(T));
		return *this;
	}
	//-------------------------------------------------------------------------
	operator int () const
	{
		return (int)get();
	}

	operator float () const
	{
		return (float)get();
	}

	operator Data () const
	{
		Data  sTemp;
		sTemp = get();
		return sTemp;
	}

	operator UInt64 () const
	{
		return (UInt64)get();
	}

	operator Int64 () const
	{
		return (Int64)(UInt64)get();
	}

	operator bool () const
	{
		return (bool)get();
	}

	operator AString () const
	{
		FieldInfo f = _getField();
		if (f)
		{
			AString s;
			f->get(_dataPtr(), s);
			return s;
		}
		return AString();
	}

	// NOTE: 只返回字符串类型的数据
	//const char* c_str() const
	//{
	//	const AutoFieldInfo &mTypeInfo = _getField();
	//	const char *szResult = NULL;
	//	if (mTypeInfo)
	//	{
	//		if (mTypeInfo->getType()==FIELD_STRING)
	//			szResult = mTypeInfo->getString(_dataPtr());
	//		else
	//		{
	//			static AString msTempData;
	//			msTempData = get();
	//			szResult = msTempData.c_str();
	//			LOG_YELLOW;
	//			printf("WARN: [%s:%s] Can not to save const char* of AData, because use temp static Data to get String", mTypeInfo->getTypeString(), szResult);
	//			LOG_WHITE;
	//		}
	//	}

	//	if (szResult==NULL)
	//		return "";

	//	return szResult;
	//}
	//-------------------------------------------------------------------------
public:
	bool set( int nVal )
	{
		FieldInfo mTypeInfo = _getField();
		if (!mTypeInfo || FIELD_INT!=mTypeInfo->getType())
		{
			mTypeInfo = setType(FIELD_INT);
			if (mTypeInfo==NULL)
				return false;
		}

		return mTypeInfo->set(_dataPtr(), nVal);
	}

	bool set( float fVal )
	{
		FieldInfo mTypeInfo = _getField();
		if (!mTypeInfo || FIELD_FLOAT!=mTypeInfo->getType())
		{
			mTypeInfo = setType(FIELD_FLOAT);
			if (!mTypeInfo)
				return false;
		}

		return mTypeInfo->set(_dataPtr(), fVal);
	}

	bool set( bool bVal )
	{
		FieldInfo mTypeInfo = _getField();
		if (!mTypeInfo || FIELD_BOOL!=mTypeInfo->getType())
		{
			mTypeInfo = setType(FIELD_BOOL);
			if (!mTypeInfo)
				return false;
		}

		return mTypeInfo->set(_dataPtr(), bVal);
	}
	bool set(unsigned char val)
	{
		FieldInfo mTypeInfo = _getField();
		if (!mTypeInfo || FIELD_BYTE!=mTypeInfo->getType())
		{
			mTypeInfo = setType(FIELD_BYTE);
			if (!mTypeInfo)
				return false;
		}

		return mTypeInfo->set(_dataPtr(), val);
	}

	bool set( const char* szVal )
	{
		FieldInfo mTypeInfo = _getField();
		if (!mTypeInfo || FIELD_STRING!=mTypeInfo->getType())
		{
			mTypeInfo = setType(FIELD_STRING);
			if (!mTypeInfo)
				return false;
		}

		return mTypeInfo->set(_dataPtr(), szVal);
	}

	bool set( UInt64 szVal )
	{
		FieldInfo mTypeInfo = _getField();
		if (!mTypeInfo || FIELD_UINT64!=mTypeInfo->getType())
		{
			mTypeInfo = setType(FIELD_UINT64);
			if (!mTypeInfo)
				return false;
		}

		return mTypeInfo->set(_dataPtr(), szVal);
	}

	bool set( const Data &kVal );

	bool set(const AString &str)
	{
		setType(FIELD_STRING);
		return _getField()->set(_dataPtr(), str);
	}
	//----------------------------------------------------------------

	Data get(void) const
	{
		Data temp;
		temp.mFieldCol = -2;
		temp.mData = (void*)this;
		
		return temp;
	}

	//----------------------------------------------------------------
	bool set(void *obj, const type_info &typeInfo);

	bool get(void *obj, const type_info &typeInfo) const
	{
		FieldInfo mTypeInfo = _getField();
		if (mTypeInfo)
		{			
			return mTypeInfo->get(_dataPtr(), obj, typeInfo);
		}
		return false;
	}


	//----------------------------------------------------------------
	FieldInfo setType(int type);

	//void setType(tFieldInfo *info)
	//{
	//	if (info==mTypeInfo)
	//		return;
	//	free();
	//	mTypeInfo = info;
	//	_createData();
	//}

	void init()
	{
		FieldInfo type = _getField();
		if (type)
		{
			type->free(_dataPtr());
		}
	}

	void free()
	{
		init();
		if (!_isThisSpace())
		{			
			Allot::freePtr(_dataPtr());			
		}
		memset(mData, 0, sizeof(mData));	
	}

	//const tFieldInfo* getTypeInfo() const{ return mTypeInfo; }

	bool serialize(DataStream *destData) const;
	bool restore(DataStream *scrData);

	void* _dataPtr() const
	{
		//if (mTypeInfo)
		//{
		//	if ( mTypeInfo->getLength()>sizeof(mData) )
		//		return mData;
		//}
		//return &mData;
		FieldInfo field = _getField();
		if (field)
		{
			if (field->getLength()>_getThisSpace())
				return (void*)*(size_t*)&mData[1];
			else
				return (void*)&mData[1];
		}
		return NULL;
	}


	//void* _createData()
	//{
	//	if ( mTypeInfo->getLength()>sizeof(mData) )
	//	{
	//		if (mData!=NULL)
	//			Allot::freePtr(mData, mTypeInfo->getLength());

	//		mData = ALLOCT_NEW(mTypeInfo->getLength());
	//		memset(mData, 0, mTypeInfo->getLength());
	//		return mData;
	//	}
	//	return &mData;
	//}

	//bool _resetTypeLength(size_t length);

	int _getThisSpace() const { static const int s = sizeof(AString); return s; }
	FieldInfo _getField() const
	{
		//return NiceDataField::getMe().getFeildInfo((int)(byte)mData[0]);
		return FieldInfoManager::getFieldInfo((int)(byte)mData[0]);
	}
	bool _isThisSpace() const 
	{
		FieldInfo type = _getField();
		if (type)
			return type->getLength()<=_getThisSpace();
		return true;
	}

	FIELD_TYPE getType() const
	{
		return (FIELD_TYPE)mData[0];
	}

protected:
	char					mData[sizeof(AString)+1];

};
//----------------------------------------------------------------
//----------------------------------------------------------------
class tBaseTable;
#pragma pack(push)
#pragma pack(1)
class BaseCommon_Export tNiceData : public AutoBase
{
protected:
	typedef Auto<tNiceData>		AutoNice;

public:
	mutable AData					mStatic;	// 支持多线, 不可使用静态
	AString							msKey;

public:
	tNiceData()
	{

	}

	tNiceData(const tNiceData &other)
	{
		*this = other;
	}

	virtual ~tNiceData()
	{

	}

	virtual NICEDATA_TYPE getType() const = 0;

public:
	Data operator [] (const char *szValueName)
	{		
		return getOrCreate(szValueName).get();
	}

	Data operator [] (const AString &szValueName)
	{		
		return getOrCreate(szValueName.c_str()).get();
	}

	virtual Data operator [] (const int intName)
	{		
		return getOrCreate(STRING(intName)).get();
	}

	virtual tNiceData& operator = (const tNiceData &other)
	{
		clear(false);
		append(other, true);
		return *this;
	}

public:
	bool set(const char* key, int nVal)
	{
		AData &d = getOrCreate(key);
		return d.set(nVal);
	}

	bool set(const char* key, float fVal)
	{
		AData &d = getOrCreate(key);
		return d.set(fVal);
	}

	bool set(const char* key, bool bVal)
	{
		AData &d = getOrCreate(key);
		return d.set(bVal);
	}

	bool set(const char* key, unsigned char val)
	{
		AData &d = getOrCreate(key);
		return d.set(val);
	}

	bool set(const char* key, const char* szVal)
	{
		AData &d = getOrCreate(key);
		return d.set(szVal);
	}

	bool set(const char* key, const AString &strVal)
	{
		AData &d = getOrCreate(key);
		return d.set(strVal);
	}

	void setFormat(const char* key, const char* szVal, ...);

	bool set(const char* key, const Data &kVal)
	{
		AData &d = getOrCreate(key);
		return d.set(kVal);
	}

	bool set(const char* key, Data &kVal)
	{
		AData &d = getOrCreate(key);
		return d.set(kVal);
	}

	bool set(const char* key, UInt64 nValU64)
	{
		AData &d = getOrCreate(key);
		return d.set(nValU64);
	}

	Data get(const char* key) const;
	Data get(const AString &key) const { return get(key.c_str()); }

	//NOTE: Result data only temp use, for autoptr [] operate
	virtual Data _getData(const char *szKey) const override
	{
		AData &d = ((tNiceData*)this)->getOrCreate(szKey);	
		return d.get();			
	}

	virtual Data _getData(int intName) const { return _getData(STRING(intName)); }

	tBaseTable* getTable(const char *key);

	AutoNice getNice(const char *key)
	{
		AutoNice d;
		get(key, &d, typeid(d));
		return d;
	}

	bool set(const char* key, void *obj, const type_info &typeInfo)
	{
		AData &d = getOrCreate(key);
		return d.set(obj, typeInfo);
	}

	bool get(const char* key, void *obj, const type_info &typeInfo) const;

	template<typename T>
	bool set(const char* key, T &obj)
	{
		AData &d = getOrCreate(key);
		return d.set(&obj, typeid(T));
	}

	template<typename T>
	bool get(const char* key, T &obj)
	{
		AData &d = getAData(key);		
		return d.get(&obj, typeid(T));		
	}

public:
	virtual void initData();

	AutoNice getData(const char *szIndex)
	{
		AutoNice result;
		get(szIndex, result);
		return result;
	}

	bool exist(const char *key) const { return !getAData(key).empty(); }
	bool existData(const char *keyList, AString *resultInfo = NULL);

	//NOTE: will change param string
	virtual bool FullJSON(AString scrJsonString);
	// Parse string array to tNiceData, ["KEY1", "1", "KEY2", "2"]
	virtual bool FullByStringArray(AString scrStringArray);

	virtual AString ToJSON() = 0;

	virtual AString dump(int sub = 0, int code = 0) const ;


	virtual bool SetStringArray(const char *szKey, StringArray &scrArray);
	virtual bool GetStringArray(const char *szKey, StringArray &resultArray);

	template <typename T>
	bool SetArray(const char *szKey, const Array<T> &scrArray)
	{
		AutoData data = MEM_NEW DataBuffer(64);
		data->write((DSIZE)scrArray.size());
		for (int i=0; i<scrArray.size(); ++i)
		{
			data->write(scrArray[i]);
		}
		return set(szKey, data);
	}
	template <typename T>
	bool GetArray(const char *szKey, Array<T> &resultArray)
	{
		AutoData data;
		get(szKey, data);
		if (data)
		{
			DSIZE count = 0;
			data->seek(0);
			if (!data->read(count))
				return false;
			resultArray.resize(count);
			for (int i=0; i<count; ++i)
			{
				if (!data->read(resultArray[i]))
					return false;
			}

			return true;
		}
		return false;
	}

	virtual bool empty() const { return count()<=0; }

public:
	virtual AData& getOrCreate(const char* key) = 0;
	//virtual const AData& get(size_t pos)const = 0;
	//virtual AData& get(size_t pos) = 0;
	virtual AData& getAData(const char *szKey) const = 0;

	//virtual const DataKeyType& getKey(size_t pos) const = 0;
	//virtual int getKeyIndex(size_t pos) const = 0;
	//virtual const AString getKeyString(size_t pos) const { return getKey(pos); }

	virtual bool append(int index, const AData &data, bool bReplace = false) = 0;
	virtual bool append( const EasyString &index, const AData &data, bool bReplace = false ) = 0;

	virtual bool serialize(DataStream *destData, bool bKeyIndex) const = 0;
	virtual bool restore(DataStream *scrData) = 0;

	bool remove(const AString &key){ return remove(key.c_str()); }
	virtual bool remove(const char* key) = 0;
	virtual void clear(bool bFreeBuffer = true) { AssertEx(0, "Can not run this"); }
	virtual void swap(tNiceData &other) = 0;

	virtual size_t append(const tNiceData &scrData, bool bReplace) = 0;
	virtual size_t count() const = 0;

public:
	class iterator : public AutoBase
	{
	public:
		iterator(){}
		virtual ~iterator(){}

		virtual AData& operator * () = 0;
		virtual const AString& key() = 0;
		virtual AData& get() = 0;

		virtual bool next() = 0;
		virtual bool have() = 0;

		virtual void reset() = 0;
	};

	virtual Auto<iterator> begin() const = 0;
};


typedef Auto<tNiceData>				AutoNice;
typedef Auto<tNiceData::iterator>	NiceIt;

//-------------------------------------------------------------------------*/
#if !USE_IDNICEDATA
class BaseCommon_Export NiceData : public tNiceData
{
	typedef tNiceData base;

public:
	NiceData(const tNiceData &other)
	{
		*(tNiceData*)this = other;
	}
	NiceData(){}

	virtual NICEDATA_TYPE getType() const { return NICEDATA; }

	virtual void initData() override;

public:	
	Data get(const char* key) const{ return base::get(key); }

	bool set(const char* key, int nVal)
	{
		return base::set(key, nVal);
	}

	bool set(const char* key, float fVal)
	{
		return base::set(key, fVal);
	}

	bool set(const char* key, bool bVal)
	{
		return base::set(key, bVal);
	}

	bool set(const char* key, unsigned char val)
	{
		return base::set(key, val);
	}

	bool set(const char* key, const char* szVal)
	{
		return base::set(key, szVal);
	}

	bool set(const char* key, const AString &strVal)
	{
		return base::set(key, strVal);
	}

	bool set(const char* key, const Data &kVal)
	{
		return base::set(key, kVal);
	}

	bool set(const char* key, Data &kVal)
	{
		return base::set(key, kVal);
	}

	bool set(const char* key, UInt64 nValU64)
	{
		return base::set(key, nValU64);
	}

	bool set(const char* key, void *obj, const type_info &typeInfo)
	{
		return base::set(key, obj, typeInfo);
	}

	bool get(const char* key, void *obj, const type_info &typeInfo) const
	{
		return base::get(key, obj, typeInfo);
	}

	template<typename T>
	bool set(const char* key, T &obj)
	{
		return base::set(key, obj);
	}

	template<typename T>
	bool get(const char* key, T &obj)
	{
		return base::get(key, obj);
	}

public:
	virtual AString ToJSON() override;

public:
	virtual AData& getOrCreate(const char* key);
	virtual AData& getOrCreate(const AString &key){ return getOrCreate(key.c_str()); }

	virtual AData& getAData(const char *szKey) const
	{
		AData *p = (AData*)mDataMap.findPtr(szKey);

		if (p!=NULL)
			return *p;

		//static AData d;
		return ((tNiceData*)(this))->mStatic;
	}

	virtual bool append(int index, const AData &data, bool bReplace = false) override;
	virtual bool append( const EasyString &index, const AData &data, bool bReplace = false ) override;

	virtual bool serialize(DataStream *destData, bool bKeyIndex) const override;
	virtual bool restore(DataStream *scrData) override;

	virtual bool remove(const char* key) override { return mDataMap.erase(key); }	

	size_t append(const tNiceData &scrData, bool bReplace);

	virtual void clear(bool bFreeBuffer = true) override
	{
		mDataMap.clear(bFreeBuffer);
	}

	virtual void swap(tNiceData &other) override
	{
		NiceData *p = dynamic_cast<NiceData*>(&other);
		AssertEx (p!=NULL, "必须相同类型交换");
	
		mDataMap.swap(p->mDataMap);		
	}

	virtual size_t count() const
	{
		return mDataMap.size();
	}

protected:
	mutable EasyHash<DataKeyType, AData>	mDataMap;	

public:
	class Iterator : public iterator
	{
	public:
		Iterator(const NiceData *pNice)
		{
			mHashIt = pNice->mDataMap.begin();
			//mpHash = &pNice->mDataMap;
		}
		virtual AData& operator * () override
		{
			return get();
		}
		virtual const AString& key() override
		{
			return mHashIt.key();
		}
		virtual AData& get()
		{
			return mHashIt.get();
		}

		virtual bool next() 
		{
			return mHashIt.next();
		}
		virtual bool have() 
		{
			return mHashIt.have();
		}

		virtual void reset() 
		{
			mHashIt.reset();
		}

	protected:
		EasyHash<DataKeyType, AData>::iterator mHashIt;
		//EasyHash<DataKeyType, AData> *mpHash;
	};

	virtual Auto<iterator> begin() const override { return MEM_NEW Iterator(this); }
};
typedef Auto<NiceData>	ANice;

#else
#	pragma message("目录还未处理好使用IDNiceData, MemoryDB 中需要名称保存表格信息")
#endif // !USE_IDNICEDATA

#pragma pack(pop)		// NiceData size = 145
//-------------------------------------------------------------------------*
// 数组方式保存数据，使用枚举下标方式取值, 应用于一些常用的优化消息中
// 序列数据小，使用取值快速, NOTE: 数据最多不能超过 0xffff
//-------------------------------------------------------------------------*
class BaseCommon_Export ArrayNiceData	: public tNiceData
{
	static int toIndex(const char *szIndex)
	{
		int x = TOINT(szIndex);
		if (x==0)
		{
			// 如果为零，则必须第1个字符为零
			if (strlen(szIndex)>0 && szIndex[0]=='0')
				return x;
			return -1;
		}
		return x;
	}

public:
	virtual NICEDATA_TYPE getType() const { return ARRAY_NICEDATA; }

	// 如果不存在增长创建
	virtual Data operator [] (const int intName) override
	{				
		AssertEx(intName>=0, "数组下标不可为负数");

		if (intName>0xFFFF)
		{
			ERROR_LOG("数组下标超过 65535");
			return Data();
		}

		if (intName>=(int)mDataList.size())
			mDataList.resize(intName+1);
		return mDataList[intName].get();
	}

	virtual Data _getData(const char *szKey) const 
	{
		int x = toIndex(szKey);
		if (x>=0)
			return _getData(x); 
		return Data();
	}
	virtual Data _getData(int intName) const 
	{
		return (*(ArrayNiceData*)this)[intName];
	}

	virtual bool empty() const
	{
		return mDataList.empty();
	}

	virtual AString dump(int sub = 0, int code=0) const override;

public:
	virtual AData& getOrCreate(const char* key) 
	{
		int x = toIndex(key);
		if (x<0)
		{
			//static AData s;
			return mStatic;
		}
		return _get(x);
	}

	virtual AData& getAData(const char *szKey) const
	{
		int x = toIndex(szKey);
		if (x<0)
		{
			//static AData s;
			return ((tNiceData*)(this))->mStatic;
		}
		return *(AData*)&_get(x);
	}

    virtual AData& getOrCreate(int pos) 
    {
        int x = pos;
        if (x<0)
        {
            //static AData s;
            return mStatic;
        }
        return _get(x);
    }

    virtual AData& getAData(int pos) const
    {
        int x = pos;
        if (x<0)
        {
            //static AData s;
            return ((tNiceData*)(this))->mStatic;
        }
        return *(AData*)&_get(x);
    }

	virtual const AString getKeyString(size_t pos) const { return AString(STRING((int)pos)); }

	virtual bool append(int index, const AData &data, bool bReplace = false);
	virtual bool append( const EasyString &index, const AData &data, bool bReplace = false )
	{
		int x = toIndex(index.c_str());
		if (x<0)
			return false;
		return append(x, data, bReplace);
	}

	virtual bool serialize(DataStream *destData, bool bKeyIndex) const;
	virtual bool restore(DataStream *scrData);

	virtual bool remove(const char* key) 
	{
		int x = toIndex(key);
		if (x>=0 && x<mDataList.size())
		{
			mDataList[x].free();
			return true;
		}
		return false;
	}
	virtual void clear(bool bFreeBuffer = true) { mDataList.clear(bFreeBuffer); }
	virtual void swap(tNiceData &other)
	{
		ArrayNiceData *pScr = dynamic_cast<ArrayNiceData*>(&other);
		if (pScr!=NULL)
		{
			mDataList.swap(pScr->mDataList);
		}
		else
		{
			ERROR_LOG("ArrayNiceData 交换对象必须是相同类型 [%s]", typeid(other).name());
		}
	}

	virtual size_t append(const tNiceData &scrData, bool bReplace)
	{
		ArrayNiceData *p = (ArrayNiceData*)dynamic_cast<const ArrayNiceData*>(&scrData);
		if (p==NULL)
		{
			NOTE_LOG("ERROR: XXX ArrayNiceData append only ArrayNiceData param type");
			return 0;
		}
		int count = scrData.count();
		if (count>0xFFFF)
		{
			ERROR_LOG("原数据下标超过 65535");
			return 0;
		}

		for (NiceIt it=p->begin(); it->have(); it->next())
		{
			int k = TOINT(it->key().c_str());
			_get(k) = it->get();		
		}
		return count;
	}
	virtual size_t count() const { return mDataList.size(); }

    //NOTE: 不支持子NiceData 生成JSON字符串
	virtual bool FullJSON(AString scrJsonString) override;
	virtual AString ToJSON() override;

protected:
	virtual AData& _get(size_t pos)const
	{
		if (pos>0xFFFF)
		{
			ERROR_LOG("数组下标超过 65535");
			//static AData s;
			return mStatic;
		}
		if (pos>=0)
		{
			if (pos>=mDataList.size())
				mDataList.resize(pos+1);
			return mDataList[pos];
		}
		//static AData s;
		return mStatic;
	}

protected:
	mutable Array<AData>	mDataList;

public:
	class Iterator : public iterator
	{
	public:
		Iterator(const ArrayNiceData *pNice)
			: mpDataList(&pNice->mDataList)
		{
			//mHashIt = pNice->mDataMap.begin();
			mCurrentPos = 0;
		}
		virtual AData& operator * () override
		{
			return get();
		}
		virtual const AString& key() override
		{
			if (mCurrentPos<mpDataList->size())
				msKey = STRING(mCurrentPos);
			else
				msKey = STRING(-1);
			return msKey;
		}
		virtual AData& get()
		{
			if (mCurrentPos<mpDataList->size())
				return (*mpDataList)[mCurrentPos];
			return msVal;
		}

		virtual bool next() 
		{
			++mCurrentPos;
			return mCurrentPos<mpDataList->size();
		}
		virtual bool have() 
		{
			return mCurrentPos<mpDataList->size();
		}

		virtual void reset() 
		{
			mCurrentPos = 0;
		}

	protected:
		size_t			mCurrentPos;
		Array<AData>	*mpDataList;
		AString			msKey;
		AData			msVal;
	};

	virtual Auto<iterator> begin() const override { return MEM_NEW Iterator(this); }
};
typedef Auto<ArrayNiceData> AutoArrayData;
//-------------------------------------------------------------------------*/
#	include "IDNiceData.h"

//-------------------------------------------------------------------------*/

#endif