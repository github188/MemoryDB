#ifndef _INCLUDE_ARRAYINDEX_H_
#define _INCLUDE_ARRAYINDEX_H_

#include "Array.h"
#include "TableTool.h"

#define ARRAY_INDEX_MAX_ID		0XFFFF
//-------------------------------------------------------------------------*/
// 数组快速索引
// NOTE: 必须覆盖Array::remove，因为Array::remove会移动下标对应的值
//-------------------------------------------------------------------------*/
template<typename T>
class ArrayIndex : public Array<T*>
{
public:
	// NOTE: Alway delete already val
	void Append(int id, T *pVal)
	{
		if (id>ARRAY_INDEX_MAX_ID)
		{
			ERROR_LOG("数组索引超过最大允许ID [%d]", ARRAY_INDEX_MAX_ID);
			return;
		}

		if (id>=size())		
			resize(id+8);				
		else if (mpArrayPtr[id]!=NULL && mpArrayPtr[id]!=pVal)
		{
			delete mpArrayPtr[id];
		}
		mpArrayPtr[id] = pVal;
	}

	T* Find(int id)
	{
		if (id>=0 && id<size())
			return mpArrayPtr[id];

		return NULL;
	}

	// NOTE: 必须覆盖此功能，Array::remove 会移动下标对应的值
	bool remove(size_t pos)
	{
		if (pos>=0 && pos<size())
		{
			bool b = mpArrayPtr[pos]!=NULL;
			if (b)
				delete mpArrayPtr[pos];
			mpArrayPtr[pos] = NULL;
			return b;
		}
		return false;
	}

public:
	~ArrayIndex()
	{
		clear(true);
	}

	void clear(bool bFreeBuffer = true)
	{
		for (size_t i=0; i<size(); ++i)
		{
			if (mpArrayPtr[i]!=NULL)
				delete mpArrayPtr[i];
		}
		Array::clear(bFreeBuffer);
	}
};

template <typename T>
class ArrayMap : public Array<T>
{
public:
	void Append(int id, const T &val)
	{
		AssertEx(id>=0, "ArrayMap must id>=0 ");
		if (id>=size())
			resize(id+8);
		mpArrayPtr[id] = val;
	}

	T& Find(int id)
	{
		if (id>=0 && id<size())
			return mpArrayPtr[id];

		return _getStatic();
	}

	// NOTE: 必须覆盖此功能，Array::remove 会移动下标对应的值
	bool remove(size_t pos)
	{
		if (pos>=0 && pos<size())
		{
			mpArrayPtr[pos] = T();
			return true;
		}
		return false;
	}
};


#endif //_INCLUDE_ARRAYINDEX_H_