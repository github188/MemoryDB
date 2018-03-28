/********************************************************************
	created:	2012/04/16
	created:	16:4:2012   21:32
	filename: 	d:\New\Common\DataBase\EasyMap.h
	file path:	d:\New\Common\DataBase
	file base:	EasyMap
	file ext:	h
	author:		杨
	
	purpose:	映射索引容器
				精巧，快速
				二分法检索算法
*********************************************************************/
#ifndef _INCLUDE_EASYMAP_H_
#define _INCLUDE_EASYMAP_H_
#include "Assertx.h"
#include <utility>

#include <stdlib.h>
#include "MemBase.h"
#include <new>
 
#define NULL_POS	(-1)
//-------------------------------------------------------------------------
// 用在Map内部，没必要对齐,且可以节省空间
#pragma pack(push)
#pragma pack(1)

template<typename K, typename T>
struct EasyMapValue
{
	K			mKey;
	mutable T	mVal;

public:
	EasyMapValue()
		: mKey()
		, mVal()
	{

	}
};

#pragma pack(pop)
//-------------------------------------------------------------------------
//用于索引映射查寻容器(如map)
template<typename K, typename T, int INIT_COUNT = 8>
class EasyMap  : public MemBase
{
public:
	typedef  EasyMapValue<K, T>	Value;

protected:
	//-------------------------------------------------------------------------
	// 内存中的对象处理, 填充, 移除
	// 填充对象初始值, 保证对象初始内存空间不为零时的正确性
	// 移除时, 可以保证正确调用对象的析构, 保证对象正确释放
	//-------------------------------------------------------------------------
	void __Set(Value &o) { new (&(o)) Value; }
	void __Free(Value &o) { (&o)->~Value();  }
	//-------------------------------------------------------------------------

protected:
	size_t	allotSize;
	size_t	count;
	Value	*ptrArray;

public:
	class Iterator
	{
		friend class EasyMap;

	public:
		T& begin()
		{
			mCurrent = 0;
			if (mMap!=NULL)
				return mMap->get(mCurrent);

			return mMap->msValue.mVal;
		}
		bool have()
		{
			if (mMap!=NULL)
				return mCurrent<mMap->size();

			return false;
		}

		bool end()
		{
			if (mMap!=NULL)
				return mCurrent >= mMap->size();

			return true;
		}

		const K& key()
		{
			if (mMap!=NULL)
				return mMap->getKey(mCurrent);

			return mMap->msValue.mKey;
		}

		T& operator * ()
		{
			if (mMap!=NULL)
				return mMap->get(mCurrent);

			return mMap->msValue.mVal;
		}

		T& get() 
		{
			if (mMap!=NULL)
				return mMap->get(mCurrent);
			AssertNote(0, "逻辑上不会出现这种情况");
			return mMap->msValue.mVal;
		}

		const Value& getValue() const
		{
			if (mMap!=NULL)
				return mMap->getValue(mCurrent);
			AssertNote(0, "逻辑上不会出现这种情况");
			return mMap->msValue;
		}

		bool next()
		{
			++mCurrent;
			return have();
		}
		T& nextValue()
		{
			++mCurrent;
			return get();
		}

		bool previou()
		{
			if (mCurrent>0 && have())
			{
				--mCurrent;
				return true;
			}

			mCurrent = NULL_POS;
			return false;
		}

		bool erase()
		{
			if (mMap!=NULL)
				return mMap->_remove(mCurrent);

			return false;
		}

		operator bool (){ return have(); }
        bool operator ++(){ return next(); }
        bool operator ++(int){ return next(); }

	public:
		Iterator()
		{
			mMap = NULL;
			mCurrent = NULL_POS;
		}
		Iterator(EasyMap *pMap)
		{
			mMap = pMap;			
			mCurrent = 0;
		}

		size_t getCurrentPos() const { return mCurrent; }

	protected:
		EasyMap		*mMap;
		size_t		mCurrent;
	};

public:
	Iterator begin()
	{
		return Iterator(this);
	}

	Iterator begin( const K &key )
	{
		Iterator it;

		size_t pos = NULL_POS;
		findFirst(key, &pos);
		if (pos!=NULL_POS)
		{
			it.mMap = this;
			it.mCurrent = pos;
		}
		return it;
	}

	Iterator GetLastIterator()
	{
		Iterator it;

		it.mMap = this;
		if (empty())
			it.mCurrent = NULL_POS;
		else
		{
			it.mCurrent = size()-1;
		}
		return it;
	}

protected:
	
