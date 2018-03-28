//#ifndef _INCLUDE_EASYONEHASH_H_
//#define _INCLUDE_EASYONEHASH_H_
//
//#include "MemBase.h"
//
//#define HASH_LOG NOTE_LOG
///*/-------------------------------------------------------------------------/
//EasyOneHash 比 ArrayListHash 慢的原因是 string_id 的计算
//	(1) 使用 string_id
//	[11:11:45] -------- My hash init create use time [0], count [29887]//
//
//[11:11:45] ==========Finish ============
//	[11:11:45] Hash_map init create use time [31], count [300]//
//
//[11:11:45] My hash Find use time [78], find count [300000], get [0]//
//
//[11:11:45] Hash_map find use time [63], find [300000], get [300000]//
//(2) 使用 AString::_hash
//	[11:13:49] -------- My hash init create use time [16], count [44832]//
//
//[11:13:49] ==========Finish ============
//	[11:13:49] Hash_map init create use time [31], count [300]//
//
//[11:13:49] My hash Find use time [47], find count [300000], get [0]//
//
//[11:13:49] Hash_map find use time [78], find [300000], get [300000]
//
////-------------------------------------------------------------------------*/
//// 快速HASH
//// 直接使用一维数组无碰撞存储，使用字符串转ID作为依据
//// NOTE: 有概率重复时，将出现不确定耗时及空间, 
//// 应用于数据量不大, 不经常增加删除, 要求查寻时间短的情况
////-------------------------------------------------------------------------*/
//template<typename K, typename V, int INIT_COUNT = 8>
//class EasyOneHash : public MemBase
//{
//public:
//	typedef EasyMapValue<K, V>	ValueBucket;
//
//public:
//	void insert(const K &key, const V &val)
//	{
//		//CheckRehash();
//		size_t k = _hashKey(key) % mValueArray.size();
//		ValueBucket &now = mValueArray[k];
//
//		if (now.mKey!="" && now.mKey!=key)
//		{
//			_rehash();
//			insert(key, val);
//			return;
//		}
//		now.mKey = key;
//		now.mVal = val;
//		//++mCount;
//	}
//
//	bool erase(const K &key)
//	{
//		if (mValueArray.empty())
//			return false;
//		size_t k = _hashKey(key) % mValueArray.size();
//		ValueBucket &now = mValueArray[k];
//		if (now.mKey==key)
//		{
//			now.mKey.setNull();
//			now.mVal = V();
//			return true;
//		}
//		return false;
//	}
//
//	bool erase(const K &key, V &exist)
//	{
//		if (mValueArray.empty())
//			return false;
//		size_t k = _hashKey(key) % mValueArray.size();
//		ValueBucket &now = mValueArray[k];
//		if (now.mKey==key)
//		{
//			exist = now.mVal;
//			now.mKey.setNull();
//			now.mVal = V();
//			return true;
//		}
//		return false;
//	}
//
//	const V& find(const K &key)
//	{
//		const V* p = findPtr(key);
//		if (p!=NULL)
//			return *p;
//
//		return mValueArray._getStatic().mVal;
//	}
//
//	const V* findPtr(const char *szKey) const
//	{
//		if (mValueArray.empty())
//			return NULL;
//
//		size_t k = _hashKey(szKey) % mValueArray.size();
//		const ValueBucket &now = mValueArray[k];
//		if (now.mKey==szKey)
//			return &now.mVal;
//
//		return NULL;
//	}
//
//	const V* findPtr(const K &key) const
//	{
//		if (mValueArray.empty())
//			return NULL;
//		size_t k = _hashKey(key) % mValueArray.size();
//		const ValueBucket &now = mValueArray[k];
//		if (now.mKey==key)
//			return &now.mVal;
//
//		return NULL;
//	}
//
//	void Dump() const
//	{
//		size_t s = mValueArray.size() * sizeof(ValueBucket);
//		HASH_LOG("*** Now dump hash array size [%llu]\r\n", mValueArray.size());	
//		int x = 0;
//		for (size_t i=0; i<mValueArray.size(); ++i)
//		{
//			//s += mValueArray[i]._useSpaceSize();
//			if (mValueArray[i].mKey!="")
//			{
//				++x;
//				HASH_LOG("%llu = 1 %s\r\n", i, mValueArray[i].mKey.c_str());
//			}
//		}
//		HASH_LOG("END Total [%d] use [%llu] [%llu]----------------------------------------\r\n", x, mValueArray.size(), s);
//	}
//
//	size_t size() const { return mValueArray.size(); }
//
//	size_t _useSpaceSize() const { return mValueArray._useSpaceSize(); }
//
//	ValueBucket& operator [] (int index)
//	{
//		return mValueArray[index];
//	}
//
//	const ValueBucket& operator [] (int index) const
//	{
//		return mValueArray[index];
//	}
//
//protected:
//	size_t _hashKey(const char *szKey) const
//	{
//		//return AString::_hash(szKey);
//		return MAKE_INDEX_ID(szKey);
//	}
//
//	size_t _hashKey(const K &key) const
//	{
//		//return (size_t)key;
//		return MAKE_INDEX_ID(key.c_str());
//	}
//
//	void _rehash()
//	{
//		Array<ValueBucket> temp((int)((mValueArray.size()+1)*1.5f));		
//		mValueArray.swap(temp);
//
//		for (size_t i=0; i<temp.size(); ++i)
//		{
//			ValueBucket &vB = temp[i];
//			if (vB.mKey!="")
//			{				
//				insert(vB.mKey, vB.mVal);
//			}
//		}
//		temp.clear(true);
//		//HASH_LOG("### HASH Rehash >[%llu]\r\n", mValueArray.size());
//	}
//
//public:
//	EasyOneHash()
//	{
//		mValueArray.resize(INIT_COUNT);
//	}
//
//public:
//	V& get(size_t pos)
//	{
//		if (pos<mValueArray.size())
//			return mValueArray[pos].mVal;
//		return mValueArray._getStatic().mVal;
//	}
//
//	const K& getKey(size_t pos)
//	{
//		if (pos<mValueArray.size())
//			return mValueArray[pos].mKey;
//		return mValueArray._getStatic().mKey;
//	}
//
//	void clear(bool bClearBuffer)
//	{
//		mValueArray.clear(bClearBuffer);
//	}
//
//protected:
//	Array<ValueBucket>	mValueArray;
//};
////-------------------------------------------------------------------------*/
//// 实际测试，效率低于EasyMapHash, 停止使用
////template<typename K, typename V, int INIT_COUNT = 8>
////class FastHash : public MemBase
////{
////	typedef EasyOneHash<K, V, 0>	ValueBucket;
////public:
////	void Add(const K &key, const V &val)
////	{
////		Remove(key);
////		
////		size_t k = HashKey(key) % mValueArray.size();
////
////		ValueBucket &vB = mValueArray[k];
////		if (vB.Find(key)!=NULL)
////		{
////			// 直接覆盖
////			vB.Add(key, val);
////			return;
////		}
////		if (vB.size()+1>=mValueArray.size())
////		{
////			Rehash();
////			Add(key, val);
////			return;
////		}
////		if (vB.size()<=0)
////			vB.Rehash();
////
////		vB.Add(key, val);
////		++mCount;
////	}
////
////	bool Remove(const K &key)
////	{
////		size_t k = HashKey(key) % mValueArray.size();
////		if (mValueArray[k].Remove(key))
////		{
////			--mCount;
////			return true;
////		}
////		return false;
////	}
////
////	bool Remove(const K &key, V &exist)
////	{
////		size_t k = HashKey(key) % mValueArray.size();
////
////		ValueBucket &vB = mValueArray[k];
////
////		V *p = vB.Find(key);
////		if (p!=NULL)
////		{
////			exist = *p;
////			vB.Remove(key);
////			--mCount;
////			return true;
////		}
////
////		return false;
////	}
////
////	const V* Find(const K &key) const
////	{
////		size_t k = HashKey(key) % mValueArray.size();
////		return mValueArray[k].Find(key);
////	}
////	const V* Find(const char *key) const
////	{
////		size_t k = HashKey(key) % mValueArray.size();
////		return mValueArray[k].Find(key);
////	}
////
////	size_t HashKey(const K &key) const
////	{
////		return (size_t)key;
////	}
////
////	size_t HashKey(const char *key) const
////	{	
////		int len = strlen(key);
////		register uint nr=1, nr2=4; 
////		while (len--) 
////		{ 
////			nr^= (((nr & 63)+nr2)*((uint) (uchar) *key++))+ (nr << 8); 
////			nr2+=3; 
////		} 
////		return((uint) nr); 
////	}
////
////	void Rehash()
////	{
////		Array<ValueBucket> temp(mValueArray.size()*2);		
////		mValueArray.swap(temp);		
////		mCount = 0;
////		for (size_t i=0; i<temp.size(); ++i)
////		{
////			ValueBucket &vB = temp[i];
////
////			int count = (int)vB.size();
////
////			for (int n=0; n<count; ++n)
////			{
////				const auto &kv = vB[n];
////				if (kv.mKey!="")
////					Add(kv.mKey, kv.mVal);
////			}
////		}
////		temp.clear(true);
////		//HASH_LOG("$$$ FastHash rehash >[%llu]\r\n", mValueArray.size());
////	}
////
////	void Dump() const
////	{
////		size_t s = mValueArray._useSpaceSize();
////		HASH_LOG("@@@ Now dump hash array size [%llu]\r\n", mValueArray.size());
////		for (size_t i=0; i<mValueArray.size(); ++i)
////		{
////			s += mValueArray[i]._useSpaceSize();
////			if (mValueArray[i].size()>0)
////			{
////				//HASH_LOG("%llu = %llu [%llu]\r\n", i, mValueArray[i].size(), mValueArray[i]._useSpaceSize());
////				HASH_LOG("		****");
////				mValueArray[i].Dump();			
////			}
////		}
////		HASH_LOG("END Total [%llu] use [%llu]----------------------------------------\r\n", mValueArray.size(), s);
////	}
////
////public:
////	FastHash()
////		: mCount(0)
////	{
////		mValueArray.resize(INIT_COUNT);
////	}
////
////public:
////	class iterator
////	{
////		friend class FastHash;
////	public:
////		iterator(FastHash &h)
////			: mFastIndex(0)
////			, mOnePos(UINT_MAX)
////		{
////			Next(h);
////		}
////
////		bool Next(FastHash &h)
////		{
////			++mOnePos;
////
////			for (size_t i=mFastIndex; i<h.mValueArray.size(); ++i)
////			{
////				ValueBucket &vB = h.mValueArray[i];
////				if (vB.size()>0)
////				{
////					int count = vB.size();
////
////					int begin = 0;
////					if (i==mFastIndex)
////						begin = mOnePos;
////
////					for (int n=begin; n<count; ++n)
////					{
////						if (vB[n].mKey!="")
////						{
////							mFastIndex = i;
////							mOnePos = n;
////							return true;
////						}
////					}
////				}
////			}
////			return false;
////		}
////
////		bool Have(FastHash &h)
////		{
////			if (mFastIndex<h.mValueArray.size())
////			{
////				ValueBucket &vB = h.mValueArray[mFastIndex];
////				if (mOnePos<vB.size())
////					return vB[mOnePos].mKey!="";
////			}
////			return false;
////		}
////
////		EasyMapValue<K, V>& Value(FastHash &h)
////		{
////			if (mFastIndex<h.mValueArray.size())
////			{
////				ValueBucket &vB = h.mValueArray[mFastIndex];
////				if (mOnePos<vB.size())
////					return vB[mOnePos];
////			}
////			return h.msVal;
////		}
////
////	protected:
////		uint	mFastIndex;
////		uint	mOnePos;
////	};
////
////public:
////	iterator Begin()
////	{
////		return iterator(*this);
////	}
////
////	void insert(const K &key, const V &val)
////	{
////		Add(key, val);
////	}
////
////	int erase(const K &key)
////	{
////		if (Remove(key))
////			return 1;
////		return 0;
////	}
////
////	void clear(bool bClearBuffer = false)
////	{
////		mValueArray.clear(bClearBuffer);
////		mValueArray.resize(INIT_COUNT);
////		mCount = 0;
////	}
////
////	V& find(const K &key)
////	{
////		V* p = Find(key);
////		if (p!=NULL)
////			return *p;
////
////		return msVal.mVal;
////	}
////
////	bool findExBy(const char *szKey, V &val) const
////	{
////		const V* p = Find(szKey);
////		if (p!=NULL)
////		{
////			val = *p;
////			return true;
////		}
////		return false;
////	}
////
////	bool exist(const K &key) const
////	{
////		return Find(key)!=NULL;
////	}
////
////	bool exist(const char *szKey) const
////	{
////		return Find(szKey)!=NULL;
////	}
////
////	bool empty() const
////	{
////		return mCount<=0;
////	}
////
////	int size() const { return mCount; }
////
////	void swap(FastHash &other)
////	{
////		mValueArray.swap(other.mValueArray);
////		size_t temp = mCount;
////		mCount = other.mCount;
////		other.mCount = temp;
////	}
////
////protected:
////	Array<ValueBucket>	mValueArray;
////	size_t				mCount;
////	EasyMapValue<K, V>	msVal;
////};

