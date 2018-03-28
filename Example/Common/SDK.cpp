#include "SDK.h"
#include <Windows.h>
#include "CommonDefine.h"
//#include "UCServerSDKTool.h"
//NOTE: 实际测试，不可以在每个线程进行加载DLL，出现不能正常验证的情况
// 加载c#DLL必须安装net4.0 才能正常运行
//-------------------------------------------------------------------------*/
#define USE_SDK_CHECK	1

void SDK::RequestCheckUCLogin(const char *sid, AutoEvent requestEvt, bool bIs360)
{
#if USE_SDK_CHECK
	Auto<TaskCheckUCLogic> t = StartTask(eTaskCheckUCLogic);
	t->mWaitCallBackEvt = requestEvt;
	t->mSid = sid;
	t->mIs360 = bIs360;
#endif
}

void SDK::NotifyLoginData(const char *szSid, int roleDB, const AString &roleName, UInt64 roleCreateTime, int roleLevel, AutoEvent requestEvt, bool bIs360)
{
#if USE_SDK_CHECK
	//??? 目前不支持360各种埋点
	if (bIs360)
		return;

	Auto<NotifyLogicData> t = StartTask(eTaskNotifyLogic);
	t->mWaitCallBackEvt = requestEvt;
	t->mSid = szSid;
	t->mRoleDB = roleDB;
	//WARN_LOG("角色名称> [%s]", AString::getANIS(roleName.c_str()));
	t->mRoleName = AString::getANIS(roleName.c_str());
	t->mLevel = roleLevel;
	t->mCreateTime = roleCreateTime;
#endif
}

CsLock gLock;

std::string SDK::CheckUCLogin(const char *sid, std::string &info, bool bIs360)
{
	// QUICK SDK, 直接使用CHttpClient 进行获取
	std::string result;
	int re = -1;
	// 尝试两次
	for (int i=0; i<2; ++i)
	{
		if (bIs360)
		{
			Array<AString> strList;
			if (AString::Split(sid, strList, " ", 2)>=2)
				re = mHttpClient.Post(strList[0].c_str(), strList[1].c_str(), result, 5);		
		}
		else
			re = mHttpClient.Get(sid, result, 5);
		if (re==0)
			break;
	}

	if (re!=0)
	{
		info = STRING(re);
		ERROR_LOG("SDK Request fail: Error code %d", re);
	}
	return result;
	//!!! SDKTool > return CheckLoginInfo(sid, info, bIs360);

//	CsLockTool l(gLock);
//
//#if !DEVELOP_MODE
//	if (mCheckLoginFun==NULL)
//	{
//#if _DEBUG
//		mDllHander=(__int64)LoadLibrary("UCServerSDKTool_d.dll");
//#else
//		mDllHander=(__int64)LoadLibrary("UCServerSDKTool.dll");
//#endif
//		if(mDllHander!=NULL)
//		{
//			mCheckLoginFun=(Dllfun)GetProcAddress((HINSTANCE)mDllHander, "CheckLoginInfo");
//			if(mCheckLoginFun==NULL)
//			{				
//				FreeLibrary((HINSTANCE)mDllHander);
//				mDllHander = NULL;
//				ERROR_LOG("CheckLoginInfo No find");
//			}
//		}
//		else
//			ERROR_LOG("加载 SDK UCServerSDKTool dll 失败");
//	}
//
//	if(mCheckLoginFun!=NULL)
//	{
//		try{
//			return mCheckLoginFun(sid, info, bIs360);
//		}
//		catch(...)
//		{
//			ERROR_LOG("调用CheckUCLogin检验UC帐号失败");
//		}
//	}
//#endif
//	return std::string();
}

bool SDK::NotifyData(const char *szSid, int zoneID, const char *zoneName, int roleID, const char *roleName, unsigned __int64 roleCreateTime, int roleLevel)
{
	//???
	return false;

	//!!! SDKTool for uc 360, now use quick sdk, only use CHttpClient
	//return NotifyGameData(szSid, zoneID, zoneName, roleID, roleName, roleCreateTime, roleLevel);

//	CsLockTool l(gLock);
//
//#if !DEVELOP_MODE
//	if (mCheckLoginFun==NULL)
//	{
//#if _DEBUG
//		mDllHander=(__int64)LoadLibrary("UCServerSDKTool_d.dll");
//#else
//		mDllHander=(__int64)LoadLibrary("UCServerSDKTool.dll");
//#endif
//
//	}
//
//	if(mDllHander!=NULL && mNotifyFun==NULL)
//	{
//		mNotifyFun=(DllNotifyFun)GetProcAddress((HINSTANCE)mDllHander, "NotifyGameData");
//		if(mNotifyFun==NULL)
//		{				
//			//FreeLibrary(hdll);
//			//hdll = NULL;
//			ERROR_LOG("NotifyGameData No find");
//		}
//	}	
//
//	if(mNotifyFun!=NULL)
//	{
//		try{
//			return mNotifyFun(szSid, zoneID, zoneName, roleID, roleName, roleCreateTime, roleLevel);
//		}
//		catch(...)
//		{
//			ERROR_LOG("调用 NotifyGameData 更新登陆信息失败");
//		}
//	}
//#endif
//	return false;
}

SDK::SDK()
	//: mDllHander(NULL)
	//, mCheckLoginFun(NULL)
	//, mNotifyFun(NULL)
{
	RegisterTask(MEM_NEW DefineTaskFactory<TaskCheckUCLogic, eTaskCheckUCLogic>);
	RegisterTask(MEM_NEW DefineTaskFactory<NotifyLogicData, eTaskNotifyLogic>);
}
SDK::~SDK()
{
	//if (mDllHander!=NULL)
	//	FreeLibrary((HINSTANCE)mDllHander);
	//mDllHander = NULL;
}
//-------------------------------------------------------------------------*/