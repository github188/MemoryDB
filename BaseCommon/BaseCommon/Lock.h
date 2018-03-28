/*/-------------------------------------------------------------------------/
临界区线程锁
性能:	实测1千万次加锁与解锁 与 1千万次空或加法计算用时毫秒 172 / 16, 
		约为1个加法计算的10倍, 差不多需要10个CPU指令
//-------------------------------------------------------------------------*/
#ifndef _INCLUDE_CSLOCK_H_
#define _INCLUDE_CSLOCK_H_

#include "BaseCommon.h"


//----------------------------------------------------------------------------

class  BaseCommon_Export CsLock
{
protected:
	void  *mLock;

public:
	friend struct Lock;

	CsLock();
	~CsLock();

	void lock();
	void unlock();
	bool trylock();

	CsLock(const CsLock &other);
	CsLock& operator = (const CsLock &other)
	{
		return *this;
	}
};
//----------------------------------------------------------------------------
struct CsLockTool
{
	CsLock   *cs;
public:
	CsLockTool( CsLock  &lock )
	{
		cs = &lock; 
		cs->lock();
	}

	~CsLockTool()
	{
		cs->unlock();
	}
};
//----------------------------------------------------------------------------
struct CsTryLock
{
	CsLock   *cs;
	bool	bLock;
public:
	CsTryLock( CsLock  &lock )
	{
		cs = &lock; 
		bLock = cs->trylock();
	}
	
	~CsTryLock()
	{
		if (bLock)
			cs->unlock();
	}

	bool locked(){ return bLock; }

	operator bool () { return bLock; }
};
//----------------------------------------------------------------------------
#if __WINDOWS__
class BaseCommon_Export MtLock
{
protected:
	void  *mLock;

public:
	friend struct LockTool;
	MtLock(const CHAR* name);
	MtLock();
	~MtLock();
	void lock();
	void unlock();
	bool trylock();

	MtLock(const MtLock &other);
	MtLock& operator = (const MtLock &other)
	{
		return *this;
	}
};
//----------------------------------------------------------------------------

struct MtLockTool
{
	MtLock   *cs;
public:
	MtLockTool( MtLock  &lock )
	{
		cs = &lock; 
		cs->lock();
	}

	~MtLockTool()
	{
		cs->unlock();
	}
};


//----------------------------------------------------------------------------
struct MtTryLock
{
	MtLock   *cs;
	bool	bLock;
public:
	MtTryLock( MtLock  &lock )
	{
		cs = &lock; 
		bLock = cs->trylock();
	}
	
	~MtTryLock()
	{
		if (bLock)
			cs->unlock();
	}

	bool locked(){ return bLock; }

	operator bool () { return bLock; }
};
//----------------------------------------------------------------------------
#endif //__WINDOWS__


#endif