//-------------------------------------------------------------------------
// 简易的Hash容器，桶使用EasyMap, 适合保存不经常变化的情况
// 重分配桶数量条件： 当数量超过桶数量的2倍，或者 某一桶的数量超过 桶数量的1/3
// 性能：15000数量下，查寻150000次，创建31 查寻用时 516， stdext::hash_map 创建31，查寻用时564
//-------------------------------------------------------------------------

//template<typename K, typename V, int INIT_COUNT = HASH_INIT_BUCKET_COUNT>
//class EasyMapHash : public MemBase
//{
//	typedef EasyMap<K, V, 1>	ValueBucket;
//public:
//	void insert(const K &key, const V &val)
//	{
//		_checkRehash();
//		size_t k = _hashKey(key) % mValueArray.size();
//		ValueBucket &vB = mValueArray[k];
//		mCount -= vB.erase(key);
//		if (vB.size()>mValueArray.size()/3)
//		{
//			_rehash();
//			insert(key, val);
//			return;
//		}
//		vB.insert(key, val);
//		++mCount;
//	}
//
//	bool erase(const K &key)
//	{
//		size_t k = _hashKey(key) % mValueArray.size();
//		size_t re = mValueArray[k].erase(key);
//		if (re>0)
//		{
//			mCount -= re;
//			return true;
//		}
//		return false;
//	}
//
//	bool erase(const K &key, V &exist)
//	{
//		size_t k = _hashKey(key) % mValueArray.size();
//		ValueBucket &vB = mValueArray[k];
//		size_t p = vB._find(key);
//		if (p!=NULL_POS)
//		{
//			exist = vB.get(p);
//			if (mValueArray[k]._remove(p))		
//			{
//				--mCount;
//				return true;
//			}
//		}
//		return false;
//	}
//
//	const V& find(const K &key)
//	{
//		const V* p = findPtr(key);
//		if (p!=NULL)
//			return *p;
//
//		return mValueArray[0]._getStaticValue().mVal;
//	}
//
//	V* findPtr(const K &key) const
//	{
//		size_t k = _hashKey(key) % mValueArray.size();
//		return mValueArray[k].findPtr(key);
//	}
//
//	const V* findPtr(const char *key) const
//	{
//		size_t k = _hashKey(key) % mValueArray.size();
//		return mValueArray[k].findPtr(key);
//	}
//
//	void Dump()
//	{
//		size_t s = mValueArray._useSpaceSize();
//		printf("Now dump hash array size [%llu]\r\n", mValueArray.size());
//		for (size_t i=0; i<mValueArray.size(); ++i)
//		{
//			s += mValueArray[i]._useSpaceSize();
//			if (mValueArray[i].size()>0)
//			{
//				printf("%llu = %llu [%llu]\r\n", i, mValueArray[i].size(), mValueArray[i]._useSpaceSize());
//			}
//		}
//		printf("END Total use [%llu]----------------------------------------\r\n", s);
//	}
//
//protected:
//	size_t _hashKey(const K &key) const
//	{
//		return (size_t)key;
//	}
//
//	size_t _hashKey(const char *key) const
//	{	
//		return AString::_hash(key);
//	}
//
//	void _checkRehash()
//	{
//		if ((double)(mCount+1)/mValueArray.size()>2)
//		{
//			_rehash();
//		}
//	}
//
//	void _rehash()
//	{
//		Array<ValueBucket> temp(mValueArray.size()*2);		
//		mValueArray.swap(temp);
//		mCount = 0;
//		for (size_t i=0; i<temp.size(); ++i)
//		{
//			ValueBucket &vB = temp[i];
//			for (size_t n=0; n<vB.size(); ++n)
//			{
//				const auto &kv = vB.getValue(n);
//				insert(kv.mKey, kv.mVal);
//			}
//		}
//		temp.clear(true);
//	}
//
//public:
//	EasyMapHash()
//		: mCount(0)
//	{
//		mValueArray.resize(INIT_COUNT);
//	}
//
//	class iterator
//	{
//		friend class EasyMapHash;
//	public:
//		iterator(EasyMapHash &h)
//			: mFastIndex(0)
//			, mMapPos(UINT_MAX)
//		{
//			Next(h);
//		}
//
//		bool Next(EasyMapHash &h)
//		{
//			++mMapPos;
//
//			if (Have(h))
//				return true;
//
//			++mFastIndex;
//			mMapPos = 0;
//
//			for (size_t i=mFastIndex; i<h.mValueArray.size(); ++i)
//			{
//				ValueBucket &vB = h.mValueArray[i];
//				int begin = 0;
//				if (i==mFastIndex)
//					begin = mMapPos;
//				int count = vB.size();
//				for (int n=begin; n<count; ++n)
//				{
//					if (vB.getKey(n)!="")
//					{
//						mFastIndex = i;
//						mMapPos = n;
//						return true;
//					}
//				}
//			}
//			return false;
//		}
//
//		bool Have(const EasyMapHash &h)const
//		{			
//			return (mFastIndex<h.mValueArray.size() && mMapPos<h.mValueArray[mFastIndex].size());
//		}
//
//		const EasyMapValue<K, V>& Value(EasyMapHash &h)
//		{		
//			if (Have(h))
//				return h.mValueArray[mFastIndex].getValue(mMapPos);
//
//			return h.mValueArray[0]._getStaticValue();
//		}
//
//	protected:
//		uint	mFastIndex;
//		uint	mMapPos;
//	};
//
//	iterator begin()
//	{
//		return iterator(*this);
//	}
//
//	void clear(bool bClearBuffer = false)
//	{
//		mValueArray.clear(bClearBuffer);
//		mValueArray.resize(INIT_COUNT);
//		mCount = 0;
//	}
//
//	bool findExBy(const char *szKey, V &val) const
//	{
//		const V* p = findPtr(szKey);
//		if (p!=NULL)
//		{
//			val = *p;
//			return true;
//		}
//		return false;
//	}
//
//	bool exist(const K &key) const
//	{
//		return findPtr(key)!=NULL;
//	}
//
//	bool exist(const char *szKey) const
//	{
//		return findPtr(szKey)!=NULL;
//	}
//
//	bool empty() const
//	{
//		return mCount<=0;
//	}
//
//	int size() const { return mCount; }
//
//	void swap(EasyMapHash &other)
//	{
//		mValueArray.swap(other.mValueArray);
//		size_t temp = mCount;
//		mCount = other.mCount;
//		other.mCount = temp;
//	}
//
//protected:
//	Array<ValueBucket>	mValueArray;
//	size_t				mCount;
//};

//////-------------------------------------------------------------------------*/
//
//#endif //_INCLUDE_EASYONEHASH_H_