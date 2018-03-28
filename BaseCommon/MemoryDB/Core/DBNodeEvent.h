
#ifndef _INCLUDE_DBNODEEVENT_H_
#define _INCLUDE_DBNODEEVENT_H_

#include "MemoryDBNode.h"
#include "ServerEvent.h"
#include "ClientEvent.h"
#include "TableUpdateLoader.h"
#include "ServerIPInfo.h"
#include "DBResultCallBack.h"

//-------------------------------------------------------------------------
class MemoryDB_Export tDBReqestEvent : public tServerEvent
{
public:
	virtual bool DoEvent( bool bImmediately = true )
	{
		if (bImmediately)
			WaitTime(10);

		return tServerEvent::DoEvent(bImmediately);
	}

	bool _OnEvent( AutoEvent &hEvent )
	{
		if (!mResponseCallBack.empty())
		{
			AutoNice resultData = hEvent->GetData().getNice("RESULT_DATA");
			if (resultData)
				mResponseCallBack(resultData.getPtr(), (int)hEvent["RESULT"]==eNoneError);
			else
				mResponseCallBack(&(hEvent->GetData()), (int)hEvent["RESULT"]==eNoneError);
		}
		return tServerEvent::_OnEvent(hEvent);
	}

public:
	Hand<MemoryDBNode> GetDBNode()
	{	
		Hand<DBMeshedNodeNet> dbNet = GetNet();
		AssertEx(dbNet, "Must is exist DBMeshedNodeNet");

		return dbNet->mpDBNode->GetSelf();
	}

	Hand<MeshedNodeNet> GetNet()
	{
		Hand<MeshedNodeNet> net = mNetConnect->GetNetHandle()->GetSelf();
		if (!net)
		{
			Hand<NodeServerNet> nodeNet = mNetConnect->GetNetHandle()->GetSelf();
			AssertEx(nodeNet, "逻辑上 当前应该是 NodeServerNet");
			net = nodeNet->mOwnerMeshNet->GetSelf();
		}
		AssertEx(net, "Must exist MeshedNodeNet");
		return net;
	}

	NetNodeConnectData* GetNodeConnectData() { return GetNet()->GetNodeConnectData(mNetConnect); }

public:
	DBResultCallBack	mResponseCallBack;
};
//-------------------------------------------------------------------------
class MemoryDB_Export tDBResponseEvent : public tClientEvent
{
public:
	Hand<MemoryDBNode> GetDBNode()
	{	
		Hand<DBMeshedNodeNet> dbNet = GetNet();
		AssertEx(dbNet, "Must is exist DBMeshedNodeNet");

		return dbNet->mpDBNode->GetSelf();
	}

	Hand<MeshedNodeNet> GetNet()
	{
		Hand<MeshedNodeNet> net = mNetConnect->GetNetHandle()->GetSelf();
		if (!net)
		{
			Hand<NodeServerNet> nodeNet = mNetConnect->GetNetHandle()->GetSelf();
			AssertEx(nodeNet, "逻辑上 当前应该是 NodeServerNet");
			net = nodeNet->mOwnerMeshNet->GetSelf();
		}
		AssertEx(net, "Must exist MeshedNodeNet");
		return net;
	}

	NetNodeConnectData* GetNodeConnectData() { return GetNet()->GetNodeConnectData(mNetConnect); }
};
//-------------------------------------------------------------------------
class DD_NodifyNodeList : public tDBReqestEvent
{
public:
	virtual bool _DoEvent();

	virtual void _OnResp(AutoEvent &respEvent)
	{

	}
};

