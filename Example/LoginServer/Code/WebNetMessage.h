#ifndef _INCLUDE_WEBNETMESSAGE_H_
#define _INCLUDE_WEBNETMESSAGE_H_

#include "ClientEvent.h"
#include "LoginThread.h"
#include "LGWebNet.h"
#include "LoginEvent.h"

class WEBMsg
{
public:
	static void Register(WebGameNet *pWebNet);
};

//-------------------------------------------------------------------------
class tWebResponseEvent : public Logic::tClientEvent
{
public:
	LoginThread* GetThread()
	{	
		Hand<LGWebNet> net = mNetConnect->GetNetHandle();
		return net->mpLoginThread;
	}
};
//-------------------------------------------------------------------------
// 请求登陆
class CS_ReuestLoginGame  : public tWebResponseEvent
{
public:
	virtual bool _DoEvent() override
	{

		AString account = get("ACCOUNT");
		AString pass = get("PASS");
		if (account.length()>0 && pass.length()>0)
		{
			AutoNice d = MEM_NEW NiceData();
			d["ACCOUNT"] = account;
			d["PASS_MD5"] = pass;
			GetThread()->GetDBWork()->ExeSqlFunction(DBCallBack(&CS_ReuestLoginGame::OnCheckAccountFinish, this), DB_ACCOUNT_TABLE, account.c_str(), "f_login_check", d);
			return true;
		}
		GetResponseEvent()["ERROR"] = eAccountIsNull;
		GetResponseEvent()["RESULT"] = false;
		Finish();
		return true;
	}

	void OnCheckAccountFinish(DBOperate *pDB, bool bSu);
};
// 客户端请求开始游戏
class CL_RequestRegisterAccount : public tWebResponseEvent
{
public:
	virtual bool _DoEvent() override
	{
		AString account = get("ACCOUNT");
		AString pass = get("PASS");
		if (account.length()>0 && pass.length()>0)
		{
			AutoNice d = MEM_NEW NiceData();
			d["ACCOUNT"] = account;
			d["PASSWORD"] = pass;
			GetThread()->GetDBWork()->ExeSqlFunction(DBCallBack(&CL_RequestRegisterAccount::OnCreateAccountFinish, this), DB_ACCOUNT_TABLE, account.c_str(), "f_create_account", d);
			return true;
		}
		GetResponseEvent()["ERROR"] = eAccountIsNull;
		GetResponseEvent()["RESULT"] = false;
		Finish();
		
		return true;
	}

	void OnCreateAccountFinish(DBOperate *pDB, bool bSu);
};

//class CL_RequestUploadPhoto : public tWebResponseEvent
//{
//public:
//	virtual bool _DoEvent() override;
//
//	void OnUploadFinish(DBOperate *p, bool bSu)
//	{
//		if (bSu)
//		{
//			int re = p->mResultNiceData["RESULT"];
//			GetResponseEvent()["RESULT"] = re;
//			if (re==eOk)
//				TABLE_LOG("上传保存照片成功");
//		}
//		else
//			GetResponseEvent()["RESULT"] = eDBQueryError;
//		
//		Finish();		
//	}
//
//};

class CL_RequestLoadPhotoData : public tWebResponseEvent
{
public:
	virtual bool _DoEvent();

	void OnLoadFinish(DBOperate *pDB, bool bSu);

public:
	virtual void SendResponce( AutoEvent hRespEvent ) override;

};
//
//class CL_RequestLoadSelfPhotoData : public tWebResponseEvent
//{
//public:
//	virtual bool _DoEvent();
//
//	void OnLoadFinish(DBOperate *pDB, bool bSu)
//	{
//		AutoData d = (DataStream*)pDB->mResultNiceData["DATA"];
//		if (d)
//			GetResponseEvent()["DATA"] = d.getPtr();
//		Finish();
//	}
//
//public:
//	virtual void SendResponce( AutoEvent hRespEvent ) override;
//};

class CL_RequestDBData : public tWebResponseEvent
{
public:
	virtual bool _DoEvent() override;

	void OnLoad(DBOperate *pDB, bool bSu)
	{
		if (bSu)
		{
			GetResponseEvent()["VALUE"].setData(pDB->mResultNiceData["VALUE"]);
		}
		Finish();
	}

	void SendResponce( AutoEvent hRespEvent );

};

class CL_RequestUpdateDBData : public tWebResponseEvent
{
public:
	virtual bool _DoEvent() override;

	void OnSaveFinish(DBOperate *pDB, bool bSu)
	{
		if (bSu)
		{
			GetResponseEvent()["RESULT"].setData(pDB->mResultNiceData["RESULT"]);
		}
		Finish();
	}

};

// 保存动态
class CL_RequestSaveNewsFeed : public tWebResponseEvent
{
public:
	virtual bool _DoEvent() override;

	void OnSaveFinish(DBOperate *pDB, bool bSu)
	{
		if (bSu)
		{
			GetResponseEvent()["RESULT"].setData(pDB->mResultNiceData["RESULT"]);
		}
		Finish();
	}
};

// 调取动态
class CL_LoadNewsFeed : public tWebResponseEvent
{
public:
	virtual bool _DoEvent() override;

	void OnLoad(DBOperate *pDB, bool bSu)
	{
		if (bSu)
		{
			GetResponseEvent()->GetData().append(*pDB->mResultNiceData, true);
		}
		Finish();
	}

	void SendResponce( AutoEvent hRespEvent );
};

class CL_LoadNewsFeedLoadBigPhoto : public tWebResponseEvent
{
public:
	virtual bool _DoEvent() override;


	void OnLoad(DBOperate *pDB, bool bSu)
	{
		if (bSu)
		{
			GetResponseEvent()->GetData().append(*pDB->mResultNiceData, true);
		}
		Finish();
	}

	void SendResponce( AutoEvent hRespEvent ) override;
};

#endif //_INCLUDE_WEBNETMESSAGE_H_