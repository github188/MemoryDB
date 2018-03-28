#ifndef _INCLUDE_DBRESULTCALLBACK_H_
#define _INCLUDE_DBRESULTCALLBACK_H_

#include "Hand.h"
#include "NiceData.h"
//-------------------------------------------------------------------------
// DB操作回调函数
//-------------------------------------------------------------------------
#define  DBResultType   AutoNice

class DBOperate;


class DBBaseFunctor : public MemBase
{
public:
	virtual ~DBBaseFunctor() {};
	virtual void operator()(DBResultType paramData, bool bSucceed) = 0;
	virtual DBBaseFunctor* Clone() { return NULL; }
};

template<typename T>
class DBResultFunction : public DBBaseFunctor
{
public:
	//! Member function slot type.
	typedef void(T::*CallBackFunction)(DBResultType , bool);

	DBResultFunction(CallBackFunction func, T* obj) 
		: d_function(func),
		d_object(NULL)
	{
		// NOTE: 如果编译错误在这里的话,说明你使用的类没有继承 public Base<T>
		// 目标是保存对象的指针对象, 防止对象被释放后, 错误执行
		d_object = obj->GetSelf();
	}


	virtual void operator()(DBResultType pDBOperate, bool bSucceed)
	{
		if (d_object)
			(d_object.getPtr()->*d_function)(pDBOperate, bSucceed);
	}

	virtual DBBaseFunctor* Clone()
	{
		DBResultFunction *p = MEM_NEW DBResultFunction(d_function, d_object.getPtr());
		return p;
	}

private:
	CallBackFunction d_function;
	Hand<T>    d_object;
};

class DBResultCallBack
{
public:
	/*!
	\brief
	Default constructor.  Creates a SubscriberSlot with no bound slot.
	*/
	DBResultCallBack() {  d_functor_impl = 0; };

	DBResultCallBack(const DBResultCallBack &other)
	{
		DBResultCallBack *p = (DBResultCallBack*)&other;
		d_functor_impl = p->d_functor_impl;
		p->d_functor_impl = NULL;
	}

	~DBResultCallBack() { if ( d_functor_impl ) delete d_functor_impl; };


	DBResultCallBack& operator = (const DBResultCallBack &other)
	{
		cleanup();
		DBResultCallBack *p = (DBResultCallBack*)&other;
		d_functor_impl = p->d_functor_impl;
		p->d_functor_impl = NULL;
		return *this;
	}

	bool empty(){ return d_functor_impl==0; }
	/*!
	\brief
	Invokes the slot functor that is bound to this Subscriber.  Returns
	whatever the slot returns, unless there is not slot bound when false is
	always returned.
	*/
	void operator()(DBResultType param, bool bSucceed) const
	{
		if (d_functor_impl)
			(*d_functor_impl)(param, bSucceed);
	}


	void run(DBResultType param, bool bSucceed)
	{
		if (d_functor_impl)
			(*d_functor_impl)(param, bSucceed);
	}
	/*!
	\brief
	Returns whether the SubscriberSlot is internally connected (bound).
	*/
	bool connected() const
	{
		return d_functor_impl != 0;
	}


	/*!
	\brief
	Disconnects the slot internally and performs any required cleanup
	operations.
	*/
	void cleanup() 
	{  
		if (d_functor_impl)
			delete d_functor_impl; 

		d_functor_impl = 0; 
	};

	void CopyFrom(DBResultCallBack &other)
	{
		cleanup();
		d_functor_impl = other.d_functor_impl->Clone();
	}

	template<typename T>
	DBResultCallBack( void(T::*function)(DBResultType, bool bSucceed), T* obj) :
	d_functor_impl(MEM_NEW DBResultFunction<T>(function, obj))
	{}


	template<typename T>
	void  setFunction(void(T::*function)(DBResultType, bool bSucceed), T* obj)
	{
		cleanup();
		d_functor_impl = MEM_NEW DBResultFunction<T>(function, obj);
	}

private:
	//! Points to the internal functor object to which we are bound
	mutable DBBaseFunctor		*d_functor_impl;
};



#endif //_INCLUDE_DBRESULTCALLBACK_H_