#ifndef _INCLUDE_BASEMEMORYDB_H_
#define _INCLUDE_BASEMEMORYDB_H_

#include "BaseTable.h"
#include "Hand.h"
#include "MemoryDBHead.h"
#include "EventCenter.h"

#include "EasyHash.h"
#include "AnyData.h"
//-------------------------------------------------------------------------
#define DB_SERVER_NET_SAFE_CODE		(-100)
//-------------------------------------------------------------------------
typedef EasyHash<EasyString, ABaseTable>		TableHashMap;

class MemoryDB_Export BaseMemoryDB : public AnyData
{
public:
	BaseMemoryDB()
	{
		mEventCenter = MEM_NEW Logic::EventCenter();
	}
	virtual ~BaseMemoryDB();

public:
	virtual ABaseTable CreateTable(const char *szIndex) = 0;
	ABaseTable GetTable(const char *szIndex);
	ABaseTable GetTable(const AString &strIndex)
	{
		return mTableList.find(strIndex);
	}

	virtual TableHashMap& GetAllTableList(){ return mTableList; }

public:
	virtual bool InitDB(NiceData &initParam){return true;}
	virtual bool Start(const char *szIp, int nPort) = 0;
	virtual void Close();

	virtual void Process()
	{
		mEventCenter->ProcessEvent();
	}

protected:
	AutoEventCenter		mEventCenter;			// 事件中心
	TableHashMap		mTableList;
};


#endif //_INCLUDE_BASEMEMORYDB_H_