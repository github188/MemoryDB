#ifndef _INCLUDE_MEMORYDBOPERATE_H_
#define _INCLUDE_MEMORYDBOPERATE_H_

#include "Operate.h"
//-------------------------------------------------------------------------
// 创建表格
class OP_CreateTable : public tOperate
{
public:
	virtual bool Execute(BaseMemoryDB  *pDB, Logic::tEvent *ownerEvent)
	{
		if (mTableIndex=="")
		{
			mResultType = eTableIndexNull
			return false;
		}		

		ABaseTable t = GetDB(pDB)->NewCreateDBTable(mTableIndex.c_str(), mRecordKey.c_str(), MAKE_INDEX_ID(mRecordKey.c_str()), mErrorInfo );
		
		if (!t)
			mErrorType = eRecordCreateFail;

		return true;
	}

};
//-------------------------------------------------------------------------
// 请求表格信息
class OP_LoadTableInfo : public tOperate
{
public:
	virtual bool Execute(BaseMemoryDB  *pDB, Logic::tEvent *ownerEvent)
	{
		int checkCode = respEvent->get("FIELD_CODE");
		AString fieldData = respEvent->get("FIELD_DATA").c_str();
		if (fieldData!="")
		{
			ABaseTable table = GetDB()->CreateTable(mTableIndex.c_str());
			table->GetField()->FullFromString(fieldData);
			table->SetMainIndex(0, false);
			table->SaveCSV("__ok.csv");
		}
		else
		{
			TABLE_LOG("[%s]表格不存在", mTableIndex.c_str());
		}
	}
};
//-------------------------------------------------------------------------

#endif //_INCLUDE_MEMORYDBOPERATE_H_