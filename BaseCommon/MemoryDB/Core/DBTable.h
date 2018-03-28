/********************************************************************
	created:	2015/04/10
	created:	10:4:2015   13:38
	filename: 	E:\HomeGame\Code\BaseCommon\BaseCommon\DBTable.h
	file path:	E:\HomeGame\Code\BaseCommon\BaseCommon
	file base:	DBTable
	file ext:	h
	author:		Yang Wenge
	
	purpose:	用于同步MYSQL DB表格操作
*********************************************************************/

#ifndef _INCLUDE_DBTABLE_H_
#define _INCLUDE_DBTABLE_H_

#include "BaseCommon.h"
#include "IndexBaseTable.h"
#include "IndexDBRecord.h"
#include "DBTool.h"
#include "MemoryDBHead.h"
//-------------------------------------------------------------------------
class MemoryDB_Export DBTable : public SkipBaseTable
{
	friend class DBRecord;
	friend class MemoryDB;

public:
	DBTable();

public:
	virtual void Init(AutoDBTool dbTool, const char* szDBTableName);
	virtual bool CreateToDB();
	virtual bool LoadDB(bool bLoadAllRecord = false, bool bLoadField =  true);

	virtual bool SaveTable(bool bTryInsert);

	virtual ARecord CreateRecord( float fIndex, bool bReplace );
	virtual ARecord CreateRecord( const char* szIndex, bool bReplace );
	virtual ARecord CreateRecord( Int64 nIndex, bool bReplace );
	virtual ARecord CreateRecord(int nIndex) { return CreateRecord((Int64)nIndex); }

	virtual bool DeleteRecord(ARecord record);

	virtual ARecord GetRecord(Int64 nIndex);
	virtual ARecord GetRecord(float fIndex);
	virtual ARecord GetRecord(const char* szIndex);

	virtual ARecord GetRecord(int nIndex) { return GetRecord((Int64)nIndex); }

public:
	virtual ARecord NewRecord();
	virtual ARecord _LoadDBRecord(const char *szKey);
	AutoDBTool GetDBTool() { return mDBTool; }

protected:
	AutoDBTool	mDBTool;
};
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
class MemoryDB_Export DBRecord : public IndexDBRecord
{
public:
	DBRecord(DBTable *pTable)
		: IndexDBRecord(pTable->GetSelf())
	{

	}

public:
	virtual void Update();
};
//-------------------------------------------------------------------------
#endif