class DD_NodifyNodeList_R : public tDBResponseEvent
{
public:
	virtual bool _DoEvent();
};
//-------------------------------------------------------------------------
// 同步数据表格分布信息
//class DD_RequestDataDistributionInfo : public tDBReqestEvent
//{
//public:
//	virtual bool _DoEvent()
//	{
//		WaitTime(120);
//		return true;
//	}
//
//	virtual void _OnResp(AutoEvent &respEvent) 
//	{
//		Log(respEvent->GetData().dump().c_str());
//		ABaseTable keyInfoTable; 
//		respEvent->get("KEY_INFO", &keyInfoTable, typeid(keyInfoTable));
//		if (keyInfoTable)
//		{
//			UInt64 dbServerIPKey = respEvent->get("DBSERVER_IPPORT");
//			// 在这里判断是否已经存在终端连接，不存在，则进行开始连接
//			GetNodeConnectData()->set("DBSERVER_IPPORT", dbServerIPKey);
//			GetDBNode()->AppendNodeDistributionData(GetNodeConnectData()->GetSelf(), keyInfoTable);
//		}
//		else
//		{
//			ERROR_LOG("未回复KEY分布信息");
//		}
//	}
//};
//
//class DD_RequestDataDistributionInfo_R : public tDBResponseEvent
//{
//public:
//	virtual bool _DoEvent()
//	{
//		Hand<MemoryDBNode> node = GetDBNode();
//		ABaseTable keyInfoTable = node->GetNowDataTableKeyInfo();
//		GetResponseEvent()->set("KEY_INFO", &keyInfoTable, typeid(keyInfoTable));
//
//		UInt64 serverIPKey = ServerIPInfo::IP2Num(node->GetDBServerNet()->GetIp(), node->GetDBServerNet()->GetPort(), 0); 
//		GetResponseEvent()->set("DBSERVER_IPPORT", serverIPKey);
//		Finish();		
//
//		return true;
//	}
//};
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// 请求所属的节点, 进行创建记录
class DD_RequestSaveRecord : public tDBReqestEvent
{
public:
	virtual bool _DoEvent()
	{
		return true;	
	}

	virtual void _OnResp(AutoEvent &respEvent)
	{
		eDBResultType resultType = (eDBResultType)(int)respEvent->get("RESULT");
		if (resultType==eNoneError)
			Log("远程创建记录成功 [%s] in table [%s]", get("RECORD_KEY").string(), get("TABLE_INDEX").string());
		else
			Log("失败[%d]:远程创建记录 [%s] in table [%s]", (int)get("RESULT"), get("RECORD_KEY").string(), get("TABLE_INDEX").string());
		if (mWaitEvent)
			mWaitEvent->OnEvent(respEvent); 
	}

public:
	AutoEvent mWaitEvent;
};

class DD_RequestSaveRecord_R : public tDBResponseEvent
{
public:
	virtual bool _DoEvent();

	virtual bool _OnEvent(AutoEvent &hEvent)
	{
		GetResponseEvent()->set("RESULT", (bool)hEvent->get("RESULT"));
		GetResponseEvent()->set("ERROR", hEvent->get("ERROR").string());
		Finish();
		return true;
	}

public:
	AutoEvent mWaitEvent;
};
//-------------------------------------------------------------------------
// 获取记录
class DD_RequestFindRecord : public tDBReqestEvent
{
public:
	virtual void _OnResp(AutoEvent &respEvent)
	{
		if (mWaitEvent)
		{
			mWaitEvent->OnEvent(respEvent);
		}
		else
		{
			Log("WARN: 未设置等待处理事件");
		}
	}

public:
	AutoEvent mWaitEvent;
};
//-------------------------------------------------------------------------
class DD_RequestFindRecord_R : public tDBResponseEvent
{
public:
	virtual bool _DoEvent();
};
//-------------------------------------------------------------------------
// 删除记录
class DD_RequestDeleteRecord : public tDBReqestEvent
{
public:
	virtual void _OnResp(AutoEvent &respEvent)
	{
		if (mWaitEvent)
		{
			mWaitEvent->OnEvent(respEvent);
		}
		else
		{
			Log("WARN: 未设置等待处理事件");
		}
	}

public:
	AutoEvent	mWaitEvent;
};

class DD_RequestDeleteRecord_R : public tDBResponseEvent
{
public:
	virtual bool _DoEvent();
};
//-------------------------------------------------------------------------
// 请求自增长创建记录
class DD_RequestCreateGrowthRecord : public tDBReqestEvent
{
public:
	virtual void _OnResp(AutoEvent &respEvent)
	{
		if (mWaitEvent)
		{
			mWaitEvent->OnEvent(respEvent);
		}
		else
		{
			Log("WARN: 未设置等待处理事件");
		}
	}

public:
	AutoEvent	mWaitEvent;
};


