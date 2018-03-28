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
				如果考虑分流, 使用UPD进行尝试连接, 得到分流后的节点地址
*********************************************************************/
#ifndef _INCLUDE_DBCLIENTREQUEST_H_
#define _INCLUDE_DBCLIENTREQUEST_H_

#include "ServerEvent.h"
#include "EventCenter.h"
#include "IOCPServerNet.h"

#include "Hand.h"
#include "MemoryDBHead.h"
#include "BaseRecord.h"
#include "BaseMemoryDB.h"
#include "TableFieldInfo.h"
#include "TableUpdateLoader.h"
#include "DBOperate.h"
#include "DBCallBack.h"

using namespace Logic;

class DBClient;
class MemoryDBNode;
//-------------------------------------------------------------------------
class MemoryDB_Export DB_Request : public DBOperate
{
public:
	DB_Request()		
		: mbNeedResponse(true)
	{
	}

public:
	DBClient* GetDB();

public:
	virtual bool DoEvent( bool bImmediately = true )
	{
		// 是否需要回复现在主要用在DB消息包中，通过事件请求的全部回复
		if (bImmediately && mbNeedResponse)
			WaitTime(20);

		return tServerEvent::DoEvent(bImmediately);
	}

public:
	virtual void _OnResp(AutoEvent &respEvent)
	{
		//Log(respEvent->GetData().dump().c_str());
		AString error = respEvent->get("ERROR");
		if (error!="")
		{
			mErrorInfo = error;
		}
		if (mResultType==eNoneError)
			mResultType = respEvent->get("RESULT");		

		Finish();
	}

	virtual void _OnFinish()
	{		
		if (mErrorInfo!="" || mResultType!=eNoneError)
		{
			AString info;
			info.Format("%s >%s Data >%s", mTableIndex.c_str(), mRecordKey.c_str(),  GetData().dump().c_str());
			OnShowErrorInfo(info);
			if (mResultType==eRequestOverTime && mbNeedResponse)
			{
				ERROR_LOG("Request db over time [%s], Info >%s", GetEventName(), info.c_str());
			}
			else
				WARN_LOG("ERROR: [%d] %s, at event[%s] INFO >%s", mResultType, mErrorInfo.c_str(), GetEventNameInfo().c_str(), info.c_str());
			//Dump();
		}
		if (mCallBack.empty())
		{
			if (mResultType!=eNoneError && (mResultType!=eRequestOverTime || mbNeedResponse))
			{
				AString info;
				info.Format("%s >%s Data >%s", mTableIndex.c_str(), mRecordKey.c_str(),  GetData().dump().c_str());
				OnShowErrorInfo(info);
				NOTE_LOG("ERROR: [%s] DB request fail >[%d] >>> %s, INFO >%s", GetEventName(), mResultType, mErrorInfo.c_str(), info.c_str());
			}
		}
		else
			mCallBack.run(this, mResultType==eNoneError);
		return;
	}

	virtual void OnShowErrorInfo(AString &info){}

	virtual bool _OnTimeOver(void)
	{
		LOG_YELLOW;
		Log(" >>> wait over time, now finish...");
		LOG_WHITE;
        mResultType = eRequestOverTime;
		Finish(); 
		return true; 
	}

protected:
	virtual void InitData() override
	{
		DBOperate::InitData();
		mbNeedResponse = true;
	}

public:
	bool		mbNeedResponse;

};
//-------------------------------------------------------------------------
class DB_RequestTableDistribution_C : public DB_Request
{
	virtual void _OnResp(AutoEvent &respEvent)
	{
		respEvent->get("INFO_TABLE", mResultTable);		
		DB_Request::_OnResp(respEvent);
	}
};
//-------------------------------------------------------------------------
// 如果存在, 是否修改表
class DB_CreateTable_C : public DB_Request
{
public:
	virtual bool _DoEvent();
	virtual void _OnResp(AutoEvent &respEvent);
};
//-------------------------------------------------------------------------
// 请求表格信息
class DB_LoadTableField_C : public DB_Request
{
public:
	virtual void _OnResp(AutoEvent &respEvent);
};
//-------------------------------------------------------------------------
//!!! 保存全部使用统一的保存更新消息包
// 是否替换, 如果不存在, 则插入, 如果存在, 是否替换, 支持部分字段, 如果是全部, 则只检查字段码, 部分字段需要全部分析通过
// set("GROWTH_KEY", true) 创建自增长记录
//class DB_SaveRecord_C : public DB_Request
//{
//public:
//	virtual bool _Serialize(DataStream *destData);
//
//	virtual bool _DoEvent()
//	{
//		if (!mResultRecord)
//		{
//			mErrorInfo = ("未指定需要保存的记录");
//			mResultType = eNoSetRequestRecord;
//		}
//
//		//const char *szTableName = mResultRecord->GetTable()->GetTableName();
//		//AString fname = "d:/tx_";
//		//fname += mResultRecord->GetTable()->GetField()->GetCheckCode();
//		//fname += ".csv";
//		//mResultRecord->GetTable()->SaveCSV(fname.c_str());
//		
//		int nowCode = mResultRecord->GetTable()->GetField()->GetCheckCode();
//		mRecordKey = mResultRecord->getIndexData().string();
//		set("CHECK_CODE", nowCode);
//		return DB_Request::_DoEvent();
//	}
//
//	virtual void _OnResp(AutoEvent &respEvent)
//	{
//		if (get("GROWTH_KEY"))
//		{
//			mRecordKey = respEvent->get("RECORD_KEY").string();
//			if (mResultRecord)
//				mResultRecord->_set(mResultRecord->GetTable()->GetMainIndexCol(), mRecordKey.c_str());
//		}
//		DB_Request::_OnResp(respEvent);
//	}
//};
//-------------------------------------------------------------------------
// 更新记录数据，只修改改变的部分
//class DB_UpdateRecord_C : public DB_Request
//{
//public:
//	virtual bool _DoEvent();
//
//	virtual bool _Serialize(DataStream *destData);
//
//	// 因为更新记录比较频繁，所以不需要回复
//	virtual void AlloctRespID() override {}
//	virtual bool WaitTime(float waitTime, EventCallBack callBack = EventCallBack()) override { return true; };
//
//	virtual void _OnResp(AutoEvent &respEvent) override
//	{
//		if (mCallBack.empty())
//			WARN_LOG("Update 无回调处理，不需要回复");
//		DB_Request::_OnResp(respEvent);
//	}
//};
//-------------------------------------------------------------------------
// 调取记录, 如果未提供字段验证 可以提供字段名列表, 进行部分检索
class DB_FindRecord_C : public DB_Request
{
public:
	virtual void _OnResp(AutoEvent &respEvent);
};

