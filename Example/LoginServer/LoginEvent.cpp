#include "LoginEvent.h"
#include "LoginThread.h"
#include "SDK.h"
#include "LG_NodeNetEvent.h"
#include "AccountNetEvent.h"

void LoginEvent::RegisterNet(AutoEventCenter center)
{
	//center->RegisterEvent("CS_CheckClientVerAndResource", MEM_NEW EventFactory<CS_CheckClientVerAndResource>());
	center->RegisterEvent("CL_RequestLogin", MEM_NEW EventFactory<CL_RequestLogin>());

	center->RegisterEvent("CL_RequestCreateAccount", MEM_NEW EventFactory<CL_RequestCreateAccount>());
	center->RegisterEvent("CL_RequestServerInfo", MEM_NEW EventFactory<CL_RequestServerInfo>());
	
	center->RegisterEvent("CL_QuickSDKRequestLogin", MEM_NEW EventFactory<CL_QuickSDKRequestLogin>());
	center->RegisterEvent("CL_ALYSDKRequestLogin", MEM_NEW EventFactory<CL_ALYSDKRequestLogin>());
	center->RegisterEvent("CL_ALYSDKRequestLoginIOS", MEM_NEW EventFactory<CL_ALYSDKRequestLoginIOS>());
	
	center->RegisterEvent("FL_RequestLGNetConnectCount", MEM_NEW EventFactory<FL_RequestLGNetConnectCount_L>());
}

LoginThread* LoginEvent::GetThread()
{
	if (mNetConnect)
	{
		Hand<LGServerNet> net = mNetConnect->GetNetHandle();
		return net->mpLoginThread;
	}
	return NULL;
}

//-------------------------------------------------------------------------*/

//-------------------------------------------------------------------------*/

void CL_RequestLogin::SucceedLogin()
{
	// 请最少在线GS, 准备迎接客户端接入
	HandConnect conn = GetThread()->GetLeastGSConnect();
	if (!conn)
	{
		GetResponseEvent()["RESULT"] = eLogin_NoExist_GameServer;
		Finish();
		return;
	}

	GetResponseEvent()["GS_IP"] = GetThread()->mLeastGSKey;

	Hand<LG_RequestReadyLogin> request = conn->StartEvent("LG_RequestReadyLogin");
	request->mpThread = GetThread();	
	request->mWaitEvent = GetSelf();
	request["ACCOUNT"] = get("ACCOUNT").string();

	request->Start();
}

bool CL_RequestLogin::_OnEvent(AutoEvent &gsRespEvent)
{
	if (gsRespEvent)
	{
		if ((int)gsRespEvent["RESULT"]==eNoneError)
		{
			GetResponseEvent()["CHECK_KEY"] = (UInt64)gsRespEvent["CHECK_KEY"];
			GetResponseEvent()["RESULT"] = eLogin_Succeed;
		}
		else
			GetResponseEvent()["RESULT"] = (int)gsRespEvent["RESULT"];
	}
	else
	{
		GetResponseEvent()["RESULT"] = eLogin_Server_Logic_Error;
	}
	Finish();
	return true;
}
//-------------------------------------------------------------------------*/
//-------------------------------------------------------------------------*/
bool CL_QuickSDKRequestLogin::_DoEvent()
{
	//NOTE_LOG(GetData().dump().c_str());

	if (GetThread()->mLeastGSKey<=0 || GetThread()->mServerVersion.length()<=0)
	{
		GetResponseEvent()["RESULT"] = eLogin_NoExist_GameServer;
		Finish();
		return true;
	}

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

	int quDao = get("QU_DAO");
	if (quDao==0 || quDao == 1)
		CheckSDKUID();
	else
	{	
		//??? 这里假设验证通过
		AString uid = get("UID");
		AString token = get("TOKEN");
		OnCheckIDSucceed(quDao, uid, token);
	}
	WaitTime(15);
	return true;
}

