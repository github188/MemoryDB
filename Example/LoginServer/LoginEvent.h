#ifndef _INCLUDE_LOGINEVENT_H_
#define _INCLUDE_LOGINEVENT_H_

#include "ClientEvent.h"
#include "EventCenter.h"
#include "GameStruct.h"
#include "GameLog.h"
#include "TimeManager.h"
#include "LoginThread.h"
#include "GameDefine.h"

using namespace Logic;



//-------------------------------------------------------------------------*/
class LoginThread;
class LoginEvent : public tClientEvent
{
public:
	static void RegisterNet(AutoEventCenter center);

public:
	LoginThread* GetThread();

};

//-------------------------------------------------------------------------*/
class CL_RequestCreateAccount : public LoginEvent
{
public:
	virtual bool _DoEvent() override
	{
		AString account = get("ACCOUNT");
		if (account.empty() || account== "" || account.length() > 30)
		{
			GetResponseEvent()->set("INFOTYPE", eLogin_Account_NoSet);
			GetResponseEvent()->set("ERROR", "No set Account at request event");
			Finish();
			return false;
		}

		AString pw = get("PASSWORD");
		if (pw=="" || pw.length() > 30)
		{
			GetResponseEvent()->set("INFOTYPE", eLogin_PassWord_Error);
			GetResponseEvent()->set("ERROR", "No set PASSWORD");
			Finish();
			return false;
		}

		AString strPhoneName = get("PHONE");
		AString strVersionName = get("VERSION");
		AString strPlatform = get("PLATFORM");

		if (strPhoneName.length() > 30)
		{
			char str[31] = {0};
			memcpy(str, strPhoneName.c_str(), 30);
			str[30] = '\0';
			strPhoneName = str;
		}
		if (strVersionName.length() > 100)
		{
			char str[101] = {0};
			memcpy(str, strVersionName.c_str(), 100);
			str[100] = '\0';
			strVersionName = str;
		}
		if (strPlatform.length() > 30)
		{
			char str[31] = {0};
			memcpy(str, strPlatform.c_str(), 30);
			str[30] = '\0';
			strPlatform = str;
		}

		AutoNice d = MEM_NEW NiceData();
		d["ACCOUNT"] = account;
		d["PASSWORD"] = pw;
		d["PHONE"] = strPhoneName;
		d["VERSION"] = strVersionName;
		d["IP"] = mNetConnect->GetIp();
		d["PLATFORM"] = strPlatform;

		GetThread()->GetDBWork()->ExeSqlFunction(DBCallBack(&CL_RequestCreateAccount::OnDBFinish, this), GLOBAL_AUCCONT_TABLE, "", "f_create_account", d);
		WaitTime(20);
		return true;
	}

	void OnDBFinish(DBOperate *pOp, bool bSu)
	{
		if (bSu )
		{
			int result = pOp->mResultNiceData["RESULT"];
			if ( result==eCreateAccountSucceed )
			{
				GetResponseEvent()["INFOTYPE"] = eLogin_CreateAccount_Succeed;
			}
			else if (result==eAccountExist)
				GetResponseEvent()["INFOTYPE"] = eLogin_Account_Repeat;
			else if (result==ePasswordIsNull)
				GetResponseEvent()["INFOTYPE"] = eLogin_Need_PassWord;
			else
				GetResponseEvent()["INFOTYPE"] =eLogin_CreateDBData_Fail;				
		}
		else
		{
			GetResponseEvent()["RESULT"] = eLogin_SQL_function_Error;
		}
		Finish();
	}

	virtual bool _OnTimeOver() override
	{
		GetResponseEvent()["INFOTYPE"] = eLogin_RequestDB_OverTime;
		GetResponseEvent()->set("ERROR", "请求创建帐号超时");
		Finish();
		return true;
	}
};
//-------------------------------------------------------------------------*/
class CL_RequestServerInfo : public LoginEvent
{
public:
	virtual bool _DoEvent() override
	{
		if (!GetThread()->GetDBWork()->IsOk())
		{
			GetResponseEvent()["RESULT"] = eLogin_ServerMaintain;
		}
		else			
		{
			GetResponseEvent()["INFO"] = GetThread()->mServerInfoData.getPtr();
		}

		int sdkQuDao = get("QU_DAO");
		AutoData annoData = GetThread()->GetAnnoDataBySDK(sdkQuDao);

		if (annoData)
			GetResponseEvent()["ANNO_DATA"] = annoData.getPtr();

		Finish();
		return true;
	}
};

