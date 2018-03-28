#ifndef _INCLUDE_ACCOUNTNETEVENT_H_
#define _INCLUDE_ACCOUNTNETEVENT_H_

#include "CEvent.h"
#include "LoginThread.h"
#include "MeshedNetNodeData.h"

using namespace Logic;

//-------------------------------------------------------------------------*/
//class tAccountDBRequest : public tServerEvent
//{
//public:
//	LoginThread* GetThread()
//	{
//		Hand<LGAccountConnectNet> net = GetEventCenter()->_GetNetTool(0);
//		if (net)
//			return net->mpLoginThread;
//
//		return NULL;
//	}
//};

//-------------------------------------------------------------------------*/
class tAccountDBEvent : public tBaseNetEvent
{
public:
	LoginThread* GetThread()
	{
		if (mNetConnect)
		{		
			Hand<DBClient::DBNet> net = mNetConnect->GetNetHandle();
			if (net)
			{
				MainNodeClient *pClient = dynamic_cast<MainNodeClient*>(net->GetDBClient());
				if (pClient!=NULL)
				return	dynamic_cast<AccountDBClient*>(pClient->mpDBManager)->mpLoginThread;
			}
		}

		return NULL;
	}
};

//-------------------------------------------------------------------------*/
class AL_SyncServerInfo : public tAccountDBEvent
{
public:
	virtual bool _DoEvent() override
	{
		AutoData serverInfo = (DataStream*)get("INFO");
		if (serverInfo)
		{
			GetThread()->UpdateServerInfo(serverInfo);
		}
		else
			ERROR_LOG("未回复服务器分区信息数据");
		return true;
	}
};
//-------------------------------------------------------------------------*/
// AG 请求更新公告
class AL_RequestUpdateAnno : public tAccountDBEvent
{
public:
	virtual bool _DoEvent() override
	{
		AutoData annoInfo = (DataStream*)get("ANNO_DATA");
		if (annoInfo)
		{
			GetThread()->UpdateAnnouncementTable(annoInfo);
		}
		else
			ERROR_LOG("未回复服务器分区信息数据");
		return true;
	}
};
//-------------------------------------------------------------------------*/
// 中转充值回调
class LG_RequestPayEvent : public Logic::tServerEvent
{
public:
	virtual void _OnResp(AutoEvent &respEvent) override
	{
		if (mWaitEvent)
			mWaitEvent->OnEvent(respEvent);
	}

public:
	AutoEvent mWaitEvent;
};

class AL_RequestPayEvent_L : public tClientEvent
{
public:
	virtual bool _DoEvent() override
	{
		//NOTE_LOG("AL_RequestPayEvent_L  >开始中转到GS > LG_RequestPayEvent");
		LoginThread *pThread = GetThread();
		if (pThread!=NULL)
		{		
			HandConnect conn = pThread->GetAnyGSConnect();
			if (conn)
			{
				Hand<LG_RequestPayEvent> evt = conn->StartEvent("LG_RequestPayEvent");
				AutoData d = (DataStream*)get("DATA");
				evt["DATA"] = d.getPtr();
				evt->mWaitEvent = GetSelf();
				evt->Start();
				evt->WaitTime(20);
				return true;
			}
		}
		Finish();
		return true;
	}

	virtual bool _OnEvent(AutoEvent &hEvent) override
	{
		AutoNice res = (tNiceData*)hEvent["RESP"];
		GetResponseEvent()["RESP"] = res.getPtr();
		Finish();
		return true;
	}

