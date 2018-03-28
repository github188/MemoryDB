
#ifndef _INCLUDE_OBJECT_H_
#define _INCLUDE_OBJECT_H_

#include "Hand.h"
#include "ObjectID.h"
//#include "TableRecord.h"
#include "ServerBaseHead.h"

class tComponent;
class tScene;
class tObjectFactory;

class ServerBase_Dll_Export tObject : public Base<tObject>
{
public:
	virtual ~tObject(){}

public:
    //virtual void SetType(int type) = 0;
	virtual int GetType(void) const;
	virtual bool NeedRemove() const { return false; }
    virtual void OnNodifyRemove(void) {}

    virtual void Die(void) {}
    virtual bool IsDied(void) const { return false; }

    virtual void SetID(ID id) = 0;
    virtual ID GetID(void)  const = 0;

    virtual void Process( void *param ) {}

	virtual void Log(const char *szInfo, ...);

    virtual void SetFactory(tObjectFactory* factory) { mFactory = factory; }
    virtual tObjectFactory* GetFactory() const { return mFactory; }

private:
    tObjectFactory* mFactory;
};


typedef Hand<tObject>		AObject;


#endif //_INCLUDE_OBJECT_H_