void CL_QuickSDKRequestLogin::CheckSDKUID()
{
	AString uid = get("UID");
	AString token = get("TOKEN");
	int quDao = get("QU_DAO");

	// 进行SDK服务器验证
	AString requestString;
	requestString.Format("http://checkuser.sdk.quicksdk.net/v2/checkUserInfo?token=%s&uid=%s&product_code=%s", token.c_str(), uid.c_str(), "84694747944028215933630569717746");
	
	GetThread()->mSDK->RequestCheckUCLogin(requestString.c_str(), GetSelf(),  false);
}

bool CL_QuickSDKRequestLogin::_OnTimeOver()
{
	DEBUG_LOG("CL_QuickSDKRequestLogin 处理超时");
	GetResponseEvent()["RESULT"] = eSDKUID_CHECK_FAIL;
	Finish();
	Free();
	return true;
}

// SDK验证后回调
bool CL_QuickSDKRequestLogin::_OnEvent(void *pData, const type_info &dataType)
{
	if (dataType==typeid(TaskCheckUCLogic))
	{
		TaskCheckUCLogic *p = (TaskCheckUCLogic*)pData;
		//??? NOTE: 如果DNS解析失败 >p->mResultInfo=="6", 如果此情况影响较大,可以考虑,遇到此情况直接验证通过
		if (p->mResultAccount=="1")
		{
			DEBUG_LOG("SDK 验证成功 %s", get("UID").string().c_str());
			//验证通过
			AString uid = get("UID");
			AString token = get("TOKEN");
			int quDao = get("QU_DAO");

			OnCheckIDSucceed(quDao, uid, token);
			return true;
		}
		else if (p->mResultInfo=="6")
		{
			ERROR_LOG("SDK请求验证DNS解析失败");
			GetResponseEvent()["RESULT"] = eSDKUID_CHECK_FAIL;
			Finish();
			return true;
		}
		else
			DEBUG_LOG("无效的登陆请求 >[%s] [%s]", p->mResultInfo.c_str(), p->mResultAccount.c_str());
	}
	else
		ERROR_LOG("不是 TaskCheckUCLogic 调用 _OnEvent");

	GetResponseEvent()["RESULT"] = eSDKUID_CHECK_FAIL;
	Finish();
	return true;
}
// 验证成功, 分配 GS
bool CL_QuickSDKRequestLogin::_OnEvent(AutoEvent &gsRespEvent)
{
	if (gsRespEvent)
	{
		if ((int)gsRespEvent["RESULT"]==eNoneError)
		{
			GetResponseEvent()["CHECK_KEY"] = (UInt64)gsRespEvent["CHECK_KEY"];
			GetResponseEvent()["ACCOUNT"] = gsRespEvent["ACCOUNT"].string();
			GetResponseEvent()["RESULT"] = eLogin_Succeed;
		}
		else
			GetResponseEvent()["RESULT"] = (int)gsRespEvent["RESULT"];
	}
	else
	{
		GetResponseEvent()["RESULT"] = eLogin_Server_Logic_Error;
	}
	DEBUG_LOG("CL_QuickSDKRequestLogin response >%s", GetResponseEvent()->GetData().dump().c_str());
	Finish();
	return true;
}



