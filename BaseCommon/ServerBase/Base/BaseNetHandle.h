
#ifndef _INCLUDE_BASENETHANDLE_H_
#define _INCLUDE_BASENETHANDLE_H_

#include "BaseEventNet.h"
#include "ServerBaseHead.h"


//----------------------------------------------------------------------------------------------
class BaseThread;
//----------------------------------------------------------------------------------------------
class ServerBase_Dll_Export  BaseNetHandle : public tBaseEventNet
{
public:
	BaseNetHandle(BaseThread *baseThread)
		: mBaseThread(baseThread)
	{
		AssertEx(mBaseThread, "线程对象为空");
		//AssertEx(GetEventCenter(), "事件中心为空");
	}

	virtual ~BaseNetHandle(){}

	virtual bool InitNet();

public:
	BaseThread* GetBaseThread(void){ return mBaseThread; }

	virtual Logic::tEventCenter* GetEventCenter(void) const;

	// 服务器才需要处理这个加入连接功能
	virtual bool OnAddConnect(tNetConnect *pConnect) = 0;



protected:
	BaseThread			*mBaseThread;
};
//----------------------------------------------------------------------------------------------

#endif //_INCLUDE_BASENETHANDLE_H_