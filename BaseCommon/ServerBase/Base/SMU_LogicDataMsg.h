/********************************************************************
	created:	2011/02/18
	created:	18:2:2011   11:46
	filename: 	d:\Work\server_trunk\Server\SMU\SMU_LogicDataMsg.h
	file path:	d:\Work\server_trunk\Server\SMU
	file base:	SMU_LogicDataMsg
	file ext:	h
	author:		���ĸ�
	
	purpose:	
*********************************************************************/
#ifndef _INCLUDE_SMU_LOGICDATAMSG_H_
#define _INCLUDE_SMU_LOGICDATAMSG_H_

#include "SMU_DataMsg.h"


enum SMU_MSG_ID
{
	SAVE_LOGIC_DATA,
	LOAD_LOGIC_DATA_REQUEST,
	LOAD_LOGIC_DATA_RESULT,

	LOGIC_LOAD_DEFINE_REQUEST,
	LOGIC_LOAD_DEFINE_RESULT,
	LOGIC_SAVE_DEFINE_REQUEST,
	LOGIC_SYNC_RESULT,
	LOGIC_COMMON_DATA_RESULT,
	LOGIC_CLOSE_SERVER_REQUEST,

	SEND_LOGIC_EVENT,
	REV_LOGIC_EVENT,
};
// �������Ϣ����,�������õ�����Ŀ��,ʹ����ͬ����
// ��ͬ��ʵ�ּ�Ŀ��,�ڲ�ͬ��Ŀ��CPP��

class CommonDataMsg : public DataMsgEvent
{
public:
	CommonDataMsg()
		: mDataSize(0)
	{
		mLogicData = new DataBuffer;
	}
public:
	virtual BOOL WriteData(CHAR *data, size_t maxLength)
	{
		if (mLogicData->size()>maxLength)
			return FALSE;
		memcpy(data, mLogicData->data(), mLogicData->size());
		return TRUE;
	}
	virtual BOOL ReadData(CHAR *data, size_t msgDataSize,VOID *userData)
	{	
		mLogicData->resize(msgDataSize);
		mLogicData->write(data,msgDataSize);
		return TRUE;
	}

	virtual VOID SetDataSize(size_t size){ mDataSize = size; }
	virtual INT GetDataSize(VOID)
	{
		if (mDataSize==0 || mDataSize>mLogicData->size())
			return mLogicData->size();
		
		return mDataSize;
	}

public:
	AutoData		mLogicData;
	size_t			mDataSize;
};

// ���� [���ĸ� 2011/2/18]
class SMU_SaveLogicMsg : public CommonDataMsg
{
public:
	virtual INT	GetID(void) { return SAVE_LOGIC_DATA; }
	virtual BOOL Run(VOID *userData);

	virtual BOOL ReadData(CHAR *data, size_t msgDataSize,VOID *userData);

	virtual BOOL IsReplaceSameSendMsg(VOID){ return TRUE; }
};

// �����ȡ
class SMU_LoadLogicMsg_Request : public DataMsgEvent
{
public:
	virtual INT	GetID(void) { return LOAD_LOGIC_DATA_REQUEST; }
	virtual BOOL Run(VOID *userData);
	virtual BOOL WriteData(CHAR *data, size_t maxLength)
	{
		return TRUE;
	}
	virtual BOOL ReadData(CHAR *data, size_t msgDataSize,VOID *userData)
	{	
		return TRUE;
	}
	virtual INT GetDataSize(VOID)
	{
		return 0;
	}
	virtual BOOL IsReplaceSameSendMsg(VOID){ return TRUE; }
};

// ��ȡ�ظ�
class SMU_LoadLogicMsg_Result : public CommonDataMsg
{
public:
	virtual INT	GetID(void) { return LOAD_LOGIC_DATA_RESULT; }
	virtual BOOL Run(VOID *userData);

	virtual BOOL IsReplaceSameSendMsg(VOID){ return TRUE; }

};

// �������߼�����ͬ��(�綯̬��ӵ�����)
class SMU_Sync_LogicData_Request : public DataMsgEvent
{
public:
	virtual INT	GetID(void) { return LOGIC_SYNC_RESULT; }
	virtual BOOL Run(VOID *userData);
	virtual BOOL WriteData(CHAR *data, size_t maxLength)
	{
		return TRUE;
	}
	virtual BOOL ReadData(CHAR *data, size_t msgDataSize,VOID *userData)
	{	
		return TRUE;
	}
	virtual INT GetDataSize(VOID)
	{
		return 0;
	}
	virtual BOOL IsReplaceSameSendMsg(VOID){ return FALSE; }
};

