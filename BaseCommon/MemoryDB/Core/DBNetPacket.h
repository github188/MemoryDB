#ifndef _INCLUDE_DBNETPACKET_H_
#define _INCLUDE_DBNETPACKET_H_

#include "EventPacket.h"
#include "DBClientRequest.h"
#include "ResponseEvent.h"
#include "DBNodeOperate.h"
#include "MemoryDBHead.h"

enum 
{
	PACKET_REQUEST_DBOPERATE = PACKET_MAX+1,
	PACKET_RESPONSE_DBOPERATE,
	PACKET_REQUEST_DBSAVE,
	PACKET_RESPONSE_DBSAVE,
};
//-------------------------------------------------------------------------*/
// 优化DB事务操作网络事件
// 1 各数据直接序列
// 2 直接使用NiceData 使用速度快
// 3 直接使用消息包进行执行，速度优化
// NOTE:  RequestDBOperatePacket 已经支持了中转功能
//				c#使用端 必须修改 需要在数据开头写入4字节整数, 及写入函数名后写入是否需要回复
//-------------------------------------------------------------------------*/
class MemoryDB_Export RequestDBOperateEvent : public DB_Request
{
public:
	static void InitReadyForNet(Hand<tBaseEventNet> net);

	virtual void OnResponse(int result, AutoNice resultData, const AString &info)
	{
		mResultType = result;
		mErrorInfo = info;
		mResultNiceData = resultData;
		Finish();
	}

	virtual void OnShowErrorInfo(AString &info) override
	{
		info += mFunctionName;
		if (mParamData)
			info += mParamData->dump();
	}

public:
	virtual bool Send(int nType /* = 0 */, int nTarget /* = 0 */ ) override;
	virtual void _OnResp(AutoEvent &respEvent){}

	virtual void InitData() override
	{
		DB_Request::InitData();
		mFunctionName.setNull();
		mParamData.setNull();
	}

public:
	AString		mFunctionName;
	AutoNice	mParamData;

	//op->mTableIndex = szTable;
	//op->mRecordKey = szRecordKey;
};

//-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*/
// 用于请求DB操作的消息
class MemoryDB_Export RequestDBOperatePacket : public DataPacket
{
public:
	virtual	PacketID_t	GetPacketID( ) const { return PACKET_REQUEST_DBOPERATE; }
	bool SendRequestEvent(Hand<RequestDBOperateEvent> requstEvt, tNetConnect *pConn);

	virtual UINT Execute( tNetConnect* pConnect ) override;

	void OnExeFinish(AutoNice resultData, bool bSu);

public:
	RequestDBOperatePacket()
		: mID(0)
		, mUserConnectID(-1)
		, mbNeedResponse(true)
	{
		mParamData = MEM_NEW NiceData();
	}
	~RequestDBOperatePacket()
	{
		mCallBackTool._free();
	}

	virtual void InitData() override
	{
		DataPacket::InitData();
		mNetConnect.setNull();
		mWaitOperate.setNull();
		mID = 0;
		mUserConnectID = -1;
		mbNeedResponse = true;
		if (mCallBackTool)
			mCallBackTool->mpPacketOwner = NULL;
	}

public:
	AutoNice		mParamData;
	AutoOpereate	mWaitOperate;
	HandConnect		mNetConnect;
	EVENT_ID		mID;
	int mUserConnectID;
	bool	mbNeedResponse;

protected:
	// NOTE: 用于支持操作回调
	class CallTool : public Base<CallTool>
	{
	public:
		CallTool(RequestDBOperatePacket *p)
			: mpPacketOwner(p){}

		CallTool(){}

	public:
		void _CallBack(AutoNice resultData, bool bSu)
		{
			if (mpPacketOwner!=NULL)
				mpPacketOwner->OnExeFinish(resultData, bSu);
		}

	public:
			RequestDBOperatePacket	*mpPacketOwner;
	};
	Hand<CallTool> mCallBackTool;
};

class MemoryDB_Export ResponseDBOperatePacket : public DataPacket
{
public:
	virtual	PacketID_t	GetPacketID( ) const { return PACKET_RESPONSE_DBOPERATE; }
	void SetResult(EVENT_ID eventID, AutoOpereate op, int reslut, int userConnectID);

	virtual UINT Execute( tNetConnect* pConnect ) override;

public:
	ResponseDBOperatePacket()
	{
		mResultData = MEM_NEW NiceData();
	}

protected:
	AutoNice mResultData;
};

//-------------------------------------------------------------------------*/
// 更新记录消息
class MemoryDB_Export RequestDBSaveEvent : public DB_Request
{
public:
	virtual void OnResponse(int result)
	{
		mResultType = result;		
		Finish();
	}

public:
	virtual bool Send(int nType /* = 0 */, int nTarget /* = 0 */ ) override;
	virtual void _OnResp(AutoEvent &respEvent){}

	virtual void InitData() override
	{
		DB_Request::InitData();
		mbNewInsert = false;
		mbReplace = false;
		mbGrowthKey = false;
	}

public:
	bool		mbNewInsert;
	bool		mbReplace;
	bool		mbGrowthKey;
};

//-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*/
// 用于DB保存记录数据的消息
class MemoryDB_Export RequestDBSavePacket : public DataPacket
{
public:
	virtual	PacketID_t	GetPacketID( ) const { return PACKET_REQUEST_DBSAVE; }
	bool SendRequestEvent(Hand<RequestDBSaveEvent> requstEvt, tNetConnect *pConn);

	virtual UINT Execute( tNetConnect* pConnect ) override;

	bool Do(MemoryDBNode *pDBNode, EVENT_ID &eventID, AString &recordKey);

public:
	AutoData		mSaveData;
	AutoData		mUpdateData;

	int				mResult;

public:
	RequestDBSavePacket()
		: mResult(eNoneError)
	{
		mSaveData = MEM_NEW SaveRecordDataBuffer(1024);
		mUpdateData = MEM_NEW UpdateDataBuffer(1024);
	}
};

class MemoryDB_Export ResponseDBSavePacket : public DataPacket
{
public:
	virtual	PacketID_t	GetPacketID( ) const { return PACKET_RESPONSE_DBSAVE; }
	void SetResult(EVENT_ID eventID, int reslut, const AString &recordKey)
	{
		if (mData)
			mData->clear(false);
		else
			mData = MEM_NEW DataBuffer(16);
		mData->write(eventID);
		mData->write(reslut);
		mData->writeString(recordKey);
	}

	virtual UINT Execute( tNetConnect* pConnect ) override;

public:
	ResponseDBSavePacket()
	{
	}

};
//-------------------------------------------------------------------------*/
class DBNetProtocol : public EventNetProtocol
{
public:
	DBNetProtocol(){ RegisterPacket(this); }

	static void RegisterPacket(tNetProtocol *pProtocol)
	{	
		pProtocol->RegisterNetPacket(MEM_NEW DefinePacketFactory<RequestDBOperatePacket, PACKET_REQUEST_DBOPERATE>());
		pProtocol->RegisterNetPacket(MEM_NEW DefinePacketFactory<ResponseDBOperatePacket, PACKET_RESPONSE_DBOPERATE>());

		pProtocol->RegisterNetPacket(MEM_NEW DefinePacketFactory<RequestDBSavePacket, PACKET_REQUEST_DBSAVE>());
		pProtocol->RegisterNetPacket(MEM_NEW DefinePacketFactory<ResponseDBSavePacket, PACKET_RESPONSE_DBSAVE>());
	}
};
//-----------------------------------------------------------------------------

#endif //_INCLUDE_DBNETPACKET_H_