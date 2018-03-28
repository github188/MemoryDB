/********************************************************************
	created:	2012/02/24
	created:	24:2:2012   23:00
	filename: 	d:\New\Common\DataBase\EasyMemory.h
	file path:	d:\New\Common\DataBase
	file base:	EasyMemory
	file ext:	h
	author:		杨文鸽
	
	purpose:	内存池
	说明：最小块优先分配算法
	注意：开始的闭块不能被全部分配掉，最少要保留８个字节，即８字节不能被使用
*********************************************************************/


#ifndef _INCLUDE_EASYMEMORY_H_
#define _INCLUDE_EASYMEMORY_H_


#include "Assertx.h"
#include "Lock.h"

#define BLOCK_SIZE				64		//must >= 8

#define POOL_USE_MULTI_THREAD	1
#define DEBUG_MODE				1


#define SAVE_SIZE_SPACE	sizeof(size_t)

//----------------------------------------------------------------------

const size_t POOL_TEMP_NUM		=	BLOCK_SIZE-1;

#define ALIGNED_BLOCK_SIZE(x)		( (x+POOL_TEMP_NUM)&~POOL_TEMP_NUM )

size_t ALIGNED_SIZE(size_t s, size_t alignedByte );
//----------------------------------------------------------------------

#if DEBUG_MODE
#include <map>
#include <list>
#endif

#define MIN_BLOCK_SIZE  (sizeof(void*)+sizeof(Block))  //前四个字节保存下一个块的地址，后８个字节是开始块信息


// DEBUG_MODE 模式时保存已经分配的信息
class BaseCommon_Export EasyMemory
{
	struct Block
	{
		Block	*mNext;
		int		mSize;

	public:
		char* endPtr()
		{
			return ((char*)this) + mSize;
		}
	};


protected:
	friend class MemoryPool;

	EasyMemory* _getNextPool() 
	{
#pragma warning(push)
#pragma warning(disable:4312)
		return (EasyMemory*)( *((int*)_getHead())); 
#pragma warning(pop)
	}
	void _setNextPool(EasyMemory *pNext) {  memcpy(_getHead(), &pNext, sizeof(size_t));   }

	char* _getHead(void){ return ((char*)mHeadBlock)-sizeof(size_t); }
	char* _getEnd(void){ return _getHead()+mSpaceSize; }

	size_t _getPoolSize(void){ return mSpaceSize - MIN_BLOCK_SIZE; }
	bool _isAllFree(void){ return (mHeadBlock->mNext) && mHeadBlock->mNext->mSize == _getPoolSize(); }

public:
	EasyMemory(size_t size)
		//: MIN_BLOCK_SIZE(sizeof(void*)+sizeof(Block))
	{
		size = ALIGNED_BLOCK_SIZE(size);
		int x = MIN_BLOCK_SIZE;
		char* mHeadPoint = (char*)malloc(size + MIN_BLOCK_SIZE);
		AssertEx(mHeadPoint, "malloc memory fail");

		if (NULL==mHeadPoint)
		{
#ifdef __WINDOWS__
			throw std::exception("Error: fail malloc memory, memory is not enough");
#else
            throw (1);
#endif
			return;
		}

		memset(mHeadPoint, 0xcd, size+MIN_BLOCK_SIZE);

		mSpaceSize = size+MIN_BLOCK_SIZE;
		mHeadBlock = (Block*)(mHeadPoint+sizeof(size_t));

		Block *pNext = (Block*)((char*)mHeadPoint+MIN_BLOCK_SIZE);
		pNext->mNext = NULL;
		pNext->mSize = size;

		mHeadBlock->mNext = pNext;
		mHeadBlock->mSize = MIN_BLOCK_SIZE-sizeof(size_t);
		
		memset(_getHead(), 0, sizeof(size_t));
		//_setNextPool(NULL);

	}
	~EasyMemory()
	{
		free((char*)mHeadBlock-sizeof(size_t));
	}

public:
	// more at point before alloc 4 bit save size for delete
	void* Malloc(size_t uSize, size_t &resultSize)
	{
#if MULTI_THREAD_LOCK
		mLock.lock();
#endif
		size_t size = uSize + SAVE_SIZE_SPACE;

#if DEBUG_MODE
		size_t needSize = size;
#endif
		size = ALIGNED_BLOCK_SIZE(size);

		resultSize = size;
		// find can malloc and min size
		Block *pParent = NULL;
		Block *pBlock = NULL;

		Block *pCurrent = mHeadBlock->mNext;
		Block *pCurrentParent = mHeadBlock;
		while (pCurrent)
		{
			if (pCurrent->mSize==size)
			{
				pParent = pCurrentParent;
				pBlock = pCurrent;
				break;
			}
			else if (pCurrent->mSize>(int)size)
			{
				if (NULL==pBlock || pCurrent->mSize<pBlock->mSize)
				{
					pParent = pCurrentParent;
					pBlock = pCurrent;
				}
			}		
			pCurrentParent = pCurrent;
			pCurrent = pCurrent->mNext;
		}
		// 开始块节点不参加分割和合并，永远是８个字节
		//if (NULL==pBlock && size<mHeadBlock->mSize)
		//	pBlock = mHeadBlock;
		if (pBlock)
		{
			void *p = NULL;
#if DEBUG_MODE
			p = Partition(pParent, pBlock, size);
			if (p)
			{
				memset(p, 0x00, size);
				mUseBlock[p] = size;
			}
#else
			// partition memory form block
			p = Partition(pParent, pBlock, size);
#endif

#if MULTI_THREAD_LOCK
			mLock.unlock();
#endif

			if (p)
			{
				// save size at head
				*((size_t*)p) = size;
				return (void*)((char*)p + SAVE_SIZE_SPACE);	
			}
			else
				return NULL;

		}
#if MULTI_THREAD_LOCK
		mLock.unlock();
#endif
		return NULL;
	}