class DD_RequestCreateGrowthRecord_R : public tDBResponseEvent
{
public:
	virtual bool _DoEvent();
	
};

//-------------------------------------------------------------------------
// 执行指定操作
class DD_RunDBOpereate : public tDBReqestEvent
{
public:
	virtual void _OnResp(AutoEvent &respEvent)
	{
		if (mWaitEvent)
		{
			mWaitEvent->OnEvent(respEvent);
		}
		else
		{
			Log("WARN: 未设置等待处理事件");
		}
	}

public:
	AutoEvent	mWaitEvent;
};

class DD_RunDBOpereate_R : public tDBResponseEvent
{
public:
	virtual bool _DoEvent();

	void OnCallFinish(AutoNice result, bool bSu)
	{
		GetResponseEvent()["RESULT"] = bSu ? eNoneError : eDBOperateResultFail;
		GetResponseEvent()["RESULT_DATA"] = result;
		Finish();
	}
};

//-------------------------------------------------------------------------
// 获取记录的指定字段的数据, 请求的字段名列表 在 [FIELD_LIST] AutoNice 的KEY
// 回复保存在 字段列表对应的, 值内


class DD_RequestGetRecordData : public tDBReqestEvent
{
public:
	virtual bool _DoEvent()
	{
		AssertEx(mFieldList, "Must set load record field list");
		return true;
	}

	virtual void _OnResp(AutoEvent &respEvent)
	{
		if (mWaitEvent)
			mWaitEvent->OnEvent(respEvent);
		else
		{
			Log("WARN: 未设置等待处理事件");
		}
	}

public:
	virtual bool _Serialize(DataStream *destData)
	{
		if (tDBReqestEvent::_Serialize(destData))
		{
			destData->write((DSIZE)mFieldList->size());
			for (size_t i=0; i<mFieldList->size(); ++i)
			{
				destData->writeString((*mFieldList)[i].c_str());
			}
			return true;
		}
		return false;
	}

public:
	AutoPtr<StringArray> mFieldList;

	AutoEvent mWaitEvent;
};

class DD_RequestGetRecordData_R : public tDBResponseEvent
{
public:
	virtual bool _DoEvent();

	virtual bool _Restore(DataStream *scrData)
	{
		if (tDBResponseEvent::_Restore(scrData))
		{
			DSIZE count = 0;
			if (!scrData->read(count))
				return false;

			mFieldList = MEM_NEW StringArray();
			mFieldList->resize(count);

			for (DSIZE i=0; i<count; ++i)
			{
				if (!scrData->readString((*mFieldList)[i]))
					return false;
			}
			return true;
		}
		return false;
	}

public:
	AutoPtr<StringArray> mFieldList;
};
//-------------------------------------------------------------------------
// 修改记录中的某个数据, 包括记录中的子表格中的值, 且支持, 相对修改加减, 如果提供参考值, 则进行比较 + 时不大于, -时不小于
class DD_RequestModifyData : public tDBReqestEvent
{
public:
	virtual void _OnResp(AutoEvent &respEvent)
	{
		if (mWaitEvent)
			mWaitEvent->OnEvent(respEvent);
		else
		{
			Log("WARN: 未设置等待处理事件");
		}
	}

public:
	AutoEvent mWaitEvent;
};

class DD_RequestModifyData_R : public tDBResponseEvent
{
public:
	virtual bool _DoEvent()
	{
		HandDBNode dbNode = GetDBNode();
		
		AString tableIndex = get("TABLE_INDEX", "Must set TABLE_INDEX"); 
		AString recordKey = get("RECORD_KEY", "Must set TABLE_INDEX");

		AString fieldInfo = get("FIELD_KEY");

		AString destField = get("DEST_FIELD", "Must set target field name");
		AString destValue = get("VALUE", "Must set dest value");

		Data limit = get("LIMIT");
		eModifyDataMode eMode = (eModifyDataMode)(int)get("MODE");

		AString resultValue, errorInfo, originalValue;
		eDBResultType result = GetDBNode()->ModifyData(tableIndex.c_str(), recordKey.c_str(), fieldInfo.c_str(), destField.c_str(), limit, eMode, destValue.c_str(), resultValue, originalValue, errorInfo);

		GetResponseEvent()->set("RESULT", result);
		GetResponseEvent()->set("ERROR", errorInfo.c_str());
		GetResponseEvent()->set("RESULT_VALUE", resultValue.c_str());
		GetResponseEvent()["ORIGINAL_VALUE"] = originalValue.c_str();

		Finish();
		return true;
	}
};

