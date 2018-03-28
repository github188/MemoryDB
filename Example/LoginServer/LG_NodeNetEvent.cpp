#include "LG_NodeNetEvent.h"
#include "MeshedNetNodeData.h"

void GL_RequestLGOnlineInfo_R::OnDBRequestFinish(DBOperate *pDB, bool bSu)
{
	if (bSu && pDB->mResultNiceData)
	{
		GetResponseEvent()["RESULT"] = pDB->mResultNiceData["RESULT"].string();
	}
	else
		GetResponseEvent()["RESULT"] = "请求AG执行失败 >f_request_total_pay_amount";
	Finish();
}

void GL_RequestLGOnlineInfo_R::OnDBGetResForeKey(DBOperate *pDB, bool bSu)
{
	if (bSu && pDB->mResultNiceData)
	{
		//NOTE_LOG(AString::getANIS(pDB->mResultNiceData->dump().c_str()).c_str());
		GetResponseEvent()["RESULT"] = pDB->mResultNiceData["FORE_NET_KEY"].string();
	}
	else
		GetResponseEvent()["RESULT"] = "请求AG 失败>f_request_res_fore_net_key";
	Finish();
}

void GL_RequestLGOnlineInfo_R::OnDBRequestAllLoginGSIP( DBOperate *pDB, bool bSu )
{
	if (bSu && pDB->mResultNiceData)
	{
		//NOTE_LOG(AString::getANIS(pDB->mResultNiceData->dump().c_str()).c_str());
		GetResponseEvent()["INFO"] = (tBaseTable*)pDB->mResultNiceData["INFO"];
		GetResponseEvent()["RESULT"] = pDB->mResultNiceData["ERROR"].string();
	}
	else
		GetResponseEvent()["RESULT"] = "请求AG 失败>f_request_all_login_gs_ip";
	Finish();
}

bool GL_RequestLGOnlineInfo_R::_DoEvent()
{
	int requestType = get("REQUEST_TYPE");
	if (requestType==TOTAL_PAY_AMOUNT)
	{
		// 请求统计支付总金额
		AutoNice p = MEM_NEW NiceData();
		GetGSNet()->mThread->GetDBWork()->ExeSqlFunction(DBCallBack(&GL_RequestLGOnlineInfo_R::OnDBRequestFinish, this), GLOBAL_AUCCONT_TABLE, NULL, "f_request_total_pay_amount", p);
		WaitTime(10);
		return true; 
	}
	else if (requestType==RES_LOGIN_KEY)
	{
		// 请求统计支付总金额
		AutoNice p = MEM_NEW NiceData();
		p["SERVER_ID"] = (int)get("PARAM");
		GetGSNet()->mThread->GetDBWork()->ExeSqlFunction(DBCallBack(&GL_RequestLGOnlineInfo_R::OnDBGetResForeKey, this), GLOBAL_AUCCONT_TABLE, NULL, "f_request_res_fore_net_key", p);
		WaitTime(10);
		return true; 
	}
	else if (requestType==REQUEST_LOGIN_AND_GS_IP)
	{
		// 请求所有登陆和GS开放地址
		AutoNice p = MEM_NEW NiceData();
		GetGSNet()->mThread->GetDBWork()->ExeSqlFunction(DBCallBack(&GL_RequestLGOnlineInfo_R::OnDBRequestAllLoginGSIP, this), GLOBAL_AUCCONT_TABLE, NULL, "f_request_all_login_gs_ip", p);
		WaitTime(10);
		return true; 
	}

	Hand<MeshedNodeNet> net = GetGSNet()->mThread->mLGNodeNet;
	if (net)
	{
		static AutoTable sInfoTable;

		if (!sInfoTable)
		{
			sInfoTable = tBaseTable::NewBaseTable(true);
			sInfoTable->AppendField("GSKEY", FIELD_UINT64);
			sInfoTable->AppendField("ONLINE",  FIELD_INT);			
		}
		else
			sInfoTable->ClearAll();

		NetNodeList &netNodeList = net->GetNetNodeList();

		for (int i=0; i<netNodeList.size(); ++i)
		{
			Hand<NetNodeConnectData> node = netNodeList.get(i);

			if (!node || (bool)node->get("NOW_CLOSE") || !node->IsConnected() || !IS_GS  )
				continue;

			ARecord re = sInfoTable->CreateRecord((Int64)node["LOGIN_GSKEY"], true);
			re["ONLINE"] = (int)node["ONLINE_COUNT"];
		}
		auto &reList = GetGSNet()->mThread->mResourcesServerList;
		for (int i=0; i<reList.size(); ++i)
		{
			ARecord re = sInfoTable->CreateRecord((Int64)reList.getKey(i), true);
			re["ONLINE"] = -1;			
		}

		GetResponseEvent()["INFO"] = sInfoTable.getPtr();
		AString info;
		info.Format("[%s] <%s> <%d>[%s] [%s]\r\nLogin [%s:%d], Connect [%d]"
			, GetGSNet()->mThread->mServerVersion.c_str()
			, SERVER_VERSION_FLAG
			, GetGSNet()->mThread->mServerID
			, GetGSNet()->mThread->mGameServerName.c_str()
			, AString::getUTF8(SERVER_STATE_TOOL::ToStringServerState(GetGSNet()->mThread->mServerShowState)).c_str()
			, GetGSNet()->mThread->mPlayerWebNet->GetIp()
			, GetGSNet()->mThread->mPlayerWebNet->GetPort()
			, GetGSNet()->mThread->mPlayerWebNet->GetConnectCount()
			);
		GetResponseEvent()["VER"] = info;
	}
	Finish();
	return true;
}



