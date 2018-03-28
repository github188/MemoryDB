/********************************************************************
	created:	2010/09/06
	created:	6:9:2010   11:13
	filename: 	f:\NewTest\common\Auto.h
	file path:	f:\NewTest\common
	file base:	AutoPtr
	file ext:	h
	author:		杨文鸽
	
	purpose:	精简的通用智能对象
				特点是只用四个字节空间, 等同于一个整形数, 传值效率高点
*********************************************************************/

#ifndef _AUTOPTR_H_
#define _AUTOPTR_H_

#include  "Assertx.h"
#include <typeinfo>
#include "MemBase.h"
#include "BaseCommon.h"
//-----------------------------------------------------------------------------------------

#define USE_DEBUG_HAND_LOG		0

#if USE_DEBUG_HAND_LOG

#	include "AutoDebug.h"
#	define NEW_HANDLER_LOG(T,P)	AutoDebugLog::GetMe().NewLog(typeid(T),P);
#	define FREE_HANDLER_LOG(T,P) AutoDebugLog::GetMe().FreeLog(typeid(T),P);

#	define NEW_USE_LOG	AutoDebugLog::GetMe().NewUse();
#	define FREE_USE_LOG  AutoDebugLog::GetMe().FreeUse();
#else
#	define NEW_HANDLER_LOG(T,P)
#	define FREE_HANDLER_LOG(T,P)
#	define NEW_USE_LOG
#	define FREE_USE_LOG
#endif


#ifndef NULL
#	define NULL	0
#endif
//--------------------------------------------------------------------------------------
extern BaseCommon_Export Bucket	mUseMemPool;
template<typename T>
class UseInfo //: public MemBase
{
	UseInfo():classObj(NULL),count(0){}
public:
	T*	classObj;
	int count;

	void Release()
	{
		FREE_USE_LOG
			mUseMemPool.deletePtr(this);
		//MEM_DELETE this;		
	}
	static UseInfo* New()
	{
		NEW_USE_LOG
			return new (mUseMemPool.newPtr()) UseInfo<T>();
		//return MEM_NEW  UseInfo<T>();		
	}
};
#	define USE_FLAG <T>

#if DEVELOP_MODE
#define CHECK_CLASS_OBJECT	\
	if (NULL==mUse || NULL==mUse->classObj)	\
	{\
		char szInfo[1024];\
		sprintf(szInfo, "[%s] class's hand is null", typeid(T).name());\
		AssertEx(0, szInfo);\
	}
	//AssertEx( mUse&& mUse->classObj, "use or class object is null" ); 
#else
#	define CHECK_CLASS_OBJECT
#endif

//-----------------------------------------------------------------------------------------

template<typename T> 
class  AutoPtr 
{

protected:
	mutable UseInfo USE_FLAG             *mUse;

public:
	inline T&   operator*()			{  CHECK_CLASS_OBJECT; return *((T*)mUse->classObj);	} 
	inline T*   getPtr()		    {  if (  mUse ) return (T*)mUse->classObj; return NULL;				}
	inline const T& operator*()const{  CHECK_CLASS_OBJECT; return *((const T*)mUse->classObj);	}
	inline const T* getPtr() const  {  if (  mUse ) return (const T*)mUse->classObj; return NULL;		}
	inline int  getUseCount() const {  if (  mUse ) return mUse->count; return 0;						}
	inline bool is() const          {  return ( mUse && mUse->classObj );								}
	inline bool isNull() const      {  return !is();													}
	inline operator bool () const   {  return is();														}
	// inline operator T& ()           {  CHECK_CLASS_OBJECT; return *((T*)mUse->classObj);				}
	inline void setNull()			{  _setnull();														}
	inline T*   operator->()		
	{
#pragma warning(push)
#pragma warning(disable:6011)
		CHECK_CLASS_OBJECT; 
		return  (T*)mUse->classObj;		
#pragma warning(pop)
	}
	inline const T*operator->()const
	{  
#pragma warning(push)
#pragma warning(disable:6011)
		CHECK_CLASS_OBJECT; 
		return  (T*)mUse->classObj;		
#pragma warning(pop)
	}
	//----------------------------------------------------------------------------
public:
	~AutoPtr()
	{
		_setnull();
	}	
	
	//----------------------------------------------------------------------------
	AutoPtr() { mUse = NULL; }
	//----------------------------------------------------------------------------

	AutoPtr( const AutoPtr &objhand ) 
	{
		mUse = NULL;
		*this = objhand;
	}
	//----------------------------------------------------------------------------

	AutoPtr( T* obj ) 
		: mUse(NULL)
	{		
		if ( obj )
		{
			_new(obj);
		}
	};


