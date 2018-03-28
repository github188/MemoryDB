
#ifndef _INCLUDE_DBCALLBACK_H_
#define _INCLUDE_DBCALLBACK_H_

#include "Hand.h"

//-------------------------------------------------------------------------
// DB操作回调函数
//-------------------------------------------------------------------------
#define  ParamType   DBOperate

class DBOperate;


class BaseFunctor : public MemBase
{
public:
	virtual ~BaseFunctor() {};
	virtual void operator()(ParamType *paramData, bool bSucceed) = 0;
	virtual BaseFunctor* Clone() { return NULL; }
};

template<typename T>
class DBCallBackFunction : public BaseFunctor
{
public:
	//! Member function slot type.
	typedef void(T::*CallBackFunction)(ParamType *, bool);

	DBCallBackFunction(CallBackFunction func, T* obj) 
		: d_function(func),
		d_object(NULL)
	{
		// NOTE: 如果编译错误在这里的话,说明你使用的类没有继承 public Base<T>
		// 目标是保存对象的指针对象, 防止对象被释放后, 错误执行
		d_object = obj->GetSelf();
	}


	virtual void operator()(ParamType *pDBOperate, bool bSucceed)
	{
		if (d_object)
			(d_object.getPtr()->*d_function)(pDBOperate, bSucceed);
	}

	virtual BaseFunctor* Clone()
	{
		DBCallBackFunction *p = MEM_NEW DBCallBackFunction(d_function, d_object.getPtr());
		return p;
	}

private:
	CallBackFunction d_function;
	Hand<T>    d_object;
};

class  DBCallBack
{
public:
	/*!
	\brief
	Default constructor.  Creates a SubscriberSlot with no bound slot.
	*/
	DBCallBack() {  d_functor_impl = 0; };

	DBCallBack(const DBCallBack &other)
	{
		DBCallBack *p = (DBCallBack*)&other;
		d_functor_impl = p->d_functor_impl;
		p->d_functor_impl = NULL;
	}

	~DBCallBack() { if ( d_functor_impl ) delete d_functor_impl; };


	DBCallBack& operator = (const DBCallBack &other)
	{
		cleanup();
		DBCallBack *p = (DBCallBack*)&other;
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
	void operator()(ParamType *param, bool bSucceed) const
	{
		if (d_functor_impl)
			(*d_functor_impl)(param, bSucceed);
	}


	void run(ParamType *param, bool bSucceed)
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

	void CopyFrom(DBCallBack &other)
	{
		cleanup();
		d_functor_impl = other.d_functor_impl->Clone();
	}

	template<typename T>
	DBCallBack( void(T::*function)(ParamType*, bool bSucceed), T* obj) :
	d_functor_impl(MEM_NEW DBCallBackFunction<T>(function, obj))
	{}


	template<typename T>
	void  setFunction(void(T::*function)(ParamType*, bool bSucceed), T* obj)
	{
		cleanup();
		d_functor_impl = MEM_NEW DBCallBackFunction<T>(function, obj);
	}

private:
	//! Points to the internal functor object to which we are bound
	mutable BaseFunctor		*d_functor_impl;
};


#endif //_INCLUDE_DBCALLBACK_H_