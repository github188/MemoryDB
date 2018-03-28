/********************************************************************
	created:	2011/05/24
	created:	24:5:2011   23:13
	filename: 	h:\2011_02_27\Code\common\DataBase\Array.h
	file path:	h:\2011_02_27\Code\common\DataBase
	file base:	Array
	file ext:	h
	author:		���ĸ�
	
	purpose:	
*********************************************************************/

#ifndef _INCLUDE_ARRAY_H_
#define _INCLUDE_ARRAY_H_

#include "Assertx.h"

#include "MemBase.h"
#include <new>

#ifndef NULL
#	define NULL		0
#endif

#pragma pack(push)
#pragma pack(1)

template<typename T, int INIT_COUNT = 8, typename _SIZE_TYPE = unsigned int>
class tArray  : public MemBase
{
protected:
	//-------------------------------------------------------------------------
	// �ڴ��еĶ�����, ���, �Ƴ�
	// �������ʼֵ, ��֤�����ʼ�ڴ�ռ䲻Ϊ��ʱ����ȷ��
	// �Ƴ�ʱ, ���Ա�֤��ȷ���ö��������, ��֤������ȷ�ͷ�
	//-------------------------------------------------------------------------
	void __Set(T &o) { new (&o) T; }
	void __Free(T &o) { (&o)->~T(); }
	//-------------------------------------------------------------------------

public:
	tArray()
		:mCount(0)
		,mpArrayPtr(NULL)
		,mSize(0)
	{		
	}

	tArray(size_t initSize)
		: mCount(0)
		, mpArrayPtr(NULL)
		, mSize(0)
	{
		resize(initSize);
	}

	~tArray()
	{
		clear(true);
	}

	size_t size() const { return mCount; }
	bool empty()const{ return 0==mCount; }

protected:
	void _resize(size_t num, T def = T())
	{
		T *p = mpArrayPtr;
		if (num==mCount)
			return;
		else if (num<mCount)
		{
			mpArrayPtr = (T*)ALLOCT_NEW(sizeof(T)*num);			
			memcpy(mpArrayPtr, p, sizeof(T)*num);

			for (size_t i=num; i<mCount; ++i)
			{
				__Free(p[i]);
			}
		}
		else if (num==0)
		{
			clear();
		}
		else if (mCount==0)
		{
			mpArrayPtr = (T*)ALLOCT_NEW(sizeof(T)*num);
			//memset(ptrArray, 0, sizeof(T)*num);
			for (size_t i=0; i<num; ++i)
			{
				__Set(mpArrayPtr[i]);
				mpArrayPtr[i] = def;
			}
		}
		else
		{
			mpArrayPtr = (T*)ALLOCT_NEW(sizeof(T)*num);
			//memset( ptrArray+count, 0, sizeof(T)*(num-count));
			memcpy(mpArrayPtr, p, sizeof(T)*mCount);

			for (size_t i=mCount; i<num; ++i)
			{
				__Set(mpArrayPtr[i]);
				mpArrayPtr[i] = def;
			}
		}
		
		Allot::freePtr(p, mCount*sizeof(T));			
		mCount = num;
	}


public:
	void push_back(T val)
	{
		size_t nowCount = mCount;
		if (nowCount >= mSize)
		{
			if (mSize<=0)
				mSize = INIT_COUNT;
			else
				mSize *=2;
			this->_resize(mSize);
		}
		mpArrayPtr[nowCount] = val;
		mCount = nowCount + 1;
	}

	void pop_back()
	{
		if (!empty()) 
		{
			__Free(mpArrayPtr[--mCount]);			
		}
	}

	bool remove(size_t pos)
	{
		if (pos < size())
		{
			__Free(mpArrayPtr[pos]);

			memmove(mpArrayPtr+pos, mpArrayPtr+pos+1, (size()-(pos+1))*sizeof(T));
			--mCount;

			__Set(mpArrayPtr[mCount]);

			return true;
		}
		return false;
	}

	void clear(bool bFreeBuffer = true)
	{
		if (mpArrayPtr)
		{			
			if (bFreeBuffer)
			{
				for (size_t i=0; i<mSize; ++i)
				{
					__Free(mpArrayPtr[i]);
				}
				Allot::freePtr(mpArrayPtr, sizeof(T)*mSize);
				mpArrayPtr = NULL;
				mSize = 0;
			}
			else
			{
				for (size_t i=0; i<mSize; ++i)
				{
					mpArrayPtr[i] = T();
				}
			}
		}
		mCount = 0;		
	}

	void resize(size_t num, T def = T())
	{
		if (mCount==num)
			return;

		size_t nowCount = mCount;

		if (mSize<num)
		{
			_resize(num, def);
			mSize = num;
		}
		else if (nowCount>num)
		{
			for (size_t i=num; i<nowCount; ++i)
			{
				mpArrayPtr[i] = T();
			}
		}
		else 
		{
			for (size_t i=nowCount; i<num; ++i)
			{
				mpArrayPtr[i] = def;
			}
		}		
		mCount = num;
	}

	void swap(tArray &other)
	{
		size_t nTemp = mCount;
		T *p = mpArrayPtr;

		mCount = other.mCount;
		mpArrayPtr = other.mpArrayPtr;

		other.mCount = nTemp;
		other.mpArrayPtr = p;

		size_t tempSize = mSize;
		mSize = other.mSize;
		other.mSize = tempSize;
	}

	size_t _useSpaceSize() const
	{
		return sizeof(tArray) + sizeof(T)*mSize;
	}

protected:
	_SIZE_TYPE		mSize;
	_SIZE_TYPE		mCount;
	T						*mpArrayPtr;
};
//-------------------------------------------------------------------------*/
template<typename T, int INIT_COUNT = 8>
class Array : public tArray<T, INIT_COUNT>
{
public:
	Array()
	{		
		mStatic = T();
	}

	Array(size_t initSize)
		: tArray(initSize)
	{
		mStatic = T();
		resize(initSize);
	}

	Array( const Array &other )
	{
		mStatic = T();
		*this = other;
	}

	~Array()
	{
		clear(true);
	}

public:
	T& back()
	{
		if (empty())
		{
			AssertEx(0, "����Ϊ��,ȡ���Ԫ��ʧ��"); 
			return _getStatic();
		}		
		return mpArrayPtr[mCount-1]; 
	}
	const T& back()const
	{
		if (!empty())
			return mpArrayPtr[mCount-1]; 
		AssertNote(0, "����Ϊ��,ȡ���Ԫ��ʧ��"); 
		return T();
	}

	T & operator [] (size_t pos)
	{
		if (pos>=mCount)
		{
			AssertNote(0, "����:ʹ������Խ��");
			return _getStatic();
		}
		return mpArrayPtr[pos];
	}

	const T & operator [] (size_t pos) const
	{
		if (pos>=mCount)
		{
			AssertEx(0, "����:ʹ������Խ��");
			return _getStatic();
		}
		return mpArrayPtr[pos];
	}

	Array& operator = (const Array &other)
	{
		if (&other==this)
			return *this;
		clear(false);
		resize(other.size());
		for (size_t i=0; i< other.size(); ++i)
		{
			mpArrayPtr[i] = other.mpArrayPtr[i];
		}
		return *this;
	}

	T& _getStatic() const
	{
		return mStatic;
	}

protected:
	mutable T	mStatic;		// ֧�ֶ���ʱ�����ڷ��ز����ڵ�����
};

//-------------------------------------------------------------------------*/
#pragma pack(pop)

#endif //_INCLUDE_ARRAY_H_