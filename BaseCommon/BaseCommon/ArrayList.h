#ifndef _INCLUDE_ARRAYLIST_H_
#define _INCLUDE_ARRAYLIST_H_

#include "Array.h"
//-------------------------------------------------------------------------*
// 数组方式实现的列表，使用于频繁增加删除的列表，但不要求元素顺序的列表
// 快速删除和增加，删除时直接将最后元素移到到删除的位置，增加只增加到最后
//-------------------------------------------------------------------------*
template<typename T, int INIT_COUNT = 8>
class ArrayList : public Array<T, INIT_COUNT>
{
public:
	void add(const T &value)
	{
		push_back(value);
	}

	bool removeAt(size_t pos)
	{
		if (pos>=0 && pos<size()-1)
		{
			mpArrayPtr[pos] = mpArrayPtr[size()-1];
			mpArrayPtr[size()-1] = T();
			--mCount;
			return true;
		}
		else if (pos==size()-1)
		{
			mpArrayPtr[size()-1] = T();
			--mCount;
			return true;
		}
		return false;
	}

	bool remove(const T &value)
	{
		for (size_t i=0; i<size(); ++i)
		{
			if (mpArrayPtr[i]==value)
			{
				return removeAt(i);
			}
		}
		return false;
	}
};
//-------------------------------------------------------------------------*

#endif //_INCLUDE_ARRAYLIST_H_