	//template<typename B>
	//AutoPtr( AutoPtr<B> &obj )
	//	: mUse(NULL)
	//{
	//	if (obj && dynamic_cast<T*>(obj.getPtr())!=NULL)
	//	{
	//		mUse =  (UseInfo USE_FLAG*)obj.mUse;
	//		mUse->count++;
	//	}
	//}

	//template<typename B>
	//AutoPtr& operator = ( const AutoPtr<B> &obj ) 
	//{
	//	if (dynamic_cast<const T*>(obj.getPtr())==getPtr())
	//		return *this;

	//	setNull();
	//	if (obj && dynamic_cast<const T*>(obj.getPtr())!=NULL)
	//	{
	//		mUse =  (UseInfo USE_FLAG*)obj.mUse;
	//		mUse->count++;
	//	}
	//	return *this;
	//}

	//----------------------------------------------------------------------------
	// 如果等于空时,直接进行清除当前对象的操作
	//inline AutoPtr & operator=( int zero ) 
	//{ 
	//	_setnull();
	//	return *this;
	//}
	//----------------------------------------------------------------------------

	inline AutoPtr & operator=( T* obj ) 
	{ 
		if ( is() )
		{
			if ( getPtr() == obj ) 
				return *this;
		}

		_setnull();
		if ( obj )
		{ 
			_new(obj);
		}

		return *this;
	}

	//----------------------------------------------------------------------------
	// 实现智能对象之前进行传值
	inline AutoPtr & operator=( const AutoPtr &obj ) 
	{ 
		if (*this==obj)
			return *this;
		_setnull();			
		if ( obj )  
		{
			mUse = obj.mUse;
			++mUse->count;
		}
		else
		{
			mUse = NULL;
		}
		return  *this;
	}

	//----------------------------------------------------------------------------

	void _free()
	{
		if ( is() )
		{
			if ( (mUse->count)==1 )
			{
				_setnull();
			}
			else
			{
				FREE_HANDLER_LOG(T,mUse->classObj)
				FreeClass();
				mUse->classObj = NULL;
				mUse->count = -(mUse->count-1);
				mUse = NULL;
			}
		}
	}
	//----------------------------------------------------------------------------
	void swap(AutoPtr &other)
	{
		UseInfo USE_FLAG *use = mUse;
		mUse = other.mUse;
		other.mUse = use;
	}
	//----------------------------------------------------------------------------
private:
	void _new(T *obj)
	{
		NEW_HANDLER_LOG(T,obj)
		mUse = NewUseInfo();
		mUse->classObj = obj; 
		mUse->count = 1;
	}
	//----------------------------------------------------------------------------
	void _setnull(void)
	{
		if ( is() )
		{
			//当最后一个给定的手柄类变量释放时,才把数据类实体释放内存并清除
			if (--(mUse->count) == 0)      
			{
				FREE_HANDLER_LOG(T,mUse->classObj)
				FreeClass();
				mUse->classObj = NULL;
				FreeUse();
				mUse = NULL;
			}
			//注意: 当保存自己时,释放自己后, 没有在析构中处理时, 会执行到这里
		}
		else      
		{
			// 强行释放后,只用来清除计数器
			if ( mUse )
			{
				if ( ++(mUse->count)==0 )
				{
					FreeUse();
					mUse = NULL;				
				}		
			}
		}		
		mUse = NULL;
	}

protected:
	//----------------------------------------------------------------------------
	inline void FreeClass()
	{
		delete getPtr();
	}
	//----------------------------------------------------------------------------
	inline UseInfo USE_FLAG * NewUseInfo()
	{		
		return UseInfo USE_FLAG::New();
	}
	//----------------------------------------------------------------------------
	inline void FreeUse()
	{		
		mUse->Release();
	}
	//----------------------------------------------------------------------------
public:
	inline bool operator==( const AutoPtr& b) const
	{
		if (mUse==NULL)
		{
			if (b.mUse==NULL)
				return true;
			return false;
		}
		return mUse==b.mUse && getPtr() == b.getPtr();
	}
	//----------------------------------------------------------------------------
	bool operator!=( const AutoPtr& b)  const
	{
		return mUse!=b.mUse || getPtr() != b.getPtr();
	}
	//----------------------------------------------------------------------------
	bool operator < (const AutoPtr& b) const
	{
		return mUse<b.mUse;
	}
};
//-----------------------------------------------------------------------------------------
#ifndef __LINUX__
//特化字符缓存
template<>
void AutoPtr<char>::FreeClass()
{
	delete[] getPtr();
}
#endif



//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
#endif	//_AUTO_H_