	size_t Free(void *freePoint)
	{
		// check in rang of memory pool
		if (freePoint<_getHead() || freePoint>=_getEnd())
			return 0;

		size_t size = *((size_t*)freePoint-1);
		if (size%BLOCK_SIZE>0)
			return 0;

		freePoint = (void*)((size_t*)freePoint-1);

#if MULTI_THREAD_LOCK
		mLock.lock();
#endif
#if DEBUG_MODE
		std::map<void*, size_t>::iterator it = mUseBlock.find(freePoint);
		if (it==mUseBlock.end() || it->second!=size)
		{
			AssertEx(0,  "Error : not find point record in debug list");
		}
		mUseBlock.erase(it);

		memset(freePoint, 0xCD, size);
#endif
		//if (mHeadBlock==NULL)
		//{
		//	// can use all
		//	// AssertEx(0);
		//}
//#if !SAVE_SIZE_MODE
//		size = (size_t)((size+(BLOCK_SIZE-1))/BLOCK_SIZE) * BLOCK_SIZE;
//#endif

		// find before block at freePoint
		void *endPoint = (char*)freePoint + size;

		Block *pCurrent = mHeadBlock;
		while (pCurrent->mNext)
		{
			if (pCurrent->mNext>=endPoint)
				break;
			pCurrent = pCurrent->mNext;
		}
		if (pCurrent)
		{
			//AssertEx(freePoint>=pCurrent->endPtr(), "space pos logic error");
		//!!!	Merge(pCurrent, freePoint, size);
//#if DEBUG_MODE
//			if (mUseBlock.empty())
//			{
//				AssertEx(mHeadBlock->mNext && mHeadBlock->mNext->mSize==mSpaceSize-MIN_BLOCK_SIZE, 
//					 "error: may be logic error, all free then next block size must is pool size");
//			}
//#endif
//#if MULTI_THREAD_LOCK
//			mLock.unlock();
//#endif
//
			return size;

		}
#if MULTI_THREAD_LOCK
		mLock.unlock();
#endif


		return 0;

	}

protected:
	void* Partition(Block *pParent, Block *pBlock, size_t size)
	{
		if (pBlock->mSize<(int)size)
			return NULL;

		if (pBlock->mSize-size>=BLOCK_SIZE)
		{
			// malloct memory at last apace
			pBlock->mSize -= size;
			return (void*)((char*)pBlock + pBlock->mSize);
		}
		else
		{
			if (pParent)
			{
				pParent->mNext = pBlock->mNext;					
			}
			else if (mHeadBlock==pBlock)
			{
				//first head 8 bit not can use.
				return NULL;
				// find first head
				//while (mHeadBlock->mNext)
				//{
				//	mHeadBlock->mNext
				//}
			}
			return ((void*)pBlock);
		}
	}
	