// GS准备成功, 回复客户端
void CL_QuickSDKRequestLogin::OnCheckIDSucceed(int quDao, const AString &id, const AString &token)
{
	AString gsUID = STRING(quDao);
	gsUID += "_";
	gsUID += id;

	AString strPhoneName = get("PHONE");
	AString strVersionName = get("VERSION");
	AString strPlatform = get("PLATFORM");

	if (strPhoneName.length() > 30)
	{
		char str[31] = {0};
		memcpy(str, strPhoneName.c_str(), 30);
		strPhoneName = str;
	}
	if (strVersionName.length() > 100)
	{
		char str[101] = {0};
		memcpy(str, strVersionName.c_str(), 100);
		strVersionName = str;
	}
	if (strPlatform.length() > 30)
	{
		char str[31] = {0};
		memcpy(str, strPlatform.c_str(), 30);
		strPlatform = str;
	}

	// 尝试创建帐号到AG,用于以后查询使用,不使用验证
	AutoNice d = MEM_NEW NiceData();
	d["ACCOUNT"] = gsUID;

	d["PHONE"] = strPhoneName;
	d["VERSION"] = strVersionName;
	d["IP"] = mNetConnect->GetIp();
	d["PLATFORM"] = strPlatform;

	GetThread()->GetDBWork()->ExeSqlFunction(DBCallBack(), GLOBAL_AUCCONT_TABLE, "", "f_sdk_try_create_account", d);

	// 请最少在线GS, 准备迎接客户端接入
	HandConnect conn = GetThread()->GetLeastGSConnect();
	if (!conn)
	{
		GetResponseEvent()["RESULT"] = eLogin_NoExist_GameServer;
		Finish();
		return;
	}

	GetResponseEvent()["GS_IP"] = GetThread()->mLeastGSKey;

	Hand<LG_RequestReadyLogin> request = conn->StartEvent("LG_RequestReadyLogin");
	request->mpThread = GetThread();	
	request->mWaitEvent = GetSelf();

	request["ACCOUNT"] = gsUID;

	request->Start();
}
//-------------------------------------------------------------------------*/
//-------------------------------------------------------------------------*/
void CL_ALYSDKRequestLogin::CheckSDKUID()
{
	const char *url = "http://sdk.99aly.com:8000/sdk.php?s=LoginNotify/login_verify";

	AString uid = get("UID");
	AString token = get("TOKEN");

	NiceData d;
	d["user_id"] = uid;
	d["token"] = token;
	AString paramString = d.ToJSON();
	AString requestString;
	requestString.Format("%s %s", url, paramString.c_str()); 
	GetThread()->mSDK->RequestCheckUCLogin(requestString.c_str(), GetSelf(),  true);

	DEBUG_LOG("爱乐游SDK 进行验证 %s, token = %s, >>>%s", get("UID").string().c_str(), token.c_str(), requestString.c_str());

}
//-------------------------------------------------------------------------*/
bool CL_ALYSDKRequestLogin::_OnEvent(void *pData, const type_info &dataType)
{
	DEBUG_LOG("CL_ALYSDKRequestLogin >处理验证结果");
	if (dataType==typeid(TaskCheckUCLogic))
	{
		TaskCheckUCLogic *p = (TaskCheckUCLogic*)pData;
		NiceData resultData;
		if (resultData.FullJSON(p->mResultAccount.c_str()) && (int)resultData["status"]==1)
		{
			DEBUG_LOG("SDK 验证成功 %s, result>%s", get("UID").string().c_str(), resultData.dump().c_str());
			//验证通过
			AString account = resultData["user_account"]; // 爱乐游根据account进行创建 get("UID");
			AString token = get("TOKEN");
			int quDao = get("QU_DAO");

			OnCheckIDSucceed(quDao, account, token);
			return true;
		}
		else
			DEBUG_LOG("无效的登陆请求 >[%s] [%s]", p->mResultInfo.c_str(), p->mResultAccount.c_str());
		DEBUG_LOG( resultData.dump().c_str() );
	}
	else
		ERROR_LOG("不是 TaskCheckUCLogic 调用 _OnEvent");

	// 应用验证
	GetResponseEvent()["RESULT"] = eSDKUID_CHECK_FAIL;
	Finish();
	return true;
}

