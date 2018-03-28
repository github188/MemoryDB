

#ifndef _INCLUDE_IFUNCITON_H_
#define _INCLUDE_IFUNCITON_H_



#include "NiceData.h"
#include "TableTool.h"


typedef NiceData  DATA;


class tFunctor
{
public:
	virtual ~tFunctor() {};
	virtual bool operator()(DATA &paramData) = 0;
};


template<typename T>
class IMemberFunction : public tFunctor
{
public:
	//! Member function slot type.
	typedef bool(T::*FunctionType)(DATA &paramData);

	IMemberFunction(FunctionType func, T* obj) :
	d_function(func),
		d_object(obj)
	{}


	virtual bool operator()(DATA &paramData)
	{
		return (d_object->*d_function)(paramData);
	}


private:
	FunctionType d_function;
	T*    d_object;
};



class  IFunction
{
public:
	/*!
	\brief
	Default constructor.  Creates a SubscriberSlot with no bound slot.
	*/
	IFunction() {  d_functor_impl = 0; };


	~IFunction() { if ( d_functor_impl ) delete d_functor_impl; };


	/*!
	\brief
	Invokes the slot functor that is bound to this Subscriber.  Returns
	whatever the slot returns, unless there is not slot bound when false is
	always returned.
	*/
	bool operator()() const
	{
		return (*d_functor_impl)(mData);
	}


	bool run()
	{
		return (*d_functor_impl)(mData);
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


	template<typename T>
	IFunction( bool(T::*function)(DATA &paramData), T* obj) :
	d_functor_impl(new IMemberFunction<T>(function, obj))
	{}


	template<typename T>
	void  setFunction(bool(T::*function)(const DATA &paramTable, DATA &resultTable), T* obj)
	{
		cleanup();
		d_functor_impl = new MemberFunction<R,T,V,V2>(function, obj);
	}

	DATA& getData(){ return mData; }

	void setFunctionName(const char* szFunctionName){ mFunctionName = szFunctionName; }
	const char* getFunctionName() const { return mFunctionName; }

private:
	//! Points to the internal functor object to which we are bound
	tFunctor		*d_functor_impl;
	EasyString		mFunctionName;

	mutable DATA	mData;
};


typedef Auto<IFunction>  AutoFunction;


class FunctionControl
{
public:
	AutoFunction getFunction(const char* functionName)
	{
		return mFunctionMap.find(MAKE_INDEX_ID(functionName), NULL);
	}


public:
	void appendFunction(const char* functionName, const AutoFunction &function)
	{
		((IFunction*)function.getPtr())->setFunctionName(functionName);
		int key = MAKE_INDEX_ID(functionName);
		mFunctionMap.erase(key);
		mFunctionMap.insert(key, function);
	}
	void appendFunction(const AutoFunction &function)
	{
		int key = MAKE_INDEX_ID(function->getFunctionName());
		mFunctionMap.erase(key);
		mFunctionMap.insert(key, function);
	}

	void removeFunction(const char* functionName)
	{
		int key = MAKE_INDEX_ID(functionName);
		mFunctionMap.erase(key);
	}



protected:
	EasyMap<int, AutoFunction> mFunctionMap;
};





#endif
//-------------------------------------------------------------------------------------------------

