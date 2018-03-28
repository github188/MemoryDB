/********************************************************************
	created:	2012/02/24
	created:	24:2:2012   23:40
	filename: 	d:\New\Common\DataBase\MemBase.h
	file path:	d:\New\Common\DataBase
	file base:	SmallMemoryBase
	file ext:	h
	author:		杨文鸽
	
	purpose:	内存池应用,小于等于4个字节时,使用固定大小可扩展池 Bucket
				大于4字节时使用活动内存池 EasyMemory
*********************************************************************/
#ifndef _INCLUDE_MEMORYBASE_H_
#define _INCLUDE_MEMORYBASE_H_

#include <string>
#include "Assertx.h"
#include "stdlib.h"
#include "BaseCommon.h"

extern BaseCommon_Export int GetBitNum(size_t);
//-----------------------------------------------------------------------------------------

#ifdef __SERVER__
#if DEVELOP_MODE
#	define USE_BIG_POOL				0
#	define USE_SYSTEM_MEMORY		0
#	define MEMORY_DEBUT_TEST		1
#	define DEBUT_STRING				1		// 字符串显示调试值
#else
#	define USE_BIG_POOL				0
#	define USE_SYSTEM_MEMORY		0
#	define MEMORY_DEBUT_TEST		0
#	define DEBUT_STRING				0
#endif
#else
#	define USE_BIG_POOL				0
#	define USE_SYSTEM_MEMORY		0		// 客户端使用系统内存, 开启内存池功能有概率出现异常错误
#	define MEMORY_DEBUT_TEST		0
#endif


#define MULTI_THREAD_LOCK			1		// NOTE: 目前Hand Use池使用 Bucket, 因此，必须使用多线锁
#define USE_SMALL_POOL				1


#define BASE_BLOCK_SIZE				8

#ifdef __SERVER__
#	define BLOCK_INIT_NUM			256 // 初始分配数量
#	define TRUCK_MAX_NUM			255	// max object size 2048
#else
#	define BLOCK_INIT_NUM			64
#	define TRUCK_MAX_NUM			128	// max object size 1024
#endif
const size_t TEMP_NUM		=	BASE_BLOCK_SIZE-1;
const size_t BIT_NUM		=	GetBitNum(BASE_BLOCK_SIZE);
const int MAX_OBJECT_SIZE	=	TRUCK_MAX_NUM * BASE_BLOCK_SIZE;

#if __SERVER__
	const size_t MEMORY_POOL_SIZE	= 100 * 1024 *1024; // 内存池容量 (100M)
#else
	const size_t MEMORY_POOL_SIZE	= 1024;//1  * 1024 *1024; // 内存池容量 (32M)
#endif

#define GET_BLOCK_SIZE(x)		( (x+TEMP_NUM)&~TEMP_NUM )
#define GET_BLOCK_POS(x)		( (GET_BLOCK_SIZE(x)>>BIT_NUM)-1 )

#if MULTI_THREAD_LOCK
#	include "Lock.h"
#endif

#if MEMORY_DEBUT_TEST
#	include <map>
#endif
//-----------------------------------------------------------------------------------------
class EasyMemory;
class MemoryPool;

struct BaseCommon_Export Bucket
{
	friend class Allot;

	struct BlockInfo
	{
		unsigned char*		mPoint;
		size_t					mSize;
	};

private:
	unsigned char			*m_NewPointer; 
	BlockInfo*					mAllocPtrList;
	size_t						mBlockCount;
	int							mObjectSize;

#if MEMORY_DEBUT_TEST
	size_t			mNewCount;
	size_t			mFreeCount;
	size_t			mTotalSize;
#endif

#if MULTI_THREAD_LOCK
	CsLock						mLock;
#endif

public:
	Bucket(size_t cellSize = 0, int initAllocCount = 0)
	{
		m_NewPointer = 0;
		mAllocPtrList = 0;
		mBlockCount = 0;
		mObjectSize = cellSize;
#if MEMORY_DEBUT_TEST
		mNewCount = 0;
		mFreeCount = 0;
		mTotalSize = 0;
#endif
		if (initAllocCount>0)
			MyAlloc(initAllocCount);
	}