//-------------------------------------------------------------------------
// 删除记录
class DB_DeleteRecord_C : public DB_Request
{

};

//-------------------------------------------------------------------------
// 修改表格结构, 需要修改当前存在的记录
class DB_ChangeTableField_C : public DB_Request
{

};
//-------------------------------------------------------------------------
// 删除表格
class DB_DeleteTable_C : public DB_Request
{

};
//-------------------------------------------------------------------------
// 调取DB中所有DB表格字段信息
class DB_LoadAllTableField_C : public DB_Request
{
public:
	virtual bool _OnEvent(AutoEvent &hEvent);

};

//-------------------------------------------------------------------------
// 执行自定义操作, 设置 mTableIndex, mRecordKey, OPERATE_TYPE, PARAM
//class DB_RunDBOperate_C : public DB_Request
//{
//public:
//	virtual void _OnResp(AutoEvent &hEvent)
//	{		
//		hEvent->get("RESULT_DATA", &mResultNiceData, typeid(mResultNiceData));
//		DB_Request::_OnResp(hEvent);
//	}
//};
//-------------------------------------------------------------------------
// 修改指定的记录数据, 一般用于对某个数值进行增量加减
// [VALUE] 修改数值
// [FIELD] 指定字段 [BOSS:102][ATTACK_LIST:12091]DAMAGE  BOSS子表中的 ATTACK_LIST子表记录 12091(玩家DBID) 对应的伤害值
// [ADD] 设定为true时, 表示进行增量加减当前值, 否则使用给定值替换
// [LIMIT] 判断增减条件, 如果指定, 增加值, 则判断小于此值, 减小值必须大于此值才进行减少
// 返回 [RESULT_VALUE] 修改后的数值
//-------------------------------------------------------------------------
class DB_RequestModifyData_C : public DB_Request
{
public:
	virtual void _OnResp(AutoEvent &hEvent)
	{		
		mResultType = hEvent->get("RESULT");

		mResultNiceData = MEM_NEW NiceData();
		mResultNiceData->set("ORIGINAL_VALUE", hEvent->get("ORIGINAL_VALUE").string());
		mResultNiceData->set("RESULT_VALUE", hEvent->get("RESULT_VALUE").string());

		DB_Request::_OnResp(hEvent);
	}
};

class DB_RequestRecordData_C : public DB_Request
{
public:
	virtual void _OnResp(AutoEvent &hEvent)
	{		
		hEvent->get("DATA_LIST", mResultNiceData);

		DB_Request::_OnResp(hEvent);
	}

	bool _Serialize( DataStream *destData )
	{
		if (DB_Request::_Serialize(destData))
		{
			destData->write((DSIZE)mFieldList.size());
			for (size_t i=0; i<mFieldList.size(); ++i)
			{
				destData->writeString(mFieldList[i].c_str());
			}

			return true;
		}
		return false;
	}

public:
	StringArray		mFieldList;
};

//-------------------------------------------------------------------------
// DB节点当分布信息变化时, 通知所有的应用终端重新刷新分布信息
//-------------------------------------------------------------------------
class DB_NotifyDBDistribution_R : public tBaseNetEvent
{
public:
	virtual bool _DoEvent();
};
//-------------------------------------------------------------------------

#endif //_INCLUDE_DBCLIENTREQUEST_H_