	void Merge(Block *pBeforeBlock, void *pPoint, size_t size)
	{
		Block *pBackBlock = pBeforeBlock->mNext;
		Block *pBackParent = pBeforeBlock;
		
		if (pBeforeBlock!=mHeadBlock && pBeforeBlock->endPtr() == pPoint)
		{
			//merge before		
			pBeforeBlock->mSize += size;
			// check merge before and back
			if (pBackBlock && pBeforeBlock->endPtr() == (void*)pBackBlock)
			{
				//merge before		
				pBeforeBlock->mSize += pBackBlock->mSize;
				// set next ptr
				pBeforeBlock->mNext = pBackBlock->mNext;
			}
		}
		else if (pBackBlock && (char*)pPoint+size == (char*)pBackBlock)
		{
			// merge back
			Block *pBlock = (Block*)pPoint;
			pBlock->mSize = size + pBackBlock->mSize;
			pBlock->mNext = pBackBlock->mNext;
			// set parent next ptr
			pBackParent->mNext = pBlock;
		}
		else
		{
			Block *pBlock = (Block*)pPoint;
			pBlock->mNext = pBackBlock;
			pBlock->mSize = size;
			pBeforeBlock->mNext = pBlock;
		}
	}

	Block* _getBlock(void *point)
	{
		return (Block*)point;
	}

#if DEBUG_MODE
public:
	void PrintMemory(const char *szInfo);
#endif

protected:
	size_t				mSpaceSize;
	Block				*mHeadBlock;

#if MULTI_THREAD_LOCK
	CsLock				mLock;
#endif

#if DEBUG_MODE
	std::map<void*, size_t>	mUseBlock;

	size_t getNowSpaceSize()
	{
		size_t nowSize = 0;
		Block *p = mHeadBlock->mNext;
		while(p)
		{
			nowSize += p->mSize;
			p = p->mNext;
		}
		return nowSize;
	}
#endif
};

//-------------------------------------------------------------------------------------
// 动态扩展分配池管理
// 如果当前所有池没有闭置，则进行创建新的池进行分配。
// 释放时，如果当前池全部为空，且当前池数量大于１,则进行释放空闲池
//-------------------------------------------------------------------------------------
class MemoryPool
{
public:
	MemoryPool(size_t initSize)
		: mHeadPool(initSize)
		, mMaxSize(initSize)
	{
		AssertEx(initSize, "init pool size can not zero");
#if DEBUG_MODE
		mAlloctSize = 0;
		mFreeSize = 0;

		mAllPoolSize = mHeadPool._getPoolSize(); 
		mFreePoolSize = 0;
#endif 
	}

	~MemoryPool()
	{
		EasyMemory *pPool = mHeadPool._getNextPool();
		while (pPool)
		{
			EasyMemory *temp = pPool;
			pPool = pPool->_getNextPool();
			delete temp;
		}
	}

