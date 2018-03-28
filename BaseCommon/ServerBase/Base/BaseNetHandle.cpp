
#include "BaseNetHandle.h"
#include "Assertx.h"

#include "BaseThread.h"

#include "NetConnect.h"


#include <exception>


//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

Logic::tEventCenter* BaseNetHandle::GetEventCenter( void ) const
{
	if (mBaseThread)
		return mBaseThread->GetMainEventCenter().getPtr();
	return NULL;
}

//-------------------------------------------------------------------------------------------------------------------

bool  BaseNetHandle::InitNet(void)
{
	AssertEx(GetEventCenter(), "�¼�����Ϊ��");
	RegisterEvent(GetEventCenter());

	// ���ⲿ�����ڴ���, ���������ż�� ?
	//if (mBaseThread)
	//	mBaseThread->AppendNet(this);

	INFO_LOG("�ɹ���ʼ����, Э��:[TCP], IP [%s], �˿�[%d]",	
		GetIp(),
		GetPort()
		);

	return true;
}

//----------------------------------------------------------------------------------------------



