#ifndef _INCLUDE_LG_NODENETEVENT_H_
#define _INCLUDE_LG_NODENETEVENT_H_

#include "ServerEvent.h"
#include "LoginThread.h"
#include "ClientEvent.h"
//-------------------------------------------------------------------------*
class LG_NetEvent : public Logic::tBaseNetEvent
{
public:
	Hand<LGNodeNet> GetGSNet()
	{	
		Hand<MeshedNodeNet> net = mNetConnect->GetNetHandle()->GetSelf();
		if (!net)
		{
			Hand<NodeServerNet> nodeNet = mNetConnect->GetNetHandle()->GetSelf();
			AssertEx(nodeNet, "逻辑上 当前应该是 NodeServerNet");
			net = nodeNet->mOwnerMeshNet->GetSelf();
		}
		Hand<LGNodeNet> gsNodeNet = net;
		AssertEx(gsNodeNet, "必须能得到 GSMeshedNodeNet");
		return gsNodeNet;
	}

	GSKEY GetTargetGSKey();
};

class LG_RequestEvent : public Logic::tServerEvent
{
public:
	virtual bool DoEvent(bool bImmediately = true)
	{
		if (bImmediately)
			WaitTime(20);

		return Logic::tServerEvent::DoEvent(bImmediately);
	}

	Hand<LGNodeNet> GetGSNet()
	{	
		Hand<MeshedNodeNet> net = mNetConnect->GetNetHandle()->GetSelf();
		if (!net)
		{
			Hand<NodeServerNet> nodeNet = mNetConnect->GetNetHandle()->GetSelf();
			AssertEx(nodeNet, "逻辑上 当前应该是 NodeServerNet");
			net = nodeNet->mOwnerMeshNet->GetSelf();
		}
		Hand<LGNodeNet> gsNodeNet = net;
		AssertEx(gsNodeNet, "必须能得到 GSMeshedNodeNet");
		return gsNodeNet;
	}

	GSKEY GetTargetGSKey();
};

class LG_ResponseEvent : public Logic::tClientEvent
{
public:
	Hand<LGNodeNet> GetGSNet()
	{	
		Hand<MeshedNodeNet> net;
		Hand<NodeServerNet> nodeNet = mNetConnect->GetNetHandle()->GetSelf();

		if (nodeNet)
			net = nodeNet->mOwnerMeshNet->GetSelf();

		if (!net)
		{
			net = mNetConnect->GetNetHandle()->GetSelf();
		}
		Hand<LGNodeNet> gsNodeNet = net;
		AssertEx(gsNodeNet, "必须能得到 GSMeshedNodeNet");
		return gsNodeNet;
	}

	GSKEY GetTargetGSKey();

};
//-------------------------------------------------------------------------*
class GL_NotifyOnlineInfo_R : public LG_NetEvent
{
public:
	virtual bool _DoEvent();
};

class GL_GetBetaTestDiamond_R : public LG_ResponseEvent
{
public:
	virtual bool _DoEvent() override;

	void OnGetDiamondFinish(DBOperate *pOp, bool bSu);
};

class GL_UseCDKey_R : public LG_ResponseEvent
{
public:
	virtual bool _DoEvent() override;

	void OnUseCDKeyFinish(DBOperate *pOp, bool bSu);
};

class GL_CreateCDKey_R : public LG_ResponseEvent
{
public:
	virtual bool _DoEvent() override;

	void OnCreateCDKeyFinish(DBOperate *pOp, bool bSu);
};

//-------------------------------------------------------------------------*/
class GL_UploadLoginInfoTable_R : public LG_NetEvent
{
public:
	virtual bool _DoEvent() override
	{
		AutoData infoData = get("INFO_DATA");
		if (infoData)
		{
			// 通知AG, 再广播到所有的LG
			AutoNice p = MEM_NEW NiceData();
			p["ANNO_DATA"] = infoData.getPtr();
			GetGSNet()->mThread->GetDBWork()->ExeSqlFunction(DBCallBack(), GLOBAL_AUCCONT_TABLE, NULL, "f_update_anno_info", p);
			//GetGSNet()->mThread->UpdateAnnouncementTable(infoData);
		}
		//else
		//	GetResponseEvent()["ERROR"] = "获取公告数据失败 [INFO_DATA]";

		Finish();
		return true;
	}
};
//-------------------------------------------------------------------------*/
class GL_RequestLGOnlineInfo_R : public LG_ResponseEvent
{
public:
	virtual bool _DoEvent() override;

	void  OnDBRequestFinish(DBOperate *pDB, bool bSu);

	void OnDBGetResForeKey(DBOperate *pDB, bool bSu);

	void OnDBRequestAllLoginGSIP(DBOperate *pDB, bool bSu);

	virtual bool _OnTimeOver() override
	{
		GetResponseEvent()["RESULT"] = "请求AG超时";
		Finish();
		return true;
	}
};
//-------------------------------------------------------------------------*/
class GL_RequestLGUpdateVersion_R : public LG_ResponseEvent
{
public:
	virtual bool _DoEvent() override;	
};

class GL_RequestLGUpdateServerState_R : public LG_ResponseEvent
{
public:
	virtual bool _DoEvent() override;

	void OnRequestFinish(DBOperate *p, bool bSu);

	virtual bool _OnTimeOver() override
	{
		GetResponseEvent()["INFO"] = "更新服务区显示状态失败";
		Finish();
		return true;
	}
};

//-------------------------------------------------------------------------*/
// 更新资源信息同步
class RG_RequestAllUpdateResources_R : public LG_NetEvent
{
	virtual bool _DoEvent() override;
};
//-------------------------------------------------------------------------*/
// RS 请求关闭服务器
class RS_ReqeustCloseServer_LR : public  LG_NetEvent
{
	virtual bool _DoEvent() override
	{
		NOTE_LOG("WARN: RS 请求关闭服务器, LG开始关闭...");
		GetGSNet()->mThread->mQuestClose = true;
		return true;
	}
};


#endif //_INCLUDE_LG_NODENETEVENT_H_
