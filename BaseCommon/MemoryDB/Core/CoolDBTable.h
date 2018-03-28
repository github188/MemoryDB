
/********************************************************************
	created:	2015/11/29
	created:	29:11:2015   18:05
	filename: 	D:\HomeGame\Code\BaseCommon\MemoryDB\Core\CoolDBTable.h
	file path:	D:\HomeGame\Code\BaseCommon\MemoryDB\Core
	file base:	CoolDBTable
	file ext:	h
	author:		Yange WenGe
	
	purpose:	��ȴ���ܵ�DB���
			1 ������ȴʱ��
			2 ����ȴ�ļ�¼��յ����ݲ���
			3 ʹ�ñ���ȴ�ļ�¼ʱ���¼���
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
	int						mCoolTime;					// �� �䴦��ʱ�䣬Ĭ�� DB_COOL_TIME
};

//-------------------------------------------------------------------------*

// �������ڵ�DBRecord
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
	UInt64				mActiveTime;		// ��DB����б�GetRecondʱ����
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