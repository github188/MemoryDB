#ifndef _FACTORYTEMPLATE_H_
#define _FACTORYTEMPLATE_H_

#include "Auto.h"
#include "AutoString.h"

#include "TableTool.h"
#include "EasyMap.h"

template<typename B>
class BaseFactory
{
public:
	virtual	~BaseFactory(){};
	BaseFactory(){}

	virtual const EasyString & GetType() = 0;
	virtual B* GetNewObj() = 0;
};

template<typename C, typename B>
class ObjFactory : public BaseFactory<B>
{
public:
	ObjFactory(const char* strType)
		: mType(strType)
	{
	}
	virtual const EasyString & GetType()
	{
		return mType;
	}

	virtual B * GetNewObj()
	{
		return MEM_NEW C();
	}

protected:
	EasyString		mType;
};

template<typename B>
class AutoFactoryManager
{
public:
	typedef Auto< BaseFactory<B> >				AutoFactory;
	typedef EasyMap<int, AutoFactory>			FactoryMap;

	void AddFactory( AutoFactory factory )
	{
		mFactoryMap.insert(MAKE_INDEX_ID(factory->GetType()), factory);
	}

	AutoFactory GetFactory( const char* type )
	{
		return mFactoryMap.find(MAKE_INDEX_ID(type), NULL);
	}

	bool RemoveFactory( const char* type )
	{
		return mFactoryMap.erase(MAKE_INDEX_ID(type))>0;
	}

	void Init()
	{
		mFactoryMap.clear();
	}

	B*  CreateNewObject(const char* type)
	{
		return CreateObject(type);
	}

	virtual B*	CreateObject(const char* type)
	{
		AutoFactory fact = GetFactory(type);
		if (fact)
		{
			return fact->GetNewObj();
		}
		else
		{
			AString temp;
			temp = type;
			temp += "->工厂不存在!";
#ifdef __WINDOWS
			throw std::exception(temp.c_str());
#else
            throw (1);
#endif
		}
		return NULL;
	}

protected:
	FactoryMap		mFactoryMap;
};


#endif