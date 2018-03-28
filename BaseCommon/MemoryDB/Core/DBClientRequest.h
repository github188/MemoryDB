/********************************************************************
	created:	2014/07/23
	created:	23:7:2014   2:10
	filename: 	H:\RemoteGame\MemorySQL\Code\DBOperate.h
	file path:	H:\RemoteGame\MemorySQL\Code
	file base:	DBOperate
	file ext:	h
	author:		Yang Wenge
	
	purpose:	DB ����
				1 �������
				2 �����¼
				3 ���Ҽ�¼				
				5 ɾ����¼
				6 �޸ı��
				7 ɾ�����
	NOTE:		���ڼ�¼, ֻ���ֶ���֤CODE �ɱ��������ϣ�����32λ���ֶ��ַ���ϣ��һ������
				������Ƿ���, ʹ��UPD���г�������, �õ�������Ľڵ��ַ
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
		// �Ƿ���Ҫ�ظ�������Ҫ����DB��Ϣ���У�ͨ���¼������ȫ���ظ�
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
// �������, �Ƿ��޸ı�
class DB_CreateTable_C : public DB_Request
{
public:
	virtual bool _DoEvent();
	virtual void _OnResp(AutoEvent &respEvent);
};
//-------------------------------------------------------------------------
// ��������Ϣ
class DB_LoadTableField_C : public DB_Request
{
public:
	virtual void _OnResp(AutoEvent &respEvent);
};
//-------------------------------------------------------------------------
//!!! ����ȫ��ʹ��ͳһ�ı��������Ϣ��
// �Ƿ��滻, ���������, �����, �������, �Ƿ��滻, ֧�ֲ����ֶ�, �����ȫ��, ��ֻ����ֶ���, �����ֶ���Ҫȫ������ͨ��
// set("GROWTH_KEY", true) ������������¼
//class DB_SaveRecord_C : public DB_Request
//{
//public:
//	virtual bool _Serialize(DataStream *destData);
//
//	virtual bool _DoEvent()
//	{
//		if (!mResultRecord)
//		{
//			mErrorInfo = ("δָ����Ҫ����ļ�¼");
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
// ���¼�¼���ݣ�ֻ�޸ĸı�Ĳ���
//class DB_UpdateRecord_C : public DB_Request
//{
//public:
//	virtual bool _DoEvent();
//
//	virtual bool _Serialize(DataStream *destData);
//
//	// ��Ϊ���¼�¼�Ƚ�Ƶ�������Բ���Ҫ�ظ�
//	virtual void AlloctRespID() override {}
//	virtual bool WaitTime(float waitTime, EventCallBack callBack = EventCallBack()) override { return true; };
//
//	virtual void _OnResp(AutoEvent &respEvent) override
//	{
//		if (mCallBack.empty())
//			WARN_LOG("Update �޻ص���������Ҫ�ظ�");
//		DB_Request::_OnResp(respEvent);
//	}
//};
//-------------------------------------------------------------------------
// ��ȡ��¼, ���δ�ṩ�ֶ���֤ �����ṩ�ֶ����б�, ���в��ּ���
class DB_FindRecord_C : public DB_Request
{
public:
	virtual void _OnResp(AutoEvent &respEvent);
};

//-------------------------------------------------------------------------
// ɾ����¼
class DB_DeleteRecord_C : public DB_Request
{

};

//-------------------------------------------------------------------------
// �޸ı��ṹ, ��Ҫ�޸ĵ�ǰ���ڵļ�¼
class DB_ChangeTableField_C : public DB_Request
{

};
//-------------------------------------------------------------------------
// ɾ�����
class DB_DeleteTable_C : public DB_Request
{

};
//-------------------------------------------------------------------------
// ��ȡDB������DB����ֶ���Ϣ
class DB_LoadAllTableField_C : public DB_Request
{
public:
	virtual bool _OnEvent(AutoEvent &hEvent);

};

//-------------------------------------------------------------------------
// ִ���Զ������, ���� mTableIndex, mRecordKey, OPERATE_TYPE, PARAM
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
// �޸�ָ���ļ�¼����, һ�����ڶ�ĳ����ֵ���������Ӽ�
// [VALUE] �޸���ֵ
// [FIELD] ָ���ֶ� [BOSS:102][ATTACK_LIST:12091]DAMAGE  BOSS�ӱ��е� ATTACK_LIST�ӱ��¼ 12091(���DBID) ��Ӧ���˺�ֵ
// [ADD] �趨Ϊtrueʱ, ��ʾ���������Ӽ���ǰֵ, ����ʹ�ø���ֵ�滻
// [LIMIT] �ж���������, ���ָ��, ����ֵ, ���ж�С�ڴ�ֵ, ��Сֵ������ڴ�ֵ�Ž��м���
// ���� [RESULT_VALUE] �޸ĺ����ֵ
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
// DB�ڵ㵱�ֲ���Ϣ�仯ʱ, ֪ͨ���е�Ӧ���ն�����ˢ�·ֲ���Ϣ
//-------------------------------------------------------------------------
class DB_NotifyDBDistribution_R : public tBaseNetEvent
{
public:
	virtual bool _DoEvent();
};
//-------------------------------------------------------------------------

#endif //_INCLUDE_DBCLIENTREQUEST_H_