//-------------------------------------------------------------------------*/
// 后端工具获取当前连接数
class FL_RequestLGNetConnectCount_L : public LoginEvent
{
public:
	virtual bool _DoEvent() override
	{
		GetResponseEvent()["COUNT"] = GetThread()->mPlayerWebNet->GetConnectCount();
		Finish();
		return true;
	}
};
//-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*/
// 根据分区连接到对应的LG后, 即发送此事件, 进行资源及帐号验证
class CL_RequestLogin : public LoginEvent
{
public:
	virtual bool _DoEvent() override
	{
		//NOTE_LOG(GetData().dump().c_str());

		if (GetThread()->mLeastGSKey<=0 || GetThread()->mServerVersion.length()<=0)
		{
			GetResponseEvent()["RESULT"] = eLogin_NoExist_GameServer;
			Finish();
			return true;
		}

		AString account = get("ACCOUNT");
		AString passMD5 = get("PASS_MD5");
		
		bool bIOS = get("IOS");

		AString verCode = get("VER");
		if (verCode=="")
		{
			GetResponseEvent()["RESULT"] = eLogin_Version_NoSet;
			//mErrorInfo = "No set VER at request event";
			Finish();
			return false;
		}
		if (strcmp(verCode.c_str(), GetThread()->mServerVersion.c_str())<0)
		{
			GetResponseEvent()["RESULT"] = eLogin_Version_Too_Old;
			//mErrorInfo = "Version too old";
			Finish();
			return false;
		}

		AString resMD5 = get("RES_MD5");
#if USE_RESOURCES_SERVER	
		if (GetThread()->mResourcesServerList.empty())
		{
			GetResponseEvent()["RESULT"] = eLogin_NoExist_ResourcesServer;
			Finish();
			return false;
		}

		if (!GetThread()->CheckUpdateResource(resMD5, get("IOS")))
		{
			//mErrorInfo = "Resources too old, then need update";
			AString resIp;
			int resPort = 0;
			if (GetThread()->GetRandResourcesServer(resIp, resPort))
			{
				GetResponseEvent()["RESULT"] = eLogin_Resources_Too_Old;
				GetResponseEvent()->set("RES_IP", resIp.c_str());
				GetResponseEvent()->set("RES_PORT", resPort);
			}
			else
				GetResponseEvent()["RESULT"] = eLogin_NoExist_ResourcesServer;
			
			Finish();
			return false;
		}
#endif

		AutoNice d = MEM_NEW NiceData();
		d["ACCOUNT"] = account;
		d["PASS_MD5"] = passMD5;
		GetThread()->GetDBWork()->ExeSqlFunction(DBCallBack(&CL_RequestLogin::OnCheckAccountFinish, this), GLOBAL_AUCCONT_TABLE, "", "f_login_check", d);

		return true;
	}

	void OnCheckAccountFinish(DBOperate *pOp, bool bSu)
	{
		if (bSu)
		{
			int result = pOp->mResultNiceData["RESULT"];
			if (result==eLoginSucceed)
			{
				SucceedLogin();
				return;
			}
			else if (result==eAccountNoExist)
				GetResponseEvent()["RESULT"] = eLogin_Account_NoExist;
			else 
				GetResponseEvent()["RESULT"] = eLogin_PassWord_Error;
		}
		Finish();
	}

	void SucceedLogin();

	virtual bool _OnEvent(AutoEvent &gsRespEvent) override;
};
//-------------------------------------------------------------------------*/
class LG_RequestReadyLogin : public tServerEvent
{
public:
	LG_RequestReadyLogin()
		: mpThread(NULL){}

	virtual bool _DoEvent() override
	{
		WaitTime(10);
		return true;
	}

	virtual void _OnResp(AutoEvent &respEvent)
	{
		if (mWaitEvent)
			mWaitEvent->OnEvent(respEvent);
	}

	virtual void InitData() override
	{
		tServerEvent::InitData();
		mpThread = NULL;
		mWaitEvent.setNull();
	}

	virtual bool _OnTimeOver()
	{
		AutoEvent e;
		if (mWaitEvent)
			mWaitEvent->OnEvent(e);
		return true;
	}

public:
	LoginThread	*mpThread;	
	AutoEvent	mWaitEvent;
};


//-------------------------------------------------------------------------*/
// 根据分区连接发送token请求登陆, 通过SDK验证后, 直接将组合UID(渠道ID_UID)发送给GS完成登陆
class CL_QuickSDKRequestLogin : public LoginEvent
{
public:
	virtual bool _DoEvent() override;

	// SDK验证后回调
	virtual bool _OnEvent(void *pData, const type_info &dataType) override;

	virtual void CheckSDKUID();

	// 验证成功, 分配 GS
	void OnCheckIDSucceed(int quDao, const AString &id, const AString &token);

	// GS准备成功, 回复客户端
	virtual bool _OnEvent(AutoEvent &gsRespEvent) override;

	// 15秒内未处理成功, 返回失败
	virtual bool _OnTimeOver() override;
};
//-------------------------------------------------------------------------*/
class CL_ALYSDKRequestLogin : public CL_QuickSDKRequestLogin
{
public:
	virtual bool _DoEvent() override
	{
		DEBUG_LOG("开始处理 CL_ALYSDKRequestLogin %s", GetData().dump().c_str());
		return CL_QuickSDKRequestLogin::_DoEvent();
	}

	virtual void CheckSDKUID() override;

	// SDK验证后回调
	bool _OnEvent(void *pData, const type_info &dataType) override;

	// ALY SB 要求知道是否是新建帐号
	void OnCheckIDSucceed(int quDao, const AString &id, const AString &token);

	void OnCreateAccoutFinish(DBOperate *p, bool bSu);

	virtual void InitData() override
	{
		CL_QuickSDKRequestLogin::InitData();
		mGSUID.setNull();
	}

public:
	AString			mGSUID;
};

class CL_ALYSDKRequestLoginIOS : public CL_QuickSDKRequestLogin
{
public:
	virtual bool _DoEvent() override
	{
		DEBUG_LOG("开始处理 CL_ALYSDKRequestLogin %s", GetData().dump().c_str());
		return CL_QuickSDKRequestLogin::_DoEvent();
	}

	virtual void CheckSDKUID() override;


	// SDK验证后回调
	bool _OnEvent(void *pData, const type_info &dataType) override;
};

#endif //_INCLUDE_LOGINEVENT_H_