void CL_ALYSDKRequestLogin::OnCheckIDSucceed(int quDao, const AString &id, const AString &token)
{
	AString gsUID = STRING(quDao);
	gsUID += "_";
	gsUID += id;

	mGSUID = gsUID;

	AString strPhoneName = get("PHONE");
	AString strVersionName = get("VERSION");
	AString strPlatform = get("PLATFORM");

	if (strPhoneName.length() > 30)
	{
		char str[31] = {0};
		memcpy(str, strPhoneName.c_str(), 30);
		strPhoneName = str;
	}
	if (strVersionName.length() > 100)
	{
		char str[101] = {0};
		memcpy(str, strVersionName.c_str(), 100);
		strVersionName = str;
	}
	if (strPlatform.length() > 30)
	{
		char str[31] = {0};
		memcpy(str, strPlatform.c_str(), 30);
		strPlatform = str;
	}

	// 尝试创建帐号到AG,用于以后查询使用,不使用验证
	AutoNice d = MEM_NEW NiceData();
	d["ACCOUNT"] = gsUID;

	d["PHONE"] = strPhoneName;
	d["VERSION"] = strVersionName;
	d["IP"] = mNetConnect->GetIp();
	d["PLATFORM"] = strPlatform;

	GetThread()->GetDBWork()->ExeSqlFunction(DBCallBack(&CL_ALYSDKRequestLogin::OnCreateAccoutFinish, this), GLOBAL_AUCCONT_TABLE, "", "f_sdk_try_create_account", d);
}

void CL_ALYSDKRequestLogin::OnCreateAccoutFinish(DBOperate *p, bool bSu)
{
	if (bSu && p->mResultNiceData)
	{		
		if ( (int)p->mResultNiceData["RESULT"] == eCreateAccountSucceed )
			GetResponseEvent()["IS_NEW_CREATE"] = true;
		
		// 请最少在线GS, 准备迎接客户端接入
		HandConnect conn = GetThread()->GetLeastGSConnect();
		if (!conn)
		{
			GetResponseEvent()["RESULT"] = eLogin_NoExist_GameServer;
			Finish();
			return;
		}

		GetResponseEvent()["GS_IP"] = GetThread()->mLeastGSKey;

		Hand<LG_RequestReadyLogin> request = conn->StartEvent("LG_RequestReadyLogin");
		request->mpThread = GetThread();	
		request->mWaitEvent = GetSelf();

		request["ACCOUNT"] = mGSUID;

		request->Start();
	}
	else
	{
		GetResponseEvent()["RESULT"] = eLogin_RequestDB_OverTime;
		Finish();
		Free();
	}
}

//-------------------------------------------------------------------------*/
void CL_ALYSDKRequestLoginIOS::CheckSDKUID()
{
	const char *url = "http://sdk.99aly.com:8000/sdk.php?s=LoginNotify/login_verify";

	AString uid = get("UID");
	AString token = get("TOKEN");

	NiceData d;
	d["user_id"] = uid;
	d["token"] = token;
	AString paramString = d.ToJSON();
	AString requestString;
	requestString.Format("%s %s", url, paramString.c_str()); 
	GetThread()->mSDK->RequestCheckUCLogin(requestString.c_str(), GetSelf(),  true);

	DEBUG_LOG("爱乐游SDK 进行验证 %s, token = %s, >>>%s", get("UID").string().c_str(), token.c_str(), requestString.c_str());

}

bool CL_ALYSDKRequestLoginIOS::_OnEvent(void *pData, const type_info &dataType)
{
	DEBUG_LOG("CL_ALYSDKRequestLogin >处理验证结果");
	if (dataType==typeid(TaskCheckUCLogic))
	{
		TaskCheckUCLogic *p = (TaskCheckUCLogic*)pData;
		NiceData resultData;
		if (resultData.FullJSON(p->mResultAccount.c_str()) && (int)resultData["status"]==1)
		{
			DEBUG_LOG("SDK 验证成功 %s, result>%s", get("UID").string().c_str(), resultData.dump().c_str());
			//验证通过
			AString account = resultData["user_account"]; // 爱乐游根据account进行创建 get("UID");
			AString token = get("TOKEN");
			int quDao = get("QU_DAO");

			OnCheckIDSucceed(quDao, account, token);
			return true;
		}
		else
			DEBUG_LOG("无效的登陆请求 >[%s] [%s]", p->mResultInfo.c_str(), p->mResultAccount.c_str());
		DEBUG_LOG( resultData.dump().c_str() );
	}
	else
		ERROR_LOG("不是 TaskCheckUCLogic 调用 _OnEvent");

	// 应用验证
	GetResponseEvent()["RESULT"] = eSDKUID_CHECK_FAIL;
	Finish();
	return true;
}
