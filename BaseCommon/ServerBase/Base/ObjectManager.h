
#ifndef _INCLUDE_OBJECTMANAGER_H_
#define _INCLUDE_OBJECTMANAGER_H_

#include "Object.h"
#include "ObjectFactory.h"
#include "Array.h"
#include "ArrayIndex.h"
#include "ServerBaseHead.h"
#include "Hand.h"

#define REGISTER_OBJECT(objMgr, objType, objClass)  \
{ tObjectFactory *p = MEM_NEW DefineObjectFactory<objClass>(); p->SetTypeName(#objType); objMgr.RegisterObject(objType, p); }

#define INIT_POOL_COUNT		100

#define OBJECT_MAX_COUNT	30000		// 注意此值与客户端约定的战斗时产生的ID要统一 30001 战斗产生 40000 防守产生 

#define OBJECT_FIRST_ID		1
//#define INVALID_ID			(-1)

class ServerBase_Dll_Export ObjectManager : public MemBase
{
public:
	ObjectManager();

	virtual ~ObjectManager();

	AObject operator [] (size_t pos){ if (pos<mCount) return mObjectList[pos]; return AObject(); }

	size_t Count(void)
	{
#if USE_CHANGE_ID
		return mCount; 
#else
		return mObjectList.size();
#endif
	}

	bool AllotCount(int needCount);

public:
	static void Log(const char *szInfo, ...);

public:
	AObject FindObject(ID iIndex);
	// create new object and append manager. 
	// Node: already append manager last create.
	AObject CreateObject(const char *szType, bool bUseFreeID, ObjectManager *pFactMgr);
	AObject CreateObject(int nType, bool bUseFreeID, ObjectManager *pFactMgr);
	AObject CreateObjectSpecifiedID(int nType, ID specifiedID, ObjectManager *pFactMgr);


	ID AppendObject(AObject hObj, bool bUseFreeID);

	bool RemoveObject(ID iIndex, bool bFree = true);
	bool RemoveObject(AObject hObj, bool bFree = true);
	void RemoveAll(bool bFreeObject);

	AutoObjectFactory GetObjectFactory(const char *szType);
	AutoObjectFactory GetObjectFactory(int nType);

public:
	virtual int Process(void *param, bool bInitCount);

public:
	void RegisterObject(const char *szType, AutoObjectFactory hFact);
	void RegisterObject(int nType, AutoObjectFactory hFact);
	void RegisterObject(AutoObjectFactory hFact){ RegisterObject(hFact->GetType(), hFact); }

	void RemoveAllFactory(void);

protected:
	Array<AObject>						mObjectList;
	size_t								mCount;

protected:
	typedef ArrayMap<AutoObjectFactory>			ObjectFactoryMap;
	ObjectFactoryMap							mObjectFactory;
	EasyHash<AString, AutoObjectFactory>		mObjectFactoryByName;
};


#endif