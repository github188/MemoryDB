/********************************************************************
	created:	2014/07/23
	created:	23:7:2014   2:10
	filename: 	H:\RemoteGame\MemorySQL\Code\DBOperate.h
	file path:	H:\RemoteGame\MemorySQL\Code
	file base:	DBOperate
	file ext:	h
	author:		Yang Wenge
	
	purpose:	DB 操作
				1 创建表格
				2 插入记录
				3 查找记录				
				5 删除记录
				6 修改表格
				7 删除表格
	NOTE:		对于记录, 只传字段验证CODE 由表格索引哈希整理高32位及字段字符哈希的一个整数
*********************************************************************/
#ifndef _INCLUDE_DBSERVEROPERATE_H_
#define _INCLUDE_DBSERVEROPERATE_H_

#include "ClientEvent.h"
#include "BaseTable.h"
#include "MemoryDBHead.h"
#include "DataBuffer.h"
#include "TableFieldInfo.h"
#include "TableUpdateLoader.h"
#include "DBCallBack.h"


class MemoryDB;
class MemoryDBNode;
//-------------------------------------------------------------------------
class MemoryDB_Export DB_Responce : public Logic::tClientEvent
{
public:
	virtual bool _Serialize(DataStream *destData)
	{
		AssertEx(0, "No use");
		//destData->writeString(mTableIndex);
		return false;
	}
	virtual bool _Restore(DataStream *scrData)
	{
		if (!tClientEvent::_Restore(scrData))
			return false;

		scrData->readString(mTableIndex);
		scrData->readString(mRecordKey);		

		return true;
	}

	virtual void InitData()
	{
		Logic::tClientEvent::InitData();
		mResultType = 0;
		mResultTable.setNull();
		mResultRecord.setNull();
		mCallBack.cleanup();
	}

public:
	MemoryDB* GetDB();
	MemoryDBNode* GetDBNode();

	static MemoryDB* _GetDB(HandConnect &conn);

public:
	void ReplyError(const char *szInfo, ...);

public:
	AString			mTableIndex;
	AString			mRecordKey;

public:
	AString			mErrorInfo;
	DBCallBack		mCallBack;

public:
	int				mResultType;
	ABaseTable		mResultTable;
	ARecord			mResultRecord;
};
//-------------------------------------------------------------------------
class DB_RequestTableDistribution_S : public DB_Responce
{
public:
	virtual bool _DoEvent();
};
//-------------------------------------------------------------------------
class DB_CreateTable : public DB_Responce
{
public:
	virtual bool _Restore(DataStream *scrData);

public:
	virtual bool _DoEvent();
};
//-------------------------------------------------------------------------
class DB_LoadTableField_S : public DB_Responce
{
public:
	virtual bool _DoEvent();

	void SetRespData(AutoEvent &hResp);

public:
	DB_LoadTableField_S()
	{
		mFieldData = MEM_NEW DataBuffer(512);
	}

protected:
	AutoData	mFieldData;
};
//-------------------------------------------------------------------------
// 是否替换, 如果不存在, 则插入, 如果存在, 是否替换
//class DB_SaveRecord_S : public DB_Responce
//{
//public:
//	virtual bool _Restore(DataStream *scrData);
//
//	virtual bool _DoEvent();
//
//	virtual bool _OnEvent(AutoEvent &hEvent);
//	virtual bool _OnEvent(void *pData, const type_info &dataType);
//
//protected:
//	DataBuffer mRecordData;
//};

//-------------------------------------------------------------------------
// 更新记录数据，只修改改变的部分
//class DB_UpdateRecord_S : public DB_Responce
//{
//public:
//	virtual bool _DoEvent();
//
//	virtual bool _Restore(DataStream *scrData);
//
//	bool _OnEvent( AutoEvent &hEvent );
//
//	virtual void InitData() override
//	{
//		DB_Responce::InitData();
//		mCheckCode = 0;
//		mUpdateData.clear(false);
//	}
//
//protected:
//	DataBuffer mUpdateData;
//	int		mCheckCode;
//};
//-------------------------------------------------------------------------
// 调取记录, 如果未提供字段验证 可以提供字段名列表, 进行部分检索
class DB_FindRecord_S : public DB_Responce
{
public:
	virtual bool _DoEvent();

	virtual bool _OnEvent(void *pData, const type_info &dataType);

	virtual bool _OnEvent(AutoEvent &evt);
};

//-------------------------------------------------------------------------
// 删除记录
class DB_DeleteRecord_S : public DB_Responce
{
public:
	virtual bool _DoEvent();

	virtual bool _OnEvent(void *pData, const type_info &dataType);

	bool _OnEvent( AutoEvent &evt );

};

//-------------------------------------------------------------------------
// 修改表格结构, 需要修改当前存在的记录
class DB_ChangeTableField : public DB_Responce
{

};
//-------------------------------------------------------------------------
// 删除表格
class DB_DeleteTable_S : public DB_Responce
{

};
//-------------------------------------------------------------------------
// 调取DB中所有DB表格字段信息
class DB_LoadAllTableField_S : public DB_Responce
{
public:
	virtual bool _DoEvent();


};

//-------------------------------------------------------------------------
// 执行自定义DB操作
class DB_RunDBOperate_S : public DB_Responce
{
public:
	virtual bool _DoEvent();

	virtual bool _OnEvent(AutoEvent &hEvent)
	{
		AutoNice resultData;
		hEvent->get("RESULT_DATA", resultData);
		GetResponseEvent()->set("RESULT_DATA", resultData);
		GetResponseEvent()->set("RESULT",  (int)hEvent->get("RESULT"));
		GetResponseEvent()->set("ERROR", hEvent->get("ERROR").string());
		Finish();

		return true;
	}

	void  OnCallFinish(AutoNice resultData, bool bSu);

	virtual void SetRespData(AutoEvent &hResp) override;
};
////-------------------------------------------------------------------------
class DB_RequestModifyData_S : public DB_Responce
{
public:
	virtual bool _DoEvent();

	virtual bool _OnEvent(AutoEvent &hEvent)
	{
		GetResponseEvent()->set("ORIGINAL_VALUE",  (int)hEvent->get("ORIGINAL_VALUE"));
		GetResponseEvent()->set("RESULT_VALUE", hEvent->get("RESULT_VALUE").string());
		GetResponseEvent()->set("RESULT",  (int)hEvent->get("RESULT"));
		GetResponseEvent()->set("ERROR", hEvent->get("ERROR").string());
		Finish();

		return true;
	}
};
//-------------------------------------------------------------------------
class DB_RequestRecordData_S : public DB_Responce
{
public:
	virtual bool _DoEvent();

	virtual bool _OnEvent(AutoEvent &hEvent);

	virtual bool _Restore(DataStream *scrData);

public:
	AutoPtr<StringArray> mFieldList;
};
//-------------------------------------------------------------------------
// DB节点当分布信息变化时, 通知所有的应用终端重新刷新分布信息
//-------------------------------------------------------------------------
class DB_NotifyDBDistribution : public Logic::tBaseNetEvent
{
public:
	virtual bool _DoEvent()
	{
		return true;
	}
};
//-------------------------------------------------------------------------

#endif //_INCLUDE_DBSERVEROPERATE_H_