GSKEY LG_RequestEvent::GetTargetGSKey()
{
	NodeRequestConnect *pRequestConnect = dynamic_cast<NodeRequestConnect*>(mNetConnect.getPtr());
	if (pRequestConnect!=NULL)
		return pRequestConnect->mNetNodeConnectData->mServerNetKey;

	NodeServerConnect *pNodeConnect = dynamic_cast<NodeServerConnect*>(mNetConnect.getPtr());
	if (pNodeConnect!=NULL)
		return pNodeConnect->mServerNetKey;

	AssertEx(0, "逻辑上必须是以上两种连接");
	return 0;
}

GSKEY LG_ResponseEvent::GetTargetGSKey()
{
	NodeRequestConnect *pRequestConnect = dynamic_cast<NodeRequestConnect*>(mNetConnect.getPtr());
	if (pRequestConnect!=NULL)
		return pRequestConnect->mNetNodeConnectData->mServerNetKey;

	NodeServerConnect *pNodeConnect = dynamic_cast<NodeServerConnect*>(mNetConnect.getPtr());
	if (pNodeConnect!=NULL)
		return pNodeConnect->mServerNetKey;

	AssertEx(0, "逻辑上必须是以上两种连接");
	return 0;
}

bool GL_NotifyOnlineInfo_R::_DoEvent()
{
	//NOTE_LOG("***************");
	//NOTE_LOG(GetData().dump().c_str());
	//NOTE_LOG("***************");

	Hand<NodeServerConnect> serverConnect = mNetConnect;
	AssertEx(serverConnect, "必定为 NodeServerConnect, 此消息由请求连接发送");
	NetNodeConnectData *pNode = serverConnect->mNetNodeConnectData;
	AssertEx(pNode!=NULL, "在连接里应该设置了 NetNodeConnectData ");

	if (get("ONLINE_COUNT").empty())
	{
		NOTE_LOG("$$$$$$ ERROR: XXX >>> NotifyOnline exist ONLINE_COUNT");
		return true;
	}

	int x = get("ONLINE_COUNT");
	//??? !!! 出现很多为0的情况, 待处理
	if (x<=0)
	{
		NOTE_LOG("@@@ ERROR: NotifyOnline exist ONLINE_COUNT = 0");
		return true;
	}

	pNode->set("ONLINE_COUNT", x);
	pNode->set("MEM_NOT_ENOUGH", (bool)get("MEM_NOT_ENOUGH"));
	pNode->set("LAST_TIME", (UInt64)TimeManager::Now());

	Hand<LGNodeNet> gsNodeNet = pNode->mMeshedNet;
	AssertEx(gsNodeNet, "必须能得到 GSMeshedNodeNet");

	gsNodeNet->mThread->UpdateLeastOnlineGS();

	Finish();

	return true;
}

bool GL_GetBetaTestDiamond_R::_DoEvent()
{
	AutoNice d = MEM_NEW NiceData();
	d["ACCOUNT"] = get("ACCOUNT").string();
	GetGSNet()->mThread->GetDBWork()->ExeSqlFunction(DBCallBack(&GL_GetBetaTestDiamond_R::OnGetDiamondFinish, this), GLOBAL_AUCCONT_TABLE, "", "f_get_beta_test_diamond", d);
	return true;
}

void GL_GetBetaTestDiamond_R::OnGetDiamondFinish(DBOperate *pOp, bool bSu)
{
	if (bSu)
	{
		int diamond = pOp->mResultNiceData["DIAMOND"];
		GetResponseEvent()["DIAMOND"] = diamond;
	}
	Finish();
}

