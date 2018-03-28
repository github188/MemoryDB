/********************************************************************
	created:	2013/12/24
	created:	24:12:2013   15:19
	filename: 	D:\Work\BaseCommon\BaseCommon\AutoPtr.h
	file path:	D:\Work\BaseCommon\BaseCommon
	file base:	Auto
	file ext:	h
	author:		Wenge Yang
	
	purpose:	内嵌式智能指针对象
				1 内存使用上比较合理
				2 可以与指针混用
	NOTE:		此对象解决不了互相包含死锁状态
				当保存此对象的指针时, 一定注意不要传值生成AutpPtr, 
				否则会自动释放使保存的指针已经释放状态, 症状为 mUseCount = 0
*********************************************************************/

#ifndef _INCLUDE_AUTO_H_
#define _INCLUDE_AUTO_H_

#include "MemBase.h"
#include "Data.h"
#include "TableTool.h"

class AutoBase : public MemBase
{
public:
	// 用来解决死锁问题, 清除包含的所有智能指针对象
	virtual void ClearAuto() {}
	virtual Data _getData(const char *szKey) const { ERROR_LOG("\r\nNo overwrite AutoBase _getData function, or load error data object [%s]\r\n", typeid(this).name()); return Data(); }
	virtual Data _getData(int nKey) const { ERROR_LOG("\r\nNo overwrite AutoBase _getData function, or load error data object [%s]\r\n", typeid(this).name()); return Data(); }

public:
	virtual ~AutoBase(){}

	AutoBase()
		: mUseCount(0)
	{

	}

	virtual void Release() { delete this; }

#if DEVELOP_MODE
	virtual void OnAddUseCount(){}
#endif

public:
	int mUseCount;
};


template <typename T>
class Auto
{
public:
	T&	operator * ()
	{ 
#if DEVELOP_MODE
		AssertEx(m_pBase!=NULL, "object is NULL"); 
#endif
		return *m_pBase; 
	}

	T* operator -> ()
	{
#if DEVELOP_MODE
		AssertEx(m_pBase!=NULL, "object is NULL"); 
#endif
		return m_pBase; 
	}

	const T* operator -> () const
	{
#if DEVELOP_MODE
		AssertEx(m_pBase!=NULL, "object is NULL"); 
#endif
		return m_pBase; 
	}

	const T& operator * () const
	{
#if DEVELOP_MODE
		AssertEx(m_pBase!=NULL, "object is NULL"); 
#endif
		return *m_pBase; 
	}

	operator bool () const { return m_pBase!=NULL; }

	Data operator [] ( const char *szKey ) const
	{ 
		if (m_pBase!=NULL) 
			return getPtr()->_getData(szKey); 
		ERROR_LOG("\r\nERROR: Data object [%s] is NULL, when get >[%s]\r\n", typeid(T).name(), szKey);
		return Data();
	}
	Data operator [] ( const AString &szKey ) const
	{ 
		return (*this)[szKey.c_str()];
	}
	Data operator [] ( int nKey ) const
	{ 
		if (m_pBase!=NULL) 
			return getPtr()->_getData(nKey); 
		ERROR_LOG("\r\nERROR: Data object [%s] is NULL, when get >[%d]\r\n", typeid(T).name(), nKey);
		return Data();
	}

public:
	~Auto()
	{
		setNull();
	}
	Auto()
		: m_pBase(NULL)
	{

	}

	Auto(T *p)
		: m_pBase(p)
	{
		if (m_pBase!=NULL)
		{
			//AssertEx(m_pBase->mUseCount>=0, "USE count less zero");
			m_pBase->mUseCount++;
#if DEVELOP_MODE
			m_pBase->OnAddUseCount();
#endif
		}
	}

	Auto(const Auto &other)
		: m_pBase(NULL)
	{
		if (other.getPtr()!=NULL)
		{
			m_pBase = (T*)(dynamic_cast<const T*>(other.getPtr()));

			if (m_pBase!=NULL)
			{
				m_pBase->mUseCount++;
#if DEVELOP_MODE
				m_pBase->OnAddUseCount();
#endif
			}
		}
	}

	template<typename B>
	Auto(B *p)
		: m_pBase( dynamic_cast<T*>(p))
	{
		if (m_pBase!=NULL)
		{
			//AssertEx(m_pBase->mUseCount>=0, "USE count less zero");
			m_pBase->mUseCount++;
#if DEVELOP_MODE
			m_pBase->OnAddUseCount();
#endif
		}
	}

	template<typename B>
	Auto(const Auto<B> &other)
		: m_pBase(NULL)
	{
		if (other.getPtr())
		{
			m_pBase = (T*)(dynamic_cast<const T*>(other.getPtr()));

			if (m_pBase!=NULL)
			{
				m_pBase->mUseCount++;
#if DEVELOP_MODE
				m_pBase->OnAddUseCount();
#endif
			}
		}
	}

	Auto& operator = (T *p)
	{
		setNull();
		m_pBase = p;
		if (m_pBase!=NULL)
		{
			m_pBase->mUseCount++;
#if DEVELOP_MODE
			m_pBase->OnAddUseCount();
#endif
		}
		return *this;
	}

	Auto<T>& operator = (const Auto<T> &other)
	{
		setNull();
		m_pBase = (T*)dynamic_cast<const T*>(other.getPtr());

		if (m_pBase!=NULL)
		{
			m_pBase->mUseCount++;
#if DEVELOP_MODE
			m_pBase->OnAddUseCount();
#endif
		}
		return *this;
	}

	template<typename B>
	Auto& operator = (B *p)
	{
		setNull();
		m_pBase = dynamic_cast<T*>(p);
		if (m_pBase!=NULL)
		{
			m_pBase->mUseCount++;
#if DEVELOP_MODE
			m_pBase->OnAddUseCount();
#endif
		}
		return *this;
	}

	template<typename B>
	Auto<T>& operator = (const Auto<B> &other)
	{
		setNull();
		m_pBase = (T*)dynamic_cast<const T*>(other.getPtr());

		if (m_pBase!=NULL)
		{
			m_pBase->mUseCount++;
#if DEVELOP_MODE
			m_pBase->OnAddUseCount();
#endif
		}
		return *this;
	}

	//----------------------------------------------------------------------------
	template<typename B>
	inline bool operator==( const Auto<B>& b) const
	{
		if (m_pBase==NULL)
		{
			if (b.getPtr()==NULL)
				return typeid(T)==typeid(B);
			return false;
		}
		return m_pBase==b.getPtr();
	}
	template<typename B>
	bool operator!=( const Auto<B>& b)  const
	{
		return !(*this==b);
	}
	//-------------------------------------------------------------------------*/
	void setNull()
	{
		if (m_pBase!=NULL)
		{
			AssertEx(m_pBase->mUseCount>0, "USE count must more zero");
			if (--(m_pBase->mUseCount)==0)
			{
				freeClass();
				m_pBase = NULL;
			}
		}
		m_pBase = NULL;
	}

	T* getPtr(){ return m_pBase; }
	const T* getPtr() const { return m_pBase; }

	int getUseCount() const{ return m_pBase!=NULL ? m_pBase->mUseCount:0; }

protected:
	void freeClass()
	{
		if (m_pBase!=NULL) {  m_pBase->Release();		m_pBase = NULL; }
	}

public:
	// 只用来方便替换Hand, 填充 Hand::_free()
	void _free() { setNull(); }

protected:
	T		*m_pBase;
};


#endif //_INCLUDE_AUTOPTR_H_