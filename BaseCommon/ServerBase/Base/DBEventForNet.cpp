
#include "DBEventForNet.h"
#include "TableManager.h"
#include "Assertx.h"

#include "BaseThread.h"
#include "CEvent.h"

#include "NetHandle.h"
#include "NetPlayer.h"

#include "GameServerEvent.h"

////---------------------------------------------------------------------------------------------------
////---------------------------------------------------------------------------------------------------
//
//class DBServerPlayer : public NetPlayer
//{
//public:
//	virtual void OnNetBegin( void )
//	{
//		TableTool::Log("DB connect succeed.");
//	}
//	virtual void OnNetClose( void )
//	{
//		TableTool::Log("DB connect is close.");
//	}
//
//	virtual void OnNodifyRemove(void) {}
//};
////---------------------------------------------------------------------------------------------------
////---------------------------------------------------------------------------------------------------
//
//class DBServerTool : public ServerNetHandle
//{
//public:
//	DBServerTool(BaseThread *pBaseThread)
//		: ServerNetHandle(pBaseThread)		
//	{
//		mEventCenter = pBaseThread->GetMainEventCenter();
//	}
//
//public:
//
//	virtual Logic::tEventCenter* GetEventCenter(void) { return mEventCenter.getPtr(); }
//
//protected:
//	AutoEventCenter		mEventCenter;
//
//};
//


//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------
//
//bool DBConnectNet::InitNet()
//{
//	if (TryConnectHandle::InitNet())
//	{
//		AssertEx(mBaseThread->GetTableManager(), "DB数据表格为空");
//		return mBaseThread->GetTableManager()->InitReady(this->GetSelf());
//	}
//	return false;
//}
//
//
////---------------------------------------------------------------------------------------------------
//
//DBConnectNet::DBConnectNet( BaseThread *pBaseThread ) 
//	: TryConnectHandle(pBaseThread)
//{
//
//}
//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------




//bool DBServerNetHandle::StartNet( void )
//{
//	if (InitNet())
//	{
//		DataTableManager *p = dynamic_cast<DataTableManager*>(GetBaseThread()->GetTableManager());
//		AssertEx(p, "table manager is not ShareTableManager");
//		return p->InitReady(GetSelf());
//	}
//	return false;
//}



//---------------------------------------------------------------------------------------------------