bool GL_UseCDKey_R::_DoEvent()
{
	AutoNice d = MEM_NEW NiceData();
	d->set("CDKEY", get("CDKEY").string());
	d->set("PLAYER_ID", (UInt64)get("PLAYER_ID"));
	GetGSNet()->mThread->GetDBWork()->ExeSqlFunction(DBCallBack(&GL_UseCDKey_R::OnUseCDKeyFinish, this), CD_KEY_TABLE, "", "f_use_cdkey", d);
	return true;
}

void GL_UseCDKey_R::OnUseCDKeyFinish(DBOperate *pOp, bool bSu)
{
	if (bSu && pOp->mResultNiceData)
	{
		int result = pOp->mResultNiceData["RESULT"];
		GetResponseEvent()["RESULT"] = result;
	}
	Finish();
}

bool GL_CreateCDKey_R::_DoEvent()
{
	AutoNice d = MEM_NEW NiceData();
	d->set("TIME", (int)get("TIME"));
	d->set("START_NUM", (int)get("START_NUM"));
	d->set("NUM", (int)get("NUM"));
	d->set("MAX_PLAYER", (int)get("MAX_PLAYER"));
	GetGSNet()->mThread->GetDBWork()->ExeSqlFunction(DBCallBack(&GL_CreateCDKey_R::OnCreateCDKeyFinish, this), CD_KEY_TABLE, "", "f_create_cdkey", d);
	return true;
}

void GL_CreateCDKey_R::OnCreateCDKeyFinish(DBOperate *pOp, bool bSu)
{
	if (bSu && pOp->mResultNiceData)
	{
		int result = pOp->mResultNiceData["RESULT"];
		GetResponseEvent()["RESULT"] = result;
	}
	Finish();
}

bool RG_RequestAllUpdateResources_R::_DoEvent()
{
	NodeServerConnect *pConnect = dynamic_cast<NodeServerConnect*>(mNetConnect.getPtr());
	if (pConnect==NULL)
	{
		ERROR_LOG("RG_RequestAllUpdateResources_R 应该由 game server NodeServerConnect 接收");
		return false;
	}

	GSKEY resNetKey = pConnect->mNetNodeConnectData->get("RESOURCES_NET_KEY");
	GetGSNet()->mThread->AppendResourcesServer(resNetKey, pConnect->mNetNodeConnectData);
	GetGSNet()->mThread->UpdateResourceMD5(get("RES_MD5"), get("RES_MD5_IOS"));	

	return true;
}

bool GL_RequestLGUpdateVersion_R::_DoEvent()
{
	bool b = GetGSNet()->mThread->UpdateServerVersion(get("VER"));
	GetResponseEvent()["RESULT"] = b;
	Finish();
	return true;
}

bool GL_RequestLGUpdateServerState_R::_DoEvent()
{
	if (get("GET_STATE"))
	{
		GetResponseEvent()["STATE_DATA"] = GetGSNet()->mThread->mServerInfoData.getPtr();
		GetResponseEvent()["SERVER_ID"] = GetGSNet()->mThread->mServerID;
		Finish();
		return true;
	}
	int state = get("STATE");

	bool b = false;
	
	int serverID = get("SERVER_ID");

	if (serverID==GetGSNet()->mThread->mServerID)
	{
		b = GetGSNet()->mThread->UpdateServerStateData(state);
	}
	else if (serverID>=0)
	{
		// 通知到AG, 让AG通知指定的LG修改
		AutoNice d = MEM_NEW NiceData();
		d["SERVER_ID"] = serverID;
		d["STATE"] = state;

		 b =  GetGSNet()->mThread->GetDBWork()->ExeSqlFunction(DBCallBack(&GL_RequestLGUpdateServerState_R::OnRequestFinish, this), GLOBAL_AUCCONT_TABLE, NULL, "e_request_modify_server_state", d)!=NULL;
		 if (b)
		 {
			 WaitTime(6);
			 return true;
		 }
	}

	if (b)
		GetResponseEvent()["INFO"] = "更新服务区显示状态成功";
	else
		GetResponseEvent()["INFO"] = "更新服务区显示状态失败";
	Finish();
	return true;
}

void GL_RequestLGUpdateServerState_R::OnRequestFinish(DBOperate *p, bool bSu)
{
	if (bSu)
	{
		if (p->mResultNiceData["RESULT"])
			GetResponseEvent()["INFO"] = "更新服务区显示状态成功";
		else
		{
			AString info = "修改游戏信息请求错误结果>";
			info += p->mResultNiceData["ERROR"].string();
			GetResponseEvent()["INFO"] = info;
		}
	}
	else
		GetResponseEvent()["INFO"] = "请求AG修改游戏信息失败";
	Finish();	
}
