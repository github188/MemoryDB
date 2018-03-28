
#ifndef _INCLUDE_POOLLIST_H_
#define _INCLUDE_POOLLIST_H_

#include "AutoPtr.h"

// 具有自身节点分配池功能链表
// 提高重复增删效率
// NOTE: 目前不支持多线

class NodePool : public MemBase
{
	friend class Allot;

	struct BlockInfo
	{
		unsigned char*		mPoint;
		size_t				mSize;
	};

private:
	unsigned char				*m_NewPointer; 
	BlockInfo*					mAllocPtrList;
	size_t						mBlockCount;
	size_t						mObjectSize;
	int							mInitCount;

#if MEMORY_DEBUT_TEST
	size_t			mNewCount;
	size_t			mFreeCount;
	size_t			mTotalSize;
#endif

public:
	NodePool(int nInitCount = BLOCK_INIT_NUM)
	{
		mInitCount = nInitCount;
		m_NewPointer = 0;
		mAllocPtrList = 0;
		mBlockCount = 0;
		mObjectSize = 4;
#if MEMORY_DEBUT_TEST
		mNewCount = 0;
		mFreeCount = 0;
		mTotalSize = 0;
#endif
	}

	~NodePool()
	{
		for( size_t i=0; i<mBlockCount; ++i )
		{
			free(mAllocPtrList[i].mPoint);
		}
		delete[] mAllocPtrList;
		mAllocPtrList = 0;
		mBlockCount = 0;
	}

	void setBlockSize(size_t size){ mObjectSize = size; }
	size_t getBlockSize(void){ return mObjectSize; }
	size_t getCount(){ return mBlockCount; }

	size_t _useSpaceSize()
	{
		size_t s = 0;
		for( size_t i=0; i<mBlockCount; ++i )
		{
			s += mAllocPtrList[i].mSize;
		}
		return s;
	}

	void *newPtr()
	{

		if(NULL==m_NewPointer) 
			MyAlloc(); 
		unsigned char *rp = m_NewPointer; 
		//由于头4个字节被“强行”解释为指向下一内存块的指针，这里m_NewPointer就指向了下一个内存块，以备下次分配使用。 
		m_NewPointer = *reinterpret_cast<unsigned char**>(rp);

#if MEMORY_DEBUT_TEST
		mNewCount++;
#endif

		return rp; 
	}