	~Bucket()
	{
		for( size_t i=0; i<mBlockCount; ++i )
		{
			free(mAllocPtrList[i].mPoint);
		}
		delete[] mAllocPtrList;
		mAllocPtrList = 0;
		mBlockCount = 0;
	}

	int getBlockSize(void) const { return mObjectSize; }

protected:	
	void setBlockSize(size_t size){ mObjectSize = size; }
	size_t getCount(){ return mBlockCount; }

public:
	void *newPtr()
	{
#if MULTI_THREAD_LOCK
		mLock.lock();
#endif
		if(NULL==m_NewPointer) 
			MyAlloc(); 
		unsigned char *rp = m_NewPointer; 
		//由于头4个字节被“强行”解释为指向下一内存块的指针，这里m_NewPointer就指向了下一个内存块，以备下次分配使用。 
		m_NewPointer = *reinterpret_cast<unsigned char**>(rp);

#if MEMORY_DEBUT_TEST
		mNewCount++;
#endif
#if MULTI_THREAD_LOCK
		mLock.unlock();
#endif
		return rp; 
	}

	bool deletePtr(void *ptr)
	{

#if MULTI_THREAD_LOCK
		mLock.lock();
#endif

#if MEMORY_DEBUT_TEST
		AssertEx(HasPoint(ptr), "必定在池范围内");
#endif
//		if (!HasPoint(ptr))
//		{
//#if MULTI_THREAD_LOCK
//			mLock.unlock();
//#endif
//			return false;
//		}

		*reinterpret_cast<unsigned char**>(ptr) = m_NewPointer; 
		m_NewPointer = static_cast<unsigned char*>(ptr); 
#if MEMORY_DEBUT_TEST
		mFreeCount++;
#endif
#if MULTI_THREAD_LOCK
		mLock.unlock();
#endif
		return true;
	}

protected:
	//判断指针是否在分配范围内
	bool HasPoint(void *ptr)
	{
		for( size_t i=0; i<mBlockCount; ++i )
		{
			if (ptr>=mAllocPtrList[i].mPoint && ptr<mAllocPtrList[i].mPoint+mAllocPtrList[i].mSize)
				return true;
		}
		return false;
	}

	void appendPtr(unsigned char* pointer, size_t size)
	{
		if (mBlockCount>0)
		{
			BlockInfo* temp = mAllocPtrList;
			mAllocPtrList = new BlockInfo[mBlockCount+1];
			memcpy(mAllocPtrList, temp, sizeof(BlockInfo)*mBlockCount);
			delete[] temp;
		}
		else
		{
			mAllocPtrList = new BlockInfo[1];
		}
		mAllocPtrList[mBlockCount].mPoint = pointer;
		mAllocPtrList[mBlockCount].mSize = size;
		++mBlockCount;
	}

	void MyAlloc(size_t allocCount = BLOCK_INIT_NUM) 
	{ 		
		if (getCount()>0)
		{
				allocCount = BLOCK_INIT_NUM * getCount() * 2;
		}
		size_t newSize = mObjectSize * allocCount;
		m_NewPointer = (unsigned char*)malloc(newSize); 
#if MEMORY_DEBUT_TEST
		mTotalSize += newSize;
#endif
		appendPtr(m_NewPointer, newSize);
		//强制转型为双指针，这将改变每个内存块头4个字节的含义。 
		unsigned char **cur = reinterpret_cast<unsigned char**>(m_NewPointer); 
		unsigned char *next = m_NewPointer; 
		for(size_t i = 0; i < allocCount-1; i++) 
		{ 
			next += mObjectSize; 
			*cur = next; 
			 //这样，所分配的每个内存块的头4个字节就被“强行“解释为指向下一个内存块的指针， 即形成了内存块的链表结构。 
			cur = reinterpret_cast<unsigned char**>(next);
		} 
		*cur = 0; 
	} 
};

class BaseCommon_Export Allot
{	

#if MEMORY_DEBUT_TEST
protected:
	size_t			mAllNewCount;
	size_t			mAllFreeCount;

	size_t			mUsePoolSize;
	size_t			mFreePoolSize;

	size_t			mUseSystemSize;
	size_t			mFreeSystemSize;
#endif

public:

