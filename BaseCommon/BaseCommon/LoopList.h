#ifndef _INCLUDE_LOOPLIST_H_
#define _INCLUDE_LOOPLIST_H_
/*/-------------------------------------------------------------------------/
双向环形列表, 用于异步顺序遍历(如LUA对象清理检查)
1 可向前向后移动
2 快速在最后插入
//-------------------------------------------------------------------------*/
template<typename T>
class LoopList : public MemBase
{
	//friend struct iterator;
public:
	struct Node : public MemBase
	{
		Node	*mpPreviou;
		Node	*mpNext;
		T		mValue;

		Node()
			: mpPreviou(NULL)
			, mpNext(NULL)
		{

		}
	};

	struct iterator : public MemBase
	{
		Node *mpCurrentNode;
		LoopList *mOwnerList;

		T& operator * () const
		{ 
			if (mpCurrentNode!=NULL)
				return mpCurrentNode->mValue;
			//static T t;
			return mOwnerList->msStatic;
		}

		operator bool () { return mpCurrentNode!=NULL; }

		bool reset(){ mpCurrentNode = mOwnerList->mpRootNode; return mpCurrentNode!=NULL; }
		void erase()
		{
			Node *p = mpCurrentNode;			
			if (p!=NULL)
			{
				++(*this);
				if (p==mpCurrentNode)
					mpCurrentNode = NULL;
				mOwnerList->_remove(p);
			}
		}

		T& get() const 
		{
			if (mpCurrentNode!=NULL)
				return mpCurrentNode->mValue;

			//static T t;
			return mOwnerList->msStatic;
		}
		bool operator ++ ()
		{
			if (mpCurrentNode!=NULL)
			{
				mpCurrentNode = mpCurrentNode->mpNext;

				if ( mpCurrentNode==mOwnerList->mpRootNode )
				{
					mpCurrentNode= NULL;			
				}
				else
					return true;
			}
			return false;
		}

		bool operator ++ (int)
		{
			return ++(*this);
		}

		bool operator -- ()
		{
			if (mpCurrentNode!=NULL)
			{
				mpCurrentNode = mpCurrentNode->mpPreviou;

				if ( mpCurrentNode==mOwnerList->mpRootNode )
				{
					mpCurrentNode= NULL;			
				}
				else
					return true;
			}
			return false;
		}

		bool operator -- (int)
		{
			return --(*this);
		}

		iterator()
			: mpCurrentNode(NULL)
			, mOwnerList(NULL)
		{

		}

		iterator(LoopList *pList)
			: mOwnerList(pList)
			, mpCurrentNode(pList->mpRootNode)
		{

		}

		iterator(LoopList *pList, Node *pNode)
			: mOwnerList(pList)
			, mpCurrentNode(pNode)
		{

		}
	};

public:
	Node* insert(const T &value)
	{
		Node *pNew = _NewNode();
		pNew->mValue = value;
		_insert(pNew);
		return pNew;
	}

	iterator begin()
	{
		return  iterator(this);
	}

	int size() const { return mSize; }
	bool empty(){ return mpRootNode ==NULL; }

	iterator findAt(int pos)
	{
		if (pos<0)
			return iterator();

		int i =0;
		iterator it = begin();
		for (; it; ++it)
		{
			if (++i>pos)
				return it;
		}
		return iterator();
	}

	iterator find(const T &target)
	{
		Node *p = _find(target);
		return iterator(this, p);
	}

	bool remove(const T &target)
	{
		Node *p = _find(target);
		if (p!=NULL)
		{
			_remove(p);
			return true;
		}
		return false;
	}

	void _remove(Node *pNode)
	{
		if (pNode==mpRootNode)
		{
			if (pNode->mpNext==pNode)
				mpRootNode = NULL;
			else
				mpRootNode = pNode->mpNext;
		}
		pNode->mpPreviou->mpNext = pNode->mpNext;
		pNode->mpNext->mpPreviou = pNode->mpPreviou;
		_FreeNode(pNode);
		--mSize;
	}

	Node* _find(const T &target)
	{
		if (mpRootNode==NULL)
			return NULL;

		Node *p = mpRootNode;		
		while (true)
		{
			if (p->mValue==target)
				return p;
			
			if (p->mpNext!=mpRootNode)
				break;

			p = p->mpNext;
		}
		return NULL;
	}

	void clear()
	{
		while (mpRootNode!=NULL)
		{
			_remove(mpRootNode);
		}
	}

public:
	LoopList()
		: mpRootNode(NULL)
		, mSize(0)
	{

	}

	virtual ~LoopList()
	{
		clear();
		mSize = 0;
	}

	LoopList(const LoopList &other)
	{
		AssertNote(0, "不支持深拷贝传值");
	}

	LoopList& operator = (const LoopList &other)
	{
		AssertNote(0, "不支持深拷贝传值");
		return *this;
	}

protected:
	virtual Node*  _NewNode(){ return MEM_NEW Node(); }
	virtual void _FreeNode(Node *pNode){ delete pNode; }

public:
	void _insert(Node *pNew)
	{
		if (mpRootNode==NULL)
		{
			mpRootNode = pNew;
			mpRootNode->mpPreviou = mpRootNode;
			mpRootNode->mpNext = mpRootNode; 
		}
		else
		{
			// NOTE: insert current list last, loop begin from mpRootNode
			mpRootNode->mpPreviou->mpNext = pNew;
			pNew->mpPreviou = mpRootNode->mpPreviou;

			pNew->mpNext = mpRootNode;
			mpRootNode->mpPreviou = pNew;
		}
		++mSize;		
	}

	Node* __getRoot(){ return mpRootNode; }

protected:
	Node	*mpRootNode;
	int		mSize;

	T		msStatic;	// 支持多线
};


//-------------------------------------------------------------------------
template <typename T>
class LoopPool : public LoopList<T>
{
public:
	LoopPool(int freeMaxCount)
		: mFreeMaxCount(freeMaxCount)
		, mFreeNodeRoot(NULL)
	{

	}

	~LoopPool()
	{
		while (mFreeNodeRoot!=NULL)
		{
			Node *p = mFreeNodeRoot;
			mFreeNodeRoot = mFreeNodeRoot->mpNext;
			delete p;
		}
	}

	LoopPool(const LoopPool &other)
	{
		AssertNote(0, "不支持深拷贝传值");
	}

	LoopPool& operator = (const LoopPool &other)
	{
		AssertNote(0, "不支持深拷贝传值");
		return *this;
	}

public:
	virtual Node*  _NewNode()
	{ 
		if (mFreeNodeRoot==NULL)
			return MEM_NEW Node(); 
		else
		{
			Node *p = mFreeNodeRoot;
			mFreeNodeRoot = mFreeNodeRoot->mpNext;
			return p;
		}
	}
	virtual void _FreeNode(Node *pNode)
	{ 
		//if (mFreeMaxCount>0 && mFreeNodeList.size()>=mFreeMaxCount)
		//	delete pNode; 
		//else
		{
			pNode->mpPreviou = NULL;
			pNode->mValue = T();
			if (mFreeNodeRoot==NULL)
			{ 
				pNode->mpNext = NULL;
				mFreeNodeRoot = pNode;
			}
			else
			{
				pNode->mpNext = mFreeNodeRoot;
				mFreeNodeRoot = pNode;
			}
		}
	}

protected:
	Node	*mFreeNodeRoot;
	int	mFreeMaxCount;
};

//-------------------------------------------------------------------------

#endif //_INCLUDE_LOOPLIST_H_