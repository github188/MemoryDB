
/********************************************************************
	created:	2015/11/29
	created:	29:11:2015   18:05
	filename: 	D:\HomeGame\Code\BaseCommon\MemoryDB\Core\CoolDBTable.h
	file path:	D:\HomeGame\Code\BaseCommon\MemoryDB\Core
	file base:	CoolDBTable
	file ext:	h
	author:		Yange WenGe
	
	purpose:	冷却功能的DB表格
			1 定制冷却时长
			2 被冷却的记录清空掉数据部分
			3 使用被冷却的记录时重新加载
*********************************************************************/

#ifndef _INCLUDE_COOLDBTABLE_H_
#define _INCLUDE_COOLDBTABLE_H_

#include "MemoryDBTable.h"
#include "CEvent.h"
using namespace Logic;


class tDBTool;

//-------------------------------------------------------------------------*
class MemoryDB_Export CoolDBTable : public MemoryDBTable
{
	friend class MemoryDB;

public:
	virtual const char* GetTableType(){ return "CoolTable"; }
	virtual void ReadyDataSource(tDataSource *pSource, NiceData &initParam) override;

	virtual void OnLoadTableAllRecord(const char*, const char*, int) override; //DBOperate *op, bool bSu ) override;

	virtual bool LoadFromDB( tDBTool *pDBTool ) override;

	virtual ARecord NewRecord() override;

	virtual ARecord GetRecord(Int64 nIndex) override;
	virtual ARecord GetRecord(float fIndex) override;
	virtual ARecord GetRecord(const char* szIndex) override;

public:
	void CheckCoolRecord();

public:
	CoolDBTable(MemoryDB *pDB);
	~CoolDBTable();

protected:
	//Hand<CoolDBRecord>		mCoolDBRecord;
	AutoEvent				mCheckCoolEvent;
	int						mCoolTime;					// 秒 冷处理时间，默认 DB_COOL_TIME
};

//-------------------------------------------------------------------------*

// 正常存在的DBRecord
class ActiveDBRecord : public MemoryDBRecord
{
	friend class CoolDBTable;
public:
	ActiveDBRecord(AutoTable ownerTable)
		: MemoryDBRecord(ownerTable)
	{

	}

public:
	virtual bool IsNull() const { return _getData()==NULL; }

protected:
	UInt64				mActiveTime;		// 在DB表格中被GetRecond时重置
};
//-------------------------------------------------------------------------*
class TM_CoolDBCheckEvent : public CEvent
{
public:
	TM_CoolDBCheckEvent()
		: mOwnerTable(NULL)
	{

	}

	virtual bool _DoEvent()
	{
		WaitTime(DB_COOL_CHECK_SPACE_TIME);
		return true;
	}

	virtual bool _OnTimeOver() override
	{
		mOwnerTable->CheckCoolRecord();
		WaitTime(DB_COOL_CHECK_SPACE_TIME);

		return true;
	}

public:
	CoolDBTable		*mOwnerTable;
};
//-------------------------------------------------------------------------*
#endif //_INCLUDE_COOLDBTABLE_H_