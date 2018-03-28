
#ifndef _INCLUDE_EASYSET_H_
#define _INCLUDE_EASYSET_H_

#include "Assertx.h"
#include <utility>

#include <stdlib.h>
#include "MemBase.h"

#ifndef NULL_POS
#define NULL_POS	(-1)
#endif


//用于索引映射查寻容器(如map)
template<typename K>
class EasySet  : public MemBase
{
public:
	typedef  K	Value;

protected:
	size_t	allotSize;
	size_t	count;
	Value	*ptrArray;


protected:

	size_t _esize() const { return sizeof(Value); }

	void _resize(size_t num)
	{
		ptrArray = (Value*)ALLOCT_NEW(num*_esize());
		//only init new more space
		if (num>allotSize)
			memset(ptrArray+allotSize,0,(num-allotSize)*_esize());
	}


	size_t _findInsertPos(K key) const
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
				if (ptrArray[begin] < (K)key)
					return begin + 1;
				return begin;
			}
			size_t m = (end-begin)/2;
			if (ptrArray[begin+m] > (K)key)
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
		return NULL_POS;
	}

public:
	bool _remove(size_t pos)
	{
		if (pos<count)
		{
			///order to call destructor function.
			Value temp;
			memcpy(&temp, ptrArray+pos, _esize());
			memmove(ptrArray+pos,ptrArray+(pos+1),(count-pos-1)*_esize());
			memset(ptrArray+count-1,0,_esize());
			//for (size_t i=pos+1; i<count; ++i)
			//{
			//	ptrArray[i-1]=ptrArray[i];
			//}
			--count;
			return true;
		}
		return false;
	}
	size_t _find(K key) const
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
				if (ptrArray[begin]!=(K)key)
					return NULL_POS;

				return begin;
			}
			size_t m = (end-begin)/2;
			if (ptrArray[begin+m] > (K)key)
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
	EasySet()
		: allotSize(0)
		, count(0)
		, ptrArray(NULL)
	{
	}

	EasySet(const EasySet &other)
		: allotSize(0)
		, count(0)
		, ptrArray(NULL)
	{
		*this = other;
	}

	~EasySet()
	{
		clear();
	}

	size_t size() const { return count; }

	bool empty()const{ return 0==count; }

	// 因为原来为方便下标取值, 但与stl map下标找到KEY值相混, 所以暂时不开放
	//const T& operator [] (const K key) const
	//{
	//	return find(pos);
	//}

	//T& operator [] (const K key)
	//{
	//	return find(key);
	//}



	void clear(bool bFreeBuffer = true)
	{
		if (ptrArray)
		{
			for (size_t i=0; i<count; ++i)
			{
				//since auto need free 
				Value temp;
				memcpy(&temp, ptrArray+i, _esize());
			}
			if (bFreeBuffer)
			{
				Allot::freePtr(ptrArray, allotSize*_esize());
				ptrArray = NULL;
				allotSize = 0;
			}
		}
		count = 0;
	}


	size_t insert(Value val)
	{
		size_t insertPos = _findInsertPos(val);
		if (allotSize==count)
		{
			size_t newSize = allotSize * 2;
			if (allotSize==0)
				newSize = 1;
			Value* pOld = ptrArray;
			_resize(newSize);
			if (pOld)
				memcpy(ptrArray, pOld, insertPos*_esize());
			memcpy(ptrArray+insertPos, &val, _esize());
			memset((void*)&val, 0, _esize());
			memcpy(ptrArray+insertPos+1, pOld+insertPos, (count-insertPos)*_esize());
			if (pOld)
				Allot::freePtr((void*)pOld, allotSize*_esize());
			allotSize = newSize;
		}
		else if (count<allotSize)
		{
			memmove(ptrArray+insertPos+1, ptrArray+insertPos, _esize()*(count-insertPos));
			memcpy(ptrArray+insertPos, &val,_esize());
			memset((void*)&val, 0, _esize());
		}
		else
		{
			AssertEx(0, "当前数量大于了分配数量, 逻辑错误");

		}
		++count;
		return insertPos;
	}


	size_t erase(K key)
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

	bool exist(K key)const
	{
		return _find(key)!=NULL_POS;
	}

	EasySet& operator = (const EasySet &other)
	{
		clear();
		if (!other.empty())
		{
			_resize(other.size());
			for ( size_t i=0; i<other.size(); ++i)
			{
				ptrArray[i] = other.ptrArray[i];
			}
			allotSize = other.size();
			count = allotSize;
		}
		return *this;
	}

	void swap(EasySet &other)
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


};



#endif //_INCLUDE_EASYSET_H_