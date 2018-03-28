#ifndef   _INCLUDE_SUBPRO_H_
#define   _INCLUDE_SUBPRO_H_

template<typename R,typename V,typename V2>
class FunctorBase
{
public:
	virtual ~FunctorBase() {};
	virtual R operator()( V * args,V2 * args2) = 0;
};

template<typename R,typename T,typename V,typename V2>
class MemberFunction : public FunctorBase<R,V,V2>
{
public:
	//! Member function slot type.
	typedef R(T::*FunctionType)( V *,V2 *);

	MemberFunction(FunctionType func, T* obj) :
	d_function(func),
		d_object(obj)
	{}

	virtual R operator()( V * args,V2 * args2 )
	{
		return (d_object->*d_function)(args,args2);
	}

private:
	FunctionType d_function;
	T* d_object;
};

template<typename R,typename V,typename V2>
class  SubPro
{
public:
	/*!
	\brief
	Default constructor.  Creates a SubscriberSlot with no bound slot.
	*/
	SubPro() {  d_functor_impl = 0; };

	~SubPro() { if ( d_functor_impl ) delete d_functor_impl; };

	/*!
	\brief
	Invokes the slot functor that is bound to this Subscriber.  Returns
	whatever the slot returns, unless there is not slot bound when false is
	always returned.
	*/
	R operator()( V * args,V2 * args2 ) const
	{
		return (*d_functor_impl)(args,args2);
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
	void cleanup() {  if (d_functor_impl) d_functor_impl; d_functor_impl = 0; };

	template<typename R,typename T,typename V,typename V2>
	SubPro(R (T::*function)( V *,V2 *), T* obj) :
	d_functor_impl(new MemberFunction<R,T,V,V2>(function, obj))
	{}

	template<typename R,typename T,typename V,typename V2>
	void  setSub(R (T::*function)( V *,V2 *), T* obj)
	{
		cleanup();
		d_functor_impl = new MemberFunction<R,T,V,V2>(function, obj);
	}

private:
	//! Points to the internal functor object to which we are bound
	FunctorBase<R,V,V2>* d_functor_impl;
};

//-------------------------------------------------------------------------------------------------
template<typename R,typename V,typename V2>
class FunctorBase_Val
{
public:
	virtual ~FunctorBase_Val() {};
	virtual R operator()( V  args,V2  args2) = 0;
};

template<typename R,typename T,typename V,typename V2>
class MemberFunction_Val : public FunctorBase_Val<R,V,V2>
{
public:
	//! Member function slot type.
	typedef R(T::*FunctionType)( V ,V2 );

	MemberFunction_Val(FunctionType func, T* obj) :
	d_function(func),
		d_object(obj)
	{}

	virtual R operator()( V  args,V2  args2 )
	{
		return (d_object->*d_function)(args,args2);
	}

private:
	FunctionType d_function;
	T* d_object;
};

template<typename R,typename V,typename V2>
class  SubPro_Val
{
public:
	/*!
	\brief
	Default constructor.  Creates a SubscriberSlot with no bound slot.
	*/
	SubPro_Val() {  d_functor_impl = 0; };

	~SubPro_Val() { if ( d_functor_impl ) delete d_functor_impl; };

	/*!
	\brief
	Invokes the slot functor that is bound to this Subscriber.  Returns
	whatever the slot returns, unless there is not slot bound when false is
	always returned.
	*/
	R operator()( V  args,V2  args2 ) const
	{
		return (*d_functor_impl)(args,args2);
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
	void cleanup() {  if (d_functor_impl) d_functor_impl; d_functor_impl = 0; };

	template<typename R,typename T,typename V,typename V2>
	SubPro_Val(R (T::*function)( V ,V2 ), T* obj) :
	d_functor_impl(new MemberFunction_Val<R,T,V,V2>(function, obj))
	{}

	template<typename R,typename T,typename V,typename V2>
	void  setSub(R (T::*function)( V ,V2 ), T* obj)
	{
		cleanup();
		d_functor_impl = new MemberFunction_Val<R,T,V,V2>(function, obj);
	}

private:
	//! Points to the internal functor object to which we are bound
	FunctorBase_Val<R,V,V2>* d_functor_impl;
};

//-------------------------------------------------------------------------------------------------


#endif   //_INCLUDE_SUBPRO_H_