#ifndef _INCLUDE_LOADDATACALLBACK_H_
#define _INCLUDE_LOADDATACALLBACK_H_


#include "Auto.h"
//-------------------------------------------------------------------------
// 事件回调函数
//-------------------------------------------------------------------------
namespace Logic
{
	class tEvent;
}

#define  ParamType1			const char*
#define  ParmaType2			const char*
#define  ParmaType3			int


class tCallFunctor : public MemBase
{
public:
	virtual ~tCallFunctor() {};
	virtual void operator()(ParamType1 paramData, ParmaType2, ParmaType3) = 0;
};

template<typename T>
class LoadCallBackFunction : public tCallFunctor
{
public:
	//! Member function slot type.
	typedef void(T::*CallBackFunction)(ParamType1, ParmaType2, ParmaType3);

	LoadCallBackFunction(CallBackFunction func, T* obj) 
		: d_function(func),
		d_object(NULL)
	{
		// NOTE: 如果编译错误在这里的话,说明你使用的类没有继承 public Base<T>
		// 目标是保存对象的指针对象, 防止对象被释放后, 错误执行
		d_object = obj->GetSelf();
	}

	virtual void operator()(ParamType1 szKeyID, ParmaType2 p2, ParmaType3 len)
	{
		if (d_object)
			(d_object.getPtr()->*d_function)(szKeyID, p2, len);
	}


private:
	CallBackFunction	d_function;
	Auto<T>				d_object;
};

class LoadDataCallBack
{
public:
	/*!
	\brief
	Default constructor.  Creates a SubscriberSlot with no bound slot.
	*/
	LoadDataCallBack() {  d_functor_impl = 0; };

	LoadDataCallBack(const LoadDataCallBack &other)
	{
		LoadDataCallBack *p = (LoadDataCallBack*)&other;
		d_functor_impl = p->d_functor_impl;
		p->d_functor_impl = NULL;
	}

	~LoadDataCallBack() { if ( d_functor_impl ) delete d_functor_impl; };


	LoadDataCallBack& operator = (const LoadDataCallBack &other)
	{
		cleanup();
		LoadDataCallBack *p = (LoadDataCallBack*)&other;
		d_functor_impl = p->d_functor_impl;
		p->d_functor_impl = NULL;
		return *this;
	}

	bool Nothing(){ return d_functor_impl == NULL; }

	/*!
	\brief
	Invokes the slot functor that is bound to this Subscriber.  Returns
	whatever the slot returns, unless there is not slot bound when false is
	always returned.
	*/
	void operator()(ParamType1 param, ParmaType2 p2, ParmaType3 len) const
	{
		if (d_functor_impl)
			(*d_functor_impl)(param, p2, len);
	}


	void run(ParamType1 param, ParmaType2 pData, ParmaType3 len)
	{
		if (d_functor_impl)
			(*d_functor_impl)(param, pData, len);
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


	template<typename T>
	LoadDataCallBack( void(T::*function)(ParamType1, ParmaType2, ParmaType3), T* obj) :
	d_functor_impl(MEM_NEW LoadCallBackFunction<T>(function, obj))
	{}


	template<typename T>
	void  setFunction(void(T::*function)(ParamType1, ParmaType2, ParmaType3), T* obj)
	{
		cleanup();
		d_functor_impl = MEM_NEW LoadCallBackFunction<T>(function, obj);
	}

private:
	//! Points to the internal functor object to which we are bound
	mutable tCallFunctor		*d_functor_impl;
};

//-------------------------------------------------------------------------

#endif //_INCLUDE_LOADDATACALLBACK_H_