	static void* newPtr(size_t size, const char* szFile, unsigned int nLine, const char* szFunction);

	static void* newPtr(size_t size);

	static void freePtr(void *p);

	static void setLogFile(const char* pathFileName);

	static void freePtr(void *ptr, size_t size);

	static size_t getSize(void *ptr)
	{
		return msAllocation._getSize(ptr);
	}

	static void dump(){ msAllocation._dump(); }

	void dumpNewInfo();

public:
	Allot();
	~Allot();

	void _dump();

public:

	void* allot(size_t &size);

	void release(void *ptr);

	void recordInfo(void *p,  const char* szFile, unsigned int nLine, const char* szFunction, size_t size);

	// 注意：尺寸会修改为4的倍数
	void* _smallMalloc(size_t &size);

	size_t _freeMalloc(void *ptr);

	size_t _getSize(void *ptr);

protected:
	Bucket					mPointPtrList[TRUCK_MAX_NUM];
	EasyMemory				*mEasyMemory;

	MemoryPool				*mMemoryPool;
#if MULTI_THREAD_LOCK
	CsLock					mLock;
#endif

public:
	static Allot			msAllocation;

#if DEVELOP_MODE 
    static char gLogPathFile[256];
#endif

protected:
#if MEMORY_DEBUT_TEST
	struct MemInfo 
	{
		const char	*mFile;
		int			mLine;
		const char	*mFunction;
		size_t		mSize;
	};
	typedef std::map<void*, MemInfo>	MemInfoMap;
	MemInfoMap			mMemInfoList;

	struct  sNEW_INFO
	{
		int mNewCount;
		int mLastCount;
		int mFreeCount;
	};

	std::map<std::string, sNEW_INFO>		mNewInfo;
#endif
};

class MemBase
{
public:
	void* operator new(size_t sz, const char* file, int line, const char* func)
	{
		return Allot::newPtr(sz, file, line, func);
	}

	void* operator new(size_t sz)
	{
		return Allot::newPtr(sz); 
	}

	/// placement operator new
	void* operator new(size_t sz, void* ptr)
	{
		(void) sz;
		return ptr;
	}
#ifndef _DEBUG
	/// array operator new, with debug line info
	void* operator new[] ( size_t sz, const char* file, int line, const char* func )
	{
		return Allot::newPtr(sz, file, line, func);
	}
#endif
	void* operator new[] ( size_t sz )
	{
		return Allot::newPtr(sz); 
	}

	void operator delete( void* ptr )
	{
		Allot::freePtr(ptr);
	}

#if _DEBUG
	void operator delete(void* p, const char *lpszFileName, int nLine)
	{
		Allot::freePtr(p);
	}
#endif
	// Corresponding operator for placement delete (second param same as the first)
	void operator delete( void* ptr, void* )
	{
		Allot::freePtr(ptr);
	}

	// only called if there is an exception in corresponding 'new'
	void operator delete( void* ptr, const char* , int , const char*  )
	{
		Allot::freePtr(ptr);
	}

	void operator delete[] ( void* ptr )
	{
		Allot::freePtr(ptr);
	}


	void operator delete[] ( void* ptr, const char* , int , const char*  )
	{
		Allot::freePtr(ptr);
	}

protected: 
	MemBase()
	{		
	}
	~MemBase()
	{ 
	} 
}; 

#if  MEMORY_DEBUT_TEST
#   define MEM_NEW_GLOBAL   new(__FILE__, __LINE__, "_Global_")
#	define MEM_NEW	new(__FILE__, __LINE__, __FUNCTION__)
#	define MEM_DELETE	delete
#	define ALLOCT_NEW(s) Allot::newPtr(s, __FILE__, __LINE__, __FUNCTION__)
#else
#   define MEM_NEW_GLOBAL   new
#	define MEM_NEW		new
#	define MEM_DELETE	delete
#	define ALLOCT_NEW(s) Allot::newPtr(s)
#endif

#define ALLOCT_FREE(p) {  if (p!=NULL) { Allot::freePtr(p); p = NULL; } }

//-----------------------------------------------------------------------------------------


#endif //_INCLUDE_MEMORYBASE_H_