#ifndef _INCLUDE_SDK_H_
#define _INCLUDE_SDK_H_

/*/-------------------------------------------------------------------------/
运营平台SDK接入
一般会有各种阻塞交互，实现异步功能
//-------------------------------------------------------------------------*/
#include "TaskManager.h"
#include "Event.h"
#include "HttpClient.h"


typedef std::string (*Dllfun)(const char *, std::string&, bool bIs360);
typedef bool (*DllNotifyFun)(const char *szSid, int zoneID, const char *zoneName, int roleID, const char *roleName, unsigned __int64 roleCreateTime, int roleLevel);

//-------------------------------------------------------------------------*/
class SDK : public TaskManager
{
public:
	void RequestCheckUCLogin(const char  *sid, AutoEvent requestEvt, bool bIs360);
	void NotifyLoginData(const char *szSid, int roleDB, const AString &roleName, UInt64 roleCreateTime, int roleLevel, AutoEvent requestEvt, bool bIs360);

	std::string CheckUCLogin(const char  *sid, std::string &info, bool bIs360);
	bool NotifyData(const char *szSid, int zoneID, const char *zoneName, int roleID, const char *roleName, unsigned __int64 roleCreateTime, int roleLevel);

public:
	SDK();
	~SDK();

	enum
	{
		eTaskNone,
		eTaskCheckUCLogic,
		eTaskNotifyLogic,
	};

protected:
	// 只能在后台线程内使用
	CHttpClient		mHttpClient;
//protected:
//	__int64 mDllHander;
//	Dllfun mCheckLoginFun;
//	DllNotifyFun mNotifyFun;
};
//-------------------------------------------------------------------------*/
class TaskCheckUCLogic : public Task
{
public:
	virtual void Execute(TaskManager *pTaskMgr)
	{
		mResultAccount = (dynamic_cast<SDK*>(pTaskMgr))->CheckUCLogin(mSid.c_str(), mResultInfo, mIs360);
	}
	virtual void OnFinished(TaskManager *pTaskMgr)
	{
		if (mWaitCallBackEvt)
			mWaitCallBackEvt->_OnEvent(this, typeid(TaskCheckUCLogic));
	}

	virtual void InitFree() override
	{
		mWaitCallBackEvt.setNull();
		mSid.setNull();
		mResultAccount.clear();
		mResultInfo.clear();
	}

public:
	AutoEvent			mWaitCallBackEvt;
	AString				mSid;
	bool				mIs360;

	std::string			mResultAccount;
	std::string			mResultInfo;

};

class NotifyLogicData : public Task
{
public:
	virtual void Execute(TaskManager *pTaskMgr)
	{
		mResult = (dynamic_cast<SDK*>(pTaskMgr))->NotifyData(mSid.c_str(), 1, "野猪森林", mRoleDB, mRoleName.c_str(), mCreateTime, mLevel);
	}
	virtual void OnFinished(TaskManager *pTaskMgr)
	{
		if (mWaitCallBackEvt)
			mWaitCallBackEvt->_OnEvent(this, typeid(NotifyLogicData));
	}

	virtual void InitFree() override
	{
		mWaitCallBackEvt.setNull();
		mSid.setNull();
		mRoleName.setNull();
		mCreateTime = 0;
		mLevel = 0;
		mRoleDB = 0;

		mResult = false;
	}

public:
	AutoEvent			mWaitCallBackEvt;
	AString				mSid;

	KEY					mRoleDB;
	AString				mRoleName;
	UInt64				mCreateTime;
	int					mLevel;

	bool				mResult;

};
//-------------------------------------------------------------------------*/

#endif //_INCLUDE_SDK_H_