	size_t _getMaxPoolSize(void){ return mMaxSize; }
	size_t _getBaseSize(void){ return mHeadPool._getPoolSize(); }

public:
	void* allocateBytes(size_t count, size_t &resultSize)
	{
#if POOL_USE_MULTI_THREAD
		mLock.lock();
#endif
		resultSize = 0;
		void *p = mHeadPool.Malloc(count, resultSize);
		if (p)
		{
#if DEBUG_MODE
			mAlloctSize += resultSize;
#endif
#if POOL_USE_MULTI_THREAD
			mLock.unlock();
#endif
			return p;
		}
		// try allot from all pool
		EasyMemory *pPool = mHeadPool._getNextPool();
		while (pPool)
		{
			p = pPool->Malloc(count, resultSize);
			if (p)
			{
#if DEBUG_MODE
				mAlloctSize += resultSize;
#endif
#if POOL_USE_MULTI_THREAD
				mLock.unlock();
#endif
				return p;
			}
			pPool = pPool->_getNextPool();
		}

		// create new pool and allot
		mMaxSize *= 2;
		size_t size = ALIGNED_SIZE(count + sizeof(size_t), _getBaseSize());
		while (mMaxSize<size)
		{
			mMaxSize *= 2;
		}
		EasyMemory *pNewPool = new EasyMemory(mMaxSize);
#if DEBUG_MODE
		mAllPoolSize += pNewPool->_getPoolSize();
		mUsePoolList.push_back(pNewPool->_getPoolSize());
#endif
		p = pNewPool->Malloc(count, resultSize);
		AssertEx( p, "allot pool error");

		pNewPool->_setNextPool(mHeadPool._getNextPool());
		mHeadPool._setNextPool(pNewPool);

		//printf("Max size is [%u]\n", mMaxSize);
#if DEBUG_MODE
		mAlloctSize += resultSize;
#endif

#if POOL_USE_MULTI_THREAD
		mLock.unlock();
#endif
		return p;
	}

	//void* allocateBytes(size_t count, size_t &resultSize, const char* file, int line, const char* func)            
	//{
	//	return allocateBytes(count, resultSize);
	//}

	size_t deallocateBytes(void* ptr)
	{
#if POOL_USE_MULTI_THREAD
		mLock.lock();
#endif
		// 这里可以使用后台机制，放到另一线程做释放操作
		size_t s = mHeadPool.Free(ptr);
		if (s)
		{
#if DEBUG_MODE
			mFreeSize += s;
#endif
#if POOL_USE_MULTI_THREAD
			mLock.unlock();
#endif
			return s;
		}

		// try free from all pool
		EasyMemory *pPool = mHeadPool._getNextPool();
		EasyMemory *pParentPool = &mHeadPool;
		while (pPool)
		{
			s = pPool->Free(ptr);
			if (s>0)
			{
				// check need free pool
				if (pPool->_isAllFree())
				{
#if DEBUG_MODE
					mFreePoolSize += pPool->_getPoolSize();
#endif
					pParentPool->_setNextPool(pPool->_getNextPool());
					delete pPool;
					pPool = NULL;
					mMaxSize /= 2;
				}
#if DEBUG_MODE
				mFreeSize += s;
#endif
#if POOL_USE_MULTI_THREAD
				mLock.unlock();
#endif
				return s;
			}
			pParentPool = pPool;
			pPool = pPool->_getNextPool();
		}

		AssertEx(0,  "free ptr point not in pool");
#if POOL_USE_MULTI_THREAD
		mLock.unlock();
#endif
		return 0;
	}


protected:
	EasyMemory		mHeadPool;
	size_t			mMaxSize;
#if POOL_USE_MULTI_THREAD
	CsLock			mLock;
#endif

#if DEBUG_MODE
public:

	size_t _alloctSize(void) { return mAlloctSize; }
	size_t _freeSize(void) { return mFreeSize; }
	size_t _allPoolSize(void) { return mAllPoolSize; }
	size_t _freePoolSize(void) { return mFreePoolSize; }
	std::list<size_t>& _getUseList(void){ return mUsePoolList; }

	size_t _nowPoolSize(void)
	{
		size_t s = mHeadPool._getPoolSize();
		EasyMemory *pPool = mHeadPool._getNextPool();
		while (pPool)
		{
			s += pPool->_getPoolSize();
			pPool = pPool->_getNextPool();
		}
		return s;
	}

	typedef std::pair<size_t, size_t>	PoolInfo;
	typedef std::list<PoolInfo>	InfoList;

	void _getNowPoolInfo( InfoList &infoList )
	{
		EasyMemory *pPool = &mHeadPool;
		while (pPool)
		{
			infoList.push_back(PoolInfo(pPool->_getPoolSize(), pPool->getNowSpaceSize()));
			pPool = pPool->_getNextPool();
		}
	}

protected:
	size_t			mAlloctSize;
	size_t			mFreeSize;

	size_t			mAllPoolSize;
	size_t			mFreePoolSize;
	std::list<size_t>	mUsePoolList;
#endif
};

#endif //_INCLUDE_EASYMEMORY_H_