//-------------------------------------------------------------------------
// 获取直接从MYSQL表格中读取的数据
class DD_LoadRecordData : public tDBReqestEvent
{
public:
	virtual void _OnResp(AutoEvent &respEvent)
	{
		AutoNice d;
		respEvent->get("RECORD_DATA", d);

		mWaitCallBack(d.getPtr(), (int)respEvent->get("RESULT")==eNoneError);
	}

public:
	DBResultCallBack	mWaitCallBack;
};

class DD_LoadRecordData_R : public tDBResponseEvent
{
public:
	virtual bool _DoEvent()
	{
		AutoTable t = GetDBNode()->GetTable(get("TABLE").string().c_str());
		if (!t)
		{
			GetResponseEvent()->set("RESULT", (int)eTableNoExist);
		}
		else
		{
			t->LoadRecordData(get("RECORD_KEY").string(), get("FIELD_LIST"), DBResultCallBack(&DD_LoadRecordData_R::OnLoadFinish, this));
			return true;
		}

		Finish();
		return true;
	}

	void OnLoadFinish(AutoNice resultData, bool bSu)
	{
		if (bSu)
		{
			AutoNice d(resultData);
			GetResponseEvent()->set("RECORD_DATA", d);
			GetResponseEvent()->set("RESULT", (int)eNoneError);
		}
		else
			GetResponseEvent()->set("RESULT", (int)eRecordNoExist);

		Finish();
	}
};
//-------------------------------------------------------------------------
// 直接MYSQL表格中保存数据
class DD_SaveRecordData : public tDBReqestEvent
{
public:
	virtual void _OnResp(AutoEvent &respEvent)
	{
		AutoNice d;
		respEvent->get("RECORD_DATA", d);

		mWaitCallBack(d, (int)respEvent->get("RESULT")==eNoneError);
	}

public:
	DBResultCallBack	mWaitCallBack;
};

class DD_SaveRecordData_R : public tDBResponseEvent
{
public:
	virtual bool _DoEvent()
	{
		AutoTable t = GetDBNode()->GetTable(get("TABLE").string().c_str());
		if (!t)
		{
			GetResponseEvent()->set("RESULT", (int)eTableNoExist);
		}
		else
		{
			AutoNice	recordData; 
			get("RECORD_DATA", recordData);
			if (recordData)
			{
				t->SaveRecordData(get("RECORD_KEY").string(), recordData, false, DBResultCallBack(&DD_SaveRecordData_R::OnLoadFinish, this));
				return true;
			}
			else
				GetResponseEvent()->set("RESULT", (int)eRecordRestoreDataFail);
		}

		Finish();
		return true;
	}

	void OnLoadFinish(AutoNice resultData, bool bSu)
	{
		if (bSu)
		{
			if (resultData)
				GetResponseEvent()->set("RECORD_DATA", resultData);
			GetResponseEvent()->set("RESULT", (int)eNoneError);
		}
		else
			GetResponseEvent()->set("RESULT", (int)eRecordNoExist);

		Finish();
	}
};
//-------------------------------------------------------------------------
// 创建节点分表
class DD_RequestCreateNodeTable : public tDBReqestEvent
{
public:
	virtual void _OnResp(AutoEvent &respEvent)
	{
		
	}
};
class DD_RequestCreateNodeTable_R : public tDBResponseEvent
{
public:
	virtual bool _DoEvent();
};
//-------------------------------------------------------------------------*/
#endif //_INCLUDE_DBNODEEVENT_H_