
#ifndef _INCLUDE_FILEDATARECEIVE_H_
#define _INCLUDE_FILEDATARECEIVE_H_

#include "NetHead.h"
#include "ServerEvent.h"



//-------------------------------------------------------------------------
//接收端
//-------------------------------------------------------------------------
class Net_Export FileNetTransferReceiveControl : public Base<FileNetTransferReceiveControl>
{

public:
	FileNetTransferReceiveControl()
		: mAlreadyReceivePartCount(0)
	{
		mReceivePartData = MEM_NEW DataBuffer();
	}

	static void RegisterMsgEvent(Logic::tEventCenter *pCenter);

public:
	virtual void OnReadyReceiveBegin() {}
	virtual void OnReceiveOnePartData(int partIndex, AutoData partData){}
	virtual int GetNextResquestPartIndex() { return mAlreadyReceivePartCount; }

	virtual AutoEvent StartEvent(const char *szEventName) = 0;

	virtual void Process(){}

public:
	void StartReadyReceiveData(DSIZE allPartCount, DSIZE scrDataSize, DSIZE zipDataSize, const char *md5);

	virtual void StartRequestData();

	// 适应顺序接收, 包序号不正确, 都以错误处理
	virtual RECEIVE_RESULT OnReceivePartData(int partIndex, AutoData partData);

	virtual RECEIVE_RESULT OnReceiveFinish();

	virtual bool OnReceiveSucceed();

	virtual void OnReceiveFail()
	{
		TABLE_LOG("接收文件失败");
	}

	virtual void OnReceiveDataCheckFail()
	{
		TABLE_LOG("接收数据验证失败");
	}

public:
	int						mAllPartCount;
	int						mAlreadyReceivePartCount;
	DSIZE					mScrDataSize;
	DSIZE					mZipDataSize;
	AString					mCheckMD5;
	AString					mResourceName;

	AutoData				mReceivePartData;

	AutoData				mResultData;
	UInt64					mStartTime;
};
//-------------------------------------------------------------------------
// 接收方请求数据信息
class Net_Export FILE_RequestDataInfo_R : public Logic::tServerEvent
{
public:
	virtual void OnRequestDataBegin() = 0;

public:
	virtual bool _DoEvent()
	{
		OnRequestDataBegin();		
		return true;
	}

	virtual void _OnResp(AutoEvent &respEvent);

public:
	Hand<FileNetTransferReceiveControl> mReceiveControl;
};
//-------------------------------------------------------------------------
// 请求数据包
//-------------------------------------------------------------------------
class Net_Export FILE_RequestPartData_R : public Logic::tServerEvent
{
public:
	virtual void OnReceiveFinish(){}
	virtual void OnReceiveFail(){}

public:
	virtual bool _NeedFinishWhenResponsed() const { return false; }

	virtual bool _DoEvent()
	{
		int mPartIndex = mReceiveControl->GetNextResquestPartIndex();
		set("PARTINDEX", mPartIndex);

		WaitTime(20);

		return true;
	}

	virtual void _OnResp(AutoEvent &respEvent);

	virtual bool _OnTimeOver()
	{
		Log("请求数据超时");
		mReceiveControl->OnReceiveFail();

		return true;
	}

public:
	Hand<FileNetTransferReceiveControl> mReceiveControl;
};
//-------------------------------------------------------------------------


#endif