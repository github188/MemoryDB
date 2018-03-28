//
//#ifndef _INCLUDE_IDNICEDATA_H_
//#define _INCLUDE_IDNICEDATA_H_
//
//#include "NiceData.h"
//
////-------------------------------------------------------------------------*/
//// 使用整数索引
//class BaseCommon_Export IDNiceData : public tNiceData
//{
//	typedef tNiceData base;
//
//public:
//	IDNiceData(const tNiceData &other)
//	{
//		*(tNiceData*)this = other;
//	}
//	IDNiceData(){}
//
//	virtual NICEDATA_TYPE getType() const { return ID_NICEDATA; }
//
//public:	
//	Data get(const char* key) const{ return base::get(key); }
//
//	bool set(const char* key, int nVal)
//	{
//		return base::set(key, nVal);
//	}
//
//	bool set(const char* key, float fVal)
//	{
//		return base::set(key, fVal);
//	}
//
//	bool set(const char* key, bool bVal)
//	{
//		return base::set(key, bVal);
//	}
//
//	bool set(const char* key, unsigned char val)
//	{
//		return base::set(key, val);
//	}
//
//	bool set(const char* key, const char* szVal)
//	{
//		return base::set(key, szVal);
//	}
//
//	bool set(const char* key, const AString &strVal)
//	{
//		return base::set(key, strVal);
//	}
//
//	bool set(const char* key, const Data &kVal)
//	{
//		return base::set(key, kVal);
//	}
//
//	bool set(const char* key, Data &kVal)
//	{
//		return base::set(key, kVal);
//	}
//
//	bool set(const char* key, UInt64 nValU64)
//	{
//		return base::set(key, nValU64);
//	}
//
//	bool set(const char* key, void *obj, const type_info &typeInfo)
//	{
//		return base::set(key, obj, typeInfo);
//	}
//
//	bool get(const char* key, void *obj, const type_info &typeInfo) const
//	{
//		return base::get(key, obj, typeInfo);
//	}
//
//	template<typename T>
//	bool set(const char* key, T &obj)
//	{
//		return base::set(key, obj);
//	}
//
//	template<typename T>
//	bool get(const char* key, T &obj)
//	{
//		return base::get(key, obj);
//	}
//
//public:
//	//const DataKeyType& getKey(size_t pos) const
//	//{
//	//	AssertNote(0, "IDNiceData 不支持 使用 getKey");
//	//	//static AString s;
//	//	//msKey.setNull();
//	//	return msKey;
//	//}
//
//	//virtual int getKeyIndex(size_t pos) const override
//	//{
//	//	AssertNote(0, "IDNiceData 不支持 使用 getKeyIndex");
//	//	return mStatic; // mDataMap.getKey(pos);
//	//}
//
//	//virtual const AString getKeyString(size_t pos) const override
//	//{
//	//	AssertNote(0, "IDNiceData 不支持 使用 getKeyString");
//	//	return AString();
//	//	//AString id ;
//	//	//id.Format("ID<%d>", getKeyIndex(pos));
//	//	//return id;
//	//}
//
//
//	//virtual const AData& get(size_t pos)const
//	//{
//	//	AssertNote(0, "IDNiceData 不支持 使用 get pos");
//	//	
//	//	//if (pos<mDataMap.size())
//	//	//	return mDataMap.get(pos);
//	//	//static AData temp;
//	//	return ((tNiceData*)(this))->mStatic;
//	//}
//
//	//virtual AData& get(size_t pos)
//	//{
//	//	AssertNote(0, "IDNiceData 不支持 使用 get pos");
//	//	//if (pos<mDataMap.size())
//	//	//	return mDataMap.get(pos);
//	//	//static AData temp;
//	//	return mStatic;
//	//}
//
//	virtual AData& getOrCreate(const char* key);
//	virtual AData& getAData(const char *szKey) const override
//	{
//		return mDataMap.find(MAKE_INDEX_ID(szKey));		
//	}
//
//	virtual size_t count() const
//	{
//		return mDataMap.size();
//	}
//
//	virtual bool serialize(DataStream *destData, bool bKeyIndex) const override;
//	virtual bool restore(DataStream *scrData) override;
//
//	virtual bool append(int index, const AData &data, bool bReplace = false) override
//	{
//		if (bReplace)
//			mDataMap.erase(index);
//		else if (mDataMap.exist(index))
//		{
//			WARN_LOG("Already exist >[%d]", index);
//			return false;
//		}
//		mDataMap.insert(index, data);
//		return true;
//	}
//	virtual bool append( const EasyString &index, const AData &data, bool bReplace = false ) override
//	{
//		return append(MAKE_INDEX_ID(index.c_str()), data, bReplace);
//	}
//
//	virtual bool remove(const char* key) override { return mDataMap.erase(MAKE_INDEX_ID(key)); }	
//
//	size_t append(const tNiceData &scrData, bool bReplace) override
//	{
//		if (scrData.getType()!=ID_NICEDATA)
//		{
//			ERROR_LOG("IDNiceData append 追加的源数据必须为 IDNiceData, now type [%d]", scrData.getType());
//			return 0;
//		}
//		size_t resultCount = 0;
//		for (auto it=scrData.begin(); it->have(); it->next())
//		{		
//			int key = TOINT(it->key().c_str());
//
//			// 如果替换, 先直接销毁元素, 不替换但如果存在,则直接处理下一个
//			if (bReplace)
//				mDataMap.erase(key);
//			else if (!mDataMap.find(key).empty())
//				continue;
//
//			const AData &d = it->get();
//			mDataMap.insert(key, d);
//			++resultCount;
//		}
//		return resultCount;
//	}
//
//	virtual void clear(bool bFreeBuffer = true) override
//	{
//		mDataMap.clear(bFreeBuffer);
//	}
//
//	virtual void swap(tNiceData &other) override
//	{
//		IDNiceData *p = dynamic_cast<IDNiceData*>(&other);
//		AssertEx (p!=NULL, "必须相同类型交换");
//
//		mDataMap.swap(p->mDataMap);		
//	}
//
//	virtual Data _getData(int nKey) const override
//	{
//		AData *dest = mDataMap.findPtr(nKey);
//
//		if (NULL==dest)
//		{
//			mDataMap.insert(nKey, AData());
//			return mDataMap.find(nKey);
//		}
//		else
//		{
//			return *dest;
//		}
//	}
//
//	virtual void initData() override
//	{
//		for (auto it=mDataMap.begin(); it.have(); it.next())
//		{
//			auto &kV = it.Value();
//			kV.mVal.free();
//		}
//	}
//
//	//virtual void initData() override { mDataMap.clear(false); }
//
//protected:
//	mutable EasyHash<int, AData>				mDataMap;
//
//public:
//	class Iterator : public iterator
//	{
//	public:
//		Iterator(const IDNiceData *pNice)
//		{
//			mHashIt = pNice->mDataMap.begin();
//			mpHash = &pNice->mDataMap;
//		}
//
//		virtual AData& operator * () override
//		{
//			return get();
//		}
//
//		virtual const AString& key() override
//		{
//			msKey = STRING(mHashIt.key());
//			return msKey;
//		}
//		virtual AData& get()
//		{
//			return mHashIt.get();
//		}
//
//		virtual bool next() 
//		{
//			return mHashIt.next();
//		}
//		virtual bool have() 
//		{
//			return mHashIt.have();
//		}
//
//		virtual void reset() 
//		{
//			mHashIt.reset();
//		}
//
//	protected:
//		EasyHash<int, AData>::iterator mHashIt;
//		EasyHash<int, AData> *mpHash;
//		AString msKey;
//	};
//
//	virtual Auto<iterator> begin() const override { return MEM_NEW Iterator(this); }
//};
////-------------------------------------------------------------------------*/
//
//#endif //_INCLUDE_IDNICEDATA_H_