#ifndef _INCLUDE_EASYSTACK_H_
#define _INCLUDE_EASYSTACK_H_

#include "Array.h"

#define  STACK_INIT_COUNT	16

template<typename T>
class EasyStack : MemBase
{
public:
	EasyStack()
		: mCount(0)
	{
		msStatic = T();
		mData.resize(STACK_INIT_COUNT);
	}
	~EasyStack()
	{
		clear();
	}

public:
	void push(const T& val)
	{
		if (mCount>=mData.size())
			mData.resize(mData.size()*2);
		mData[mCount++] = val;
	}

	T pop(void)
	{
		if (mCount>0)
		{
			T temp = mData[--mCount];
			mData[mCount] = T();
			return temp;
		}
		return T();
	}
	void clear()
	{
		for (size_t i=0; i<mCount; ++i)
		{
			mData[i] = T();
		}
		mData.resize(STACK_INIT_COUNT);
		mCount = 0;
	}

	bool empty(){ return mCount==0; }
	size_t size(){ return mCount; }

	T& operator [] (size_t pos)
	{
		if (pos<mCount)
			return mData[pos];

		//static T s;
		return msStatic;
	}

	void swap(EasyStack &other)
	{
		mData.swap(other.mData);
		size_t t = mCount;
		mCount = other.mCount;
		other.mCount = t;
	}

	Array<T>& _getList(){ return mData; }

protected:
	Array<T>		mData;
	size_t		mCount;

	T			msStatic;
};


#endif //_INCLUDE_EASYSTACK_H_