#ifndef _INCLUDE_ARRAYLIST_H_
#define _INCLUDE_ARRAYLIST_H_

#include "Array.h"
//-------------------------------------------------------------------------*
// ���鷽ʽʵ�ֵ��б�ʹ����Ƶ������ɾ�����б�����Ҫ��Ԫ��˳����б�
// ����ɾ�������ӣ�ɾ��ʱֱ�ӽ����Ԫ���Ƶ���ɾ����λ�ã�����ֻ���ӵ����
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