// ��ȡ�����߼����ݵĽ���ظ�
class SMU_LogicCommonData_Result : public DataMsgEvent
{
public:
	virtual INT	GetID(void) { return LOGIC_COMMON_DATA_RESULT; }
	virtual BOOL Run(VOID *userData);
	virtual BOOL WriteData(CHAR *data, size_t maxLength)
	{
		if (mLogicData.size()>maxLength)
			return FALSE;
		memcpy(data, mLogicData.data(), mLogicData.size());
		return TRUE;
	}
	virtual BOOL ReadData(CHAR *data, size_t msgDataSize,VOID *userData)
	{	
		mLogicData.resize(msgDataSize);
		mLogicData.write(data,msgDataSize);
		return TRUE;
	}
	virtual INT GetDataSize(VOID) 
	{
		return mLogicData.size(); 
	}
	virtual BOOL IsReplaceSameSendMsg(VOID){ return FALSE; }

public:
	DataBuffer		mLogicData;

};
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
// �����ȡ�߼���������
class SMU_Load_LogicDefine_Result;

class SMU_Load_LogicDefine_Request : public CommonDataMsg
{
public:
	virtual INT	GetID(void) { return LOGIC_LOAD_DEFINE_REQUEST; }
	virtual BOOL Run(VOID *userData);
	virtual BOOL IsReplaceSameSendMsg(VOID){ return FALSE; }

protected:
	SMU_Load_LogicDefine_Result* _LoadDataDefineFromDB(__int64 key);
	void	_SendDataDefine(__int64 key,DataBuffer &data,VOID *useData);
};
//-----------------------------------------------------------------------------------

// �ظ���ȡ�߼��������ݽ��
class SMU_Load_LogicDefine_Result : public CommonDataMsg
{
public:
	virtual INT	GetID(void) { return LOGIC_LOAD_DEFINE_RESULT; }
	virtual BOOL Run(VOID *userData);
	virtual BOOL IsReplaceSameSendMsg(VOID){ return FALSE; }

};
//-----------------------------------------------------------------------------------

// �����߼���������
class SMU_Save_LogicDefine_Request : public CommonDataMsg
{
public:
	virtual INT	GetID(void) { return LOGIC_SAVE_DEFINE_REQUEST; }
	virtual BOOL Run(VOID *userData){ return TRUE; }
	virtual BOOL ReadData(CHAR *data, size_t msgDataSize,VOID *userData);
	virtual BOOL IsReplaceSameSendMsg(VOID){ return FALSE; }
};
//-----------------------------------------------------------------------------------
// �رշ�����
class SMU_Close_Server_Request : public CommonDataMsg
{
public:
	virtual INT	GetID(void) { return LOGIC_CLOSE_SERVER_REQUEST; }
	virtual BOOL Run(VOID *userData);
	virtual BOOL IsReplaceSameSendMsg(VOID){ return FALSE; }

};
//-----------------------------------------------------------------------------------
// �����¼�
namespace Logic
{
	class tEvent;
	class tEventCenter;
}
class SMU_SendLogicEvent : public CommonDataMsg
{
public:
	virtual INT	GetID(void) { return SEND_LOGIC_EVENT; }
	virtual BOOL Run(VOID *userData);
	virtual BOOL IsReplaceSameSendMsg(VOID){ return FALSE; }
	
	bool SetSendEvent(Logic::tEvent *event);
};
//-----------------------------------------------------------------------------------
// �����¼�
class SMU_RevLogicEvent : public CommonDataMsg
{
public:
	SMU_RevLogicEvent(Logic::tEventCenter *center)
		: mEventCenter(center)
	{

	}
public:
	virtual INT	GetID(void) { return REV_LOGIC_EVENT; }
	virtual BOOL Run(VOID *userData);
	virtual BOOL IsReplaceSameSendMsg(VOID){ return FALSE; }

protected:
	Logic::tEventCenter		*mEventCenter;
};
//-----------------------------------------------------------------------------------

DEFINE_MSG_FACTORY(LogicSaveMsgFactory,SAVE_LOGIC_DATA,SMU_SaveLogicMsg);
DEFINE_MSG_FACTORY(LogicLoadRequestMsgFactory,LOAD_LOGIC_DATA_REQUEST,SMU_LoadLogicMsg_Request);
DEFINE_MSG_FACTORY(LogicLoadResultMsgFactory,LOAD_LOGIC_DATA_RESULT,SMU_LoadLogicMsg_Result);

DEFINE_MSG_FACTORY(SMU_Sync_LogicData_RequestFactory,LOGIC_SYNC_RESULT,SMU_Sync_LogicData_Request);
DEFINE_MSG_FACTORY(SMU_LogicCommonData_ResultFactory,LOGIC_COMMON_DATA_RESULT,SMU_LogicCommonData_Result);

DEFINE_MSG_FACTORY(SMU_Load_LogicDefine_RequestFactory,LOGIC_LOAD_DEFINE_REQUEST,SMU_Load_LogicDefine_Request);
DEFINE_MSG_FACTORY(SMU_Load_LogicDefine_ResultFactory,LOGIC_LOAD_DEFINE_RESULT,SMU_Load_LogicDefine_Result);
DEFINE_MSG_FACTORY(SMU_Save_LogicDefine_RequestFactory,LOGIC_SAVE_DEFINE_REQUEST,SMU_Save_LogicDefine_Request);

#endif //_INCLUDE_SMU_LOGICDATAMSG_H_