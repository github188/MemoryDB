#ifndef _INCLUDE_NETPLAYER_H_
#define _INCLUDE_NETPLAYER_H_

#include "AutoPtr.h"

#include "ServerBaseHead.h"
#include "Object.h"
#include "NetConnect.h"

class ServerNet;
class tNetConnect;
class tNetHandle;

namespace Logic
{
	class tEvent;
	class tEventCenter;
}

class tPlayer : public tObject
{
public:
	tPlayer()
	: mCode(0)
	, mDBKey(0)
	, mName("")
	{

	}
	virtual ~tPlayer(){}

public:
	virtual void SetName(const char* szName){ mName = szName;}
	virtual const char* GetName()  const  { return mName.c_str(); }


public:
	virtual void SetID(ID id) { mCode = id; }
	virtual ID GetID(void) const { return (ID)mCode; }
	virtual void SetKey(KEY dbKey) { mDBKey = dbKey; }
	virtual KEY GetKey(void) const { return mDBKey; }
	virtual void SetThreadID(TD threadID) { mCode = threadID; }
	virtual TD GetThreadID(void) const { return (TD)mCode; }

	virtual void SetCode(CODE cCode) { mCode = cCode; }
	virtual CODE GetCode(void) const { return mCode; }

	virtual void SetMasterCode(CODE id) {}
	virtual CODE GetMasterCode(void) { return 0; }

	virtual tComponent* GetComponent(const char* szIndex) { return NULL; }

public:
	virtual tNetHandle* GetNetHandle(void) = 0;
	virtual void SetNetHandle(tNetHandle *pTool) = 0;

	virtual bool SendEvent(Logic::tEvent *pEvent) = 0;

	virtual void BindConnect(tNetConnect *pConnect) = 0;

public:
	virtual void OnNetBegin( void ) = 0;
	virtual void OnNetClose( void ) = 0;

protected:
	CODE		mCode;
	KEY			mDBKey;
	AString		mName;

};

class tScene;
// 这是服务器上的玩家对象的基类
class ServerBase_Dll_Export NetPlayer : public tPlayer
{
public:
	NetPlayer()
		 : mConnect(NULL) 
	{

	}
public:

	virtual tNetHandle* GetNetHandle(void);
	virtual void SetNetHandle(tNetHandle *pTool){}

	virtual tNetConnect* GetConnect(void){ return mConnect.getPtr(); }
	virtual void BindConnect(tNetConnect *pConnect);

	virtual bool SendEvent(Logic::tEvent *pEvent);

public:
	virtual void Process( void *param );

	virtual void InitReadyRecord(AutoRecord hRecord) { }

public:
	virtual void OnEnterScene(tScene *pScene ) {}
	virtual void OnLeaveScene(tScene *pScene) {}

protected:
	HandConnect		mConnect;
};


#endif //_INCLUDE_NETPLAYER_H_