	bool deletePtr(void *ptr)
	{
		if (!HasPoint(ptr))
		{
			return false;
		}

		*reinterpret_cast<unsigned char**>(ptr) = m_NewPointer; 
		m_NewPointer = static_cast<unsigned char*>(ptr); 
#if MEMORY_DEBUT_TEST
		mFreeCount++;
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

	void MyAlloc() 
	{ 
		size_t allocCount = mInitCount;
		if (getCount()>0)
			allocCount = mInitCount * getCount() * 2;
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
typedef AutoPtr<NodePool>	AutoNodePool;
//----------------------------------------------------------------------------------

template <typename T, bool bInitPool = true>
class PoolList : public MemBase
{
public:
	class ListNode
	{
	public:
		ListNode()
			: next(NULL)
		{

		}
		~ListNode()
		{
			
		}

		ListNode	*next;
		T			mVal;
	};

	ListNode* __newNode(void)
	{
		void *p = mNodePool->newPtr();
		//printf( "\n new  [%p]", p);
		//memset(p, 0 , sizeof(ListNode));
		return	new (p) ListNode();
	}

	void __freeNode( ListNode *p )
	{
		//printf( "\n free [%p]", p);
		p->next = NULL;
		p->mVal = T();
		
		if (mNodePool)
			mNodePool->deletePtr( p );
	}

public:
	PoolList()
		: mRootNode()
	{
		msVal = T();
		if (bInitPool)
		{
			mNodePool = MEM_NEW NodePool();
			mNodePool->setBlockSize(sizeof(ListNode));
		}
	}
	~PoolList()
	{
		clear();
	}

	void ReadyNodePool(AutoPtr<NodePool> hNodePool)
	{
		if (mNodePool && hNodePool->getBlockSize()!=mNodePool->getBlockSize())
		{
			AssertNote(0, "Pool block size is not same.");
			return;
		}
		AssertNote(empty(), "Warn: now list is not empty.");
		clear();
		mNodePool = hNodePool;
		AssertEx(mNodePool, "Error : ready node pool is null.");
	}

	AutoNodePool GetNodePool(void){ return mNodePool; }

public:
	//virtual void onRaversal(T &noodVal) = 0;

	class iterator
	{
		friend class PoolList<T, bInitPool>;
	public:
		iterator()
			: mNode(NULL)
		{

		}
		iterator(ListNode *node)
			: mNode(node)
		{

		}

		operator bool ()
		{
			return mNode!=NULL;
		}

		void* getNode(){ return mNode; }

		T& operator  *()
		{
			if (mNode)
				return mNode->mVal;
			return msVal;
		}
		iterator& operator ++ ()
		{
			if (mNode)
				mNode = mNode->next;
			else
				mNode = NULL;
			return *this;
		}

		iterator& operator ++ (int)
		{
			if (mNode)
				mNode = mNode->next;
			else
				mNode = NULL;
			return *this;
		}


		bool operator != (const iterator &other) const
		{
			return mNode != other.mNode; 
		}

	private:
		ListNode *mNode; 
		T		msVal;
	};

public:
	iterator begin()
	{
		return iterator(mRootNode);
	}

	iterator& end()
	{
		//static iterator msIt;
		return msIt;
	}

	iterator insert(const iterator &whereIt, const T &val)
	{
		if (mRootNode==NULL || mRootNode==whereIt.mNode)
		{
			insert(val);
			return begin();
		}
		else
		{
			ListNode *pNood = mRootNode;
			while (pNood)
			{
				if (pNood->next==whereIt.mNode)
				{
					pNood->next = __newNode();
					pNood->next->mVal = val;
					pNood->next->next = whereIt.mNode;
					return iterator(pNood->next);
				}
				pNood = pNood->next;
			}
		}
		return end();
	}

	iterator erase(const iterator &whereIt)
	{
		if (NULL == mRootNode || NULL==whereIt.mNode)
			return end();
		if (mRootNode==whereIt.mNode)
		{
			remove(NULL, mRootNode);
			return iterator(mRootNode);
		}
		ListNode *pNood = mRootNode;
		while (pNood)
		{
			if (pNood->next && pNood->next==whereIt.mNode)
			{
				remove(pNood, pNood->next);
				return iterator(pNood->next);
			}
			pNood = pNood->next;
		}
		//AssertEx(0, "不是这个链表的元素")
		return end();
	}

public:
	bool empty()const{ return mRootNode==NULL; }
	size_t size() const
	{
		size_t count = 0;
		ListNode *pNood = mRootNode;
		while (pNood)
		{
			++count;
			pNood = pNood->next;
		}
		return count;
	}
	void clear()
	{
		while (mRootNode)
		{
			remove(NULL, mRootNode);
		}

	}

public:
	void insert(const T &val)
	{
		ListNode *pNood = __newNode();
		pNood->mVal = val;
		pNood->next = mRootNode;
		mRootNode = pNood;
	}

	void* push_back(const T &val)
	{
		if (NULL==mRootNode)
		{
			insert(val);
			return mRootNode;
		}

		ListNode *pNode = mRootNode;
		while (pNode)
		{
			if (NULL==pNode->next)
				break;
			pNode = pNode->next;
		}
		ListNode *newNode = __newNode();
		newNode->mVal = val;
		newNode->next = NULL;
		pNode->next = newNode;

		return (void*)newNode;
	}

	bool pop_back(T &result)
	{
		if (NULL==mRootNode)
		{			
			return false;
		}

		ListNode *pNode = mRootNode;
		ListNode *up = NULL;
		while (pNode)
		{
			if (NULL==pNode->next)
			{
				result = pNode->mVal;
				remove(up, pNode);
				return true;
			}
			up = pNode;
			pNode = pNode->next;			
		}
		return false;
	}

	void push_front(const T &val)
	{
		insert(val);
	}

	bool pop_front(T &result)
	{
		if (mRootNode)
		{
			result = mRootNode->mVal;
			remove(NULL, mRootNode);
			return true;
		}
		return false;
	}


	bool remove(const T &val)
	{
		if (NULL == mRootNode)
			return false;
		if (mRootNode->mVal==val)
			return remove(NULL, mRootNode);
		ListNode *pNood = mRootNode;
		while (pNood)
		{
			if (pNood->next && pNood->next->mVal==val)
			{
				return remove(pNood, pNood->next);
			}
			pNood = pNood->next;
		}
		return false;
	}

	void append(PoolList &other)
	{
		ListNode *pNood = other.mRootNode;
		while(pNood)
		{
			push_back(pNood->mVal);
			pNood = pNood->next;
		}
	}

	bool exist(T &element)
	{
		ListNode *pNood = mRootNode;
		while (pNood)
		{
			if (pNood->mVal==element)
			{
				return true;
			}
			pNood = pNood->next;
		}
		return false;
	}


public:
	bool remove(ListNode *up, ListNode *delNood)
	{
		if (NULL==up) //root
			mRootNode = delNood->next;
		else
			up->next = delNood->next;
		
		delNood->next = NULL;
		__freeNode(delNood);
		return true;
	}


	PoolList& operator = (const PoolList &other)
	{
		//AssertEx(0, "Not can use this function");
		clear();
		const ListNode *pNood = other.mRootNode;
		while (pNood)
		{
			push_back(pNood->mVal); 
			pNood = pNood->next;
		}
		return *this;
	}

	PoolList(const PoolList &other)
		: mRootNode(NULL)
	{
		//AssertEx(0, "Not can use this function");
		*this = other;
	}

	void swap(PoolList &other)
	{
		ListNode *pNood = mRootNode;
		mRootNode = other.mRootNode;
		other.mRootNode = pNood;

		AutoPtr<NodePool> t = mNodePool;
		mNodePool = other.mNodePool; 
		other.mNodePool = t;
	}

	// 不包括池的空间
	size_t _useSpaceSize(bool bAddPoolSize)
	{		
		if (bAddPoolSize)
			return mNodePool->_useSpaceSize() + sizeof(PoolList);
		return sizeof(PoolList);
	}

protected:
	ListNode				*mRootNode;
	T						msVal;		// 支持多线，不可再定义为静态
	iterator				msIt;

	AutoPtr<NodePool>			mNodePool;
};

//template<typename T>
//T PoolList<T>::msVal;

#endif //_INCLUDE_POOLLIST_H_