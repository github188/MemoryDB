#ifndef _INCLUDE_RECORDMEMPOOL_H_
#define _INCLUDE_RECORDMEMPOOL_H_

#include "MemBase.h"

#define RECORD_POOL_DEBUG	0
//-------------------------------------------------------------------------*/
// 记录中使用的内存池
// NOTE: 1 内存分配大小必须大于8，否则保存不了块的指针，出现严重指针错误
// 2 索引为短整数，最大记录数量数支持 32 * 0xFFFE, DBServer指定为1000，约6500万
// 3 如果池块数超过 0XFFFE, 则直接从系统分配, 池块标识为 0xFFFF, 
//   但分配时需要检查,会影响分配速度
//-------------------------------------------------------------------------*/
struct RecordMemPool
{
	friend class Allot;

	struct BlockInfo
	{
		unsigned char*		mpNewPointer;
		unsigned char*		mPoint;
		int					mSize;
		unsigned short		mFreeCount;
		unsigned short		mIndex;

		BlockInfo()
			: mPoint(NULL)
			, mpNewPointer(NULL)
			, mSize(0)
			, mFreeCount(0)
			, mIndex(0)
		{

		}
	};

private:
	Array<BlockInfo>			mAllocList;
	uint						mObjectSize;
	int							mCurrentAllotBlock;
	uint						mSystemMemCount;

#if RECORD_POOL_DEBUG
	size_t			mNewCount;
	size_t			mFreeCount;
	size_t			mAllotCount;
	size_t			mSysCount;
#endif

public:
	RecordMemPool()
	{
		mObjectSize = 8;
		mCurrentAllotBlock = -1;
		mSystemMemCount = 0;
#if RECORD_POOL_DEBUG
		mNewCount = 0;

		mFreeCount = 0;
		mAllotCount = 0;
		mSysCount = 0;
#endif
	}

	~RecordMemPool()
	{
		clear();
#if RECORD_POOL_DEBUG
		NOTE_LOG("记录内存池[%u] Size[%llu = %llu] 分配[%d] 释放[%d], 系统分配[%llu]", mObjectSize, mAllotCount * mObjectSize, mAllotCount, mNewCount, mFreeCount, mSysCount);
#endif
		if (mSystemMemCount>0)
			ERROR_LOG("记录系统内存未完全释放 >[%u]", mSystemMemCount);
	}

	void clear()
	{
		for( size_t i=0; i<mAllocList.size(); ++i )
		{
			if (mAllocList[i].mPoint!=NULL)
			{			
#if DEVELOP_MODE
			ALLOCT_FREE(mAllocList[i].mPoint);
#else
			free(mAllocList[i].mPoint);
#endif
			}
		}
		mAllocList.clear(false);
	}

	void setBlockSize(size_t size)
	{ 
		mObjectSize = size+sizeof(unsigned short); 
		if (mObjectSize<8)
			mObjectSize = 8;
	}
	size_t getBlockSize()const {return mObjectSize-sizeof(unsigned short); }

	void *newPtr()
	{
		BlockInfo *pBlock = NULL;
		if (mCurrentAllotBlock>=0 && mCurrentAllotBlock<mAllocList.size())
		{
			pBlock = &mAllocList[mCurrentAllotBlock];
			if (pBlock->mpNewPointer==NULL)
			{
				pBlock = NULL;
				mCurrentAllotBlock = -1;
				for (size_t i=0; i<mAllocList.size(); ++i)
				{
					if (mAllocList[i].mpNewPointer!=NULL)
					{
						mCurrentAllotBlock = i;
						pBlock = &mAllocList[i];
						break;
					}
				}
			}
		}

		if (pBlock==NULL)
		{
			mCurrentAllotBlock = -1;
			for (size_t i=0; i<mAllocList.size(); ++i)
			{
				if (mAllocList[i].mPoint==NULL)
				{
					mCurrentAllotBlock = i;					
					break;
				}
			}
			if (mCurrentAllotBlock<0)
			{
				if (mAllocList.size()>=0xFFFF)
				{
					++mSystemMemCount;
					// 如果超过了 0xFFFF个块，则直接从系统分配
					if (mSystemMemCount % 100==0)
						WARN_LOG("WARN: [%u] Record memory pool count over 0xFFFF, then use system malloc, now system count >[%u]", mObjectSize-sizeof(ushort), mSystemMemCount);
#if RECORD_POOL_DEBUG
					char *pNew = (char*)ALLOCT_NEW(mObjectSize);
					mNewCount++;
					++mSysCount;
#else
					char *pNew = (char*)::malloc(mObjectSize);
#endif
					*((ushort*)pNew) = 0xFFFF;
					return pNew + sizeof(ushort);
				}

				mAllocList.push_back(BlockInfo());				
				mCurrentAllotBlock = mAllocList.size()-1;
			}

			BlockInfo block = MyAlloc(); 
			mAllocList[mCurrentAllotBlock] = block;
			block.mIndex = mCurrentAllotBlock;

			pBlock = &mAllocList[mCurrentAllotBlock];
		}		
		AssertEx(pBlock->mFreeCount>0, "空闲数量应该大于0");
		unsigned char *rp = pBlock->mpNewPointer; 
		//由于头4个字节被“强行”解释为指向下一内存块的指针，这里m_NewPointer就指向了下一个内存块，以备下次分配使用。 
		pBlock->mpNewPointer = *reinterpret_cast<unsigned char**>(rp);

		*((unsigned short*)rp) = (unsigned short)mCurrentAllotBlock;
		rp += sizeof(unsigned short);

		--pBlock->mFreeCount;

#if RECORD_POOL_DEBUG
		mNewCount++;
#endif
		//memset(rp, 0, getBlockSize());
		return rp; 
	}