	void _allot(size_t num)
	{
		clear(true);
		if (num==0)
		{
			return;
		}		
		
		ptrArray = (Value*)ALLOCT_NEW(num*_esize());		
		memset(ptrArray, 0, num*_esize());

		allotSize = num;
	}

public:
	// Warn : Use For fast serialization
	void _resize(size_t num)
	{
		if (num==allotSize)
			return;

		if (num==0)
		{
			clear(false);
			return;
		}		

		Value* pOld = ptrArray;	

		ptrArray = (Value*)ALLOCT_NEW(num*_esize());

		//only init new more space
		if (num>count)
		{
			if (pOld)
				memcpy(ptrArray, pOld, count*_esize());

			memset(ptrArray+count, 0, (num-count)*_esize());
		}
		else 
		{
			if (pOld != NULL)
			{
				memcpy(ptrArray, pOld, num*_esize());

				for (size_t i = count; i < num; ++i)
				{
					__Free(pOld[i]);
				}
			}
		}

		if (pOld)
			Allot::freePtr((void*)pOld, allotSize*_esize());

		allotSize = num;
	}

public:
	void* _data(){ return ptrArray; }
	void _setCount(size_t uCount, size_t uAllotSize){ count = uCount; allotSize = uAllotSize; }

protected:

	size_t _esize() const { return sizeof(Value); }

	size_t _findInsertPos(const K &key) const
	{
		if (count==0)
		{
			return 0;
		}
		size_t end = count;
		size_t begin = 0;
		while (true)
		{
			if (end-begin==0)
				return begin;
			else if (end-begin==1)
			{
				if (ptrArray[begin].mKey < (K)key)
					return begin + 1;
				return begin;
			}
			size_t m = (end-begin)/2;
			if (ptrArray[begin+m].mKey > (K)key)
			{
				//left
				end = begin+m;				
			}
			else
			{
				//right
				begin += m; 				
			}
		}
	}

public:
	bool _remove(size_t pos)
	{
		if (pos<count)
		{
			///order to call destructor function.
			__Free(ptrArray[pos]);

			memmove(ptrArray+pos, ptrArray+(pos+1), (count-pos-1)*_esize());
			memset(&(ptrArray[count-1]), 0, _esize());

			--count;
			return true;
		}
		return false;
	}
	size_t _find(const K &key) const
	{
		if (count==0)
		{
			return NULL_POS;
		}
		size_t end = count;
		size_t begin = 0;
		while (true)
		{
			if (end-begin==0)
				return NULL_POS;
			else if (end-begin==1)
			{
				if (ptrArray[begin].mKey!=(K)key)
					return NULL_POS;

				return begin;
			}
			size_t m = (end-begin)/2;
			if (ptrArray[begin+m].mKey > (K)key)
			{
				//left
				end = begin+m;				
			}
			else
			{
				//right
				begin += m; 				
			}
		}
	}

	template<typename KT>
	size_t _findBy(const KT &key) const
	{
		if (count==0)
		{
			return NULL_POS;
		}
		size_t end = count;
		size_t begin = 0;
		while (true)
		{
			if (end-begin==0)
				return NULL_POS;
			else if (end-begin==1)
			{
				if (ptrArray[begin].mKey!=key)
					return NULL_POS;

				return begin;
			}
			size_t m = (end-begin)/2;
			if (ptrArray[begin+m].mKey > key)
			{
				//left
				end = begin+m;				
			}
			else
			{
				//right
				begin += m; 				
			}
		}
	}

public:
	EasyMap()
		: allotSize(0)
		, count(0)
		, ptrArray(NULL)
	{
	}

	EasyMap(const EasyMap &other)
		: allotSize(0)
		, count(0)
		, ptrArray(NULL)
	{
		*this = other;
	}

	~EasyMap()
	{
		clear(true);
	}

public:
	size_t size() const { return count; }

	bool empty()const{ return 0==count; }

	// 因为原来为方便下标取值, 但与stl map下标找到KEY值相混, 所以暂时不开放
	const T& operator [] (const K &key) const
	{
		return find(key);
	}

	T& operator [] (const K &key)
	{
		return find(key);
	}

	const T& get(size_t pos) const
	{
		if (pos<count)
			return ptrArray[pos].mVal;
		//AssertEx(0, "错误: 给定位置下标越界");
		return msValue.mVal;
	}

	T& get(size_t pos)
	{
		if (pos<count)
			return ptrArray[pos].mVal;
		//AssertEx(0, "错误: 给定位置下标越界");
		return msValue.mVal;
	}

	const K& getKey(size_t pos) const
	{
		if (pos<count)
			return ptrArray[pos].mKey;
		//AssertEx(0, "错误: 给定位置下标越界");
		return msValue.mKey;
	}

	const Value& getValue(size_t pos) const
	{
		if (pos<count)
			return ptrArray[pos];
		//AssertEx(0, "错误: 给定位置下标越界");
		return msValue;
	}

	T* findPtr(const K &key) const
	{
		size_t pos = _find(key);
		if (pos<count)
			return &(ptrArray[pos].mVal);
		return NULL;
	}

	bool findEx(const K &key, T &result) const
	{
		T *p = findPtr(key);
		if (p)
		{
			result = *p;
			return true;
		}
		return false;
	}

