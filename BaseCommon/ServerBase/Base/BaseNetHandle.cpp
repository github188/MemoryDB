
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
	AssertEx(GetEventCenter(), "事件中心为空");
	RegisterEvent(GetEventCenter());

	// 在外部代码内处理, 不能在这儿偶合 ?
	//if (mBaseThread)
	//	mBaseThread->AppendNet(this);

	INFO_LOG("成功初始网络, 协议:[TCP], IP [%s], 端口[%d]",	
		GetIp(),
		GetPort()
		);

	return true;
}

//----------------------------------------------------------------------------------------------