	LoginThread* GetThread()
	{
		if (mNetConnect)
		{		
			Hand<DBClient::DBNet> net = mNetConnect->GetNetHandle();
			if (net)
			{
				MainNodeClient *pClient = dynamic_cast<MainNodeClient*>(net->GetDBClient());
				if (pClient!=NULL)
					return	dynamic_cast<AccountDBClient*>(pClient->mpDBManager)->mpLoginThread;
			}
		}

		return NULL;
	}
};
//-------------------------------------------------------------------------*/
// 请求前端登陆地址
class AL_RequestResForeNetKey_L : public tClientEvent
{
public:
	virtual bool _DoEvent() override
	{
		//NOTE_LOG("AL_RequestPayEvent_L  >开始中转到GS > LG_RequestPayEvent");
		LoginThread *pThread = GetThread();
		if (pThread!=NULL)
		{		
			GetResponseEvent()["FORE_NET_KEY"] = pThread->mResForeNetKey;
		}
		Finish();
		return true;
	}
	LoginThread* GetThread()
	{
		if (mNetConnect)
		{		
			Hand<DBClient::DBNet> net = mNetConnect->GetNetHandle();
			if (net)
			{
				MainNodeClient *pClient = dynamic_cast<MainNodeClient*>(net->GetDBClient());
				if (pClient!=NULL)
					return	dynamic_cast<AccountDBClient*>(pClient->mpDBManager)->mpLoginThread;
			}
		}

		return NULL;
	}
};
//-------------------------------------------------------------------------
class AL_RequestAllLoginGSIP_L : public tClientEvent
{
public:
	virtual bool _DoEvent() override
	{
		LoginThread *pThread = GetThread();
		if (pThread!=NULL)
		{		
			AutoTable t = tBaseTable::NewBaseTable();
			t->AppendField("ID", FIELD_INT);
			t->AppendField("SERVER_ID", FIELD_INT);
			t->AppendField("IP", FIELD_UINT64);
			t->AppendField("TYPE", FIELD_BYTE);
			t->AppendField("VER", FIELD_STRING);
			Hand<MeshedNodeNet> net = pThread->mLGNodeNet;
			NetNodeList &netNodeList = net->GetNetNodeList();
			
			for (int i=0; i<netNodeList.size(); ++i)
			{
				Hand<NetNodeConnectData> node = netNodeList.get(i);

				if (!node || (bool)node->get("NOW_CLOSE") || !node->IsConnected() || !IS_GS  )
					continue;				

				GSKEY ipKey = node["LOGIN_GSKEY"];
				ARecord r = t->GrowthNewRecord(NULL);
				r["SERVER_ID"] = pThread->mServerID;
				r["IP"] = ipKey;
				r["TYPE"] = LOGIN_IP_GS;
				r["VER"] = node["VER"].string();
			}
			for (int i=0; i<pThread->mResourcesServerList.size(); ++i)
			{
				GSKEY resNetKey = pThread->mResourcesServerList.getKey(i);
				AutoEvent nodeDataEvent = pThread->mResourcesServerList.get(i);
				ARecord r = t->GrowthNewRecord(NULL);
				r["SERVER_ID"] = pThread->mServerID;
				r["IP"] = resNetKey;
				r["TYPE"] = LOGIN_IP_RS;
				r["VER"] = nodeDataEvent["VER"].string();
			}
			
			GetResponseEvent()["INFO"] = t.getPtr();
		}
		Finish();
		return true;
	}
	LoginThread* GetThread()
	{
		if (mNetConnect)
		{		
			Hand<DBClient::DBNet> net = mNetConnect->GetNetHandle();
			if (net)
			{
				MainNodeClient *pClient = dynamic_cast<MainNodeClient*>(net->GetDBClient());
				if (pClient!=NULL)
					return	dynamic_cast<AccountDBClient*>(pClient->mpDBManager)->mpLoginThread;
			}
		}

		return NULL;
	}
};
//-------------------------------------------------------------------------*/
class AL_RequestModifyServerShowState_L : public tBaseNetEvent
{
public:
	virtual bool _DoEvent() override
	{
		if (mNetConnect)
		{		
			Hand<DBClient::DBNet> net = mNetConnect->GetNetHandle();
			if (net)
			{
				MainNodeClient *pClient = dynamic_cast<MainNodeClient*>(net->GetDBClient());
				if (pClient!=NULL)
					dynamic_cast<AccountDBClient*>(pClient->mpDBManager)->mpLoginThread->UpdateServerStateData(get("STATE"));
			}
		}
		return true;
	}
};
//-------------------------------------------------------------------------*/
#endif //_INCLUDE_ACCOUNTNETEVENT_H_