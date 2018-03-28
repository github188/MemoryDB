#ifndef _INCLUDE_OBJECTFACTORY_H_
#define _INCLUDE_OBJECTFACTORY_H_

#include "BaseTable.h"

namespace Logic
{
	class tEvent;
}
class BaseThread;
class tObjectFactory : public Base<tObjectFactory>
{
public:
	virtual ~tObjectFactory(){}

	virtual const char* GetTypeName(void) const = 0;
	virtual void SetTypeName(const char* szType) = 0;

	virtual int GetType(void) const = 0;
	virtual void SetType(int nType) = 0;
   
	virtual AutoTable GetConfigTable(BaseThread*){ return AutoTable(); }

public:
	virtual tObject* NewObject(void) = 0;


protected:
	friend class ObjectManager;

};


typedef Hand<tObjectFactory>		AutoObjectFactory;

template<typename T>
class  DefineObjectFactory : public tObjectFactory
{
public:
	virtual const char* GetTypeName(void) const override { return mTypeName.c_str(); }
	virtual void SetTypeName(const char* szType) override { mTypeName = szType; }

	virtual int GetType(void) const override { return mType; }
	virtual void SetType(int nType) override { mType = nType; }

	virtual tObject* NewObject(void) override { return MEM_NEW T(); }

public:
	DefineObjectFactory()
		: mType(0){}

protected:
	AString		mTypeName;
	int				mType;
};


#endif //_INCLUDE_OBJECTFACTORY_H_