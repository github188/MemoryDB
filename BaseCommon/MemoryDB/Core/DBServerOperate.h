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
// �Ƿ��滻, ���������, �����, �������, �Ƿ��滻
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
// ���¼�¼���ݣ�ֻ�޸ĸı�Ĳ���
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
// ��ȡ��¼, ���δ�ṩ�ֶ���֤ �����ṩ�ֶ����б�, ���в��ּ���
class DB_FindRecord_S : public DB_Responce
{
public:
	virtual bool _DoEvent();

	virtual bool _OnEvent(void *pData, const type_info &dataType);

	virtual bool _OnEvent(AutoEvent &evt);
};

//-------------------------------------------------------------------------
// ɾ����¼
class DB_DeleteRecord_S : public DB_Responce
{
public:
	virtual bool _DoEvent();

	virtual bool _OnEvent(void *pData, const type_info &dataType);

	bool _OnEvent( AutoEvent &evt );

};

//-------------------------------------------------------------------------
// �޸ı��ṹ, ��Ҫ�޸ĵ�ǰ���ڵļ�¼
class DB_ChangeTableField : public DB_Responce
{

};
//-------------------------------------------------------------------------
// ɾ�����
class DB_DeleteTable_S : public DB_Responce
{

};
//-------------------------------------------------------------------------
// ��ȡDB������DB����ֶ���Ϣ
class DB_LoadAllTableField_S : public DB_Responce
{
public:
	virtual bool _DoEvent();


};

//-------------------------------------------------------------------------
// ִ���Զ���DB����
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
// DB�ڵ㵱�ֲ���Ϣ�仯ʱ, ֪ͨ���е�Ӧ���ն�����ˢ�·ֲ���Ϣ
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