	bool deletePtr(unsigned char *ptr)
	{
		ptr -= sizeof(unsigned short);

		unsigned short index = *((unsigned short*)ptr);
		if (index==0xFFFF)
		{
			--mSystemMemCount;
#if RECORD_POOL_DEBUG
			ALLOCT_FREE(ptr);
			mFreeCount++;
#else
			::free(ptr);
#endif
			return true;
		}
#if RECORD_POOL_DEBUG
		AssertEx(HasPoint(ptr), "必定在池范围内");
#endif
		AssertNote(index>=0 && index<mAllocList.size(), "必定在分配列表中");

		BlockInfo &info = mAllocList[index];

		*reinterpret_cast<unsigned char**>(ptr) = info.mpNewPointer; 
		info.mpNewPointer = static_cast<unsigned char*>(ptr); 

		++info.mFreeCount;

		if (info.mFreeCount==info.mSize)
		{
			// 移除块
#if DEVELOP_MODE
			ALLOCT_FREE(info.mPoint); 
#else
			free(info.mPoint);
#endif
			info.mPoint = NULL;
			info.mpNewPointer = NULL;
			info.mSize = 0;
			info.mFreeCount = 0;
		}

#if RECORD_POOL_DEBUG
		mFreeCount++;
#endif
		return true;
	}

protected:
	BlockInfo MyAlloc() 
	{ 
		//AssertEx(m_NewPointer==NULL, "Must is null when alloc");
		size_t allotCount = tBaseTable::msCommonConfig.mRecordMemPoolBlockRecordCount;

		size_t newSize = mObjectSize * allotCount;

		BlockInfo block;
#if DEVELOP_MODE
		block.mpNewPointer = (unsigned char*)ALLOCT_NEW(newSize); 
#else
		block.mpNewPointer = (unsigned char*)malloc(newSize); 
#endif
		block.mPoint = block.mpNewPointer;
		block.mSize = allotCount;
		block.mFreeCount = allotCount;

#if RECORD_POOL_DEBUG
		mAllotCount += allotCount;
#endif		
		//强制转型为双指针，这将改变每个内存块头4个字节的含义。 
		unsigned char **cur = reinterpret_cast<unsigned char**>(block.mpNewPointer); 
		unsigned char *next = block.mpNewPointer; 
		for(size_t i = 0; i < allotCount-1; i++) 
		{ 
			next += mObjectSize; 
			*cur = next; 
			//这样，所分配的每个内存块的头4个字节就被“强行“解释为指向下一个内存块的指针， 即形成了内存块的链表结构。 
			cur = reinterpret_cast<unsigned char**>(next);
		} 
		*cur = 0; 

		return block;
	} 
#if RECORD_POOL_DEBUG
	bool HasPoint(void *ptr)
	{
		for( size_t i=0; i<mAllocList.size(); ++i )
		{
			BlockInfo &info = mAllocList[i];
			if (ptr>=info.mPoint && ptr<info.mPoint+info.mSize*mObjectSize)
				return true;
		}
		return false;
	}
#endif
};

#endif //_INCLUDE_RECORDMEMPOOL_H_