	template<typename KT>
	bool findExBy(const KT &key, T &result) const
	{
		size_t pos = NULL_POS;
		result = findBy(key, &pos);
		return (pos!=NULL_POS);
	}
	template<typename KT>
	T* findPtrBy(const KT &key) const
	{
		size_t pos = NULL_POS;
		T& result = findBy(key, &pos);
		if (pos!=NULL_POS)
			return &result;
		return NULL;
	}

	void clear(bool bFreeBuffer = true)
	{
		if (ptrArray)
		{		
			for (size_t i=0; i<count; ++i)
			{
				__Free(ptrArray[i]);
			}

			if (bFreeBuffer)
			{
				Allot::freePtr(ptrArray, allotSize*_esize());
				ptrArray = NULL;
				allotSize = 0;
			}
			else
				memset(ptrArray, 0, _esize()*allotSize);
		}
		count = 0;
	}

	size_t insert(const K &key, const T &val )
	{
		Value v;
		v.mKey = key;
		v.mVal = val;
		return insert(v);
	}

	size_t insert(const Value &val)
	{
#pragma warning(push)
#pragma warning(disable:6387)
		size_t insertPos = _findInsertPos(val.mKey);

		if (allotSize<count+1)
		{			
			size_t newSize = INIT_COUNT;
			if (allotSize>0)
				newSize = allotSize * 2;			
			_resize(newSize);		
		}
		
		memmove(ptrArray+insertPos+1, ptrArray+insertPos, _esize()*(count-insertPos));
		__Set(ptrArray[insertPos]);
		ptrArray[insertPos] = val;

		++count;
		return insertPos;
#pragma warning(pop)
	}

	T& find(const K &key, size_t *resultPos = NULL) const
	{
		size_t pos = _find(key);
		if (pos==NULL_POS)
		{
			if (resultPos)
				*resultPos = NULL_POS;
			return ((EasyMap*)(this))->msValue.mVal;
		}
		
		if (resultPos!=NULL)
			*resultPos = pos;

		return ptrArray[pos].mVal;
	}

	template<typename KT>
	T& findBy(const KT &key, size_t *resultPos = NULL) const
	{
		size_t pos = _findBy(key);
		if (pos==NULL_POS)
		{
			if (resultPos)
				*resultPos = NULL_POS;
			return ((EasyMap*)(this))->msValue.mVal;
		}

		if (resultPos!=NULL)
			*resultPos = pos;

		return ptrArray[pos].mVal;
	}

	const T& findFirst(const K &key, size_t *resultPos = NULL) const
	{
		size_t pos;
		T &result = find(key, &pos);

		if (pos==NULL_POS)
			return msValue.mVal;

		while (pos>0)
		{
			if (getKey(pos-1)!=key)
				break;
			--pos;
		}
		
		if (resultPos!=NULL)
			*resultPos = pos;

		return get(pos);
	}

	size_t erase(const K &key)
	{
		size_t num = 0;
		while ( true )
		{
			size_t pos = _find(key);
			if (pos==NULL_POS)
				break;
			_remove(pos);
			++num;
		}
		return num;
	}

	// delete value from multiple key map
	size_t erase(const K &key, const T &delValue)
	{
		size_t pos = _find(key);

		if (pos!=NULL_POS)
		{
			if (get(pos)==delValue)
			{
				_remove(pos);
				return 1;
			}
			size_t right = pos;
			while (true)
			{				
				if (++right>=size())
					break;

				if (getKey(right)!=key)
					break;

				if (get(right)==delValue)
				{
					_remove(right);
					return 1;
				}
			}
			//left
			while (pos>0)
			{				
				--pos;
				if (getKey(pos)!=key)
					return false;

				if (get(pos)==delValue)
				{
					_remove(pos);
					return 1;
				}
			}
		}

		return 0;
	}

	bool exist(const K &key)const
	{
		return _find(key)!=NULL_POS;
	}

	EasyMap& operator = (const EasyMap &other)
	{
		clear();
		if (!other.empty())
		{
			_allot(other.size());
			for ( size_t i=0; i<other.size(); ++i)
			{
				__Set(ptrArray[i]);
				ptrArray[i] = other.ptrArray[i];
			}
			count = allotSize;
		}
		return *this;
	}

	void swap(EasyMap &other)
	{
		size_t s = other.allotSize;
		other.allotSize = allotSize;
		allotSize = s;
		s = other.count;
		other.count = count;
		count = s;
		Value *p = other.ptrArray;
		other.ptrArray = ptrArray;
		ptrArray = p;
	}

	const Value* first()
	{
		if (!empty())
			return &(getValue(0));

		return NULL;
	}

	const Value* last()
	{
		if (!empty())
			return &(getValue(size()-1));

		return NULL;
	}

	size_t _useSpaceSize()
	{
		return sizeof(EasyMap) + sizeof(Value) * allotSize;
	}

	const Value& _getStaticValue() const {return msValue; }

protected:
	Value	msValue;				// 为了支持多线，每个实例必须保存一个临时的实例
};
//-------------------------------------------------------------------------
//template<typename K, typename T>
//EasyMapValue<K, T>  EasyMap<K, T>::msValue = { K(), T() };


#endif