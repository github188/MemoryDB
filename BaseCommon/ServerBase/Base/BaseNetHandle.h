
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
		AssertEx(mBaseThread, "�̶߳���Ϊ��");
		//AssertEx(GetEventCenter(), "�¼�����Ϊ��");
	}

	virtual ~BaseNetHandle(){}

	virtual bool InitNet();

public:
	BaseThread* GetBaseThread(void){ return mBaseThread; }

	virtual Logic::tEventCenter* GetEventCenter(void) const;

	// ����������Ҫ��������������ӹ���
	virtual bool OnAddConnect(tNetConnect *pConnect) = 0;



protected:
	BaseThread			*mBaseThread;
};
//----------------------------------------------------------------------------------------------

#endif //_INCLUDE_BASENETHANDLE_H_